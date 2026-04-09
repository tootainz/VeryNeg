#pragma once

#include <SFML/Graphics.hpp>
#include <RmlUi/Core.h>

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
    float previewScale;
    int previewWidth = 800;
    int previewHeight = 800;
    float previewX = 0;
    float previewY = 0;
    float previewCenterOffsetX = 0;
    float previewCenterOffsetY = 0;

    // Sharpness preview
    std::unique_ptr<sf::Texture> sharpnessPreviewTexture;
    sf::Sprite sharpnessPreviewSprite;
    int sharpnessPreviewWidth = 200;
    int sharpnessPreviewHeight = 200;

    // Window
    sf::RenderWindow& window;
    
    // RmlUI stuff
    Rml::Context* rmlContext;
    Rml::ElementDocument* rmlDocument;

    // Selection
    sf::RectangleShape selection;

    // Thumbnails
    int lastThumbnailposition = 0;
    std::vector<std::unique_ptr<Thumbnail>> thumbnails;

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
    void updatePreviewSize();
    void updatePreviewScale();
    void updatePreviewPos();
    std::tuple<int, int> previewCoordsToTextureCoords(int x, int y);
    std::tuple<int, int> textureCoordsToPreviewCoords(int x, int y);
    
    // SETTERS
    void setSliderValue(std::string name, float value);
    void setCheckboxValue(std::string name, bool value);
    void setNumbericValue(std::string name, float value);
    void setPreset(std::string name, float value);
    void setSelection(ImageArea area);
    void setPreviewTexture(std::unique_ptr<sf::Texture> texture);
    void setSharpnessPreviewTexture(std::unique_ptr<sf::Texture> texture);

    // GETTERS
    Rml::Context* getRmlContext();
    ImageArea getPreviewArea();

    // RENDERING
    void render();
};