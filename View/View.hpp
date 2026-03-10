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
        exposureSlider->setPosition(500, 500);
        exposureSlider->setSize(300, 20);
        exposureSlider->setMinimum(-3.0f);
        exposureSlider->setMaximum(3.0f);
        exposureSlider->setStep(0.001f);
        exposureSlider->setValue(0.0f);
        gui.add(exposureSlider, "exposureSlider");

        exposureSlider->onValueChange(this->onSliderChange_SetExposure);

        tgui::Label::Ptr labelExposure = tgui::Label::create();
        labelExposure->setText("Density");
        labelExposure->setPosition(500, 500-20);
        labelExposure->setTextSize(14);
        labelExposure->getRenderer()->setTextColor(sf::Color::White);
        gui.add(labelExposure);

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

        tgui::Label::Ptr labelRBalance = tgui::Label::create();
        labelRBalance->setText("red-cyan balance");
        labelRBalance->setPosition(500, 600-20);
        labelRBalance->setTextSize(14);
        labelRBalance->getRenderer()->setTextColor(sf::Color::White);
        gui.add(labelRBalance);

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

        tgui::Label::Ptr labelGBalance = tgui::Label::create();
        labelGBalance->setText("green-magenta balance");
        labelGBalance->setPosition(500, 650-20);
        labelGBalance->setTextSize(14);
        labelGBalance->getRenderer()->setTextColor(sf::Color::White);
        gui.add(labelGBalance);

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

        tgui::Label::Ptr labelBBalance = tgui::Label::create();
        labelBBalance->setText("blue-yellow balance");
        labelBBalance->setPosition(500, 700-20);
        labelBBalance->setTextSize(14);
        labelBBalance->getRenderer()->setTextColor(sf::Color::White);
        gui.add(labelBBalance);
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


        std::array line =
        {
            sf::Vertex{sf::Vector2f(10.f, 10.f)},
            sf::Vertex{sf::Vector2f(150.f, 150.f)}
        };

        // define a 120x50 rectangle
        sf::RectangleShape rectangle({120.f, 50.f});

        // change the size to 100x100
        rectangle.setSize({100.f, 100.f});
        rectangle.setFillColor(sf::Color::Transparent);
        rectangle.setOutlineThickness(1.0f);
        rectangle.setOutlineColor(sf::Color::White);

        this->window.draw(previewSprite);
        gui.draw();
        this->window.draw(line.data(), line.size(), sf::PrimitiveType::Lines);
        this->window.draw(rectangle);
    };
};