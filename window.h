#ifndef COLOR_CODE_SRC_WINDOW_H
#define COLOR_CODE_SRC_WINDOW_H

#include <gtkmm.h>

class Window: public Gtk::Window
{
public:
    Window();
    ~Window();

public:
    sigc::signal<void> need_restart_;
    sigc::signal<void, __int32_t, const char*, const char*, const char*> thread_message_;
};

#endif // COLOR_CODE_SRC_WINDOW_H
