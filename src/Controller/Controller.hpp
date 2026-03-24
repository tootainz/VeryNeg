#pragma once

#include <SFML/Graphics.hpp>

#include "../Model/Model.hpp"
#include "../View/View.hpp"
#include "../Command/Command.hpp"
#include "../Command/CommandHistory.hpp"


/**

The Controller Class

Comes from the MVC architecture pattern.
The Controller handles a lot of the application that enables it to run.
It bridges the view and the model, contains the main loop, handles events,
handles commands, sets the data in both the view and model.
A lot of the borign but important stuff happens here

Derives from Rml::EventListener in order to be able to handle the events of RmlUi along the other events
Not sure if this is the best approach but its the easiest for now and it works.
Have to mix raw SFML events with RmlUi to implement the preview and cropping and to render the thumbnails.

*/

class Controller : public Rml::EventListener {
    
private:
    // PRIVATE DATA MEMBERS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // References to objects that controller controls
    View& view;
    Model& model;
    sf::RenderWindow& window;
    CommandHistory history; // Controlelr owns the history

    // UI state
    sf::Vector2i selectionStart;  
    bool selecting = false;
    bool readyToSelect = false;
    bool selectingCrop = false;
    bool selectingScanArea = false;
    bool selectingBorder = false;
    bool selectingDensest = false;
    bool selectingNeutral = false;
    bool disableCallbacks = false;
    

public:
    // PUBLIC METHODS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // CONSTRUCTOR
    Controller(sf::RenderWindow& window, View& view, Model& model);

    // MAIN LOOP
    void mainLoop();


private:
    // PRIVATE METHODS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // UPDATE GUI
    void updatePreview();
    void updateEditSettings(Negative* negative);

    // UNDO REDO
    void undo();
    void redo();

    // GUI EVENTS

    // NEGATIVE NAVIGATION
    void ButtonPressAddNegative();
    void ButtonPressRemoveNegative(int id);
    void ButtonPressNextNegative();
    void ButtonPressPreviousNegative();
    void ButtonPressThumbnail(int id);
    
    // PRE-CONVERT
    void ButtonPressSetScanGamma(float value);
    void ButtonPressSetBorder();
    void SetBorder(int x, int y);
    void ButtonPressSetDensest();
    void SetDensest(int x, int y);
    void ButtonPressSetScanArea();
    void ButtonPressConvert();
    void ButtonPressResetConversion();

    // POST-CONVERT

    // Intensity
    void SliderChangeSetDensity(float value);
    void SliderChangeSetContrast(float value);
    void SliderChangeSetWhites(float value);
    void SliderChangeSetHighlights(float value);
    void SliderChangeSetShadows(float value);
    void SliderChangeSetBlacks(float value);

    // White balance
    void ButtonPressAutoWhiteBalance();
    void ButtonPressChooseNeutralBalance();
    void SetNeutralBalance(int x, int y);
    void SliderChangeSetRBalance(float value);
    void SliderChangeSetGBalance(float value);
    void SliderChangeSetBBalance(float value);

    // EXPORTING
    void ButtonPressSavePositive();

    // EVENT LISTENER
    void ProcessEvent(Rml::Event& event) override;
    
    // EVENT LOOP
    bool handleKeyboardEvents(std::optional<sf::Event> event);
    bool handleMouseEvents(std::optional<sf::Event> event);
    void eventLoop();
};