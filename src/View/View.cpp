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
    sharpeningPreviewTexture(std::make_unique<sf::Texture>()),
    sharpeningPreviewSprite(*this->sharpeningPreviewTexture),
    rmlContextUi(Rml::CreateContext("ui", Rml::Vector2i(this->window.getSize().x, this->window.getSize().y))),
    rmlContextPopups(Rml::CreateContext("popups", Rml::Vector2i(this->window.getSize().x, this->window.getSize().y))),
    selection({0.0f, 0.0f}),
    thumbnails()
{
    Rml::LoadFontFace("./resources/fonts/oceert_smooth.otf");
    Rml::ElementDocument* uiDocument = this->rmlContextUi->LoadDocument("./resources/ui/veryNegUi.rml");
    Rml::ElementDocument* popupsDocument = this->rmlContextUi->LoadDocument("./resources/ui/veryNegPopups.rml");
    this->rmlDocumentUi = uiDocument;
    this->rmlDocumentPopups = popupsDocument;
    if (this->rmlDocumentUi) {
        this->rmlDocumentUi->Show();
    }
    this->updatePreviewSize();
    this->updatePreviewScale();
    this->updatePreviewPos();

    this->updateFilmRollRenderArea();
    this->updateSettingsRenderArea();

    this->selection.setFillColor(sf::Color::Transparent);
    this->selection.setOutlineThickness(1.0f);
    this->selection.setOutlineColor(sf::Color::White);
}


// THUMBNAIL MANAGEMENT
// ----------------------------------------------------------------------------------------------------------------

void View::addThumbnail(std::unique_ptr<sf::Texture> thumbnailTexture, int id) {
    Rml::Element* filmRoll = this->rmlDocumentUi->GetElementById("filmRoll");
    Rml::ElementPtr thumbnailElement = this->rmlDocumentUi->CreateElement("div");
    std::string name = std::format("thumbnail_{}", id);
    thumbnailElement->SetId(name);
    thumbnailElement->SetClass("thumbnail", true);
    filmRoll->AppendChild(std::move(thumbnailElement));
    std::println("added thumbnailElement");

    this->rmlContextUi->Update();

    Rml::Element* thumbnailElementPointer = this->rmlDocumentUi->GetElementById(name);

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
    Rml::Element* element = this->rmlDocumentUi->GetElementById(name);

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
        Rml::Element* thumbnailElement = this->rmlDocumentUi->GetElementById(thumbnailName);

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
    Rml::Element* previewElement = this->rmlDocumentUi->GetElementById("preview");
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
    Rml::Element* previewElement = this->rmlDocumentUi->GetElementById("preview");

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

// I didn't have time to think how to unmap the orientation so this is by ChatGPT
void View::setPreviewOrientation(int value) {
    this->previewOrientation = value;

    // Reset everything first
    this->previewSprite.setRotation(sf::degrees(0));
    this->previewSprite.setScale({this->previewScale, this->previewScale});

    sf::Vector2u size = this->previewTexture->getSize();

    // Default origin (top-left)
    float originX = 0.0f;
    float originY = 0.0f;

    switch (value) {
        case 1: // normal
            break;

        case 2: // flip horizontal
            this->previewSprite.setScale({-this->previewScale, this->previewScale});
            originX = size.x;
            break;

        case 3: // rotate 180
            this->previewSprite.setRotation(sf::degrees(180));
            originX = size.x;
            originY = size.y;
            break;

        case 4: // flip vertical
            this->previewSprite.setScale({this->previewScale, -this->previewScale});
            originY = size.y;
            break;

        case 5: // transpose (flip + rotate 90)
            this->previewSprite.setRotation(sf::degrees(90));
            this->previewSprite.setScale({-this->previewScale, this->previewScale});
            originX = size.x;
            break;

        case 6: // rotate 90 CW
            this->previewSprite.setRotation(sf::degrees(90));
            originY = size.y;
            break;

        case 7: // transverse
            this->previewSprite.setRotation(sf::degrees(270));
            this->previewSprite.setScale({-this->previewScale, this->previewScale});
            originX = size.x;
            break;

        case 8: // rotate 270 CW
            this->previewSprite.setRotation(sf::degrees(270));
            originX = size.x;
            break;
    }

    this->previewSprite.setOrigin({originX, originY});

    // Recalculate position because bounds changed after rotation
    this->updatePreviewPos();
    this->updatePreviewSize();
    this->updatePreviewScale();
}

// This was originally by me but i didn't have time to think how to unmap the orientation so this version is by ChatGPT
std::tuple<int, int> View::previewCoordsToTextureCoords(int x, int y) {
    // Remove preview offset + scale
    float px = (x - this->previewX) / this->previewScale;
    float py = (y - this->previewY) / this->previewScale;

    float tx = px;
    float ty = py;

    sf::Vector2u size = this->previewTexture->getSize();
    float w = size.x;
    float h = size.y;

    // Apply inverse orientation transform
    switch (this->previewOrientation) {
        case 1:
            break;

        case 2: // flip horizontal
            tx = w - px;
            ty = py;
            break;

        case 3: // rotate 180
            tx = w - px;
            ty = h - py;
            break;

        case 4: // flip vertical
            tx = px;
            ty = h - py;
            break;

        case 5: // transpose
            tx = py;
            ty = px;
            break;

        case 6: // rotate 90 CW
            tx = py;
            ty = h - px;
            break;

        case 7: // transverse
            tx = h - py;
            ty = w - px;
            break;

        case 8: // rotate 270 CW
            tx = w - py;
            ty = px;
            break;
    }

    return {static_cast<int>(tx), static_cast<int>(ty)};
}

// This was originally by me but i didn't have time to think how to unmap the orientation so this version is by ChatGPT
std::tuple<int, int> View::textureCoordsToPreviewCoords(int x, int y) {
    float tx = x;
    float ty = y;

    float px = tx;
    float py = ty;

    sf::Vector2u size = this->previewTexture->getSize();
    float w = size.x;
    float h = size.y;

    // Apply forward orientation transform
    switch (this->previewOrientation) {
        case 1:
            break;

        case 2: // flip horizontal
            px = w - tx;
            py = ty;
            break;

        case 3: // rotate 180
            px = w - tx;
            py = h - ty;
            break;

        case 4: // flip vertical
            px = tx;
            py = h - ty;
            break;

        case 5: // transpose
            px = ty;
            py = tx;
            break;

        case 6: // rotate 90 CW
            px = h - ty;
            py = tx;
            break;

        case 7: // transverse
            px = h - ty;
            py = w - tx;
            break;

        case 8: // rotate 270 CW
            px = ty;
            py = w - tx;
            break;
    }

    // Apply scale + offset
    float previewX = px * this->previewScale + this->previewX;
    float previewY = py * this->previewScale + this->previewY;

    return {static_cast<int>(previewX), static_cast<int>(previewY)};
}

void View::updateFilmRollRenderArea() {
    Rml::Element* filmRollElement = this->rmlDocumentUi->GetElementById("filmRoll");

    int width = filmRollElement->GetOffsetWidth();
    int height = filmRollElement->GetOffsetHeight();

    int left = filmRollElement->GetAbsoluteLeft();
    int top = filmRollElement->GetAbsoluteTop();

    this->filmRollRenderArea = {left, top, left+width, top+height};
}

// SHARPENING MANAGEMENT
// ----------------------------------------------------------------------------------------------------------------

void View::updateSharpeningPreviewPos() {
    Rml::Element* previewElement = this->rmlDocumentUi->GetElementById("sharpeningPreview");
    float x = previewElement->GetAbsoluteLeft();
    float y = previewElement->GetAbsoluteTop();
    this->sharpeningPreviewSprite.setPosition({x, y});
}

void View::updateSettingsRenderArea() {
    Rml::Element* settingsRenderArea = this->rmlDocumentUi->GetElementById("settings");

    int width = settingsRenderArea->GetOffsetWidth();
    int height = settingsRenderArea->GetOffsetHeight();

    int left = settingsRenderArea->GetAbsoluteLeft();
    int top = settingsRenderArea->GetAbsoluteTop();

    this->settingsRenderArea= {left, top, left+width, top+height};
}

// SETTERS
// ----------------------------------------------------------------------------------------------------------------

void View::setSliderValue(std::string name, float value) {
    Rml::Element* slider = this->rmlDocumentUi->GetElementById(name);
    if (slider) {
        slider->SetAttribute("value", value);
    }
}

void View::setCheckboxValue(std::string name, bool value) {
    std::println("settign checkbox value with name {} to value {}", name, value);
    Rml::Element* checkbox = this->rmlDocumentUi->GetElementById(name);
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
    this->setPreviewOrientation(this->previewOrientation);
}

void View::setSharpeningPreviewTexture(std::unique_ptr<sf::Texture> texture) {
    this->sharpeningPreviewTexture = std::move(texture);
    this->sharpeningPreviewSprite = sf::Sprite(*this->sharpeningPreviewTexture);
}

void View::setPopUp(std::string name, bool value) {
    if (this->rmlDocumentPopups) {
        Rml::Element* popUpElement = this->rmlDocumentPopups->GetElementById(name);
        if (popUpElement) {
            if (value) {
                this->rmlDocumentPopups->Show();
                std::println("popup shown");
                popUpElement->SetClass("hidden", false);
            }
            else {
                this->rmlDocumentPopups->Hide();
                std::println("popup hidden");
                popUpElement->SetClass("hidden", true);
            }
        }
    }
}

// GETTERS
// ----------------------------------------------------------------------------------------------------------------

Rml::Context* View::getRmlContextUi() {
    return this->rmlContextUi;
}

Rml::Context* View::getRmlContextPopups() {
    return this->rmlContextPopups;
}

ImageArea View::getPreviewArea() {
    Rml::Element* preview = this->rmlDocumentUi->GetElementById("preview");
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
    this->updateSharpeningPreviewPos();
    this->rmlContextUi->Update();
    this->rmlContextPopups->Update();

    // -------------------------------------------------------
    // RENDERING
    this->window.clear();
    
    // Clears all OpenGlstates as well
	window.resetGLStates();

    // Render the RmlUi main ui
    RmlBackend::BeginFrame();
    this->rmlContextUi->Render();

    // Draw the non RmlUi stuff

    // Needed for mixing SFML and RmlUi rendering apparently, i have no idea what it does
    window.pushGLStates();

    // Preview
    this->window.draw(previewSprite);

    // Sharpness preview
    glEnable(GL_SCISSOR_TEST);    // This is some weird opengl stuff that i dont have experience with for maskign parts of the sprite
    glScissor(
        this->settingsRenderArea.left,
        this->window.getSize().y - this->settingsRenderArea.bottom,
        this->settingsRenderArea.right - this->settingsRenderArea.left,
        this->settingsRenderArea.bottom - this->settingsRenderArea.top
    );

    // Draw the actual sprite
    this->window.draw(sharpeningPreviewSprite);

    glDisable(GL_SCISSOR_TEST);

    // Selection
    if (this->displaySelection) {
        this->window.draw(this->selection);
    }

    // Thumbnails
    glEnable(GL_SCISSOR_TEST);    // This is some weird opengl stuff that i dont have experience with for maskign parts of the sprite
    glScissor(
        this->filmRollRenderArea.left,
        this->window.getSize().y - this->filmRollRenderArea.bottom,
        this->filmRollRenderArea.right - this->filmRollRenderArea.left,
        this->filmRollRenderArea.bottom - this->filmRollRenderArea.top
    );
    // Draw the actual sprites
    for (const auto& thumbnail : this->thumbnails) {
        this->window.draw(thumbnail->sprite);
    }
    glDisable(GL_SCISSOR_TEST);

    // Needed for mixing SFML and RmlUi rendering apparently, i have no idea what it does
    window.popGLStates();

    // Render the RmlUi popups ui
    this->rmlContextPopups->Render();
    RmlBackend::PresentFrame();

    this->window.display();
    // -------------------------------------------------------
}