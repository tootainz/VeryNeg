#pragma once

#include <print>

#include <SFML/Graphics.hpp>
#include "../libraries/portable-file-dialogs.h"

#include "../Model/Model.hpp"
#include "../View/View.hpp"

// SFML specific helper function for transforming the preview ImageData from Negative to a texture for SFML
sf::Texture createPreviewtexture(ImageData previewData) {
    std::cout << "creating texture" << std::endl;
    sf::Vector2u size(previewData.width, previewData.height);
    sf::Image previewImage(size, previewData.data.data());
    auto previewTexture = sf::Texture(previewImage);
    return previewTexture;
}

class Controller {

    private:
    
    View& view;
    Model& model;
    sf::RenderWindow& window;

    public:

    Controller(sf::RenderWindow& window, View& view, Model& model) :
        view(view),
        model(model),
        window(window)
    {
        this->updatePreview();
        view.onButtonPress_Convert = [this]() { this->ButtonPressConvert(); };
        view.onButtonPress_LoadNegative = [this]() { this->ButtonPressChooseNegative(); };
        view.addGuiWidgets();
    }

    void updatePreview() {
        sf::Texture previewTexture = createPreviewtexture(model.getPreview());
        view.setPreviewTexture(previewTexture);
    }

    // GUI CALLBACKS
    // These will be passsed to view after it is constructed

    void ButtonPressConvert() {
        std::println("button pressed convert");
        model.render();
        this->updatePreview();
    }

    void ButtonPressChooseNegative() {
        std::println("button pressed choose negative");
        pfd::open_file fileOpener("Choose negative", "/");
        std::vector<std::string> paths = fileOpener.result();
        if (paths.size() == 0) {
            std::cout << "didn't choose a file" << std::endl;
            return;
        } else {
            std::string path = paths[0];
            std::cout << "Trying to open negative at: " << std::endl << path << std::endl;
            model.initializeNegative(path);
        }
        this->updatePreview();
    }

    // MAIN LOOP

    void mainLoop() {
        while (this->window.isOpen())
            {
                while (const std::optional event = this->window.pollEvent())
                {
                    this->view.handleEvent(*event);

                    if (event->is<sf::Event::Closed>())
                        this->window.close();
                }

                this->window.clear();

                // Draw the view
                this->view.draw();

                this->window.display();
            }
    }
};