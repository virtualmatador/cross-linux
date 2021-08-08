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
#include "extern/core/src/interface.h"

#include "window.h"

Window* Window::window_;

void sound_callback(struct SoundIoOutStream *outstream,
        int frame_count_min, int frame_count_max)
{
    PlayBackInfo* playback_info = (PlayBackInfo*)outstream->userdata;
    int frames_left = (playback_info->data->size() - playback_info->progress) / (outstream->bytes_per_sample * outstream->layout.channel_count);
    if (frames_left == 0)
    {
        Window::window_->push_audio_destroy(outstream, true);
        return;
    }
    if (frames_left > frame_count_max)
    {
        frames_left = frame_count_max;
    }
    while (frames_left > 0)
    {
        int frame_count = frames_left;
        SoundIoChannelArea *areas;
        if (soundio_outstream_begin_write(outstream, &areas, &frame_count))
        {
        }
        else
        {
            for (int frame = 0; frame < frame_count; ++frame)
            {
                for (int channel = 0; channel < outstream->layout.channel_count; ++channel)
                {
                    std::memcpy(areas[channel].ptr + areas[channel].step * frame,
                        playback_info->data->data() +
                            playback_info->progress +
                            frame * outstream->bytes_per_sample * outstream->layout.channel_count +
                            channel * outstream->bytes_per_sample,
                        outstream->bytes_per_sample);
                }
            }
            if (soundio_outstream_end_write(outstream))
            {
            }
            playback_info->progress += frame_count * outstream->bytes_per_sample * outstream->layout.channel_count;
            frames_left -= frame_count;
        }
    }
}

Window::Window()
    : started_{ false }
    , sender_{ 0 }
    , soundio_{ nullptr }
    , sound_device_{ nullptr }
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
    signal_window_state_event().connect([this](GdkEventWindowState *state){
        if (state->changed_mask & GdkWindowState::GDK_WINDOW_STATE_ICONIFIED)
        {
            if (state->new_window_state & GdkWindowState::GDK_WINDOW_STATE_ICONIFIED)
            {
                if (started_)
                {
                    started_ = false;
                    interface::Stop();
                }
            }
            else
            {
                if (!started_)
                {
                    started_ = true;
                    interface::Start();
                }
            }
        }
        return true;
    });
    signal_show().connect([this]()
    {
        interface::Create();
        if (!started_)
        {
            started_ = true;
            interface::Start();
        }
    });
    signal_hide().connect([this]()
    {
        if (started_)
        {
            started_ = false;
            interface::Stop();
        }
        interface::Destroy();
    });
    interface::Begin();
}

Window::~Window()
{
    interface::End();
    clear_tracks();
    soundio_device_unref(sound_device_);
    soundio_destroy(soundio_);
}

void Window::post_restart_message()
{
    need_restart_();
}

void Window::post_thread_message(std::int32_t receiver, const char* id, const char* command, const char* info)
{
    post_message_lock_.lock();
    post_message_queue_.push({receiver, id, command, info});
    post_message_lock_.unlock();
    post_message_();
}

void Window::load_view(const std::int32_t sender, const std::int32_t view_info, const char* waves, const char* view_name)
{
    sender_ = sender;
    container_.set_visible_child(view_name);
    clear_tracks();
    std::istringstream is(waves);
    std::string wave;
    while (is >> wave)
    {
        std::ifstream wave_file {assets_path_ / "assets" / "wave" / (wave + ".wav"), std::ios_base::openmode::_S_bin};
        if (!wave_file)
        {
            throw std::runtime_error("No file");
        }
        char buffer[4];
        wave_file.read(buffer, sizeof(buffer));
        if (std::string_view(buffer, 4) != "RIFF")
        {
            throw std::runtime_error("No RIFF");
        }
        wave_file.read(buffer, sizeof(buffer));
        wave_file.read(buffer, sizeof(buffer));
        if (std::string_view(buffer, 4) != "WAVE")
        {
            throw std::runtime_error("No WAVE");
        }
        wave_file.read(buffer, sizeof(buffer));
        if (std::string_view(buffer, 4) != "fmt ")
        {
            throw std::runtime_error("No fmt");
        }
        wave_file.read(buffer, sizeof(buffer));
        if (*(std::int32_t*)buffer != 16)
        {
            throw std::runtime_error("No 16");
        }
        wave_file.read(buffer, sizeof(buffer));
        if (*(short*)buffer != 1)
        {
            throw std::runtime_error("No PCM");
        }
        if (*(short*)(buffer + 2) != 2)
        {
            throw std::runtime_error("No Stereo");
        }
        wave_file.read(buffer, sizeof(buffer));
        if (*(std::int32_t*)buffer != 44100)
        {
            throw std::runtime_error("No 44100");
        }
        wave_file.read(buffer, sizeof(buffer));
        if (*(std::int32_t*)buffer != 176400)
        {
            throw std::runtime_error("No 176400");
        }
        wave_file.read(buffer, sizeof(buffer));
        if (*(short*)buffer != 4)
        {
            throw std::runtime_error("No 4 BPS");
        }
        if (*(short*)(buffer + 2) != 16)
        {
            throw std::runtime_error("No 16 bit");
        }
        wave_file.read(buffer, sizeof(buffer));
        if (std::string_view(buffer, 4) != "data")
        {
            throw std::runtime_error("No data");
        }
        wave_file.read(buffer, sizeof(buffer));
        std::vector<char> data(*(std::int32_t*)buffer);
        wave_file.read(data.data(), data.size());
        waves_.emplace_back(std::move(data));
    }
}

void Window::play_audio(const std::int32_t index)
{
    SoundIoOutStream* outstream = soundio_outstream_create(sound_device_);
    if (!outstream)
    {
        throw std::runtime_error("soundio_outstream_create");
    }
    tracks_.emplace_back(outstream);
    outstream->write_callback = sound_callback;
    outstream->format = SoundIoFormatS16LE;
    outstream->sample_rate = 44100;
    PlayBackInfo* playback_info = new PlayBackInfo({&waves_[index], std::prev(tracks_.end()), 0, false});
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

void Window::push_audio_destroy(SoundIoOutStream* outstream, bool callback)
{
    PlayBackInfo* playback_info = (PlayBackInfo*)outstream->userdata;
    destroy_stream_lock_.lock();
    if (!playback_info->done)
    {
        playback_info->done = true;
        destroy_stream_queue_.push(outstream);
    }
    destroy_stream_lock_.unlock();
    if (callback)
    {
        destroy_stream_dispatcher_();
    }
    else
    {
        pop_audio_destroy();
    }
}

void Window::pop_audio_destroy()
{
    for (;;)
    {
        SoundIoOutStream* outstream;
        destroy_stream_lock_.lock();
        if (destroy_stream_queue_.size())
        {
            outstream = destroy_stream_queue_.front();
            destroy_stream_queue_.pop();
        }
        else
        {
            outstream = nullptr;
        }    
        destroy_stream_lock_.unlock();
        if (outstream)
        {
            PlayBackInfo* playback_info = (PlayBackInfo*)outstream->userdata;
            soundio_outstream_destroy(outstream);
            tracks_.erase(playback_info->it);
            delete playback_info;
        }
        else
        {
            break;
        }
    }
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

void Window::set_paths()
{
#if defined(__APPLE__) || defined(__linux__)
    char path[PATH_MAX];
    if (readlink("/proc/self/exe", path, PATH_MAX) == -1)
    {
        strcpy(path, ".");
    }
#endif
#if defined(_WIN32)
    char path[MAX_PATH]; 
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule != NULL)
    {
        GetModuleFileName(hModule, path, sizeof(path));
    }
    else
    {
        strcpy(path, ".");
    }
#endif
    assets_path_ = std::filesystem::path(path).parent_path().parent_path() / "share" / PROJECT_NAME;
#if defined(__APPLE__) || defined(__linux__)
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
#endif
#if defined(_WIN32)
    if (config_path_.empty())
    {
        const char* home_path = getenv("USERPROFILE");
        if (home_path)
        {
            config_path_ = home_path;
        }
    }
    if (config_path_.empty())
    {
        const char* home_drive = getenv("HOMEDRIVE");
        const char* home_path = getenv("HOMEPATH");
        if (home_drive && home_path)
        {
            config_path_ = std::string(home_drive) + std::string(home_path);
        }
    }
#endif
    if (config_path_.empty())
    {
        config_path_ = ".";
    }
    config_path_.append(".sanke");
    config_path_.append("config");
    std::filesystem::create_directories(config_path_);
}

void Window::clear_tracks()
{
    while (!tracks_.empty())
    {
        push_audio_destroy(tracks_.front(), false);
    }
    waves_.clear();
}

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
