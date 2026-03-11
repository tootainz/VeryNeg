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
    view.onButtonPress_Convert = [this]() { this->ButtonPressConvert(); };
    view.onButtonPress_LoadNegative = [this]() { this->ButtonPressChooseNegative(); };
    view.onButtonPress_SavePositive = [this]() { this->ButtonPressSavePositive(); };
    view.onButtonPress_NextNegative = [this]() { this->ButtonPressNextNegative(); };
    view.onButtonPress_PreviousNegative = [this]() { this->ButtonPressPreviousNegative(); };
    view.onSliderChange_SetExposure = [this](float value) { this->SliderChangeSetExposure(value); };
    view.onSliderChange_SetRBalance = [this](float value) { this->SliderChangeSetRBalance(value); };
    view.onSliderChange_SetGBalance = [this](float value) { this->SliderChangeSetGBalance(value); };
    view.onSliderChange_SetBBalance = [this](float value) { this->SliderChangeSetBBalance(value); };
    view.addGuiWidgets();
}

void Controller::updatePreview() {
    sf::Texture previewTexture = createPreviewtexture(this->model.getPreview());
    std::println("preview successfully recovered from model");
    this->view.setPreviewTexture(previewTexture);
}

void Controller::ButtonPressConvert() {
    std::println("button pressed convert");
    this->model.renderWorking();
    this->updatePreview();
}

void Controller::SliderChangeSetRBalance(float value) {
    std::println("R balance slider value was changed to {}", value);
    this->model.setRBalance(value);
    this->model.renderEdits();
    this->updatePreview();
}

void Controller::SliderChangeSetGBalance(float value) {
    std::println("G balance slider value was changed to {}", value);
    this->model.setGBalance(value);
    this->model.renderEdits();
    this->updatePreview();
}

void Controller::SliderChangeSetBBalance(float value) {
    std::println("R balance slider value was changed to {}", value);
    this->model.setBBalance(value);
    this->model.renderEdits();
    this->updatePreview();
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
        ImageData thumbnail = model.addNegative(path);
        view.addThumbnail(createPreviewtexture(thumbnail), 1);
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

void Controller::ButtonPressNextNegative() {
    std::println("button pressed next negative");
    this->model.nextNegative();
    this->updatePreview();
}
void Controller::ButtonPressPreviousNegative() {
    std::println("button pressed next negative");
    this->model.previousNegative();
    this->updatePreview();
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