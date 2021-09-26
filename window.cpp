//
//  window.cpp
//  cross
//
//  Created by Ali Asadpoor on 7/5/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#include <cstring>
#include <fstream>
#include <sstream>
#include <string_view>

#if defined(__APPLE__) || defined(__linux__)
#include <pwd.h>
#endif
#if defined(_WIN32)
#include <Windows.h>
#endif

#include "extern/core/src/bridge.h"
#include "extern/core/src/cross.h"

#include "window.h"

Window* Window::window_;

Window::Window()
    : started_{ false }
    , sender_{ 0 }
{
    window_ = this;
    set_paths();
    add_events(Gdk::KEY_PRESS_MASK);
    signal_key_release_event().connect(sigc::mem_fun(*this, &Window::handle_key));
    need_restart_.connect(sigc::mem_fun(*this, &Window::on_need_restart));
    post_message_.connect(sigc::mem_fun(*this, &Window::on_post_message));
    container_.add(*web_view_.web_widget_, "web");
    container_.set_visible_child("web");
    container_.add(image_view_, "image");
    container_.set_visible_child("image");
    add(container_);
    container_.show();
    signal_window_state_event().connect([this](GdkEventWindowState *state){
        if (state->changed_mask & GdkWindowState::GDK_WINDOW_STATE_ICONIFIED)
        {
            if (state->new_window_state & GdkWindowState::GDK_WINDOW_STATE_ICONIFIED)
            {
                if (started_)
                {
                    started_ = false;
                    cross::Stop();
                }
            }
            else
            {
                if (!started_)
                {
                    started_ = true;
                    cross::Start();
                }
            }
        }
        return true;
    });
    signal_show().connect([this]()
    {
        cross::Create();
        if (!started_)
        {
            started_ = true;
            cross::Start();
        }
    });
    signal_hide().connect([this]()
    {
        if (started_)
        {
            started_ = false;
            cross::Stop();
        }
        cross::Destroy();
    });
    cross::Begin();
}

Window::~Window()
{
    cross::End();
}

void Window::post_restart_message()
{
    need_restart_();
}

void Window::async_message(std::int32_t receiver,
    const char* id, const char* command, const char* info)
{
    post_message_lock_.lock();
    post_message_queue_.push({receiver, id, command, info});
    post_message_lock_.unlock();
    post_message_();
}

void Window::load_view(const std::int32_t sender, const std::int32_t view_info,
    const char* view_name)
{
    sender_ = sender;
    container_.set_visible_child(view_name);
}

bool Window::handle_key(GdkEventKey *event)
{
    if (event->keyval == GDK_KEY_Escape)
    {
        cross::Escape();
        return false;
    }
    return true;
};

void Window::set_paths()
{
    char path[PATH_MAX];
    if (readlink("/proc/self/exe", path, PATH_MAX) == -1)
    {
        strcpy(path, ".");
    }
    assets_path_ = std::filesystem::path(path).parent_path().parent_path() /
        "share" / PROJECT_NAME;
    if (config_path_.empty())
    {
        const char* home_path = getenv("HOME");
        if (home_path)
        {
            config_path_ = home_path;
        }
    }
    if (config_path_.empty())
    {
        passwd* pwd = getpwuid(getuid());
        if (pwd)
        {
            config_path_ = pwd->pw_dir;
        }
    }
    if (config_path_.empty())
    {
        config_path_ = ".";
    }
    config_path_.append(PROJECT_NAME);
    config_path_.append("config");
    std::filesystem::create_directories(config_path_);
}

void Window::on_post_message()
{
    post_message_lock_.lock();
    auto dispatch_info = post_message_queue_.front();
    post_message_queue_.pop();
    post_message_lock_.unlock();
    cross::HandleAsync(dispatch_info.sender,
        dispatch_info.id, dispatch_info.command, dispatch_info.info);
}

void Window::on_need_restart()
{
    cross::Restart();
}
