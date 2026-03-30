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
    ImageArea selectionArea = {0,0,0,0};
    bool readyToSelect = false;

    bool selecting = false;
    bool selectingLeft = false;
    bool selectingTop = false;
    bool selectingRight = false;
    bool selectingBottom = false;
    bool selectingWhole = false;

    // Specific Selection
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

    void resetGeneralSelectionState () {
        this->selecting = false;
        this->selectingLeft = false;
        this->selectingTop = false;
        this->selectingRight = false;
        this->selectingBottom = false;
        this->selectingCrop = false;
        this->selectingWhole = false;
    }

    void resetAllSelectionState () {
        this->readyToSelect = false;
        this->selectingCrop = false;
        this->selectingScanArea = false;
        this->selectingBorder = false;
        this->selectingDensest = false;
        this->selectingNeutral = false;
        this->resetGeneralSelectionState();
    }
};