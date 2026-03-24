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
    sf::Texture previewTexture;
    sf::Sprite previewSprite;

    // Window
    sf::RenderWindow& window;
    
    // RmlUI stuff
    Rml::Context* rmlContext;
    Rml::ElementDocument* rmlDocument;

    // Selection
    sf::RectangleShape selection;

    // Thumbnails
    int lastThumbnailposition = 0;
    std::vector<Thumbnail> thumbnails;

public:
    // PUBLIC METHODS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // CONSTRUCTOR
    View(sf::RenderWindow& window);

    // THUMBNAIL MANAGEMENT
    void addThumbnail(sf::Texture thumbnailTexture, int id);
    void LoadThumbnails();
    void updateThumbnails();

    // SETTERS
    void setSliderValue(std::string name, float value);
    void setSelection(ImageArea area);
    void setPreviewTexture(sf::Texture texture);

    // GETTERS
    Rml::Context* getRmlContext();

    // RENDERING
    void render();
};