#include <gtkmm.h>

#include "window.h"

int main(int argc, const char* argv[])
{
    auto app = Gtk::Application::create(APPLICATION_ID);
    Window window{ argv[0] };
    window.set_title(PROJECT_NAME);
    return app->run(window);
}
