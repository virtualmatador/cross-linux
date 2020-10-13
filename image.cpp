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
    dispatcher_.connect(sigc::mem_fun(*this, &ImageWidget::on_load_image_view));
    signal_draw().connect([this](const Cairo::RefPtr<Cairo::Context>& cr)
    {
        auto allocation = get_allocation();
        cr->scale((double)allocation.get_width() / (double)pixels_->get_width(), (double)allocation.get_height() / (double)pixels_->get_height());
        Gdk::Cairo::set_source_pixbuf(cr, pixels_);
        cr->paint();
        return true;
    });
    signal_configure_event().connect([this](GdkEventConfigure* event)
    {
        image_view_width_ = event->width;
        image_view_height_ = event->height;
        image_resized_ = true;
        return true;
    });
    show();
}

ImageWidget::~ImageWidget()
{
    
}

__uint32_t* ImageWidget::get_pixels()
{
    return (__uint32_t*) pixels_->get_pixels();
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

void ImageWidget::on_load_image_view()
{
    Window::window_->dispatch_lock_.lock();
    auto dispatch_info = dispatch_queue_.front();
    dispatch_queue_.pop();
    Window::window_->dispatch_lock_.unlock();
    load_image_view(dispatch_info.sender, dispatch_info.view_info, dispatch_info.image_width, dispatch_info.waves);
}

int ImageWidget::create_pixels(int image_width)
{
    float scale = (float)image_width / (float)image_view_width_;
    int image_height = (__int32_t)(image_view_height_ * scale);
    guint8* rgb_data = new guint8[4 * image_width * image_height];
    pixels_ = Gdk::Pixbuf::create_from_data(rgb_data, Gdk::Colorspace::COLORSPACE_RGB, true, 8, image_width, image_height, 4 * image_width,
    [rgb_data](const guint8*)
    {
        delete[] rgb_data;
    });
    image_resized_ = false;
    return image_height;
}

void ImageWidget::load_image_view(const __int32_t sender, const __int32_t view_info,
    const __int32_t image_width, const char* waves)
{
    Window::window_->container_.set_visible_child("image");
    int image_height = create_pixels(image_width);
    Window::window_->load_view(sender, view_info, waves);
    std::ostringstream info;
    info << image_width / 10 << " " << image_width << " " << image_height << " " << 0x02010003;
    interface::HandleAsync(Window::window_->sender_, "body", "ready", info.str().c_str());
}
