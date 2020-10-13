//
//  image.h
//  cross
//
//  Created by Ali Asadpoor on 10/12/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#ifndef DESKTOP_IMAGE_H
#define DESKTOP_IMAGE_H

#include <queue>

#include <gtkmm.h>

struct LoadImageViewDispatch
{
    const __int32_t sender;
    const __int32_t view_info;
    const __int32_t image_width;
    const char* waves;
};

class ImageWidget : public Gtk::DrawingArea
{
public:
    ImageWidget();
    ~ImageWidget();
    __uint32_t* get_pixels();
    void refresh_image_view();
    void reset_pixels();

private:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_button_press_event(GdkEventButton* button_event) override;
    bool on_motion_notify_event(GdkEventMotion* motion_event) override;
    bool on_button_release_event(GdkEventButton* release_event) override;
    bool on_configure_event(GdkEventConfigure* configure_event) override;

private:
    void load_image_view(const __int32_t sender, const __int32_t view_info,
        const __int32_t image_width, const char* waves);
    void on_load_image_view();
    int create_pixels(int image_width);

private:
    Glib::RefPtr<Gdk::Pixbuf> pixels_;
    int image_view_width_;
    int image_view_height_;
    bool image_resized_;

public:
    Glib::Dispatcher dispatcher_;
    std::queue<LoadImageViewDispatch> dispatch_queue_;
};

#endif // DESKTOP_IMAGE_H
