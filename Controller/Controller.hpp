#pragma once

#include <SFML/Graphics.hpp>

#include "../Model/Model.hpp"
#include "../View/View.hpp"

class Controller {

    private:
    
    View& view;
    Model& model;
    sf::RenderWindow& window;

    public:

    Controller(sf::RenderWindow& window, View& view, Model& model);

    void updatePreview();

    // GUI CALLBACKS
    // These will be passsed to view after it is constructed

    void ButtonPressConvert();

    void ButtonPressChooseNegative();

    void ButtonPressSavePositive();

    void SliderChangeSetExposure(float value);

    // MAIN LOOP

    void mainLoop();
};