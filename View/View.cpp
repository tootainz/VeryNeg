#include "View.hpp"

#include <print>
#include <format>

#include "../rmlui-backend/RmlUi_Backend.h"


View::View(sf::RenderWindow& window) :
    window(window),
    previewTexture(),
    previewSprite(this->previewTexture),
    rmlContext(Rml::CreateContext("default", Rml::Vector2i(this->window.getSize().x, this->window.getSize().y)))
{
    bool success = Rml::LoadFontFace("assets/oceert_pixel.otf");
    Rml::ElementDocument* document = this->rmlContext->LoadDocument("assets/hello.rml");
    this->rmlDocument = document;
    if (document) {
        document->Show();
    }
}

void View::addThumbnail(sf::Texture thumbnailTexture, int id) {
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

void View::setSliderValue(std::string name, float value) {
    // auto slider = gui.get<tgui::Slider>(name);
    // if (slider) {
    //     slider->setValue(value);
    // }
}

void View::setPreviewTexture(sf::Texture texture) {
    this->previewTexture = texture;
    this->previewSprite = sf::Sprite(this->previewTexture);
}

Rml::Context* View::getRmlContext() {
    return this->rmlContext;
}

// Draw the complete view
void View::render() {
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

    this->window.clear();
    
    // Draw the non RmlUi stuff
    this->window.draw(previewSprite);
    this->window.draw(line.data(), line.size(), sf::PrimitiveType::Lines);
    this->window.draw(rectangle);

    // Update and render the RmlUi
    RmlBackend::BeginFrame();
    this->rmlContext->Update();
    this->rmlContext->Render();
    RmlBackend::PresentFrame();
    this->window.display();
}