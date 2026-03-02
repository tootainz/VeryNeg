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
        exposureSlider->setMinimum(-20.0f);
        exposureSlider->setMaximum(20.0f);
        exposureSlider->setStep(2.0f);
        exposureSlider->setValue(0.0f);
        gui.add(exposureSlider, "exposureSlider");

        exposureSlider->onValueChange(this->onSliderChange_SetExposure);
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