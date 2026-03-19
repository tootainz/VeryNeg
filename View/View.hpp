#pragma once

#include <SFML/Graphics.hpp>
#include <RmlUi/Core.h>


class View {

    private:

        sf::Texture previewTexture;
        sf::Sprite previewSprite;
        sf::RenderWindow& window;
        int lastThumbnailposition = 0;
        Rml::Context* rmlContext;
        Rml::ElementDocument* rmlDocument;

    public:

        View(sf::RenderWindow& window);

        void addThumbnail(sf::Texture thumbnailTexture, int id);

        void setSliderValue(std::string name, float value);

        void setPreviewTexture(sf::Texture texture);

        Rml::Context* getRmlContext();

        void render();
};