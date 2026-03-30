#pragma once

#include <SFML/Graphics.hpp>

// Container for storing the applications ui state for the controller
struct UiState {

    // Pre convert
    bool usingBorder = false;
    bool usingDensest = false;
    bool usingScanArea = false;
    bool usingScanBorder = false;

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
    bool autoWB = true;
    bool neutralSample = false;

    // General
    bool disableCallbacks = false;
};