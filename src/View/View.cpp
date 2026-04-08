#include "View.hpp"

#include <print>
#include <format>

#include "../RmlUi_Backend/RmlUi_Backend.hpp"


// CONSTRUCTOR
// ----------------------------------------------------------------------------------------------------------------

View::View(sf::RenderWindow& window) :
    window(window),
    previewTexture(std::make_unique<sf::Texture>()),
    previewSprite(*this->previewTexture),
    rmlContext(Rml::CreateContext("default", Rml::Vector2i(this->window.getSize().x, this->window.getSize().y))),
    selection({0.0f, 0.0f}),
    thumbnails()
{
    std::string fontPath = std::format("./resources/fonts/oceert_smooth.otf");
    Rml::LoadFontFace(fontPath);

    std::string documentPath = std::format("./resources/ui/veryNegConvert.rml");
    Rml::ElementDocument* document = this->rmlContext->LoadDocument(documentPath);
    this->rmlDocument = document;
    if (document) {
        document->Show();
    }

    this->updatePreviewSize();
    this->updatePreviewScale();
    this->updatePreviewPos();

    this->selection.setFillColor(sf::Color::Transparent);
    this->selection.setOutlineThickness(1.0f);
    this->selection.setOutlineColor(sf::Color::White);
}


// THUMBNAIL MANAGEMENT
// ----------------------------------------------------------------------------------------------------------------

void View::addThumbnail(std::unique_ptr<sf::Texture> thumbnailTexture, int id) {
    Rml::Element* filmRoll = this->rmlDocument->GetElementById("filmRoll");
    Rml::ElementPtr thumbnailElement = this->rmlDocument->CreateElement("div");
    std::string name = std::format("thumbnail_{}", id);
    thumbnailElement->SetId(name);
    thumbnailElement->SetClass("thumbnail", true);
    filmRoll->AppendChild(std::move(thumbnailElement));
    std::println("added thumbnailElement");

    this->rmlContext->Update();

    Rml::Element* thumbnailElementPointer = this->rmlDocument->GetElementById(name);

    auto thumbnail = std::make_unique<Thumbnail>(id, std::move(thumbnailTexture));
    std::println("created thumbnail struct");

    // little help from chatgpt in getting the correct size since i was lazy and need to get this done
    const Rml::Box& box = thumbnailElementPointer->GetBox();
    Rml::Vector2f pos = thumbnailElementPointer->GetAbsoluteOffset(Rml::BoxArea::Border);
    Rml::Vector2f size = box.GetSize(Rml::BoxArea::Border);
    std::println("got dimensions");

    thumbnail->sprite.setPosition(sf::Vector2f(pos.x, pos.y));
    thumbnail->sprite.setScale(sf::Vector2f(size.x / thumbnail->texture->getSize().x, size.y / thumbnail->texture->getSize().y));
    std::println("set size and pos");

    std::println("Texture size: {} x {}", thumbnail->texture->getSize().x, thumbnail->texture->getSize().y);
    std::println("Sprite position: {}, {}", pos.x, pos.y);

    this->thumbnails.push_back(std::move(thumbnail));
    std::println("added to the vector");
}

// Written by AI since I was lazy
void View::removeThumbnail(int id) {
    auto iterator = std::find_if(
        this->thumbnails.begin(),
        this->thumbnails.end(),
        [&id](const std::unique_ptr<Thumbnail>& thumbnailPtr) {
            return thumbnailPtr->id == id;
        }
    );

    if (iterator != this->thumbnails.end()) {
        this->thumbnails.erase(iterator);
    }

    // Also remove from the RML document
    std::string name = std::format("thumbnail_{}", id);
    Rml::Element* element = this->rmlDocument->GetElementById(name);

    if (element && element->GetParentNode()) {
        element->GetParentNode()->RemoveChild(element);
    }
}

void View::LoadThumbnails() {
    for (const auto& thumbnail : this->thumbnails) {
        thumbnail->loadTexture();
    }
}

void View::updateThumbnailsPos() {
    for (const auto& thumbnail : this->thumbnails) {
        std::string thumbnailName = std::format("thumbnail_{}", thumbnail->id);
        Rml::Element* thumbnailElement = this->rmlDocument->GetElementById(thumbnailName);

        const Rml::Box& box = thumbnailElement->GetBox();
        Rml::Vector2f pos = thumbnailElement->GetAbsoluteOffset(Rml::BoxArea::Border);
        Rml::Vector2f size = box.GetSize(Rml::BoxArea::Border);

        thumbnail->sprite.setPosition(sf::Vector2f(pos.x, pos.y));
        thumbnail->sprite.setScale(sf::Vector2f(size.x / thumbnail->texture->getSize().x, size.y / thumbnail->texture->getSize().y));
    }
}

void View::updateThumbnail(std::unique_ptr<sf::Texture> thumbnailTexture, int id) {
    auto iterator = std::find_if(
        this->thumbnails.begin(),
        this->thumbnails.end(),
        [&id](const std::unique_ptr<Thumbnail>& thumbnailPtr) {
            return thumbnailPtr->id == id;
        }
    );

    if (iterator != this->thumbnails.end()) {
        std::println("Updating thumbnail id: {}", (*iterator)->id);
        (*iterator)->updateTexture(std::move(thumbnailTexture));
    }
}


// PREVIEW MANAGEMENT
// ----------------------------------------------------------------------------------------------------------------

void View::updatePreviewSize() {
    Rml::Element* previewElement = this->rmlDocument->GetElementById("preview");
    this->previewWidth = previewElement->GetOffsetWidth();
    this->previewHeight = previewElement->GetOffsetHeight();
    std::println("preveiw size is {} * {}", this->previewWidth, this->previewHeight);
}

void View::updatePreviewScale() {
    sf::Vector2u textureSize = this->previewTexture->getSize();

    // Calculate scale factors to fit the target size
    float scaleX = this->previewWidth / (1.0f * textureSize.x);
    float scaleY = this->previewHeight / (1.0f * textureSize.y);
    this->previewScale = std::min(scaleX, scaleY);

    std::println("sprite scales are x:{} y: {}", scaleX, scaleY);

    // Apply the scale to the sprite
    this->previewSprite.setScale({this->previewScale, this->previewScale});
}

void View::updatePreviewPos() {
    Rml::Element* previewElement = this->rmlDocument->GetElementById("preview");

    float spriteWidth = this->previewSprite.getGlobalBounds().size.x;
    float spriteHeight = this->previewSprite.getGlobalBounds().size.y;
    
    std::println("spritewidth: {}, previewWidth: {}", spriteWidth, this->previewWidth);

    // if sprite width < preview width then center horizontal
    if (spriteWidth < this->previewWidth) {
        std::println("center preview horizontal");
        this->previewCenterOffsetX = (this->previewWidth-spriteWidth)/2.0f;
        this->previewCenterOffsetY = 0.0f;
    }
    // center vertical
    else {
        std::println("center preview vertical");
        this->previewCenterOffsetX = 0.0f;
        this->previewCenterOffsetY = (this->previewHeight-spriteHeight)/2.0f;
    }

    this->previewX = previewElement->GetAbsoluteLeft()+this->previewCenterOffsetX;
    this->previewY = previewElement->GetAbsoluteTop()+this->previewCenterOffsetY;
    std::println("the preview will be drawn on {},{}", this->previewX, this->previewY);
}

std::tuple<int, int> View::previewCoordsToTextureCoords(int x, int y) {
    // First remove the preview offset, then remove the preview scale
    // This should give us the original coords
    float textureX = (x-this->previewX)*(1.0f/this->previewScale);
    float textureY = (y-this->previewY)*(1.0f/this->previewScale);
    std::println("the original coords are {},{}", textureX, textureY);
    return std::tuple<int, int>(textureX, textureY);
}

std::tuple<int, int> View::textureCoordsToPreviewCoords(int x, int y) {
    // First scale by preview scale, then add the preview offset
    // This should give us the preview coords
    float previewX = x*this->previewScale+this->previewX;
    float previewY = y*this->previewScale+this->previewY;
    std::println("the preview coords are {},{}", previewX, previewY);
    return std::tuple<int, int>(previewX, previewY);
}

// SETTERS
// ----------------------------------------------------------------------------------------------------------------

void View::setSliderValue(std::string name, float value) {
    Rml::Element* slider = this->rmlDocument->GetElementById(name);
    if (slider) {
        slider->SetAttribute("value", value);
    }
}

void View::setCheckboxValue(std::string name, bool value) {
    std::println("settign checkbox value with name {} to value {}", name, value);
    Rml::Element* checkbox = this->rmlDocument->GetElementById(name);
    if (checkbox) {
        if (!value) {
            checkbox->RemoveAttribute("checked");
        }
        else {
            checkbox->SetAttribute("checked", true);
        }
    }
}

void View::setSelection(ImageArea area) {
    auto correctedTopLeft = this->textureCoordsToPreviewCoords(area.left, area.top);
    auto correctedBottomRight = this->textureCoordsToPreviewCoords(area.right, area.bottom);
    int left = std::get<0>(correctedTopLeft);
    int top = std::get<1>(correctedTopLeft);
    int right = std::get<0>(correctedBottomRight);
    int bottom = std::get<1>(correctedBottomRight);
    this->selection.setPosition({static_cast<float>(left), static_cast<float>(top)});
    float width = right - left;
    float height = bottom - top;
    this->selection.setSize({width, height});
}

void View::setPreviewTexture(std::unique_ptr<sf::Texture> texture) {
    this->previewTexture = std::move(texture);
    this->previewSprite = sf::Sprite(*this->previewTexture);
    this->previewSprite.setPosition({this->previewX, this->previewY});
    this->updatePreviewScale();
}


// GETTERS
// ----------------------------------------------------------------------------------------------------------------

Rml::Context* View::getRmlContext() {
    return this->rmlContext;
}

ImageArea View::getPreviewArea() {
    Rml::Element* preview = this->rmlDocument->GetElementById("preview");
    if (preview) {
        int left = preview->GetAbsoluteLeft();
        int top = preview->GetAbsoluteTop();
        int right = left+preview->GetOffsetWidth();
        int bottom = left+preview->GetOffsetHeight();
        return {left, top, right, bottom};
    }
    return ImageArea{0,0,0,0};
}

// RENDERING
// ----------------------------------------------------------------------------------------------------------------

// Draw the complete view
void View::render() {
    // -------------------------------------------------------
    // UPDATING

    this->updateThumbnailsPos();

    // -------------------------------------------------------
    this->window.clear();
    // RENDERING

    // Update and render the RmlUi
    RmlBackend::BeginFrame();
    this->rmlContext->Update();
    this->rmlContext->Render();
    RmlBackend::PresentFrame();

    // Draw the non RmlUi stuff
    
    // Preview
    this->window.draw(previewSprite);

    // Selection
    if (this->displaySelection) {
        this->window.draw(this->selection);
    }

    // Thumbnails
    for (const auto& thumbnail : this->thumbnails) {
        this->window.draw(thumbnail->sprite);
    }

    this->window.display();
    // -------------------------------------------------------
}