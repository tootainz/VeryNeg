#pragma once

#include <SFML/Graphics.hpp>

#include "../Model/Model.hpp"
#include "../View/View.hpp"
#include "../Command/Command.hpp"
#include "../Command/CommandHistory.hpp"


class Controller {
    
    private:

    // Data members
    View& view;
    Model& model;
    sf::RenderWindow& window;
    CommandHistory history;
    bool disableCallbacks = false; 

    public:

    // Constructor
    Controller(sf::RenderWindow& window, View& view, Model& model);

    // Update GUI
    void updatePreview();
    void updateEditSettings();

    // Undo & Redo
    void undo();
    void redo();

    // Gui callbacks
    // These will be passsed to view after it is constructed

    void ButtonPressAddNegative();
    void ButtonPressSavePositive();
    
    void ButtonPressConvert();

    void SliderChangeSetExposure(float value);
    
    void SliderChangeSetRBalance(float value);
    void SliderChangeSetGBalance(float value);
    void SliderChangeSetBBalance(float value);

    void ButtonPressNextNegative();
    void ButtonPressPreviousNegative();
    void ButtonPressThumbnail(int id);
    
    // Main loop
    void mainLoop();
};