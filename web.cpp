//
//  web.cpp
//  cross
//
//  Created by Ali Asadpoor on 10/12/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#include <cstring>
#include <string>
#include <sstream>

#include "extern/core/src/cross.h"

#include "window.h"

#include "web.h"

void WebKitURISchemeRequestedCross(
    WebKitURISchemeRequest *request, gpointer user_data)
{
    void* data = nullptr;
    std::size_t size = 0;
    cross::FeedUri(webkit_uri_scheme_request_get_uri(request),
    [&](const std::vector<unsigned char>& input)
    {
        size = input.size();
        data = new unsigned char[size];
        std::memcpy(data, input.data(), size);
    });
    if (data)
    {
        auto base_stream = g_memory_input_stream_new_from_data(
            data, size, [](gpointer data){ delete[] (unsigned char*)data; });
        webkit_uri_scheme_request_finish(request, base_stream, size, nullptr);
        g_object_unref(base_stream);
    }
    else
    {
        GError* er = g_error_new(0, 0, "no data");
        webkit_uri_scheme_request_finish_error(request, er);
    }
}

void WebKitURISchemeRequestedAsset(
    WebKitURISchemeRequest *request, gpointer user_data)
{
    auto path =
        Window::window_->assets_path_ / "assets" /
        (webkit_uri_scheme_request_get_uri(request) + sizeof("asset://") - 1);
    GFile *file = g_file_new_for_path(path.string().c_str());
    GFileInputStream *stream = g_file_read(file, NULL, NULL);
    g_object_unref(file);
    webkit_uri_scheme_request_finish(request, (GInputStream*)stream, -1, NULL);
    g_object_unref(stream);
}

void web_view_script_message_received(WebKitUserContentManager* manager,
    WebKitJavascriptResult* js_result, gpointer user_data)
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
    cross::HandleAsync(sender, id.c_str(), command.c_str(), info.c_str());
    webkit_javascript_result_unref(js_result);
}

void web_view_run_javascript_finished(GObject* sourceObject, GAsyncResult* res,
    gpointer userData)
{
    if (userData == (void*)0x01)
    {
        cross::HandleAsync(Window::window_->sender_, "body", "ready", "");
    }
}

void web_view_load_changed(WebKitWebView* web_view, WebKitLoadEvent load_event,
    gpointer user_data)
{
    if (load_event == WEBKIT_LOAD_FINISHED && user_data ==
        reinterpret_cast<void*>(Window::window_->sender_))
    {
        std::ostringstream os;
        os <<
            "var Handler = window.webkit.messageHandlers.Handler_;"
            "var Handler_Receiver = "
            << Window::window_->sender_ << ";"
            "function CallHandler(id, command, info)"
            "{"
            "    Handler.postMessage(Handler_Receiver.toString() "
                    "+ \" \" + id + \" \" + command + \" \" + info);"
            "}"
            "var cross_asset_domain_ = 'asset://';"
            "var cross_asset_async_ = false;"
            "var cross_pointer_type_ = 'mouse';"
            "var cross_pointer_upsidedown_ = false;"
            ;
        webkit_web_view_evaluate_javascript(web_view, os.str().c_str(), -1, nullptr, nullptr, nullptr,
            web_view_run_javascript_finished, (void*)0x01);
    }
}

WebWidget::WebWidget()
    : std::reference_wrapper<WebKitWebView>{
        *(WebKitWebView*)webkit_web_view_new() }
{
    dispatcher_.connect(sigc::mem_fun(*this, &WebWidget::pop_load));
    WebKitUserContentManager *manager =
        webkit_web_view_get_user_content_manager(&get());
    g_signal_connect(manager, "script-message-received::Handler_",
        G_CALLBACK(web_view_script_message_received), nullptr);
    webkit_user_content_manager_register_script_message_handler(
        manager, "Handler_");
    webkit_web_context_register_uri_scheme(webkit_web_context_get_default(),
        "cross", WebKitURISchemeRequestedCross, nullptr, nullptr);
    webkit_web_context_register_uri_scheme(webkit_web_context_get_default(),
        "asset", WebKitURISchemeRequestedAsset, nullptr, nullptr);
    web_widget_ = Glib::wrap((GtkWidget*)&get());
    web_widget_->show();
}

WebWidget::~WebWidget()
{
    Glib::unwrap(web_widget_);
}

void WebWidget::push_load(const std::int32_t sender,
    const std::int32_t view_info, const char* html)
{
    dispatch_lock_.lock();
    dispatch_queue_.push({ sender, view_info, html });
    dispatch_lock_.unlock();
    dispatcher_();
}

void WebWidget::evaluate(const char* function)
{
    webkit_web_view_evaluate_javascript(&get(), function, -1, nullptr, nullptr, nullptr,
        web_view_run_javascript_finished, nullptr);
}

void WebWidget::pop_load()
{
    dispatch_lock_.lock();
    auto dispatch_info = dispatch_queue_.front();
    dispatch_queue_.pop();
    dispatch_lock_.unlock();
    on_load(dispatch_info.sender, dispatch_info.view_info, dispatch_info.html);
}

void WebWidget::on_load(const std::int32_t sender, const std::int32_t view_info,
    const char* html)
{
    Window::window_->sender_ = sender;
    g_signal_connect(&get(), "load-changed", GCallback(web_view_load_changed),
        reinterpret_cast<void*>(sender));
    std::string path = "file://" + (Window::window_->assets_path_ / "assets" /
        (std::string(html) + ".htm")).string();
    webkit_web_view_load_uri(&get(), path.c_str());
}
