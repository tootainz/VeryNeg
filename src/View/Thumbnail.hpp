#pragma once

#include <SFML/Graphics.hpp>

// A small helper struct storing data needed to render the thumbnails
struct Thumbnail {

    int id;
    sf::Texture texture;
    sf::Sprite sprite;

    Thumbnail(int id, sf::Texture texture) :
        id(id),
        texture(texture),
        // !! It seems that this might not load the texture
        sprite(this->texture)
    {}

    // Load the texture manually later
    void loadTexture() {
        this->sprite.setTexture(this->texture);
    }
};