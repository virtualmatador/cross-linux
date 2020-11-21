//
//  image.cpp
//  cross
//
//  Created by Ali Asadpoor on 10/12/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#include "../core/src/interface.h"

#include "window.h"

#include "image.h"

ImageWidget::ImageWidget()
    : pixels_{ nullptr }
    , image_view_width_{ 0 }
    , image_view_height_{ 0 }
    , image_resized_ { false }
{
    add_events(
        Gdk::EventMask::BUTTON_PRESS_MASK |
        Gdk::EventMask::BUTTON_RELEASE_MASK |
        Gdk::EventMask::BUTTON1_MOTION_MASK);
    dispatcher_.connect(sigc::mem_fun(*this, &ImageWidget::pop_load));
    show();
}

ImageWidget::~ImageWidget()
{
    
}

void ImageWidget::push_load(const std::int32_t sender, const std::int32_t view_info,
    const std::int32_t image_width, const char* waves)
{
    dispatch_lock_.lock();
    dispatch_queue_.push({sender, view_info, image_width, waves});
    dispatch_lock_.unlock();
    dispatcher_();
}

bool ImageWidget::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    auto allocation = get_allocation();
    cr->scale((double)allocation.get_width() / (double)pixels_->get_width(), (double)allocation.get_height() / (double)pixels_->get_height());
    pixels_lock_.lock();
    Gdk::Cairo::set_source_pixbuf(cr, pixels_);
    cr->paint();
    pixels_lock_.unlock();
    return true;
}

bool ImageWidget::on_button_press_event(GdkEventButton* button_event)
{
    std::stringstream is;
    is << button_event->x * pixels_->get_width() / image_view_width_;
    is << " ";
    is << button_event->y * pixels_->get_height() / image_view_height_;
    interface::Handle("body", "touch-begin", is.str().c_str());
    return true;
}

bool ImageWidget::on_motion_notify_event(GdkEventMotion* motion_event)
{
    std::stringstream is;
    is << motion_event->x * pixels_->get_width() / image_view_width_;
    is << " ";
    is << motion_event->y * pixels_->get_height() / image_view_height_;
    interface::Handle("body", "touch-move", is.str().c_str());
    return true;
}

bool ImageWidget::on_button_release_event(GdkEventButton* release_event)
{
    std::stringstream is;
    is << release_event->x * pixels_->get_width() / image_view_width_;
    is << " ";
    is << release_event->y * pixels_->get_height() / image_view_height_;
    interface::Handle("body", "touch-end", is.str().c_str());
    return true;
}

bool ImageWidget::on_configure_event(GdkEventConfigure* configure_event)
{
    image_view_width_ = configure_event->width;
    image_view_height_ = configure_event->height;
    image_resized_ = true;
    return true;
}

std::uint32_t* ImageWidget::get_pixels()
{
    pixels_lock_.lock();
    return (std::uint32_t*) pixels_->get_pixels();
}

void ImageWidget::release_pixels(std::uint32_t*)
{
    pixels_lock_.unlock();
}

void ImageWidget::refresh_image_view()
{
    // consider double buffer
    if (!image_resized_)
    {
        queue_draw();
    }
    else
    {
        int image_height = create_pixels(pixels_->get_width());
        std::ostringstream info;
        info << pixels_->get_width() << " " << image_height;
        interface::Handle("body", "resize", info.str().c_str());
    }
}

void ImageWidget::reset_pixels()
{
    pixels_.reset();
}

void ImageWidget::pop_load()
{
    dispatch_lock_.lock();
    auto dispatch_info = dispatch_queue_.front();
    dispatch_queue_.pop();
    dispatch_lock_.unlock();
    on_load(dispatch_info.sender, dispatch_info.view_info, dispatch_info.image_width, dispatch_info.waves);
}

int ImageWidget::create_pixels(int image_width)
{
    float scale = (float)image_width / (float)image_view_width_;
    int image_height = (std::int32_t)(image_view_height_ * scale);
    guint8* rgb_data = new guint8[4 * image_width * image_height];
    pixels_ = Gdk::Pixbuf::create_from_data(rgb_data, Gdk::Colorspace::COLORSPACE_RGB, true, 8, image_width, image_height, 4 * image_width,
    [rgb_data](const guint8*)
    {
        delete[] rgb_data;
    });
    image_resized_ = false;
    return image_height;
}

void ImageWidget::on_load(const std::int32_t sender, const std::int32_t view_info,
    const std::int32_t image_width, const char* waves)
{
    Window::window_->load_view(sender, view_info, waves, "image");
    int image_height = create_pixels(image_width);
    std::ostringstream info;
    info << image_width / 10 << " " << image_width << " " << image_height << " " << 0x02010003;
    interface::HandleAsync(Window::window_->sender_, "body", "ready", info.str().c_str());
}
