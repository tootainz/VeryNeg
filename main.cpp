#include <iostream>
#include <filesystem>
#include <vector>

#include <OpenImageIO/imageio.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include "rmlui-backend/RmlUi_Backend.h"
#include <RmlUi/Core.h>
#include <RmlUi_Platform_SFML.h>
#include <RmlUi_Renderer_GL2.h>

#include "Negative/Negative.hpp"
#include "View/View.hpp"
#include "Controller/Controller.hpp"
#include "Model/Model.hpp"

int main() {

    // SETUP

    // SFML window
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 0; // 0 = no anti-aliasing, higher values = more smoothing
    sf::RenderWindow window(sf::VideoMode({1000, 1000}), "Very Negative Image Editor", sf::State::Windowed, settings);

    // RmlUi
    RmlBackend::Initialize(window, true);
    Rml::SetRenderInterface(RmlBackend::GetRenderInterface());
    Rml::SetSystemInterface(RmlBackend::GetSystemInterface());
    if (!Rml::Initialise()) {
        std::cout << "RmlUi failed to initialise\n";
    }

    // Application logic
    Model model;
    View view(window);
    Controller controller(window, view, model);

    // MAIN LOOP
    controller.mainLoop();

    // CLEANUP
    Rml::Shutdown();
}