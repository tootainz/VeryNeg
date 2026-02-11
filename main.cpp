#include <iostream>
#include <filesystem>
#include <vector>

#include <OpenImageIO/imageio.h>
#include <SFML/Graphics.hpp>

#include "Negative/Negative.hpp"

std::unique_ptr<sf::Texture> createPreviewtexture(ImageData previewData) {
    std::cout << "creating texture" << std::endl;
    sf::Vector2u size(previewData.width, previewData.height);
    sf::Image previewImage(size, previewData.data.data());
    auto previewTexture = std::make_unique<sf::Texture>(previewImage);
    return previewTexture;
}

int main()
{

    // Model data
    auto testNegative = std::make_unique<Negative>("input.tif");

    testNegative->render();

    // Rendering data
    sf::RenderWindow window(sf::VideoMode({1000, 1000}), "Very Negative Image Editor");

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        std::unique_ptr<sf::Texture> previewTexture = createPreviewtexture(testNegative->getPreview());
        sf::Sprite previewSprite(*previewTexture);

        window.draw(previewSprite);

        window.display();
    }
}