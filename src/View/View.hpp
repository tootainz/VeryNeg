#pragma once

#include <SFML/Graphics.hpp>
#include <RmlUi/Core.h>
#include <SFML/OpenGL.hpp>

#include "../Negative/ImageArea.hpp"
#include "Thumbnail.hpp"


/**

The View Class

The View comes from the MVC architecture pattern.
It is responsible for defining how all of the rendering is done and where

*/

class View {

public:
    // PUBLIC DATA MEMBERS
    // ------------------------------------------------------------------------------------------------------------------------------------

    bool displaySelection = false;

private:
    // PRIVATE DATA MEMBERS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // Preview
    std::unique_ptr<sf::Texture> previewTexture;
    sf::Sprite previewSprite;
    int previewElementWidth = 800;
    int previewElementHeight = 800;
    int previewElementLeft = 0;
    int previewElementTop = 0;
    float previewOffsetX = 0.0f;
    float previewOffsetY = 0.0f;
    float previewX = 0.0f;
    float previewY = 0.0f;
    float previewScaleX;
    float previewScaleY;
    int previewOrientation = 1;

    // Cursors
    std::optional<sf::Cursor> cursorSampleBorder;
    std::optional<sf::Cursor> cursorSampleDensest;
    std::optional<sf::Cursor> cursorSampleNeutral;

    // Sharpness preview
    std::unique_ptr<sf::Texture> sharpeningPreviewTexture;
    sf::Sprite sharpeningPreviewSprite;
    int sharpeningPreviewWidth = 200;
    int sharpeningPreviewHeight = 200;
    ImageArea settingsRenderArea;

    // Window
    sf::RenderWindow& window;
    
    // RmlUI stuff
    Rml::Context* rmlContextUi;
    Rml::ElementDocument* rmlDocumentUi;
    Rml::Context* rmlContextPopups; // This is for rendering stuff that should be on top of sfml sprites
    Rml::ElementDocument* rmlDocumentPopups;

    // Selection
    sf::RectangleShape selection;

    // Thumbnails
    int lastThumbnailposition = 0;
    std::vector<std::unique_ptr<Thumbnail>> thumbnails;
    ImageArea filmRollRenderArea;

public:
    // PUBLIC METHODS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // CONSTRUCTOR
    View(sf::RenderWindow& window);

    // THUMBNAIL MANAGEMENT
    void addThumbnail(std::unique_ptr<sf::Texture> thumbnailTexture, int id);
    void removeThumbnail(int id);
    void LoadThumbnails();
    void updateThumbnailsPos();
    void updateThumbnail(std::unique_ptr<sf::Texture> thumbnailTexture, int id);

    // PREVIEW MANAGEMENT
    void updatePreviewElementSize();
    void updatePreviewSpriteTransform();
    void setPreviewOrientation(int value);
    std::tuple<int, int> previewCoordsToTextureCoords(int x, int y);
    std::tuple<int, int> textureCoordsToPreviewCoords(int x, int y);
    void updateFilmRollRenderArea();

    // SHARPENING MANAGEMENT
    void updateSharpeningPreviewPos();
    void updateSettingsRenderArea();
    
    // SETTERS
    void setCursorSampleDensest();
    void setCursorSampleBorder();
    void setCursorSampleNeutral();
    void setCursorDefault();
    void setSliderValue(std::string name, float value);
    void setCheckboxValue(std::string name, bool value);
    void setNumbericValue(std::string name, float value);
    void setPreset(std::string name, float value);
    void setSelection(ImageArea area);
    void setPreviewTexture(std::unique_ptr<sf::Texture> texture);
    void setSharpeningPreviewTexture(std::unique_ptr<sf::Texture> texture);
    void setPopUp(std::string name, bool value);

    // GETTERS
    Rml::Context* getRmlContextUi();
    Rml::Context* getRmlContextPopups();
    ImageArea getPreviewArea();

    // RENDERING
    void render();
};