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
#include "image.h"

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
    void post_thread_message(std::int32_t receiver, const char* id, const char* command, const char* info);
    void load_view(const std::int32_t sender, const std::int32_t view_info, const char* waves, const char* view_name);
    void play_audio(const std::int32_t index);
    void push_audio_destroy(SoundIoOutStream* outstream, bool callback);
    void pop_audio_destroy();

private:
    bool handle_key(GdkEventKey* event);

private:
    void set_paths();
    void clear_tracks();
    void on_need_restart();
    void on_post_message();

public:
    bool started_;
    Gtk::Stack container_;
    WebWidget web_view_;
    ImageWidget image_view_;
    std::filesystem::path assets_path_;
    std::filesystem::path config_path_;
    std::int32_t sender_;

private:
    Glib::Dispatcher need_restart_;
    std::mutex post_message_lock_;
    Glib::Dispatcher post_message_;
    std::queue<PostMessageDispatch> post_message_queue_;
    SoundIo* soundio_;
    SoundIoDevice* sound_device_;
    std::vector<std::vector<char>> waves_;
    std::list<SoundIoOutStream*> tracks_;
    std::mutex destroy_stream_lock_;
    Glib::Dispatcher destroy_stream_dispatcher_;
    std::queue<SoundIoOutStream*> destroy_stream_queue_;
};

#endif // DESKTOP_WINDOW_H
