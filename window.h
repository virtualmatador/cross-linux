#ifndef COLOR_CODE_SRC_WINDOW_H
#define COLOR_CODE_SRC_WINDOW_H

#include <filesystem>
#include <queue>
#include <mutex>
#include <string>

#include <gtkmm.h>
#include <webkit2/webkit2.h>

struct PostMessageDispatch
{
    __int32_t sender;
    const char* id;
    const char* command;
    const char* info;
};

struct LoadWebViewDispatch
{
    const __int32_t sender;
    const __int32_t view_info;
    const char* html;
    const char* waves;
};

struct LoadImageViewDispatch
{
    const __int32_t sender;
    const __int32_t view_info;
    const __int32_t image_width;
    const char* waves;
};

class Window: public Gtk::Window
{
public:
    static Window* window_;

public:
    Window(std::string path);
    ~Window();
    __uint32_t* get_pixels();
    void refresh_image_view();

private:
    void on_need_restart();
    void on_post_message();
    void on_load_web_view();
    void on_load_image_view();

private:
    void load_view(const __int32_t sender, const __int32_t view_info, const char* waves);
    void load_web_view(const __int32_t sender, const __int32_t view_info,
        const char* html, const char* waves);
    void load_image_view(const __int32_t sender, const __int32_t view_info,
        const __int32_t image_width, const char* waves);
    bool handle_key(GdkEventKey *event);
    int create_pixels(int image_width);

private:
    Gtk::Stack container_;
    std::string path_;
    WebKitWebView* web_view_;
    Gtk::Widget* web_view_widget_;
    Glib::RefPtr<Gdk::Pixbuf> pixels_;
    Gtk::DrawingArea image_view_widget_;
    int image_view_width_;
    int image_view_height_;
    bool image_resized_;

public:
    __int32_t sender_;
    std::mutex dispatch_lock_;
    Glib::Dispatcher need_restart_;
    Glib::Dispatcher post_message_;
    std::queue<PostMessageDispatch> post_message_queue_;
    Glib::Dispatcher load_web_view_;
    std::queue<LoadWebViewDispatch> load_web_view_queue_;
    Glib::Dispatcher load_image_view_;
    std::queue<LoadImageViewDispatch> load_image_view_queue_;
};

#endif // COLOR_CODE_SRC_WINDOW_H
