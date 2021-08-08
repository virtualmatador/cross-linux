//
//  web.cpp
//  cross
//
//  Created by Ali Asadpoor on 10/12/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#include <string>
#include <sstream>

#include "extern/core/src/interface.h"

#include "window.h"

#include "web.h"

void web_view_script_message_received(WebKitUserContentManager* manager, WebKitJavascriptResult* js_result, gpointer user_data)
{
    auto value = webkit_javascript_result_get_js_value(js_result);
    std::istringstream is{ std::string(jsc_value_to_string(value)) };
    std::int32_t sender;
    std::string id, command, info;
    is >> sender;
    is >> id;
    is >> command;
    is.ignore(1);
    std::getline(is, info);
    interface::HandleAsync(sender, id.c_str(), command.c_str(), info.c_str());
    webkit_javascript_result_unref(js_result);
}

void web_view_run_javascript_finished(GObject* sourceObject, GAsyncResult* res, gpointer userData)
{
    if (userData == (void*)0x01)
    {
        interface::HandleAsync(Window::window_->sender_, "body", "ready", "");
    }
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
        webkit_web_view_run_javascript(web_view, os.str().c_str(), nullptr, web_view_run_javascript_finished, (void*)0x01);
    }
}

WebWidget::WebWidget()
    : std::reference_wrapper<WebKitWebView>{*(WebKitWebView*)webkit_web_view_new()}
{
    dispatcher_.connect(sigc::mem_fun(*this, &WebWidget::pop_load));
    WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(&get());
    g_signal_connect(manager, "script-message-received::Handler_", G_CALLBACK(web_view_script_message_received), nullptr);
    webkit_user_content_manager_register_script_message_handler (manager, "Handler_");
    web_widget_ = Glib::wrap((GtkWidget*)&get());
    web_widget_->show();
}

WebWidget::~WebWidget()
{
    Glib::unwrap(web_widget_);
}

void WebWidget::push_load(const std::int32_t sender, const std::int32_t view_info,
    const char* html, const char* waves)
{
    dispatch_lock_.lock();
    dispatch_queue_.push({sender, view_info, html, waves});
    dispatch_lock_.unlock();
    dispatcher_();
}

void WebWidget::evaluate(const char* function)
{
    webkit_web_view_run_javascript(&get(), function, nullptr, web_view_run_javascript_finished, nullptr);
}

void WebWidget::pop_load()
{
    dispatch_lock_.lock();
    auto dispatch_info = dispatch_queue_.front();
    dispatch_queue_.pop();
    dispatch_lock_.unlock();
    on_load(dispatch_info.sender, dispatch_info.view_info, dispatch_info.html, dispatch_info.waves);
}

void WebWidget::on_load(const std::int32_t sender, const std::int32_t view_info,
    const char* html, const char* waves)
{
    Window::window_->load_view(sender, view_info, waves, "web");
    Window::window_->image_view_.reset_pixels();

    // @Override public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request)
    // {
    //     Intent intent = new Intent(Intent.ACTION_VIEW, request.getUrl());
    //     view.getContext().startActivity(intent);
    //     return true;
    // }
    g_signal_connect(&get(), "load-changed", GCallback(web_view_load_changed), reinterpret_cast<void*>(sender));
    std::string path = "file://" + (Window::window_->assets_path_ / "assets" / "html" / (std::string(html) + ".htm")).string();
    webkit_web_view_load_uri(&get(), path.c_str());
}
