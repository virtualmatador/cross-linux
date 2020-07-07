#include <fstream>

#include "../core/src/bridge.h"
#include "../core/src/interface.h"

#include "window.h"

Window* window_;

Window::Window(std::string path)
    : path_{ "file://" + std::filesystem::path(path).parent_path().string() + "/assets/" }
{
    window_ = this;
    need_restart_.connect(sigc::mem_fun(*this, &Window::on_need_restart));
    post_message_.connect(sigc::mem_fun(*this, &Window::on_post_message));
    load_web_view_.connect(sigc::mem_fun(*this, &Window::on_load_web_view));
    load_image_view_.connect(sigc::mem_fun(*this, &Window::on_load_image_view));
    add_events(Gdk::KEY_PRESS_MASK);
    signal_key_release_event().connect([](GdkEventKey *event)
    {
        if (event->keyval == GDK_KEY_Escape)
        {
            interface::Escape();
            return false;
        }
        return true;
    });
    web_view_ = (WebKitWebView*)webkit_web_view_new();
    web_view_widget_ = Glib::wrap((GtkWidget*)web_view_);
    web_view_widget_->show();
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

void Window::load_web_view(const __int32_t sender, const __int32_t view_info,
    const char* html, const char* waves)
{
    auto path = path_ + "html/" + html + ".htm";
    webkit_web_view_load_uri(web_view_, path.c_str());
}

void Window::load_image_view(const __int32_t sender, const __int32_t view_info,
    const __int32_t image_width, const char* waves)
{

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
