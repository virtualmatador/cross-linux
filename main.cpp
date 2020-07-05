#include <gtkmm.h>

#include "window.h"

int main()
{
    auto app = Gtk::Application::create(PROJECT_NAME);
    Window window;
    return app->run(window);
}
