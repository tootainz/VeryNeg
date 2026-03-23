#pragma once

#include <SFML/Graphics.hpp>

#include "../Model/Model.hpp"
#include "../View/View.hpp"
#include "../Command/Command.hpp"
#include "../Command/CommandHistory.hpp"


class Controller : public Rml::EventListener {
    
private:

    // Data members
    View& view;
    Model& model;
    sf::RenderWindow& window;
    CommandHistory history;
    bool disableCallbacks = false;

    // UI interactions
    sf::Vector2i selectionStart;  
    bool selecting = false;
    bool readyToSelect = false;
    bool selectingCrop = false;
    bool selectingScanArea = false;
    bool selectingBorder = false;
    bool selectingDensest = false;
    bool selectingNeutral = false;

public:

    // Constructor
    Controller(sf::RenderWindow& window, View& view, Model& model);

    // Main loop
    void mainLoop();

private:

    // Update GUI
    void updatePreview();
    void updateEditSettings();

    // Undo & Redo
    void undo();
    void redo();

    // Gui events
    void ButtonPressAddNegative();
    void ButtonPressNextNegative();
    void ButtonPressPreviousNegative();
    void ButtonPressThumbnail(int id);
    
    void ButtonPressSetScanGamma(float value);
    void ButtonPressSetBorder();
    void SetBorder(int x, int y);
    void ButtonPressSetDensest();
    void SetDensest(int x, int y);
    void ButtonPressSetScanArea();
    void ButtonPressConvert();
    void ButtonPressResetConversion();

    void SliderChangeSetDensity(float value);
    void SliderChangeSetContrast(float value);
    void SliderChangeSetWhites(float value);
    void SliderChangeSetHighlights(float value);
    void SliderChangeSetShadows(float value);
    void SliderChangeSetBlacks(float value);

    void ButtonPressAutoWhiteBalance();
    void ButtonPressChooseNeutralBalance();
    void SetNeutralBalance(int x, int y);
    void SliderChangeSetRBalance(float value);
    void SliderChangeSetGBalance(float value);
    void SliderChangeSetBBalance(float value);

    void ButtonPressSavePositive();

    // Event listener
    void ProcessEvent(Rml::Event& event) override;
    
    // Main Loop helpers
    bool handleKeyboardEvents(std::optional<sf::Event> event);
    bool handleMouseEvents(std::optional<sf::Event> event);
    void eventLoop();
};