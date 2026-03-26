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
    void updateThumbnails();
    void updateThumbnail(std::unique_ptr<sf::Texture> thumbnailTexture, int id);

    // SETTERS
    void setSliderValue(std::string name, float value);
    void setSelection(ImageArea area);
    void setPreviewTexture(std::unique_ptr<sf::Texture> texture);

    // GETTERS
    Rml::Context* getRmlContext();

    // RENDERING
    void render();
};