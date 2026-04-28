#pragma once

#include <SFML/Graphics.hpp>

// Container for storing the applications ui state for the controller
struct UiState {

    // Orientation & Crop
    bool heldOrientCrop = false;
    bool hasCrop = false;

    // Pre convert
    bool heldConvert = false;
    bool hasBorder = false;
    bool hasDensest = false;
    bool hasScanArea = false;
    bool hasScanBorder = false;

    // Selection
    sf::Vector2i selectionStart;  
    ImageArea selectionArea = {0,0,0,0};
    ImageArea oldSelectionArea = {0,0,0,0};
    bool readyToSelect = false;

    void resetSelectionArea() {
        this->selectionArea = ImageArea{0,0,0,0};
    }

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
    bool heldEdits = false;
    bool isDragging = false;
    bool autoWB = true;
    bool heldColor = false;
    bool heldIntensity = false;
    bool heldSharpening = false;

    // Helper for held settings
    bool somethingIsHolding() {
        return
            this->heldOrientCrop    ||
            this->heldConvert       ||
            this->heldEdits         ||
            this->heldColor         ||
            this->heldIntensity     ||
            this->heldSharpening;
    }

    // Export
    std::string exportFileFormat = "jpeg";
    std::string exportProfile = "sRGB";

    // General
    bool disableCallbacks = false;

    void resetGeneralSelectionState () {
        this->selecting = false;
        this->selectingLeft = false;
        this->selectingTop = false;
        this->selectingRight = false;
        this->selectingBottom = false;
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