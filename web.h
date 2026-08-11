//
//  web.h
//  cross
//
//  Created by Ali Asadpoor on 10/12/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#ifndef DESKTOP_WEB_H
#define DESKTOP_WEB_H

#include <mutex>
#include <queue>
#include <string>

#include <webkit/webkit.h>

struct LoadWebViewDispatch
{
    std::int32_t sender;
    std::string html;
};

class WebWidget : public std::reference_wrapper<WebKitWebView>
{
public:
    WebWidget();
    ~WebWidget();
    void push_load(const std::int32_t sender, const char* html);
    void evaluate(const char* function);

private:
    static void load_changed(WebKitWebView* web_view,
        WebKitLoadEvent load_event, gpointer user_data);
    void on_load(const std::int32_t sender, const std::string& html);
    void on_load_changed(WebKitWebView* web_view,
        WebKitLoadEvent load_event);
    void pop_load();

public:
    Gtk::Widget* web_widget_;

private:
    std::mutex dispatch_lock_;
    Glib::Dispatcher dispatcher_;
    std::queue<LoadWebViewDispatch> dispatch_queue_;
    std::int32_t sender_ = 0;
    std::string uri_;
    bool committed_ = false;
};

#endif // DESKTOP_WEB_H
