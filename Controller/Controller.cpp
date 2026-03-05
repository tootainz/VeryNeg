#include "Controller.hpp"

#include <print>

#include "../libraries/portable-file-dialogs.h"


// SFML specific helper function for transforming the preview ImageData from Negative to a texture for SFML
static sf::Texture createPreviewtexture(ImageData previewData) {
    std::cout << "creating texture" << std::endl;
    sf::Vector2u size(previewData.width, previewData.height);
    sf::Image previewImage(size, previewData.data.data());
    auto previewTexture = sf::Texture(previewImage, false);
    return previewTexture;
}

Controller::Controller(sf::RenderWindow& window, View& view, Model& model) :
    view(view),
    model(model),
    window(window)
{
    this->updatePreview();
    view.onButtonPress_Convert = [this]() { this->ButtonPressConvert(); };
    view.onButtonPress_LoadNegative = [this]() { this->ButtonPressChooseNegative(); };
    view.onButtonPress_SavePositive = [this]() { this->ButtonPressSavePositive(); };
    view.onSliderChange_SetExposure = [this](float value) { this->SliderChangeSetExposure(value); };
    view.onSliderChange_SetBSlope = [this](float value) { this->SliderChangeSetBSlope(value); };
    view.onSliderChange_SetGSlope = [this](float value) { this->SliderChangeSetGSlope(value); };
    view.addGuiWidgets();
}

void Controller::updatePreview() {
    sf::Texture previewTexture = createPreviewtexture(this->model.getPreview());
    this->view.setPreviewTexture(previewTexture);
}

void Controller::ButtonPressConvert() {
    std::println("button pressed convert");
    this->model.render();
    this->updatePreview();
}

void Controller::SliderChangeSetBSlope(float value) {
    std::println("B slope slider value was changed to {}", value);
    this->model.setBSlope(value);
}

void Controller::SliderChangeSetGSlope(float value) {
    std::println("G slope slider value was changed to {}", value);
    this->model.setGSlope(value);
}

void Controller::SliderChangeSetExposure(float value) {
    std::println("Slider value was changed to {}", value);
    this->model.setExposure(value);
    this->model.renderEdits();
    this->updatePreview();
}

void Controller::ButtonPressChooseNegative() {
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

void Controller::ButtonPressSavePositive() {
    std::println("button pressed save positive");
    pfd::save_file fileSaver("Choose positive location", "/");
    std::string path = fileSaver.result();
    std::println("save path is {}", path);
    this->model.savePositive(path);
}

void Controller::mainLoop() {
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