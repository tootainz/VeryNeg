#pragma once

#include <SFML/Graphics.hpp>

#include "../Model/Model.hpp"
#include "../View/View.hpp"


class Controller {
    
    private:

    // Data members
    View& view;
    Model& model;
    sf::RenderWindow& window;

    public:

    // Constructor
    Controller(sf::RenderWindow& window, View& view, Model& model);

    // Update GUI preview
    void updatePreview();

    // Gui callbacks
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
    
    // Main loop
    void mainLoop();
};