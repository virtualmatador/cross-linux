//
//  bridge.cpp
//  cross
//
//  Created by Ali Asadpoor on 10/11/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <system_error>
#include <utility>

#include "extern/core/src/bridge.h"

#include "window.h"

namespace
{
constexpr const char* save_name = "SAVE";

std::filesystem::path SavePath()
{
    return Window::window_->config_path_ / save_name;
}

std::filesystem::path TemporarySavePath()
{
    auto path = SavePath();
    path += ".tmp";
    return path;
}

void RemoveTemporarySave(const std::filesystem::path& path)
{
    std::error_code error;
    std::filesystem::remove(path, error);
}

std::shared_ptr<std::istream> RestoreInput()
{
    return std::make_shared<std::ifstream>(SavePath(),
        std::ios::in | std::ios::binary);
}
}

void bridge::LoadView(const std::int32_t sender, const char* html)
{
    Window::window_->web_view_.push_load(sender, html);
}

void bridge::SetScreenOn(bool)
{
}

void bridge::SetAudioNoSolo(bool)
{
}

void bridge::SetLayout(bool, bool)
{
}

void bridge::CallFunction(const char* function)
{
    Window::window_->web_view_.evaluate(function);
}

void bridge::Restore(application::Completion completion)
{
    auto input = RestoreInput();
    application::Restore(*input,
        [input, completion = std::move(completion)]() mutable
        {
            completion();
            input.reset();
        });
}

void bridge::Checkpoint()
{
    const auto save_path = SavePath();
    const auto temporary_path = TemporarySavePath();
    std::ofstream output(temporary_path,
        std::ios::out | std::ios::binary | std::ios::trunc);
    application::Checkpoint(output);
    output.flush();
    output.close();
    if (!output)
    {
        RemoveTemporarySave(temporary_path);
        return;
    }

    std::error_code error;
    std::filesystem::rename(temporary_path, save_path, error);
    if (error)
        RemoveTemporarySave(temporary_path);
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
