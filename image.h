//
//  image.h
//  cross
//
//  Created by Ali Asadpoor on 10/12/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#ifndef DESKTOP_IMAGE_H
#define DESKTOP_IMAGE_H

#include <mutex>
#include <queue>

#include <gtkmm.h>

struct LoadImageViewDispatch
{
    const std::int32_t sender;
    const std::int32_t view_info;
    const std::int32_t image_width;
};

class ImageWidget : public Gtk::DrawingArea
{
public:
    ImageWidget();
    ~ImageWidget();
    void push_load(const std::int32_t sender, const std::int32_t view_info,
        const std::int32_t image_width);
    std::uint32_t* get_pixels();
    void release_pixels(std::uint32_t*);
    void refresh_image_view();
    void reset_pixels();

private:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_button_press_event(GdkEventButton* button_event) override;
    bool on_motion_notify_event(GdkEventMotion* motion_event) override;
    bool on_button_release_event(GdkEventButton* release_event) override;
    bool on_configure_event(GdkEventConfigure* configure_event) override;

private:
    void on_load(const std::int32_t sender, const std::int32_t view_info,
        const std::int32_t image_width);
    void pop_load();
    int create_pixels(int image_width);

private:
    Glib::RefPtr<Gdk::Pixbuf> pixels_;
    std::mutex pixels_lock_;
    int image_view_width_;
    int image_view_height_;
    bool image_resized_;
    std::mutex dispatch_lock_;
    Glib::Dispatcher dispatcher_;
    std::queue<LoadImageViewDispatch> dispatch_queue_;
};

#endif // DESKTOP_IMAGE_H
