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
    Rml::Context* rmlContext = this->view.getRmlContext();
    rmlContext->AddEventListener("click", this);
    rmlContext->AddEventListener("change", this);
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
        float exposure = this->model.getDensity();
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


// GUI EVENTS
// ----------------------------------------------------------------------------------------------------------------

void Controller::ButtonPressAddNegative() {
    std::println("button pressed choose negative");

    auto execute = [this]() -> void {
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
            this->view.addThumbnail(createPreviewtexture(thumbnail), id);
        }
        this->updateEditSettings();
    };

    auto undo = [this]() -> void {
    };

    this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
    this->updatePreview();
}

void Controller::ButtonPressNextNegative() {
    std::println("button pressed next negative");
    auto execute = [this]() -> void {
        this->model.nextNegative();
    };

    auto undo = [this]() -> void {
        this->model.previousNegative();
    };

    this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
    this->updatePreview();
}

void Controller::ButtonPressPreviousNegative() {
    std::println("button pressed previous negative");

    auto execute = [this]() -> void {
        this->model.previousNegative();
    };

    auto undo = [this]() -> void {
        this->model.nextNegative();
    };

    this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
    this->updatePreview();
}

void Controller::ButtonPressThumbnail(int id) {
    std::println("thumbnail {} pressed", id);

    int previousId = this->model.getCurrentNegativeId();

    auto execute = [this, id]() {
        this->model.changeCurrentNegativeById(id);
    };

    auto undo = [this, previousId]() {
        this->model.changeCurrentNegativeById(previousId);
    };

    this->history.addCommand(
        std::make_unique<Command_Lambda>(execute, undo)
    );

    this->updatePreview();
}

void Controller::ButtonPressSetScanGamma(float value) {
    std::println("scan gamma set to {}", value);
}

void Controller::ButtonPressSetBorder() {
    std::println("set border pressed");
}

void Controller::ButtonPressSetDensest() {
    std::println("set densest pressed");
}

void Controller::ButtonPressSetScanArea() {
    std::println("set scan area pressed");
}

void Controller::ButtonPressConvert() {
    std::println("button pressed convert");

    auto execute = [this]() -> void {
        this->model.renderWorking();
    };

    auto undo = [this] -> void {
        this->model.resetConversion();
    };

    this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
    this->updatePreview();
}

void Controller::ButtonPressResetConversion() {
    std::println("reset conversion pressed");

    auto execute = [this]() -> void {
        this->model.resetConversion();
    };

    auto undo = [this]() -> void {
        this->model.convert();
    };

    this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
    this->updatePreview();
}

void Controller::SliderChangeSetDensity(float value) {
    std::println("Slider value was changed to {}", value);

    auto setter = [this](float value) -> float {
        return this->model.setDensity(value);
    };

    auto getter = [this]() -> float {
        return this->model.getDensity();
    };

    this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
    this->model.renderEdits();
    this->updatePreview();
}

void Controller::SliderChangeSetContrast(float value) {
    std::println("Contrast slider value was changed to {}", value);

    auto setter = [this](float value) -> float {
        return this->model.setContrast(value);
    };

    auto getter = [this]() -> float {
        return this->model.getContrast();
    };

    this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
    this->model.renderEdits();
    this->updatePreview();
}

void Controller::SliderChangeSetWhites(float value) {
    std::println("Whites slider value was changed to {}", value);

    auto setter = [this](float value) -> float {
        return this->model.setWhites(value);
    };

    auto getter = [this]() -> float {
        return this->model.getWhites();
    };

    this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
    this->model.renderEdits();
    this->updatePreview();
}

void Controller::SliderChangeSetHighlights(float value) {
    std::println("Highlights slider value was changed to {}", value);

    auto setter = [this](float value) -> float {
        return this->model.setHighlights(value);
    };

    auto getter = [this]() -> float {
        return this->model.getHighlights();
    };

    this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
    this->model.renderEdits();
    this->updatePreview();
}

void Controller::SliderChangeSetShadows(float value) {
    std::println("Shadows slider value was changed to {}", value);

    auto setter = [this](float value) -> float {
        return this->model.setShadows(value);
    };

    auto getter = [this]() -> float {
        return this->model.getShadows();
    };

    this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
    this->model.renderEdits();
    this->updatePreview();
}

void Controller::SliderChangeSetBlacks(float value) {
    std::println("Blacks slider value was changed to {}", value);

    auto setter = [this](float value) -> float {
        return this->model.setBlacks(value);
    };

    auto getter = [this]() -> float {
        return this->model.getBlacks();
    };

    this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
    this->model.renderEdits();
    this->updatePreview();
}

void Controller::ButtonPressAutoWhiteBalance() {
}

void Controller::ButtonPressChooseNeutralBalance() {
}

void Controller::SliderChangeSetRBalance(float value) {
    std::println("R balance slider value was changed to {}", value);

    auto setter = [this](float value) -> float {
        return this->model.setRBalance(value);
    };

    auto getter = [this]() -> float {
        return this->model.getRBalance();
    };

    this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
    this->model.renderEdits();
    this->updatePreview();
}

void Controller::SliderChangeSetGBalance(float value) {
    std::println("G balance slider value was changed to {}", value);

    auto setter = [this](float value) -> float {
        return this->model.setGBalance(value);
    };

    auto getter = [this]() -> float {
        return this->model.getGBalance();
    };

    this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
    this->model.renderEdits();
    this->updatePreview();
}

void Controller::SliderChangeSetBBalance(float value) {
    std::println("B balance slider value was changed to {}", value);

    auto setter = [this](float value) -> float {
        return this->model.setBBalance(value);
    };

    auto getter = [this]() -> float {
        return this->model.getBBalance();
    };

    this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
    this->model.renderEdits();
    this->updatePreview();
}

void Controller::ButtonPressSavePositive() {
    std::println("button pressed save positive");
    pfd::save_file fileSaver("Choose positive location", "/");
    std::filesystem::path path = fileSaver.result();
    std::println("save path is {}", path.string());
    this->model.exportPositive(path);
}


// EVENT LISTENER
// ----------------------------------------------------------------------------------------------------------------

void Controller::ProcessEvent(Rml::Event& event) {
    Rml::Element* element = event.GetTargetElement();
    const std::string id = element->GetId();

    // BUTTONS (Click)
    if (event.GetId() == Rml::EventId::Click) {

        if (id == "convert") {
            ButtonPressConvert();
        }
        else if (id == "reset") {
            ButtonPressResetConversion();
        }
        else if (id == "import") {
            ButtonPressAddNegative();
        }
        else if (id == "export") {
            ButtonPressSavePositive();
        }
        else if (id == "scanGamma") {
            // TODO
        }
        else if (id == "sampleBorder") {
            ButtonPressSetBorder();
        }
        else if (id == "sampleDensest") {
            ButtonPressSetDensest();
        }
        else if (id == "scanArea") {
            ButtonPressSetScanArea();
        }
        else if (id == "autoWB") {
            ButtonPressAutoWhiteBalance();
        }
        else if (id == "selectNeutral") {
            ButtonPressChooseNeutralBalance();
        }
    }

    // SLIDERS (Change)
    else if (event.GetId() == Rml::EventId::Change) {

        if (element->GetTagName() == "input") {

            Rml::Variant* valueVar = element->GetAttribute("value");
            if (!valueVar) return;

            float value = valueVar->Get<float>();

            if (id == "density") {
                SliderChangeSetDensity(value);
            }
            else if (id == "contrast") {
                SliderChangeSetContrast(value);
            }
            else if (id == "whites") {
                SliderChangeSetWhites(value);
            }
            else if (id == "highlights") {
                SliderChangeSetHighlights(value);
            }
            else if (id == "shadows") {
                SliderChangeSetShadows(value);
            }
            else if (id == "blacks") {
                SliderChangeSetBlacks(value);
            }
            else if (id == "c-r") {
                SliderChangeSetRBalance(value);
            }
            else if (id == "m-g") {
                SliderChangeSetGBalance(value);
            }
            else if (id == "y-b") {
                SliderChangeSetBBalance(value);
            }
        }
    }
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

        // Render the view
        this->view.render();
    }
}