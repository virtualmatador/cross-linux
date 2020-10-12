#include <fstream>

#include "../core/src/bridge.h"

#include "window.h"

void bridge::NeedRestart()
{
    Window::window_->need_restart_();
}

void bridge::LoadWebView(const __int32_t sender, const __int32_t view_info,
    const char* html, const char* waves)
{
    Window::window_->dispatch_lock_.lock();
    Window::window_->load_web_view_queue_.push({sender, view_info, html, waves});
    Window::window_->dispatch_lock_.unlock();
    Window::window_->load_web_view_();
}

void bridge::LoadImageView(const __int32_t sender, const __int32_t view_info,
    const __int32_t image_width, const char* waves)
{
    Window::window_->dispatch_lock_.lock();
    Window::window_->load_image_view_queue_.push({sender, view_info, image_width, waves});
    Window::window_->dispatch_lock_.unlock();
    Window::window_->load_image_view_();
}

__uint32_t* bridge::GetPixels()
{
    return Window::window_->get_pixels();
}

void bridge::ReleasePixels(__uint32_t* const pixels)
{
}

void bridge::RefreshImageView()
{
    Window::window_->refresh_image_view();
}

void bridge::CallFunction(const char* function)
{
    //window_->web_view_.evaluate(function);
}

std::string bridge::GetAsset(const char* key)
{
    std::ifstream asset(key);
    return {std::istreambuf_iterator<char>(asset), std::istreambuf_iterator<char>()};
}

std::string bridge::GetPreference(const char* key)
{
    // jstring jKey = env_->NewStringUTF(key);
    // jstring jValue = (jstring)env_->CallObjectMethod(me_, get_preference_, jKey);
    // const char* szValue = env_->GetStringUTFChars(jValue, nullptr);
    // std::string value = szValue;
    // env_->ReleaseStringUTFChars(jValue, szValue);
    // env_->DeleteLocalRef(jKey);
    // env_->DeleteLocalRef(jValue);
    // return value;
    return "";
}

void bridge::SetPreference(const char* key, const char* value)
{
    // jstring jKey = env_->NewStringUTF(key);
    // jstring jValue = env_->NewStringUTF(value);
    // env_->CallVoidMethod(me_, set_preference_, jKey, jValue);
    // env_->DeleteLocalRef(jKey);
    // env_->DeleteLocalRef(jValue);
}

void bridge::PostThreadMessage(__int32_t receiver, const char* id, const char* command, const char* info)
{
    Window::window_->dispatch_lock_.lock();
    Window::window_->post_message_queue_.push({receiver, id, command, info});
    Window::window_->dispatch_lock_.unlock();
    Window::window_->post_message_();
}

void bridge::AddParam(const char *key, const char *value)
{
    // jstring jKey = env_->NewStringUTF(key);
    // jstring jValue = env_->NewStringUTF(value);
    // env_->CallVoidMethod(me_, add_param_, jKey, jValue);
    // env_->DeleteLocalRef(jKey);
    // env_->DeleteLocalRef(jValue);
}

void bridge::PostHttp(const __int32_t sender, const char* id, const char* command, const char *url)
{
    // jstring jId = env_->NewStringUTF(id);
    // jstring jCommand = env_->NewStringUTF(command);
    // jstring jUrl = env_->NewStringUTF(url);
    // env_->CallVoidMethod(me_, post_http_, sender, jId, jCommand, jUrl);
    // env_->DeleteLocalRef(jId);
    // env_->DeleteLocalRef(jCommand);
    // env_->DeleteLocalRef(jUrl);
}

void bridge::PlayAudio(const __int32_t index)
{
    // env_->CallVoidMethod(me_, play_audio_, index);
}

void bridge::Exit()
{
    Window::window_->close();
}
