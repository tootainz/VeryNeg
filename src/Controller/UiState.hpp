#pragma once

#include <SFML/Graphics.hpp>

// Container for storing the applications ui state for the controller
struct UiState {

    // Pre convert
    bool autoWB = true;
    bool autocConvert = true;
    bool usingScanArea = false;
    bool usingDensest = false;
    bool usingBorder = false;

    // Selection
    sf::Vector2i selectionStart;  
    bool selecting = false;
    bool readyToSelect = false;
    bool selectingCrop = false;
    bool selectingScanArea = false;
    bool selectingBorder = false;
    bool selectingDensest = false;
    bool selectingNeutral = false;

    // Post-convert
    bool isDragging = false;

    // General
    bool disableCallbacks = false;
};