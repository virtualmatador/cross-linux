#include <fstream>
#include <sstream>

#include "../core/src/bridge.h"
#include "../core/src/interface.h"

#include "window.h"

Window* window_;

void web_view_script_message_received(WebKitUserContentManager* manager, WebKitJavascriptResult* js_result, gpointer user_data)
{
    auto value = webkit_javascript_result_get_js_value(js_result);
    std::istringstream is{ std::string(jsc_value_to_string(value)) };
    __int32_t sender;
    std::string id, command, info;
    is >> sender;
    is >> id;
    is >> command;
    std::getline(is, info);
    interface::HandleAsync(sender, id.c_str(), command.c_str(), info.c_str());
    webkit_javascript_result_unref(js_result);
}

void web_view_run_javascript_finished(GObject* sourceObject, GAsyncResult* res, gpointer userData)
{
}

void web_view_load_changed(WebKitWebView* web_view, WebKitLoadEvent load_event, gpointer user_data)
{
    if (load_event == WEBKIT_LOAD_FINISHED && user_data == reinterpret_cast<void*>(window_->sender_))
    {
        std::ostringstream os;
        os <<
            "var Handler = window.webkit.messageHandlers.Handler_;"
            "var Handler_Receiver = "
            << window_->sender_ << ";"
            "function CallHandler(id, command, info)"
            "{"
            "    Handler.postMessage(Handler_Receiver.toString() + \" \" + id + \" \" + command + \" \" + info);"
            "}";
        webkit_web_view_run_javascript(web_view, os.str().c_str(), nullptr, web_view_run_javascript_finished, nullptr);
    }
}

bool Window::handle_key(GdkEventKey *event)
{
    if (event->keyval == GDK_KEY_Escape)
    {
        interface::Escape();
        return false;
    }
    return true;
};

Window::Window(std::string path)
    : path_{ "file://" + std::filesystem::path(path).parent_path().string() + "/assets/" }
    , sender_{ 0 }
{
    window_ = this;
    add_events(Gdk::KEY_PRESS_MASK);
    signal_key_release_event().connect(sigc::mem_fun(*this, &Window::handle_key));
    need_restart_.connect(sigc::mem_fun(*this, &Window::on_need_restart));
    post_message_.connect(sigc::mem_fun(*this, &Window::on_post_message));
    load_web_view_.connect(sigc::mem_fun(*this, &Window::on_load_web_view));
    load_image_view_.connect(sigc::mem_fun(*this, &Window::on_load_image_view));
    web_view_ = (WebKitWebView*)webkit_web_view_new();
    WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(web_view_);
    g_signal_connect(manager, "script-message-received::Handler_", G_CALLBACK(web_view_script_message_received), nullptr);
    webkit_user_content_manager_register_script_message_handler (manager, "Handler_");
    web_view_widget_ = Glib::wrap((GtkWidget*)web_view_);
    add(*web_view_widget_);
    interface::Begin();
    interface::Create();
    interface::Start();
}

Window::~Window()
{
    interface::Stop();
    interface::Destroy();
    interface::End();
}

void Window::on_post_message()
{
    dispatch_lock_.lock();
    auto dispatch_info = post_message_queue_.front();
    post_message_queue_.pop();
    dispatch_lock_.unlock();
    interface::HandleAsync(dispatch_info.sender, dispatch_info.id, dispatch_info.command, dispatch_info.info);
}

void Window::on_load_web_view()
{
    auto w = web_view_widget_->get_width();
    auto h = web_view_widget_->get_height();
    dispatch_lock_.lock();
    auto dispatch_info = load_web_view_queue_.front();
    load_web_view_queue_.pop();
    dispatch_lock_.unlock();
    load_web_view(dispatch_info.sender, dispatch_info.view_info, dispatch_info.html, dispatch_info.waves);
}

void Window::on_load_image_view()
{
    dispatch_lock_.lock();
    auto dispatch_info = load_image_view_queue_.front();
    load_web_view_queue_.pop();
    dispatch_lock_.unlock();
    load_image_view(dispatch_info.sender, dispatch_info.view_info, dispatch_info.image_width, dispatch_info.waves);
}

void Window::on_need_restart()
{
    interface::Restart();
}

void Window::load_view(const __int32_t sender, const __int32_t view_info, const char* waves)
{
    sender_ = sender;
    // LoadAudio(waves, new Runnable()
    // {
    //     public void run()
    //     {
    //         HandleAsync(sender, "body", "ready", "");
    //     }
    // });
}

void Window::load_web_view(const __int32_t sender, const __int32_t view_info,
    const char* html, const char* waves)
{
    load_view(sender, view_info, waves);
    //pixels_ = null;
    //image_view_.setVisibility(View.GONE);
    //LoadView(web_view_, view_info, sender);
    web_view_widget_->show();

    // @Override public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request)
    // {
    //     Intent intent = new Intent(Intent.ACTION_VIEW, request.getUrl());
    //     view.getContext().startActivity(intent);
    //     return true;
    // }

    g_signal_connect(web_view_, "load-changed", GCallback(web_view_load_changed), reinterpret_cast<void*>(sender));
    auto path = path_ + "html/" + html + ".htm";
    webkit_web_view_load_uri(web_view_, path.c_str());
}

void Window::load_image_view(const __int32_t sender, const __int32_t view_info,
    const __int32_t image_width, const char* waves)
{
    load_view(sender, view_info, waves);
}

void bridge::NeedRestart()
{
    window_->need_restart_();
}

void bridge::LoadWebView(const __int32_t sender, const __int32_t view_info,
    const char* html, const char* waves)
{
    window_->dispatch_lock_.lock();
    window_->load_web_view_queue_.push({sender, view_info, html, waves});
    window_->dispatch_lock_.unlock();
    window_->load_web_view_();
}

void bridge::LoadImageView(const __int32_t sender, const __int32_t view_info,
    const __int32_t image_width, const char* waves)
{
    window_->dispatch_lock_.lock();
    window_->load_image_view_queue_.push({sender, view_info, image_width, waves});
    window_->dispatch_lock_.unlock();
    window_->load_image_view_();
}

__uint32_t* bridge::GetPixels()
{
    //return window_->pixels_;
    return nullptr;
}

void bridge::ReleasePixels(__uint32_t* const pixels)
{
    //window_->pixels_.clear();
}

void bridge::RefreshImageView()
{
    //window_->refresh_image_view_();
}

void bridge::CallFunction(const char* function)
{
    //window_->web_view_.evaluate(function);
}

std::string bridge::GetAsset(const char* key)
{
    std::ifstream asset(key);
    return {std::istreambuf_iterator<char>(asset), std::istreambuf_iterator<char>()};
}

std::string bridge::GetPreference(const char* key)
{
    // jstring jKey = env_->NewStringUTF(key);
    // jstring jValue = (jstring)env_->CallObjectMethod(me_, get_preference_, jKey);
    // const char* szValue = env_->GetStringUTFChars(jValue, nullptr);
    // std::string value = szValue;
    // env_->ReleaseStringUTFChars(jValue, szValue);
    // env_->DeleteLocalRef(jKey);
    // env_->DeleteLocalRef(jValue);
    // return value;
    return "";
}

void bridge::SetPreference(const char* key, const char* value)
{
    // jstring jKey = env_->NewStringUTF(key);
    // jstring jValue = env_->NewStringUTF(value);
    // env_->CallVoidMethod(me_, set_preference_, jKey, jValue);
    // env_->DeleteLocalRef(jKey);
    // env_->DeleteLocalRef(jValue);
}

void bridge::PostThreadMessage(__int32_t receiver, const char* id, const char* command, const char* info)
{
    window_->dispatch_lock_.lock();
    window_->post_message_queue_.push({receiver, id, command, info});
    window_->dispatch_lock_.unlock();
    window_->post_message_();
}

void bridge::AddParam(const char *key, const char *value)
{
    // jstring jKey = env_->NewStringUTF(key);
    // jstring jValue = env_->NewStringUTF(value);
    // env_->CallVoidMethod(me_, add_param_, jKey, jValue);
    // env_->DeleteLocalRef(jKey);
    // env_->DeleteLocalRef(jValue);
}

void bridge::PostHttp(const __int32_t sender, const char* id, const char* command, const char *url)
{
    // jstring jId = env_->NewStringUTF(id);
    // jstring jCommand = env_->NewStringUTF(command);
    // jstring jUrl = env_->NewStringUTF(url);
    // env_->CallVoidMethod(me_, post_http_, sender, jId, jCommand, jUrl);
    // env_->DeleteLocalRef(jId);
    // env_->DeleteLocalRef(jCommand);
    // env_->DeleteLocalRef(jUrl);
}

void bridge::PlayAudio(const __int32_t index)
{
    // env_->CallVoidMethod(me_, play_audio_, index);
}

void bridge::Exit()
{
    window_->close();
}
