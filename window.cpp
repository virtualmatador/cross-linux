//
//  window.cpp
//  cross
//
//  Created by Ali Asadpoor on 7/5/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#include <sstream>

#include "../core/src/bridge.h"
#include "../core/src/interface.h"

#include "window.h"

Window* Window::window_;

Window::Window(std::string path)
    : path_{ "file://" + std::filesystem::path(path).parent_path().string() + "/assets/" }
    , sender_{ 0 }
{
    window_ = this;
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
    interface::Begin();
    interface::Create();
    interface::Start();
}

Window::~Window()
{
    interface::Stop();
    interface::Destroy();
    interface::End();
}

bool Window::handle_key(GdkEventKey *event)
{
    if (event->keyval == GDK_KEY_Escape)
    {
        interface::Escape();
        return false;
    }
    return true;
};

void Window::on_post_message()
{
    dispatch_lock_.lock();
    auto dispatch_info = post_message_queue_.front();
    post_message_queue_.pop();
    dispatch_lock_.unlock();
    interface::HandleAsync(dispatch_info.sender, dispatch_info.id, dispatch_info.command, dispatch_info.info);
}

void Window::on_need_restart()
{
    interface::Restart();
}

void Window::load_view(const __int32_t sender, const __int32_t view_info, const char* waves)
{
    sender_ = sender;
    //LoadAudio(waves, new Runnable()
}
