#include <gtkmm.h>

#include "window.h"

int main(int argc, const char* argv[])
{
    auto app = Gtk::Application::create(PROJECT_NAME);
    Window window{ argv[0] };
    return app->run(window);
}
