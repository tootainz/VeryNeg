#pragma once

#include <print>

#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

class View {

    private:

    tgui::Gui gui;
    sf::Texture previewTexture;
    sf::Sprite previewSprite;
    sf::RenderWindow& window;

    public:

    View(sf::RenderWindow& window) :
        window(window),
        gui(window),
        previewTexture(),
        previewSprite(this->previewTexture)
    {}

    std::function<void()> onButtonPress_Convert;
    std::function<void()> onButtonPress_LoadNegative;
    std::function<void()> onButtonPress_SavePositive;
    std::function<void(float)> onSliderChange_SetExposure;
    std::function<void(float)> onSliderChange_SetBSlope;
    std::function<void(float)> onSliderChange_SetGSlope;
    std::function<void(float)> onSliderChange_SetRBalance;
    std::function<void(float)> onSliderChange_SetGBalance;
    std::function<void(float)> onSliderChange_SetBBalance;

    void addGuiWidgets() {
        // Open negative button
        tgui::Button::Ptr openNegativeButton = tgui::Button::create("Open negative");
        openNegativeButton->setPosition(500, 100);
        openNegativeButton->setSize(120, 20);
        openNegativeButton->onPress(this->onButtonPress_LoadNegative);
        gui.add(openNegativeButton, "openNegativeButton");

        // Save negative button
        tgui::Button::Ptr savePositiveButton = tgui::Button::create("Save positive");
        savePositiveButton->setPosition(500, 150);
        savePositiveButton->setSize(120, 20);
        gui.add(savePositiveButton, "savePositiveButton");
        
        savePositiveButton->onPress(this->onButtonPress_SavePositive);

        // Convert button
        tgui::Button::Ptr convertButton = tgui::Button::create("Convert");
        convertButton->setPosition(500, 200);
        convertButton->setSize(120, 20);
        gui.add(convertButton, "convertButton");
        
        convertButton->onPress(this->onButtonPress_Convert);

        // Exposure slider
        tgui::Slider::Ptr exposureSlider = tgui::Slider::create();
        exposureSlider->setPosition(500, 300);
        exposureSlider->setSize(140, 20);
        exposureSlider->setMinimum(-3.0f);
        exposureSlider->setMaximum(3.0f);
        exposureSlider->setStep(0.01f);
        exposureSlider->setValue(0.0f);
        gui.add(exposureSlider, "exposureSlider");

        exposureSlider->onValueChange(this->onSliderChange_SetExposure);

        // Blue slope
        tgui::Slider::Ptr bSlopeSlider = tgui::Slider::create();
        bSlopeSlider->setPosition(500, 400);
        bSlopeSlider->setSize(300, 20);
        bSlopeSlider->setMinimum(-2.0f);
        bSlopeSlider->setMaximum(2.0f);
        bSlopeSlider->setStep(0.1f);
        bSlopeSlider->setValue(1.0f);
        gui.add(bSlopeSlider, "bSlopeSlider");

        bSlopeSlider->onValueChange(this->onSliderChange_SetBSlope);

        // Green slope
        tgui::Slider::Ptr gSlopeSlider = tgui::Slider::create();
        gSlopeSlider->setPosition(500, 500);
        gSlopeSlider->setSize(300, 20);
        gSlopeSlider->setMinimum(-2.0f);
        gSlopeSlider->setMaximum(2.0f);
        gSlopeSlider->setStep(0.1f);
        gSlopeSlider->setValue(1.0f);
        gui.add(gSlopeSlider, "gSlopeSlider");

        gSlopeSlider->onValueChange(this->onSliderChange_SetGSlope);

        // R balance
        tgui::Slider::Ptr rBalanceSlider = tgui::Slider::create();
        rBalanceSlider->setPosition(500, 600);
        rBalanceSlider->setSize(300, 20);
        rBalanceSlider->setMinimum(0.0f);
        rBalanceSlider->setMaximum(2.0f);
        rBalanceSlider->setStep(0.01f);
        rBalanceSlider->setValue(1.0f);
        gui.add(rBalanceSlider, "cBalanceSlider");

        rBalanceSlider->onValueChange(this->onSliderChange_SetRBalance);

        // G balance
        tgui::Slider::Ptr gBalanceSlider = tgui::Slider::create();
        gBalanceSlider->setPosition(500, 650);
        gBalanceSlider->setSize(300, 20);
        gBalanceSlider->setMinimum(0.0f);
        gBalanceSlider->setMaximum(2.0f);
        gBalanceSlider->setStep(0.01f);
        gBalanceSlider->setValue(1.0f);
        gui.add(gBalanceSlider, "gBalanceSlider");

        gBalanceSlider->onValueChange(this->onSliderChange_SetGBalance);

        // B balance
        tgui::Slider::Ptr bBalanceSlider = tgui::Slider::create();
        bBalanceSlider->setPosition(500, 700);
        bBalanceSlider->setSize(300, 20);
        bBalanceSlider->setMinimum(0.0f);
        bBalanceSlider->setMaximum(2.0f);
        bBalanceSlider->setStep(0.01f);
        bBalanceSlider->setValue(1.0f);
        gui.add(bBalanceSlider, "bBalanceSlider");

        bBalanceSlider->onValueChange(this->onSliderChange_SetBBalance);
    }

    void handleEvent(sf::Event event){
        gui.handleEvent(event);
    }

    void setPreviewTexture(sf::Texture texture) {
        this->previewTexture = texture;
        this->previewSprite = sf::Sprite(this->previewTexture);
    }

    // Draw the complete view
    void draw() {
        // std::println("drawing view");

        this->window.draw(previewSprite);
        gui.draw();
    };
};