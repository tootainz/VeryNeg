#include <iostream>
#include <filesystem>
#include <vector>

#include <OpenImageIO/imageio.h>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

#include "Negative/Negative.hpp"
#include "View/View.hpp"
#include "Controller/Controller.hpp"
#include "Model/Model.hpp"

int main() {
    // SETUP
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 0; // 0 = no anti-aliasing, higher values = more smoothing
    sf::RenderWindow window(sf::VideoMode({1000, 1000}), "Very Negative Image Editor", sf::State::Windowed, settings);
    Model model;
    View view(window);
    Controller controller(window, view, model);

    // MAIN LOOP
    controller.mainLoop();
}