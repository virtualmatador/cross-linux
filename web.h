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

#include <webkit2/webkit2.h>

struct LoadWebViewDispatch
{
    const std::int32_t sender;
    const std::int32_t view_info;
    const char* html;
    const char* waves;
};

class WebWidget : public std::reference_wrapper<WebKitWebView>
{
public:
    WebWidget();
    ~WebWidget();
    void push_load(const std::int32_t sender, const std::int32_t view_info,
        const char* html, const char* waves);
    void evaluate(const char* function);

private:
    void on_load(const std::int32_t sender, const std::int32_t view_info,
        const char* html, const char* waves);
    void pop_load();

public:
    Gtk::Widget* web_widget_;

private:
    std::mutex dispatch_lock_;
    Glib::Dispatcher dispatcher_;
    std::queue<LoadWebViewDispatch> dispatch_queue_;
};

#endif // DESKTOP_WEB_H
