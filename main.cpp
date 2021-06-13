//
//  main.cpp
//  cross
//
//  Created by Ali Asadpoor on 7/5/20.
//  Copyright © 2020 Shaidin. All rights reserved.
//

#include <cstring>
#include <iostream>

#include <gtkmm.h>

#include "window.h"

int main(int argc, const char* argv[])
{
    if (argc > 1)
    {
        if (argc == 2 && std::strcmp("--version", argv[1]) == 0)
        {
            std::cout << PROJECT_NAME << " " << PROJECT_VERSION << std::endl;
            return 0;
        }
        else
        {
            std::cerr << "Unknown option" << std::endl;
            return -1;
        }
    }
    else
    {
        auto app = Gtk::Application::create(APPLICATION_ID);
        Window window;
        window.maximize();
        window.set_title(PROJECT_NAME);
        return app->run(window);
    }
}
