//
//  bridge.cpp
//  cross
//
//  Created by Ali Asadpoor on 10/11/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#include <cstdint>
#include <fstream>

#include "extern/core/src/bridge.h"

#include "window.h"

void bridge::NeedRestart()
{
    Window::window_->post_restart_message();
}

void bridge::LoadView(const std::int32_t sender,
    const std::int32_t view_info, const char* html)
{
    Window::window_->web_view_.push_load(sender, view_info, html);
}

void bridge::CallFunction(const char* function)
{
    Window::window_->web_view_.evaluate(function);
}

std::string bridge::GetPreference(const char* key)
{
    std::filesystem::path value_path = Window::window_->config_path_ / key;
    std::ifstream value_stream (value_path.string());
    std::string value((std::istreambuf_iterator<char>(value_stream)),
        std::istreambuf_iterator<char>());
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

void bridge::AsyncMessage(std::int32_t receiver,
    const char* id, const char* command, const char* info)
{
    Window::window_->async_message(receiver, id, command, info);
}

void bridge::AddParam(const char *key, const char *value)
{
    // jstring jKey = env_->NewStringUTF(key);
    // jstring jValue = env_->NewStringUTF(value);
    // env_->CallVoidMethod(me_, add_param_, jKey, jValue);
    // env_->DeleteLocalRef(jKey);
    // env_->DeleteLocalRef(jValue);
}

void bridge::PostHttp(const std::int32_t sender,
    const char* id, const char* command, const char *url)
{
    // jstring jId = env_->NewStringUTF(id);
    // jstring jCommand = env_->NewStringUTF(command);
    // jstring jUrl = env_->NewStringUTF(url);
    // env_->CallVoidMethod(me_, post_http_, sender, jId, jCommand, jUrl);
    // env_->DeleteLocalRef(jId);
    // env_->DeleteLocalRef(jCommand);
    // env_->DeleteLocalRef(jUrl);
}

void bridge::CreateImage(const char* id, const char* parent)
{
    std::ostringstream js;
    js <<
        "var img = document.createElement('img');"
        "img.setAttribute('id', '" << id << "');"
        "document.getElementById('" << parent << "').appendChild(img);";
    bridge::CallFunction(js.str().c_str());
}

void bridge::ResetImage(const std::int32_t sender, const std::int32_t index, const char* id)
{
    std::ostringstream js;
    js << "resetImage(" << sender << "," << index << ",'" << id << "');";
    bridge::CallFunction(js.str().c_str());
}

void bridge::Exit()
{
    Window::window_->close();
}
