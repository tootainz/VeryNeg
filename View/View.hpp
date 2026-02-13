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

    void addGuiWidgets() {
        // Open file button
        tgui::Button::Ptr openNegativeButton = tgui::Button::create("Open negative");
        openNegativeButton->setPosition(500, 400);
        openNegativeButton->setSize(70, 20);
        openNegativeButton->onPress(this->onButtonPress_LoadNegative);
        gui.add(openNegativeButton, "openNegativeButton");

        // Convert button
        tgui::Button::Ptr convertButton = tgui::Button::create("Convert");
        convertButton->setPosition(500, 200);
        convertButton->setSize(70, 20);
        convertButton->onPress(this->onButtonPress_Convert);
        gui.add(convertButton, "convertButton");
    }

    void handleEvent(sf::Event event){
        gui.handleEvent(event);
    }

    void setPreviewTexture(sf::Texture texture) {
        this->previewTexture = texture;
        this->previewSprite = sf::Sprite(this->previewTexture);
    }

    void draw() {
        std::println("drawing view");
        this->window.draw(previewSprite);
        gui.draw();
    };
};