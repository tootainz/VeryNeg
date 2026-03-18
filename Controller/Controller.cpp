#include "Controller.hpp"

#include <print>
#include <filesystem>

#include "../libraries/portable-file-dialogs.h"
#include "../rmlui-backend/RmlUi_Backend.h"
#include <RmlUi/Core.h>
#include <RmlUi_Platform_SFML.h>
#include <RmlUi_Renderer_GL2.h>

#include "commands.hpp"


// HELPERS
// ----------------------------------------------------------------------------------------------------------------

// SFML specific helper function for transforming the preview ImageData from Negative to a texture for SFML
static sf::Texture createPreviewtexture(ImageData previewData) {
    std::cout << "creating texture" << std::endl;
    sf::Vector2u size(previewData.width, previewData.height);
    sf::Image previewImage(size, previewData.data.data());
    auto previewTexture = sf::Texture(previewImage, false);
    return previewTexture;
}


// CONSTRUCTOR
// ----------------------------------------------------------------------------------------------------------------

Controller::Controller(sf::RenderWindow& window, View& view, Model& model) :
    view(view),
    model(model),
    window(window),
    history(200)
{
    this->window.setKeyRepeatEnabled(false);
    view.onButtonPress_Convert = [this]() { this->ButtonPressConvert(); };
    view.onButtonPress_LoadNegative = [this]() { this->ButtonPressAddNegative(); };
    view.onButtonPress_SavePositive = [this]() { this->ButtonPressSavePositive(); };
    view.onButtonPress_NextNegative = [this]() { this->ButtonPressNextNegative(); };
    view.onButtonPress_PreviousNegative = [this]() { this->ButtonPressPreviousNegative(); };
    view.onButtonPress_Thumbnail = [this](int id) {this->ButtonPressThumbnail(id); };
    view.onSliderChange_SetExposure = [this](float value) { this->SliderChangeSetExposure(value); };
    view.onSliderChange_SetRBalance = [this](float value) { this->SliderChangeSetRBalance(value); };
    view.onSliderChange_SetGBalance = [this](float value) { this->SliderChangeSetGBalance(value); };
    view.onSliderChange_SetBBalance = [this](float value) { this->SliderChangeSetBBalance(value); };
}


// METHODS
// ----------------------------------------------------------------------------------------------------------------

void Controller::updatePreview() {
    sf::Texture previewTexture = createPreviewtexture(this->model.getPreview());
    std::println("preview successfully recovered from model");
    this->view.setPreviewTexture(previewTexture);
}

void Controller::updateEditSettings() {
    this->disableCallbacks = true;
        float exposure = this->model.getExposure();
        float rBalance = this->model.getRBalance();
        float gBalance = this->model.getGBalance();
        float bBalance = this->model.getBBalance();
        this->view.setSliderValue("rBalanceSlider", rBalance);
        this->view.setSliderValue("gBalanceSlider", gBalance);
        this->view.setSliderValue("bBalanceSlider", bBalance);
        this->view.setSliderValue("exposureSlider", exposure);
    this->disableCallbacks = false;
}

void Controller::undo() {
    if (this->history.undo()) {
        this->updateEditSettings();
        this->model.renderEdits();
        this->updatePreview();
    }
}

void Controller::redo() {
    if (this->history.redo()) {
        this->updateEditSettings();
        this->model.renderEdits();
        this->updatePreview();
    }
}


// GUI CALLBACKS
// ----------------------------------------------------------------------------------------------------------------

void Controller::ButtonPressConvert() {
    std::println("button pressed convert");
    this->model.renderWorking();
    this->updatePreview();
}

void Controller::SliderChangeSetRBalance(float value) {
    if (!this->disableCallbacks) {
        std::println("R balance slider value was changed to {}", value);
        this->history.addCommand(std::make_unique<Command_SetRBalance>(this->model, value));
        this->model.renderEdits();
        this->updatePreview();
    }
}

void Controller::SliderChangeSetGBalance(float value) {
    if (!this->disableCallbacks) {
        std::println("G balance slider value was changed to {}", value);
        this->history.addCommand(std::make_unique<Command_SetGBalance>(this->model, value));
        this->model.renderEdits();
        this->updatePreview();
    }
}

void Controller::SliderChangeSetBBalance(float value) {
    if (!this->disableCallbacks) {
        std::println("R balance slider value was changed to {}", value);
        this->history.addCommand(std::make_unique<Command_SetBBalance>(this->model, value));
        this->model.renderEdits();
        this->updatePreview();
    }
}

void Controller::SliderChangeSetExposure(float value) {
    if (!this->disableCallbacks) {
        std::println("Slider value was changed to {}", value);
        this->history.addCommand(std::make_unique<Command_SetExposure>(this->model, value));
        this->model.renderEdits();
        this->updatePreview();
    }
}

void Controller::ButtonPressAddNegative() {
    std::println("button pressed choose negative");
    pfd::open_file fileOpener("Choose negative", "/");
    std::vector<std::string> paths = fileOpener.result();
    if (paths.size() == 0) {
        std::cout << "didn't choose a file" << std::endl;
        return;
    } else {
        std::filesystem::path path = paths[0];
        std::cout << "Trying to open negative at: " << std::endl << path << std::endl;
        Negative& negative = model.addNegative(path);
        int id = negative.getId();
        ImageData thumbnail = negative.getThumbnail();
        view.addThumbnail(createPreviewtexture(thumbnail), id);
    }
    this->updateEditSettings();
    this->updatePreview();
}

void Controller::ButtonPressSavePositive() {
    std::println("button pressed save positive");
    pfd::save_file fileSaver("Choose positive location", "/");
    std::filesystem::path path = fileSaver.result();
    std::println("save path is {}", path.string());
    this->model.exportPositive(path);
}

void Controller::ButtonPressNextNegative() {
    std::println("button pressed next negative");
    this->history.addCommand(std::make_unique<Command_NextNegative>(this->model));
    this->updatePreview();
}

void Controller::ButtonPressPreviousNegative() {
    std::println("button pressed next negative");
    this->history.addCommand(std::make_unique<Command_PreviousNegative>(this->model));
    this->updatePreview();
}

void Controller::ButtonPressThumbnail(int id) {
    std::println("thumbnail {} pressed", id);
    this->model.changeCurrentNegativeById(id);
    this->updatePreview();
}


// MAIN LOOP
// ----------------------------------------------------------------------------------------------------------------

void Controller::mainLoop() {

    while (this->window.isOpen() && RmlBackend::ProcessEvents(this->view.getRmlContext())) {

        while (const std::optional event = this->window.pollEvent()) {

            if (event->is<sf::Event::Closed>()) {
                this->window.close();

            } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {   
                if (keyPressed->scancode == sf::Keyboard::Scancode::Z) {
                    std::println("z pressed");
                    this->undo();
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::X) {
                    std::println("x pressed");
                    this->redo();
                }
            }
        }

        this->window.clear();
        RmlBackend::BeginFrame();

        // Draw the view
        this->view.draw();

        RmlBackend::PresentFrame();
        this->window.display();
    }
}