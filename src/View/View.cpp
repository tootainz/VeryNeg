#include "View.hpp"

#include <print>
#include <format>

#include "../RmlUi_Backend/RmlUi_Backend.hpp"
#include "../getResourcesPath.hpp"


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
    Rml::LoadFontFace(getResourcesPath("fonts/oceert_smooth.otf"));
    Rml::ElementDocument* uiDocument = this->rmlContextUi->LoadDocument(getResourcesPath("ui/veryNegUi.rml"));
    Rml::ElementDocument* popupsDocument = this->rmlContextUi->LoadDocument(getResourcesPath("ui/veryNegPopups.rml"));
    this->rmlDocumentUi = uiDocument;
    this->rmlDocumentPopups = popupsDocument;
    if (this->rmlDocumentUi) {
        this->rmlDocumentUi->Show();
    }
    this->updatePreviewElementSize();
    this->updatePreviewSpriteTransform();

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

void View::updatePreviewElementSize() {
    Rml::Element* previewElement = this->rmlDocumentUi->GetElementById("preview");
    this->previewElementWidth = previewElement->GetOffsetWidth();
    this->previewElementHeight = previewElement->GetOffsetHeight();

    this->previewElementTop = previewElement->GetAbsoluteTop();
    this->previewElementLeft = previewElement->GetAbsoluteLeft();
}

void View::updatePreviewSpriteTransform() {

    sf::Sprite& sprite = previewSprite;

    // sprite.setTextureRect({{10,10},{200,200}});

    const float spriteWidth = sprite.getLocalBounds().size.x;
    const float spriteHeight = sprite.getLocalBounds().size.y;

    std::println("original sprite w {}, h {}", spriteWidth, spriteHeight);

    // 0. Reset everything
    // ---------------------------------------------------------
    sprite.setRotation(sf::degrees(0.0f));
    sprite.setScale({1.0f, 1.0f});
    sprite.setPosition({0.0f, 0.0f});

    // 1. Set origin to center
    // ---------------------------------------------------------
    sprite.setOrigin({spriteWidth/2.0f, spriteHeight/2.0f});

    std::println("set sprite origin {}, h {}", spriteWidth/2.0f, spriteHeight/2.0f);

    // 2. Apply rotation to the sprite
    // ---------------------------------------------------------
    switch (this->previewOrientation) {
    // These come from what exif orientation data means
    case 1: // normal
        break;
    case 3: // rotate 180
        this->previewSprite.setRotation(sf::degrees(180.0f));
        break;
    case 5: // transpose (flip + rotate 90)
        this->previewSprite.setRotation(sf::degrees(90.0f));
        // the flipping is done when setting the scale later
        break;
    case 6: // rotate 90 CW
        this->previewSprite.setRotation(sf::degrees(90.0f));
        break;
    case 7: // transverse
        this->previewSprite.setRotation(sf::degrees(270.0f));
        // the flipping is done when setting the scale later
        break;
    case 8: // rotate 270 CW
        this->previewSprite.setRotation(sf::degrees(270.0f));
        break;
    }
    
    // 3. Calculate preview scale to fit preview element
    // ---------------------------------------------------------

    // Get new sprite dimensions after the rotation
    const float newSpriteWidth = sprite.getGlobalBounds().size.x;
    const float newSpriteHeight = sprite.getGlobalBounds().size.y;

    std::println("new after rotation sprite w {}, h {}", newSpriteWidth, newSpriteHeight);

    // Calculate scales for both axis adn choose the smaller one
    float scaleX = this->previewElementWidth / (1.0f * newSpriteWidth);
    float scaleY = this->previewElementHeight / (1.0f * newSpriteHeight);
    this->previewScaleX = this->previewScaleY = std::min(scaleX, scaleY);

    // Update scale from mirroring info from orientation
    switch (this->previewOrientation) {
    case 2: // flip horizontal
        this->previewScaleX = -this->previewScaleX;
        break;
    case 4: // flip vertical
        this->previewScaleY = -this->previewScaleY;
        break;
    case 5: // transpose (flip + rotate 90)
        this->previewScaleX = -this->previewScaleX;
        break;
    case 7: // transverse
        this->previewScaleX = -this->previewScaleX;
        break;
    }

    // Apply the scale to the sprite by taking into consideration the mirroring from orientation
    this->previewSprite.setScale({this->previewScaleX, this->previewScaleY});

    std::println("sprite scale is w {}, h {}", this->previewScaleX, this->previewScaleY);

    // 4. Update previewSprite Position
    // ---------------------------------------------------------

    // Get new sprite dimensions after the scaling
    const float finalSpriteWidth = sprite.getGlobalBounds().size.x;
    const float finalSpriteHeight = sprite.getGlobalBounds().size.y;

    std::println("final sprite w {}, h {}", finalSpriteWidth, finalSpriteHeight);

    // if sprite width < preview width then center horizontal
    if (finalSpriteWidth < this->previewElementWidth) {
        this->previewOffsetX = (this->previewElementWidth-finalSpriteWidth)/2.0f;
        this->previewOffsetY = 0.0f;
    }
    // center vertical
    else {
        this->previewOffsetX = 0.0f;
        this->previewOffsetY = (this->previewElementHeight-finalSpriteHeight)/2.0f;
    }

    std::println("sprite offsets are w {}, h {}", this->previewOffsetX, this->previewOffsetY);

    this->previewX = this->previewElementLeft + this->previewOffsetX + finalSpriteWidth/2.0f;
    this->previewY = this->previewElementTop + this->previewOffsetY + finalSpriteHeight/2.0f;
    sprite.setPosition({this->previewX, this->previewY});

    std::println("sprite location is w {}, h {}", this->previewX, this->previewY);
}

void View::setPreviewOrientation(int value) {
    this->previewOrientation = value;
    this->updatePreviewSpriteTransform();
}

// Written by ChatGPT based on my coordinate transforms with the preview above
std::tuple<int, int> View::previewCoordsToTextureCoords(int x, int y) {
    sf::Sprite& sprite = previewSprite;

    // 1. Convert from preview space into sprite local space
    float localX = x - this->previewX;
    float localY = y - this->previewY;

    // 2. Undo scale (handle flip safely)
    float sx = this->previewScaleX;
    float sy = this->previewScaleY;

    if (sx == 0.f || sy == 0.f)
        return {0, 0};

    localX /= sx;
    localY /= sy;

    // 3. Undo rotation (center-based)
    sf::Angle rot = sprite.getRotation();
    float deg = rot.asDegrees();

    float x2, y2;

    if (deg == 0.f)
    {
        x2 = localX;
        y2 = localY;
    }
    else if (deg == 180.f)
    {
        x2 = -localX;
        y2 = -localY;
    }
    else if (deg == 90.f)
    {
        x2 = localY;
        y2 = -localX;
    }
    else // 270
    {
        x2 = -localY;
        y2 = localX;
    }

    // 4. Convert from centered sprite space → texture space
    const auto bounds = sprite.getLocalBounds();
    float halfW = bounds.size.x / 2.0f;
    float halfH = bounds.size.y / 2.0f;

    int texX = static_cast<int>(x2 + halfW);
    int texY = static_cast<int>(y2 + halfH);

    return {texX, texY};
}

// Written by ChatGPT based on my coordinate transforms with the preview above
std::tuple<int, int> View::textureCoordsToPreviewCoords(int x, int y) {
    sf::Sprite& sprite = previewSprite;

    const auto bounds = sprite.getLocalBounds();
    float halfW = bounds.size.x / 2.0f;
    float halfH = bounds.size.y / 2.0f;

    // 1. texture → centered sprite space
    float cx = x - halfW;
    float cy = y - halfH;

    // 2. apply rotation
    sf::Angle rot = sprite.getRotation();
    float deg = rot.asDegrees();

    float lx, ly;

    if (deg == 0.f)
    {
        lx = cx;
        ly = cy;
    }
    else if (deg == 180.f)
    {
        lx = -cx;
        ly = -cy;
    }
    else if (deg == 90.f)
    {
        lx = -cy;
        ly = cx;
    }
    else // 270
    {
        lx = cy;
        ly = -cx;
    }

    // 3. apply scale (including flip)
    lx *= this->previewScaleX;
    ly *= this->previewScaleY;

    // 4. convert to preview space
    int px = static_cast<int>(lx + this->previewX);
    int py = static_cast<int>(ly + this->previewY);

    return {px, py};
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
    this->updatePreviewSpriteTransform();
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