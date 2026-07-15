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

int main(int argc, char* argv[])
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
        return app->make_window_and_run<Window>(argc, argv);
    }
}
