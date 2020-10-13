//
//  web.h
//  cross
//
//  Created by Ali Asadpoor on 10/12/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#ifndef DESKTOP_WEB_H
#define DESKTOP_WEB_H

#include <queue>

#include <webkit2/webkit2.h>

struct LoadWebViewDispatch
{
    const __int32_t sender;
    const __int32_t view_info;
    const char* html;
    const char* waves;
};

class WebWidget : public std::reference_wrapper<WebKitWebView>
{
public:
    WebWidget();
    ~WebWidget();

private:
    void load_web_view(const __int32_t sender, const __int32_t view_info,
        const char* html, const char* waves);
    void on_load_web_view();

public:
    Gtk::Widget* web_widget_;
    Glib::Dispatcher dispatcher_;
    std::queue<LoadWebViewDispatch> dispatch_queue_;
};

#endif // DESKTOP_WEB_H
