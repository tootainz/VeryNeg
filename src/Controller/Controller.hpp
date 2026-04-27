#pragma once

#include <SFML/Graphics.hpp>

#include "../Model/Model.hpp"
#include "../View/View.hpp"
#include "../Command/Command.hpp"
#include "../Command/CommandHistory.hpp"
#include "UiState.hpp"


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
    CommandHistory history; // Controller owns the history

    // UI state
    UiState uiState;

public:
    // PUBLIC METHODS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // CONSTRUCTOR
    Controller(sf::RenderWindow& window, View& view, Model& model);

    // MAIN LOOP
    void mainLoop();
    void cleanup();


private:
    // PRIVATE METHODS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // UPDATE GUI
    void updatePreview(bool dragging);
    void updateSharpnessPreview();
    void updateEditSettings(Negative& negative);

    // UNDO REDO
    void undo();
    void redo();

    // GUI EVENTS

    // Apply held settings
    void applyHeld();

    // Reset sliders
    void SliderReset(std::string name);

    // NEGATIVE NAVIGATION
    void ButtonPressAddNegative();
    void ButtonPressRemoveNegative(int id);
    void ButtonPressNextNegative();
    void ButtonPressPreviousNegative();
    void ButtonPressThumbnail(int id);

    // ORIENTATION
    void CheckboxPressHoldOrientation(bool checked);
    void ButtonPressResetOrientation();
    void ButtonPressRotateClock();
    void ButtonPressRotateCounterClock();
    void ButtonPressFlipHorizontal();
    void ButtonPressFlipVertical();

    // CROP
    void ButtonPressSetCrop();
    
    // PRE-CONVERT
    void CheckboxPressHoldConvert(bool checked);
    void OptionPressSetScanGamma(float value);
    void CheckboxPressBorder(bool checked);
    void ButtonPressSetBorder();
    void SetBorder(int x, int y);
    void CheckboxPressDensest(bool checked);
    void ButtonPressSetDensest();
    void SetDensest(int x, int y);
    void CheckboxPressScanArea(bool checked);
    void ButtonPressSetScanArea();
    void ButtonPressSetScanAreaNumber();
    void ButtonPressConvert();
    void ButtonPressResetConversion();

    // POST-CONVERT
    void CheckboxPressHoldEdits(bool checked);
    void ButtonPressResetEdits();
    void OptionPressSetPreset(std::string name);

    // Intensity
    void CheckboxPressHoldIntensity(bool checked);
    void ButtonPressResetIntensity();
    void SliderChangeSetDensity(float value, bool dragging);
    void SliderChangeSetContrast(float value);
    void SliderChangeSetWhites(float value);
    void SliderChangeSetHighlights(float value);
    void SliderChangeSetShadows(float value);
    void SliderChangeSetBlacks(float value);

    // Color
    void CheckboxPressHoldColor(bool checked);
    void ButtonPressResetColor();
    void CheckboxPressAutoWhiteBalance(bool checked);
    void ButtonPressSetNeutralSample();
    void SetNeutralSample(int x, int y);
    void SliderChangeSetRBalance(float value);
    void SliderChangeSetGBalance(float value);
    void SliderChangeSetBBalance(float value);
    void SliderChangeSetSaturation(float value);

    // Sharpening
    void CheckboxPressHoldSharpening(bool checked);
    void ButtonPressResetSharpening();
    void SliderChangeSetSharpeningAmount(float value);
    void SliderChangeSetSharpeningDiameter(float value);

    // EXPORTING
    void ButtonPressExport();
    void ButtonPressExportCurrent();
    void ButtonPressExportAll();
    void ButtonPressExportCancel();
    void OptionPressImageFormat(std::string name);
    void OptionPressExportProfile(std::string name);

    // EVENT LISTENER
    void ProcessEvent(Rml::Event& event) override;
    
    // EVENT LOOP
    bool handleKeyboardEvents(std::optional<sf::Event> event);
    bool handleMouseEvents(std::optional<sf::Event> event);
    void eventLoop();
};