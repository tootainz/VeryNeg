#include "View.hpp"

#include <print>
#include <format>

#include "../RmlUi_Backend/RmlUi_Backend.hpp"


// CONSTRUCTOR
// ----------------------------------------------------------------------------------------------------------------

View::View(sf::RenderWindow& window) :
    window(window),
    previewTexture(),
    previewSprite(this->previewTexture),
    rmlContext(Rml::CreateContext("default", Rml::Vector2i(this->window.getSize().x, this->window.getSize().y))),
    selection({0.0f, 0.0f}),
    thumbnails()
{
    bool success = Rml::LoadFontFace("assets/oceert_pixel.otf");
    Rml::ElementDocument* document = this->rmlContext->LoadDocument("assets/veryNegConvert.rml");
    this->rmlDocument = document;
    if (document) {
        document->Show();
    }
}


// THUMBNAIL MANAGEMENT
// ----------------------------------------------------------------------------------------------------------------

void View::addThumbnail(sf::Texture thumbnailTexture, int id) {
    Rml::Element* filmRoll = this->rmlDocument->GetElementById("filmRoll");
    Rml::ElementPtr thumbnailElement = this->rmlDocument->CreateElement("div");
    std::string name = std::format("thumbnail_{}", id);
    thumbnailElement->SetId(name);
    thumbnailElement->SetClass("thumbnail", true);
    filmRoll->AppendChild(std::move(thumbnailElement));
    std::println("added thumbnailElement");

    this->rmlContext->Update();

    Rml::Element* thumbnailElementPointer = this->rmlDocument->GetElementById(name);

    Thumbnail thumbnail({id, thumbnailTexture});
    std::println("created thumbnail struct");

    // little help from chatgpt in getting the correct size since i was lazy and need to get this done
    const Rml::Box& box = thumbnailElementPointer->GetBox();
    Rml::Vector2f pos = thumbnailElementPointer->GetAbsoluteOffset(Rml::BoxArea::Border);
    Rml::Vector2f size = box.GetSize(Rml::BoxArea::Border);
    std::println("got dimensions");

    thumbnail.sprite.setPosition(sf::Vector2f(pos.x, pos.y));
    thumbnail.sprite.setScale(sf::Vector2f(size.x / thumbnail.texture.getSize().x, size.y / thumbnail.texture.getSize().y));
    std::println("set size and pos");

    std::println("Texture size: {} x {}", thumbnail.texture.getSize().x, thumbnail.texture.getSize().y);
    std::println("Sprite position: {}, {}", pos.x, pos.y);

    this->thumbnails.push_back(thumbnail);
    std::println("added to the vector");
}

void View::LoadThumbnails() {
    for (Thumbnail& thumbnail : this->thumbnails) {
        thumbnail.loadTexture();
    }
}

void View::updateThumbnails() {
    for (Thumbnail& thumbnail : this->thumbnails) {
        std::string thumbnailName = std::format("thumbnail_{}", thumbnail.id);
        Rml::Element* thumbnailElement = this->rmlDocument->GetElementById(thumbnailName);

        const Rml::Box& box = thumbnailElement->GetBox();
        Rml::Vector2f pos = thumbnailElement->GetAbsoluteOffset(Rml::BoxArea::Border);
        Rml::Vector2f size = box.GetSize(Rml::BoxArea::Border);

        thumbnail.sprite.setPosition(sf::Vector2f(pos.x, pos.y));
        thumbnail.sprite.setScale(sf::Vector2f(size.x / thumbnail.texture.getSize().x, size.y / thumbnail.texture.getSize().y));
    }
}


// SETTERS
// ----------------------------------------------------------------------------------------------------------------

void View::setSliderValue(std::string name, float value) {
    // auto slider = gui.get<tgui::Slider>(name);
    // if (slider) {
    //     slider->setValue(value);
    // }
}

void View::setSelection(ImageArea area) {
    this->selection.setPosition({static_cast<float>(area.left), static_cast<float>(area.top)});
    float width = area.right - area.left;
    float height = area.bottom - area.top;
    this->selection.setSize({width, height});
}

void View::setPreviewTexture(sf::Texture texture) {
    this->previewTexture = texture;
    this->previewSprite = sf::Sprite(this->previewTexture);
}


// GETTERS
// ----------------------------------------------------------------------------------------------------------------

Rml::Context* View::getRmlContext() {
    return this->rmlContext;
}


// RENDERING
// ----------------------------------------------------------------------------------------------------------------

// Draw the complete view
void View::render() {

    this->selection.setFillColor(sf::Color::Transparent);
    this->selection.setOutlineThickness(1.0f);
    this->selection.setOutlineColor(sf::Color::White);
    this->updateThumbnails();

    this->window.clear();
    
    // Draw the non RmlUi stuff
    this->window.draw(previewSprite);
    if (this->displaySelection) {
        this->window.draw(this->selection);
    }

    // Thumbnails
    for (Thumbnail& thumbnail : this->thumbnails) {
        this->window.draw(thumbnail.sprite);
    }

    // Update and render the RmlUi
    RmlBackend::BeginFrame();
    this->rmlContext->Update();
    this->rmlContext->Render();
    RmlBackend::PresentFrame();
    this->window.display();
}