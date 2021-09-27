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
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include <gtkmm.h>

#include <soundio/soundio.h>

#include "web.h"

struct PostMessageDispatch
{
    std::int32_t sender;
    const char* id;
    const char* command;
    const char* info;
};

struct PlayBackInfo
{
    const std::vector<char>* data;
    std::list<SoundIoOutStream*>::iterator it;
    std::size_t progress;
    bool done;
};

class Window: public Gtk::Window
{
public:
    static Window* window_;

public:
    Window();
    ~Window();
    void post_restart_message();
    void async_message(std::int32_t receiver, const char* id,
        const char* command, const char* info);

private:
    bool handle_key(GdkEventKey* event);

private:
    void set_paths();
    void clear_tracks();
    void on_need_restart();
    void on_post_message();

public:
    bool started_;
    WebWidget web_view_;
    std::filesystem::path assets_path_;
    std::filesystem::path config_path_;
    std::int32_t sender_;

private:
    Glib::Dispatcher need_restart_;
    std::mutex post_message_lock_;
    Glib::Dispatcher post_message_;
    std::queue<PostMessageDispatch> post_message_queue_;
};

#endif // DESKTOP_WINDOW_H
