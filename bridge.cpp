//
//  bridge.cpp
//  cross
//
//  Created by Ali Asadpoor on 10/11/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#include <cstdint>
#include <fstream>

#include "../core/src/bridge.h"

#include "window.h"

void bridge::NeedRestart()
{
    Window::window_->post_restart_message();
}

void bridge::LoadWebView(const std::int32_t sender, const std::int32_t view_info,
    const char* html, const char* waves)
{
    Window::window_->web_view_.push_load(sender, view_info, html, waves);
}

void bridge::LoadImageView(const std::int32_t sender, const std::int32_t view_info,
    const std::int32_t image_width, const char* waves)
{
    Window::window_->image_view_.push_load(sender, view_info, image_width, waves);
}

std::uint32_t* bridge::GetPixels()
{
    return Window::window_->image_view_.get_pixels();
}

void bridge::ReleasePixels(std::uint32_t* const pixels)
{
    Window::window_->image_view_.release_pixels(pixels);
}

void bridge::RefreshImageView()
{
    Window::window_->image_view_.refresh_image_view();
}

void bridge::CallFunction(const char* function)
{
    Window::window_->web_view_.evaluate(function);
}

std::string bridge::GetAsset(const char* key)
{
    std::ifstream asset(key);
    return {std::istreambuf_iterator<char>(asset), std::istreambuf_iterator<char>()};
}

std::string bridge::GetPreference(const char* key)
{
    std::filesystem::path value_path = Window::window_->config_path_ / key;
    std::ifstream value_stream (value_path.string());
    std::string value((std::istreambuf_iterator<char>(value_stream)), std::istreambuf_iterator<char>());
    return value;
}

void bridge::SetPreference(const char* key, const char* value)
{
    std::filesystem::path value_path = Window::window_->config_path_ / key;
    std::ofstream value_stream (value_path.string());
    if (value_stream)
    {
        value_stream << value;
    }
}

void bridge::PostThreadMessage(std::int32_t receiver, const char* id, const char* command, const char* info)
{
    Window::window_->post_thread_message(receiver, id, command, info);
}

void bridge::AddParam(const char *key, const char *value)
{
    // jstring jKey = env_->NewStringUTF(key);
    // jstring jValue = env_->NewStringUTF(value);
    // env_->CallVoidMethod(me_, add_param_, jKey, jValue);
    // env_->DeleteLocalRef(jKey);
    // env_->DeleteLocalRef(jValue);
}

void bridge::PostHttp(const std::int32_t sender, const char* id, const char* command, const char *url)
{
    // jstring jId = env_->NewStringUTF(id);
    // jstring jCommand = env_->NewStringUTF(command);
    // jstring jUrl = env_->NewStringUTF(url);
    // env_->CallVoidMethod(me_, post_http_, sender, jId, jCommand, jUrl);
    // env_->DeleteLocalRef(jId);
    // env_->DeleteLocalRef(jCommand);
    // env_->DeleteLocalRef(jUrl);
}

void bridge::PlayAudio(const std::int32_t index)
{
    Window::window_->play_audio(index);
}

void bridge::Exit()
{
    Window::window_->close();
}
