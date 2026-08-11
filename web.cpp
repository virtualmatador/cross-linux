//
//  web.cpp
//  cross
//
//  Created by Ali Asadpoor on 10/12/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#include <cstring>
#include <memory>
#include <sstream>
#include <string>

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
    GError *error = nullptr;
    GFileInputStream *stream = g_file_read(file, nullptr, &error);
    g_object_unref(file);

    if (!stream)
    {
        webkit_uri_scheme_request_finish_error(request, error);
        g_error_free(error);
        return;
    }

    auto response = webkit_uri_scheme_response_new(
        G_INPUT_STREAM(stream), -1);
    webkit_uri_scheme_response_set_status(response, 200, nullptr);
    webkit_uri_scheme_response_set_content_type(response, "audio/wav");

    auto headers = soup_message_headers_new(SOUP_MESSAGE_HEADERS_RESPONSE);
    soup_message_headers_append(headers, "Access-Control-Allow-Origin", "*");
    webkit_uri_scheme_response_set_http_headers(response, headers);

    webkit_uri_scheme_request_finish_with_response(request, response);
    g_object_unref(response);
    g_object_unref(stream);
}

void web_view_script_message_received(WebKitUserContentManager* manager,
    JSCValue* value, gpointer user_data)
{
    auto message = jsc_value_to_string(value);
    std::istringstream is{ std::string(message) };
    g_free(message);
    std::int32_t sender = 0;
    std::string id, command, info;
    is >> sender;
    is >> id;
    is >> command;
    is.ignore(1);
    std::getline(is, info);
    cross::HandleAsync(sender, id.c_str(), command.c_str(), info.c_str());
}

namespace
{
struct ReadyDispatch
{
    std::int32_t receiver;
    std::string uri;
};

bool finish_javascript(GObject* source_object, GAsyncResult* result)
{
    GError* error = nullptr;
    auto value = webkit_web_view_evaluate_javascript_finish(
        WEBKIT_WEB_VIEW(source_object), result, &error);
    const bool succeeded = value != nullptr;
    if (value)
        g_object_unref(value);
    if (error)
    {
        g_error_free(error);
        return false;
    }
    return succeeded;
}

void web_view_javascript_finished(GObject* source_object, GAsyncResult* result,
    gpointer)
{
    finish_javascript(source_object, result);
}

void web_view_ready_finished(GObject* source_object, GAsyncResult* result,
    gpointer user_data)
{
    std::unique_ptr<ReadyDispatch> dispatch{
        static_cast<ReadyDispatch*>(user_data)};
    if (!finish_javascript(source_object, result))
        return;
    const auto uri = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(source_object));
    if (!uri || dispatch->uri != uri)
        return;
    cross::HandleAsync(dispatch->receiver, "body", "ready", "");
}
} // namespace

void WebWidget::load_changed(WebKitWebView* web_view,
    WebKitLoadEvent load_event, gpointer user_data)
{
    static_cast<WebWidget*>(user_data)->on_load_changed(web_view, load_event);
}

void WebWidget::on_load_changed(WebKitWebView* web_view,
    WebKitLoadEvent load_event)
{
    const auto uri = webkit_web_view_get_uri(web_view);
    if (load_event == WEBKIT_LOAD_STARTED)
    {
        if (!uri || uri_ != uri)
            committed_ = false;
        return;
    }
    if (load_event == WEBKIT_LOAD_COMMITTED)
    {
        committed_ = uri && uri_ == uri && sender_ > 0 &&
            sender_ == cross::StageIndex();
        return;
    }
    if (load_event != WEBKIT_LOAD_FINISHED || !committed_ || !uri ||
        uri_ != uri || sender_ != cross::StageIndex())
        return;

    std::ostringstream os;
    os <<
        "var Handler = window.webkit.messageHandlers.Handler_;"
        "var Handler_Receiver = "
        << sender_ << ";"
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
    auto dispatch = new ReadyDispatch{sender_, uri_};
    webkit_web_view_evaluate_javascript(web_view, os.str().c_str(), -1,
        nullptr, nullptr, nullptr, web_view_ready_finished, dispatch);
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
        manager, "Handler_", nullptr);
    webkit_web_context_register_uri_scheme(webkit_web_context_get_default(),
        "cross", WebKitURISchemeRequestedCross, nullptr, nullptr);
    webkit_web_context_register_uri_scheme(webkit_web_context_get_default(),
        "asset", WebKitURISchemeRequestedAsset, nullptr, nullptr);
    g_signal_connect(&get(), "load-changed", G_CALLBACK(WebWidget::load_changed),
        this);
    web_widget_ = Glib::wrap((GtkWidget*)&get());
}

WebWidget::~WebWidget()
{
    Glib::unwrap(web_widget_);
}

void WebWidget::push_load(const std::int32_t sender, const char* html)
{
    if (!html)
        return;
    {
        std::lock_guard<std::mutex> guard{dispatch_lock_};
        dispatch_queue_.push({sender, html});
    }
    dispatcher_();
}

void WebWidget::evaluate(const char* function)
{
    webkit_web_view_evaluate_javascript(&get(), function, -1, nullptr, nullptr,
        nullptr, web_view_javascript_finished, nullptr);
}

void WebWidget::pop_load()
{
    LoadWebViewDispatch dispatch_info;
    {
        std::lock_guard<std::mutex> guard{dispatch_lock_};
        if (dispatch_queue_.empty())
            return;
        dispatch_info = std::move(dispatch_queue_.back());
        while (!dispatch_queue_.empty())
            dispatch_queue_.pop();
    }
    on_load(dispatch_info.sender, dispatch_info.html);
}

void WebWidget::on_load(const std::int32_t sender, const std::string& html)
{
    if (sender <= 0 || sender != cross::StageIndex() || html.empty())
        return;
    sender_ = sender;
    committed_ = false;
    uri_ = "file://" + (Window::window_->assets_path_ / "assets" /
        (html + ".htm")).string() + "?cross_receiver=" +
        std::to_string(sender) + "#cross-" + std::to_string(sender);
    webkit_web_view_load_uri(&get(), uri_.c_str());
}
