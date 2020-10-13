//
//  window.h
//  cross
//
//  Created by Ali Asadpoor on 7/5/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#ifndef DESKTOP_WINDOW_H
#define DESKTOP_WINDOW_H

#include <filesystem>
#include <queue>
#include <mutex>
#include <string>

#include <gtkmm.h>

#include "web.h"
#include "image.h"

struct PostMessageDispatch
{
    __int32_t sender;
    const char* id;
    const char* command;
    const char* info;
};

class Window: public Gtk::Window
{
public:
    static Window* window_;

public:
    Window(std::string path);
    ~Window();
    void load_view(const __int32_t sender, const __int32_t view_info, const char* waves);

private:
    void on_need_restart();
    void on_post_message();

private:
    bool handle_key(GdkEventKey *event);

public:
    std::string path_;
    Gtk::Stack container_;
    WebWidget web_view_;
    ImageWidget image_view_;
    __int32_t sender_;
    std::mutex dispatch_lock_;
    Glib::Dispatcher need_restart_;
    Glib::Dispatcher post_message_;
    std::queue<PostMessageDispatch> post_message_queue_;
};

#endif // DESKTOP_WINDOW_H
