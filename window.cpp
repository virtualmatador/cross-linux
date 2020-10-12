#include <sstream>

#include "../core/src/bridge.h"
#include "../core/src/interface.h"

#include "window.h"

Window* Window::window_;

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
    if (load_event == WEBKIT_LOAD_FINISHED && user_data == reinterpret_cast<void*>(Window::window_->sender_))
    {
        std::ostringstream os;
        os <<
            "var Handler = window.webkit.messageHandlers.Handler_;"
            "var Handler_Receiver = "
            << Window::window_->sender_ << ";"
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
    , pixels_{ nullptr }
    , image_view_width_{ 0 }
    , image_view_height_{ 0 }
    , image_resized_ { false }
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
    web_view_widget_->show();
    image_view_widget_.signal_draw().connect([this](const Cairo::RefPtr<Cairo::Context>& cr)
    {
        auto allocation = image_view_widget_.get_allocation();
        cr->scale((double)allocation.get_width() / (double)pixels_->get_width(), (double)allocation.get_height() / (double)pixels_->get_height());
        Gdk::Cairo::set_source_pixbuf(cr, pixels_);
        cr->paint();
        return true;
    });
    image_view_widget_.signal_configure_event().connect([this](GdkEventConfigure* event)
    {
        image_view_width_ = event->width;
        image_view_height_ = event->height;
        image_resized_ = true;
        return true;
    });
    image_view_widget_.show();
    container_.add(*web_view_widget_, "web");
    container_.set_visible_child("web");
    container_.add(image_view_widget_, "image");
    container_.set_visible_child("image");
    add(container_);
    container_.show();
    interface::Begin();
    interface::Create();
    interface::Start();
}

Window::~Window()
{
    Glib::unwrap(web_view_widget_);
    interface::Stop();
    interface::Destroy();
    interface::End();
}

__uint32_t* Window::get_pixels()
{
    return (__uint32_t*) pixels_->get_pixels();
}

void Window::refresh_image_view()
{
    // consider double buffer
    if (!image_resized_)
    {
        image_view_widget_.queue_draw();
    }
    else
    {
        int image_height = create_pixels(pixels_->get_width());
        std::ostringstream info;
        info << pixels_->get_width() << " " << image_height;
        interface::Handle("body", "resize", info.str().c_str());
    }
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
    load_image_view_queue_.pop();
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
    //LoadAudio(waves, new Runnable()
}

void Window::load_web_view(const __int32_t sender, const __int32_t view_info,
    const char* html, const char* waves)
{
    container_.set_visible_child("web");
    pixels_.reset();

    // @Override public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request)
    // {
    //     Intent intent = new Intent(Intent.ACTION_VIEW, request.getUrl());
    //     view.getContext().startActivity(intent);
    //     return true;
    // }
    g_signal_connect(web_view_, "load-changed", GCallback(web_view_load_changed), reinterpret_cast<void*>(sender));
    auto path = path_ + "html/" + html + ".htm";
    webkit_web_view_load_uri(web_view_, path.c_str());
    load_view(sender, view_info, waves);
    interface::HandleAsync(sender, "body", "ready", "");
}

void Window::load_image_view(const __int32_t sender, const __int32_t view_info,
    const __int32_t image_width, const char* waves)
{
    container_.set_visible_child("image");
    int image_height = create_pixels(image_width);
    load_view(sender, view_info, waves);
    std::ostringstream info;
    info << image_width / 10 << " " << image_width << " " << image_height << " " << 0x02010003;
    interface::HandleAsync(sender_, "body", "ready", info.str().c_str());
}

int Window::create_pixels(int image_width)
{
    float scale = (float)image_width / (float)image_view_width_;
    int image_height = (__int32_t)(image_view_height_ * scale);
    guint8* rgb_data = new guint8[4 * image_width * image_height];
    pixels_ = Gdk::Pixbuf::create_from_data(rgb_data, Gdk::Colorspace::COLORSPACE_RGB, true, 8, image_width, image_height, 4 * image_width,
    [rgb_data](const guint8*)
    {
        delete[] rgb_data;
    });
    image_resized_ = false;
    return image_height;
}