#include "../core/src/bridge.h"
#include "../core/src/interface.h"

#include "window.h"

Window* window_;

Window::Window()
{
    window_ = this;
    need_restart_.connect(&interface::Restart);
    thread_message_.connect(&interface::HandleAsync);
    add_events(Gdk::KEY_PRESS_MASK);
    signal_key_release_event().connect([](GdkEventKey *event)
    {
        if (event->keyval == GDK_KEY_Escape)
        {
            interface::Escape();
            return false;
        }
        return true;
    });
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


void bridge::NeedRestart()
{
    window_->need_restart_();
}

void bridge::LoadWebView(const __int32_t sender, const __int32_t view_info, const char* html, const char* waves)
{
    //window_->load_web_view_(sender, view_info, html, waves);
}

void bridge::LoadImageView(const __int32_t sender, const __int32_t view_info, const __int32_t image_width, const char* waves)
{
    //window_->load_image_view_(sender, view_info, image_width, waves);
}

__uint32_t* bridge::GetPixels()
{
    //return window_->pixels_;
    return nullptr;
}

void bridge::ReleasePixels(__uint32_t* const pixels)
{
    //window_->pixels_.clear();
}

void bridge::RefreshImageView()
{
    //window_->refresh_image_view_();
}

void bridge::CallFunction(const char* function)
{
    //window_->web_view_.evaluate(function);
}

std::string bridge::GetAsset(const char* key)
{
    // jstring jKey = env_->NewStringUTF(key);
    // jstring jValue = (jstring)env_->CallObjectMethod(me_, get_asset_, jKey);
    // const char* szValue = env_->GetStringUTFChars(jValue, nullptr);
    // std::string value = szValue;
    // env_->ReleaseStringUTFChars(jValue, szValue);
    // env_->DeleteLocalRef(jKey);
    // env_->DeleteLocalRef(jValue);
    // return value;
    return "";
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
    window_->thread_message_(receiver, id, command, info);
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
    window_->close();
}
