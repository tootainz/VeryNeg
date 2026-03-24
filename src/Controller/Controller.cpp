#include "Controller.hpp"

#include <print>
#include <filesystem>

#include "../../libraries/portable-file-dialogs.h"
#include "../RmlUi_Backend/RmlUi_Backend.hpp"
#include <RmlUi/Core.h>
#include <RmlUi_Platform_SFML.h>
#include <RmlUi_Renderer_GL2.h>

#include "commands.hpp"
#include "../Negative/ImageArea.hpp"


// STATIC HELPERS
// ----------------------------------------------------------------------------------------------------------------

// SFML specific helper function for transforming the preview ImageData from Negative to a texture for SFML
static std::unique_ptr<sf::Texture> createPreviewtexture(ImageData previewData) {
    std::println("creating preview texture");
    sf::Vector2u size(previewData.width, previewData.height);
    sf::Image previewImage(size, previewData.data.data());
    auto previewTexture = std::make_unique<sf::Texture>(previewImage, false);
    return previewTexture;
}


// CONSTRUCTOR AND MAIN LOOP
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


void Controller::mainLoop() {
    
    // MAIN LOOP CHECKS IF WINDOW IS OPEN
    while (this->window.isOpen()) {

        // EVENTS
        this->eventLoop();

        // RENDERING
        this->view.render();
	};
}


// UPDATE GUI
// ----------------------------------------------------------------------------------------------------------------

void Controller::updatePreview() {
    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        std::println("preview negative exists");
        Negative* negative = isNegative.value();
        std::unique_ptr<sf::Texture> previewTexture = std::move(createPreviewtexture(negative->getPreview()));
        std::println("preview successfully recovered from model");
        this->view.setPreviewTexture(std::move(previewTexture));
    }
    else {
        std::println("preview negative doesnt exist");
        // This is supposed to clear the preview but not workign at the moment
        this->view.setPreviewTexture(std::make_unique<sf::Texture>());
    }
}

void Controller::updateEditSettings(Negative* negative) {
    this->disableCallbacks = true;
        float exposure = negative->getDensity();
        float rBalance = negative->getRBalance();
        float gBalance = negative->getGBalance();
        float bBalance = negative->getBBalance();
        this->view.setSliderValue("rBalanceSlider", rBalance);
        this->view.setSliderValue("gBalanceSlider", gBalance);
        this->view.setSliderValue("bBalanceSlider", bBalance);
        this->view.setSliderValue("exposureSlider", exposure);
    this->disableCallbacks = false;
}


// UNDO REDO
// ----------------------------------------------------------------------------------------------------------------

void Controller::undo() {
    if (this->history.undo()) {
        std::optional<Negative*> isNegative = this->model.getCurrentNegative();
        if (isNegative) {
            Negative* negative = isNegative.value();
            this->updateEditSettings(negative);
            negative->renderEdits();
            this->updatePreview();
        }
    }
}

void Controller::redo() {
    if (this->history.redo()) {
        std::optional<Negative*> isNegative = this->model.getCurrentNegative();
        if (isNegative) {
            Negative* negative = isNegative.value();
            this->updateEditSettings(negative);
            negative->renderEdits();
            this->updatePreview();
        }
    }
}


// GUI EVENTS NEGATIVE NAVIGATION
// ----------------------------------------------------------------------------------------------------------------

void Controller::ButtonPressAddNegative() {
    std::println("button pressed choose negative");

    auto execute = [this]() -> void {

        pfd::open_file fileOpener("Choose negative", "/");
        std::vector<std::string> paths = fileOpener.result();

        if (paths.size() == 0) {
            std::println("didn't choose a file");
            return;
        }
        else {
            std::filesystem::path path = paths[0];
            std::println("Trying to open negative at: {}", path.string());

            Negative& negative = model.addNegative(path);
            int id = negative.getId();
            ImageData thumbnail = negative.getThumbnail();
            this->view.addThumbnail(createPreviewtexture(thumbnail), id);
            this->updateEditSettings(&negative);
        }
    };

    auto undo = [this]() -> void {

    };

    this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
    this->view.LoadThumbnails();
    this->updatePreview();
}

void Controller::ButtonPressRemoveNegative(int id) {
    std::println("button pressed remove negative");

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();
        std::filesystem::path path = negative->getPath();

        auto execute = [this, id]() -> void {
            this->model.removeNegativeById(id);
            this->view.removeThumbnail(id);
        };

        auto undo = [this, path, id]() -> void {
            std::println("Trying to open negative at: {}", path.string());
            Negative& negative = model.addNegative(path, id);
            ImageData thumbnail = negative.getThumbnail();
            this->view.addThumbnail(createPreviewtexture(thumbnail), id);
            this->updateEditSettings(&negative);
        };

        this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
        this->view.LoadThumbnails();
    }
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

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();

        int previousId = negative->getId();

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
}


// GUI EVENTS PRE-CONVERT
// ----------------------------------------------------------------------------------------------------------------

void Controller::ButtonPressSetScanGamma(float value) {
    std::println("scan gamma set to {}", value);
}

void Controller::ButtonPressSetBorder() {
    std::println("Set Border pressed");
    this->selectingBorder = !this->selectingBorder;
    this->readyToSelect = !this->readyToSelect;
}

void Controller::SetBorder(int x, int y) {
    std::println("Seting Border");

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();

        std::tuple<float, float, float> previousBorderData = negative->getBorder();

        auto execute = [&negative, x, y]() {
            negative->setBorderByCoords(x, y);
        };

        auto undo = [&negative, previousBorderData]() {
            negative->setBorder(std::get<0>(previousBorderData), std::get<1>(previousBorderData), std::get<2>(previousBorderData));
        };

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, undo)
        );

        this->updatePreview();
    }
}


void Controller::ButtonPressSetDensest() {
    std::println("Set Densest pressed");
    this->selectingDensest = !this->selectingDensest;
    this->readyToSelect = !this->readyToSelect;
}

void Controller::SetDensest(int x, int y) {
    std::println("Setting Densest");

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();
        std::tuple<float, float, float> previousDensestData = negative->getDensest();

        auto execute = [&negative, x, y]() {
            negative->setDensestByCoords(x, y);
        };

        auto undo = [&negative, previousDensestData]() {
            negative->setDensest(std::get<0>(previousDensestData),
                                std::get<1>(previousDensestData),
                                std::get<2>(previousDensestData));
        };

        this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
        this->updatePreview();
    }
}

void Controller::ButtonPressSetScanArea() {
    std::println("set scan area pressed");
    this->selectingScanArea = !this->selectingScanArea;
    this->readyToSelect = !this->readyToSelect;
}

void Controller::ButtonPressConvert() {
    std::println("Button pressed convert");

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();

        auto execute = [&negative]() {
            negative->renderWorking();
        };

        auto undo = [&negative]() {
            negative->resetConversion();
        };

        this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
        this->updatePreview();
    }
}

void Controller::ButtonPressResetConversion() {
    std::println("Reset conversion pressed");

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();

        auto execute = [&negative]() {
            negative->resetConversion();
        };

        auto undo = [&negative]() {
            negative->convert();
        };

        this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
        this->updatePreview();
    }
}

void Controller::SliderChangeSetDensity(float value) {
    std::println("Slider value was changed to {}", value);

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();

        auto setter = [&negative](float v) -> float { return negative->setDensity(v); };
        auto getter = [&negative]() -> float { return negative->getDensity(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits();
        this->updatePreview();
    }
}

void Controller::SliderChangeSetContrast(float value) {
    std::println("Contrast slider value was changed to {}", value);

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();

        auto setter = [&negative](float v) -> float { return negative->setContrast(v); };
        auto getter = [&negative]() -> float { return negative->getContrast(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits();
        this->updatePreview();
    }
}

void Controller::SliderChangeSetWhites(float value) {
    std::println("Whites slider value was changed to {}", value);

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();

        auto setter = [&negative](float v) -> float { return negative->setWhites(v); };
        auto getter = [&negative]() -> float { return negative->getWhites(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits();
        this->updatePreview();
    }
}

void Controller::SliderChangeSetHighlights(float value) {
    std::println("Highlights slider value was changed to {}", value);

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();

        auto setter = [&negative](float v) -> float { return negative->setHighlights(v); };
        auto getter = [&negative]() -> float { return negative->getHighlights(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits();
        this->updatePreview();
    }
}

void Controller::SliderChangeSetShadows(float value) {
    std::println("Shadows slider value was changed to {}", value);

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();

        auto setter = [&negative](float v) -> float { return negative->setShadows(v); };
        auto getter = [&negative]() -> float { return negative->getShadows(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits();
        this->updatePreview();
    }
}

void Controller::SliderChangeSetBlacks(float value) {
    std::println("Blacks slider value was changed to {}", value);

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();

        auto setter = [&negative](float v) -> float { return negative->setBlacks(v); };
        auto getter = [&negative]() -> float { return negative->getBlacks(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits();
        this->updatePreview();
    }
}

void Controller::SetNeutralBalance(int x, int y) {
    std::println("Setting Neutral balance");

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();
        std::tuple<float, float, float> previousNeutralData = negative->getNeutral();

        auto execute = [&negative, x, y]() {
            negative->setNeutralByCoords(x, y);
        };

        auto undo = [&negative, previousNeutralData]() {
            negative->setNeutral(std::get<0>(previousNeutralData),
                                std::get<1>(previousNeutralData),
                                std::get<2>(previousNeutralData));
        };

        this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
        this->updatePreview();
    }
}

void Controller::ButtonPressAutoWhiteBalance() {
}

void Controller::ButtonPressChooseNeutralBalance() {
    std::println("Neutral balanbce button pressed");
    this->selectingNeutral = !this->selectingNeutral;
    this->readyToSelect = !this->readyToSelect;
}

void Controller::SliderChangeSetRBalance(float value) {
    std::println("R balance slider value was changed to {}", value);

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();

        auto setter = [&negative](float v) -> float { return negative->setRBalance(v); };
        auto getter = [&negative]() -> float { return negative->getRBalance(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits();
        this->updatePreview();
    }
}

void Controller::SliderChangeSetGBalance(float value) {
    std::println("G balance slider value was changed to {}", value);

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();

        auto setter = [&negative](float v) -> float { return negative->setGBalance(v); };
        auto getter = [&negative]() -> float { return negative->getGBalance(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits();
        this->updatePreview();
    }
}

void Controller::SliderChangeSetBBalance(float value) {
    std::println("B balance slider value was changed to {}", value);

    std::optional<Negative*> isNegative = this->model.getCurrentNegative();
    if (isNegative) {
        Negative* negative = isNegative.value();

        auto setter = [&negative](float v) -> float { return negative->setBBalance(v); };
        auto getter = [&negative]() -> float { return negative->getBBalance(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits();
        this->updatePreview();
    }
}


// EXPORTING
// ----------------------------------------------------------------------------------------------------------------

void Controller::ButtonPressSavePositive() {
    // std::println("button pressed save positive");
    // pfd::save_file fileSaver("Choose positive location", "/");
    // std::filesystem::path path = fileSaver.result();
    // std::println("save path is {}", path.string());
    // this->model.getCurrentNegative().exportPositive(path);
}


// EVENT LISTENER
// ----------------------------------------------------------------------------------------------------------------

void Controller::ProcessEvent(Rml::Event& event) {
    Rml::Element* element = event.GetTargetElement();
    const std::string id = element->GetId();
    const std::string className = element->GetId();

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
        else if (id == "remove") {
            std::optional<Negative*> isNegative = this->model.getCurrentNegative();
            if (isNegative) {
                Negative* negative = isNegative.value();
                int id = negative->getId();
                ButtonPressRemoveNegative(id);
            }
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
        else if (id.contains("thumbnail")) {
            int thumbnailId = 0;
            size_t pos = id.find('_'); // find underscore
            if (pos != std::string::npos) {
                thumbnailId = std::stoi(id.substr(pos + 1)); // substring after underscore
            }
            ButtonPressThumbnail(thumbnailId);
            std::println("pressed thumbnail {}", thumbnailId);
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


// EVENT LOOP
// ----------------------------------------------------------------------------------------------------------------

bool Controller::handleKeyboardEvents(std::optional<sf::Event> event) {
    if (auto keyPressed = event->getIf<sf::Event::KeyPressed>()) {

        // Global shortcuts that take priority over the context go here.

        // Otherwise, hand the event over to the context by calling the input handler as normal.
        RmlSFML::InputHandler(this->view.getRmlContext(), *event);

        // The key was not consumed by the context either, try keyboard shortcuts of lower priority.

        // Command pressed
        if (keyPressed->system) {
            // Cmd Z
            if (keyPressed->code == sf::Keyboard::Key::Z) {
                this->undo();
            }
            // Shift Cmd Z
            else if (keyPressed->shift && keyPressed->code == sf::Keyboard::Key::Z) {
                this->redo();
            }
        }
        return true;
    }
    else {
        return false;
    }
}

bool Controller::handleMouseEvents(std::optional<sf::Event> event) {

    // MOUSE PRESSED
    if (auto mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
        std::println("mouse pressed at ({},{})", mousePressed->position.x,  mousePressed->position.y);
        if (this->readyToSelect) {
            // Selecting only a sample point
            if (this->selectingBorder || this->selectingDensest || this->selectingNeutral) {
                if (this->selectingBorder) {
                    this->SetBorder( mousePressed->position.x,  mousePressed->position.y);
                }
                else if (this->selectingDensest) {
                    this->SetDensest( mousePressed->position.x,  mousePressed->position.y);
                }
                else if (this->selectingNeutral) {
                    this->SetNeutralBalance( mousePressed->position.x,  mousePressed->position.y);
                }
                this->readyToSelect = false;
                this->selectingBorder = false;
                this->selectingDensest = false;
                this->selectingNeutral = false;
            }
            else {
                this->view.displaySelection = true;
                std::println("starting dragging");
                this->selecting = true;
                this->readyToSelect = false;
                this->selectionStart = { mousePressed->position.x, mousePressed->position.y };
            }
        }

        // Otherwise, hand the event over to the RmlUi context.
        RmlSFML::InputHandler(this->view.getRmlContext(), *event);
        return true;
    }

    // MOUSE MOVED
    else if (auto mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
        if (this->selecting) {
        std::println("Dragging at ({},{})", mouseMoved->position.x, mouseMoved->position.y);
            if (this->selectingScanArea) {
                std::optional<Negative*> isNegative = this->model.getCurrentNegative();
                if (isNegative) {
                    Negative* negative = isNegative.value();
                    ImageArea scanArea = negative->getScanArea();
                    scanArea.left   = std::min(selectionStart.x, mouseMoved->position.x);
                    scanArea.top    = std::min(selectionStart.y, mouseMoved->position.y);
                    scanArea.right  = std::max(selectionStart.x, mouseMoved->position.x);
                    scanArea.bottom = std::max(selectionStart.y, mouseMoved->position.y);
                    negative->setScanArea(scanArea);
                    this->view.setSelection(scanArea);
                }
            }
        }

        // Hand the event over to the RmlUi context.
        RmlSFML::InputHandler(this->view.getRmlContext(), *event);
        return true;
    }

    // MOUSE RELEASED
    else if (auto mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {

        // Process mouse after RmlUi
        std::println("mouse Released at ({},{})", mouseReleased->position.x,  mouseReleased->position.y);
        if (this->selecting) {
            std::println("Finished dragging");
            this->selecting = false;
            this->readyToSelect = false;
            if (this->selectingScanArea) {
                this->selectingScanArea = false;
            }
        }

        // Hand the event over to the RmlUi context.
        RmlSFML::InputHandler(this->view.getRmlContext(), *event);
        return true;
    }

    else {
        return false;
    }
}

void Controller::eventLoop() {

    while (const std::optional event = this->window.pollEvent()) {

        // RESIZED
        if (event->is<sf::Event::Resized>()) {
            RmlBackend::Resize(this->view.getRmlContext());
        }

        // KEY PRESSED
        else if (this->handleKeyboardEvents(event)) {}

        // MOUSE
        else if (this->handleMouseEvents(event)) {}

        // WINDOW CLOSED
        else if (event->is<sf::Event::Closed>()) {
            this->window.close();
        }

        // REST OF THE EVENTS
        else {
            RmlSFML::InputHandler(this->view.getRmlContext(), *event);
        }
    }
}