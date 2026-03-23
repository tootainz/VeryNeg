#pragma once

#include <SFML/Graphics.hpp>
#include <RmlUi/Core.h>

#include "../Negative/ImageArea.hpp"


struct Thumbnail {
    int id;
    sf::Texture texture;
    sf::Sprite sprite;

    Thumbnail(int id, sf::Texture texture) :
        id(id),
        texture(texture),
        sprite(this->texture)
    {}

    void loadTexture() {
        this->sprite.setTexture(this->texture);
    }
};

class View {

public:

    bool displaySelection = false;

private:

    sf::Texture previewTexture;
    sf::Sprite previewSprite;
    sf::RenderWindow& window;
    int lastThumbnailposition = 0;
    Rml::Context* rmlContext;
    Rml::ElementDocument* rmlDocument;
    sf::RectangleShape selection;
    std::vector<Thumbnail> thumbnails;

public:

    View(sf::RenderWindow& window);

    void addThumbnail(sf::Texture thumbnailTexture, int id);

    void LoadThumbnails();

    void updateThumbnails();

    void setSliderValue(std::string name, float value);

    void setSelection(ImageArea area);

    void setPreviewTexture(sf::Texture texture);

    Rml::Context* getRmlContext();

    void render();
};