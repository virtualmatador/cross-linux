//
//  main.cpp
//  cross
//
//  Created by Ali Asadpoor on 7/5/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#include <gtkmm.h>

#include "window.h"

int main()
{
    auto app = Gtk::Application::create(APPLICATION_ID);
    Window window;
    window.set_size_request(800, 600);
    window.set_title(PROJECT_NAME);
    return app->run(window);
}
