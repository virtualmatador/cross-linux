//
//  window.cpp
//  cross
//
//  Created by Ali Asadpoor on 7/5/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#include <fstream>
#include <sstream>

#include "../core/src/bridge.h"
#include "../core/src/interface.h"

#include "window.h"

Window* Window::window_;

void sound_callback(struct SoundIoOutStream *outstream,
        int frame_count_min, int frame_count_max)
{
    static const float PI = 3.1415926535f;

    PlayBackInfo* playback_info = (PlayBackInfo*)outstream->userdata;
    const struct SoundIoChannelLayout *layout = &outstream->layout;
    float float_sample_rate = outstream->sample_rate;
    float seconds_per_frame = 1.0f / float_sample_rate;
    struct SoundIoChannelArea *areas;
    int frames_left = playback_info->audio_info->frames - playback_info->progress;
    if (frames_left == 0)
    {
        if (!playback_info->done)
        {
            playback_info->done = true;
            Window::window_->push_audio_destroy(outstream);
        }
        return;
    }
    if (frames_left > frame_count_max)
    {
        frames_left = frame_count_max;
    }
    while (frames_left > 0)
    {
        int frame_count = frames_left;
        if (soundio_outstream_begin_write(outstream, &areas, &frame_count))
        {
        }
        else
        {
            for (int frame = 0; frame < frame_count; ++frame)
            {
                float sample = sin((playback_info->progress + frame) * seconds_per_frame * playback_info->pitch * 2.0f * PI) / 4.0f;
                playback_info->pitch += 0.01f;
                for (int channel = 0; channel < layout->channel_count; channel += 1)
                {
                    float *ptr = (float*)(areas[channel].ptr + areas[channel].step * frame);
                    *ptr = sample;
                }
            }
            if (soundio_outstream_end_write(outstream))
            {
            }
            playback_info->progress += frame_count;
            frames_left -= frame_count;
        }
    }
}

Window::Window(std::string path)
    : path_{ std::filesystem::path(path).parent_path().string() }
    , sender_{ 0 }
    , soundio_{ nullptr }
    , sound_device_{ nullptr }
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
    destroy_stream_dispatcher_.connect(sigc::mem_fun(*this, &Window::pop_audio_destroy));
    soundio_ = soundio_create();
    if (!soundio_)
    {
        throw std::runtime_error("soundio_create");
    }
    if (soundio_connect(soundio_))
    {
        throw std::runtime_error("soundio_connect");
    }
    soundio_flush_events(soundio_);
    int default_out_device_index = soundio_default_output_device_index(soundio_);
    if (default_out_device_index < 0)
    {
        throw std::runtime_error("soundio_default_output_device_index");
    }
    sound_device_ = soundio_get_output_device(soundio_, default_out_device_index);
    if (!sound_device_)
    {
        throw std::runtime_error("soundio_get_output_device");
    }
}

Window::~Window()
{
    interface::Stop();
    interface::Destroy();
    interface::End();
    // TODO stop playing waves
    soundio_device_unref(sound_device_);
    soundio_destroy(soundio_);
}

void Window::post_restart_message()
{
    need_restart_();
}

void Window::post_thread_message(__int32_t receiver, const char* id, const char* command, const char* info)
{
    post_message_lock_.lock();
    post_message_queue_.push({receiver, id, command, info});
    post_message_lock_.unlock();
    post_message_();
}

void Window::load_view(const __int32_t sender, const __int32_t view_info, const char* waves, const char* view_name)
{
    sender_ = sender;
    container_.set_visible_child(view_name);
    waves_.clear();
    std::istringstream is(waves);
    std::string wave;
    while (is >> wave)
    {
        std::ifstream wave_file(path_ + "/assets/wave/" + wave + ".wav");
        if (!wave_file)
        {
            throw std::runtime_error("wave_file");
        }
        waves_.push_back({SoundIoFormatFloat32NE, 440, 60000});
    }
}

void Window::play_audio(const __int32_t index)
{
    SoundIoOutStream* outstream = soundio_outstream_create(sound_device_);
    if (!outstream)
    {
        throw std::runtime_error("soundio_outstream_create");
    }
    outstream->write_callback = sound_callback;
    PlayBackInfo* playback_info = new PlayBackInfo({&waves_[index], waves_[index].pitch, 0, false});
    outstream->format = playback_info->audio_info->format;
    outstream->userdata = playback_info;
    if (soundio_outstream_open(outstream))
    {
        throw std::runtime_error("soundio_outstream_open");
    }
    if (outstream->layout_error)
    {
        throw std::runtime_error("layout_error");
    }
    if (soundio_outstream_start(outstream))
    {
        throw std::runtime_error("soundio_outstream_start");
    }
}

void Window::push_audio_destroy(SoundIoOutStream* outstream)
{
    destroy_stream_lock_.lock();
    destroy_stream_queue_.push(outstream);
    destroy_stream_lock_.unlock();
    destroy_stream_dispatcher_();
}

void Window::pop_audio_destroy()
{
    destroy_stream_lock_.lock();
    SoundIoOutStream* outstream = destroy_stream_queue_.front();
    destroy_stream_queue_.pop();
    destroy_stream_lock_.unlock();
    PlayBackInfo* playback_info = (PlayBackInfo*)outstream->userdata;
    soundio_outstream_destroy(outstream);
    delete playback_info;
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
    post_message_lock_.lock();
    auto dispatch_info = post_message_queue_.front();
    post_message_queue_.pop();
    post_message_lock_.unlock();
    interface::HandleAsync(dispatch_info.sender, dispatch_info.id, dispatch_info.command, dispatch_info.info);
}

void Window::on_need_restart()
{
    interface::Restart();
}
