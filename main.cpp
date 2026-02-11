#include <iostream>
#include <filesystem>
#include <vector>

#include <OpenImageIO/imageio.h>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

#include "Negative/Negative.hpp"
#include "libraries/portable-file-dialogs.h"

// SFML specific helper function for transforming the preview ImageData from Negative to a texture for SFML
std::shared_ptr<sf::Texture> createPreviewtexture(ImageData previewData) {
    std::cout << "creating texture" << std::endl;
    sf::Vector2u size(previewData.width, previewData.height);
    sf::Image previewImage(size, previewData.data.data());
    auto previewTexture = std::make_shared<sf::Texture>(previewImage);
    return previewTexture;
}

int main()
{
    // SETUP

    // Model data
    // Create a test Negative object
    auto testNegative = std::make_unique<Negative>("input.tif");
    // get the initial preview of the image contained in the test Negative
    std::shared_ptr<sf::Texture> previewTexture = createPreviewtexture(testNegative->getPreview());
    sf::Sprite previewSprite(*previewTexture);

    // Setup renderer and gui
    sf::RenderWindow window(sf::VideoMode({1000, 1000}), "Very Negative Image Editor");
    tgui::Gui gui{window};

    // GUI

    // Open file button
    tgui::Button::Ptr openNegativeButton = tgui::Button::create("Open negative");
    openNegativeButton->setPosition(500, 400);
    openNegativeButton->setSize(70, 20);
    openNegativeButton->onPress([&]{
        pfd::open_file fileOpener("Choose negative", "/");
        std::vector<std::string> paths = fileOpener.result();
        if (paths.size() == 0) {
            std::cout << "didn't choose a file" << std::endl;
            return;
        } else {
            std::string path = paths[0];
            std::cout << "Trying to open negative at: " << std::endl << path << std::endl;
            testNegative->initializeNegative(path);
        }
        previewTexture = createPreviewtexture(testNegative->getPreview());
        previewSprite.setTexture(*previewTexture);
    });
    gui.add(openNegativeButton, "openNegativeButton");

    // Convert button
    tgui::Button::Ptr convertButton = tgui::Button::create("Convert");
    convertButton->setPosition(500, 200);
    convertButton->setSize(70, 20);
    convertButton->onPress([&]{
        testNegative->render();
        previewTexture = createPreviewtexture(testNegative->getPreview());
        previewSprite.setTexture(*previewTexture);
    });
    gui.add(convertButton, "convertButton");

    // MAIN LOOP

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            gui.handleEvent(*event);

            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear();

        // Draw the preview
        window.draw(previewSprite);
        // Draw the gui
        gui.draw();

        window.display();
    }
}