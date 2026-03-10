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

    void ButtonPressChooseNegative();
    void ButtonPressSavePositive();
    
    void ButtonPressConvert();

    void SliderChangeSetExposure(float value);

    void SliderChangeSetBSlope(float value);
    void SliderChangeSetGSlope(float value);
    
    void SliderChangeSetRBalance(float value);
    void SliderChangeSetGBalance(float value);
    void SliderChangeSetBBalance(float value);

    void ButtonPressNextNegative();
    void ButtonPressPreviousNegative();
    
    // MAIN LOOP

    void mainLoop();
};