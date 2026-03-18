#pragma once

#include <print>
#include <format>

#include <SFML/Graphics.hpp>
#include <RmlUi/Core.h>


class View {

    private:

    sf::Texture previewTexture;
    sf::Sprite previewSprite;
    sf::RenderWindow& window;
    int lastThumbnailposition = 0;
    Rml::Context* rmlContext;

    public:

    View(sf::RenderWindow& window) :
        window(window),
        previewTexture(),
        previewSprite(this->previewTexture),
        rmlContext(Rml::CreateContext("default", Rml::Vector2i(this->window.getSize().x, this->window.getSize().y)))
    {
        bool success = Rml::LoadFontFace("assets/oceert_pixel.otf");
        Rml::ElementDocument* document = this->rmlContext->LoadDocument("assets/hello.rml");
        if (document)
            document->Show();
    }

    std::function<void()> onButtonPress_Convert;
    std::function<void()> onButtonPress_LoadNegative;
    std::function<void()> onButtonPress_SavePositive;
    std::function<void()> onButtonPress_NextNegative;
    std::function<void()> onButtonPress_PreviousNegative;
    std::function<void(int)> onButtonPress_Thumbnail;

    std::function<void(float)> onSliderChange_SetExposure;
    std::function<void(float)> onSliderChange_SetRBalance;
    std::function<void(float)> onSliderChange_SetGBalance;
    std::function<void(float)> onSliderChange_SetBBalance;

    void addThumbnail(sf::Texture thumbnailTexture, int id) {
        // auto thumbnail = tgui::Picture::create(thumbnailTexture);
        // //thumbnail->getRenderer()->setTexture("image2.png"); // To change image after construction
        // //thumbnail->onClick(this->onButtonPress_Thumbnail);
        // thumbnail->setSize(50,50);
        // thumbnail->setPosition(100+60*this->lastThumbnailposition, 800);
        // this->lastThumbnailposition += 1;
        // std::string name = std::format("thumbnail_{}", id);
        // thumbnail->setUserData(id);
        // thumbnail->onMouseEnter([thumbnail]{
        //     thumbnail->getRenderer()->setOpacity(0.7f);
        // });
        // thumbnail->onMouseLeave([thumbnail]{
        //     thumbnail->getRenderer()->setOpacity(1.0f);
        // });
        // gui.add(thumbnail, name);

        // thumbnail->onClick([this, thumbnail] {
        //     int id = thumbnail->getUserData<int>();
        //     this->onButtonPress_Thumbnail(id);
        // });
    }

    void setSliderValue(std::string name, float value) {
        // auto slider = gui.get<tgui::Slider>(name);
        // if (slider) {
        //     slider->setValue(value);
        // }
    }

    void setPreviewTexture(sf::Texture texture) {
        this->previewTexture = texture;
        this->previewSprite = sf::Sprite(this->previewTexture);
    }

    Rml::Context* getRmlContext() {
        return this->rmlContext;
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
        this->window.draw(line.data(), line.size(), sf::PrimitiveType::Lines);
        this->window.draw(rectangle);

        this->rmlContext->Update();
        this->rmlContext->Render();
    };
};