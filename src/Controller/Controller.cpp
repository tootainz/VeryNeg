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
    std::println("preview texture size is: x:{} y:{}", previewData.width, previewData.height);
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

void Controller::updatePreview(bool dragging) {
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {
        std::println("preview negative exists");
        std::unique_ptr<sf::Texture> previewTexture = std::move(createPreviewtexture(negative->getPreview(dragging)));
        std::println("preview successfully recovered from model");
        this->view.setPreviewTexture(std::move(previewTexture));
    }
    else {
        std::println("preview negative doesnt exist");
        // Clear the preview
        this->view.setPreviewTexture(std::make_unique<sf::Texture>());
    }
}

void Controller::updateEditSettings(Negative& negative) {
    this->uiState.disableCallbacks = true;

    float scanGamma = negative.getScanGamma();
    ImageArea scanArea = negative.getScanArea();

    float density = negative.getDensity();
    float contrast = negative.getContrast();
    float whites = negative.getWhites();
    float highlights = negative.getHighlights();
    float shadows = negative.getShadows();
    float blacks = negative.getBlacks();

    float rBalance = negative.getRBalance();
    float gBalance = negative.getGBalance();
    float bBalance = negative.getBBalance();

    this->view.setSliderValue("scanGamma", scanGamma);
    this->view.setSelection(scanArea);

    this->view.setSliderValue("density", density);
    this->view.setSliderValue("contrast", contrast);
    this->view.setSliderValue("whites", whites);
    this->view.setSliderValue("highlights", highlights);
    this->view.setSliderValue("shadows", shadows);
    this->view.setSliderValue("blacks", blacks);

    this->view.setSliderValue("c-r", rBalance);
    this->view.setSliderValue("m-g", gBalance);
    this->view.setSliderValue("y-b", bBalance);

    this->uiState.disableCallbacks = false;
}

// UNDO REDO
// ----------------------------------------------------------------------------------------------------------------

void Controller::undo() {
    if (this->history.undo()) {
        Negative* negative = this->model.getCurrentNegative();
        if (negative) {
            this->updateEditSettings(*negative);
            negative->renderEdits(this->uiState.isDragging);
        }
        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::redo() {
    if (this->history.redo()) {
        Negative* negative = this->model.getCurrentNegative();
        if (negative) {
            this->updateEditSettings(*negative);
            negative->renderEdits(this->uiState.isDragging);
        }
        this->updatePreview(this->uiState.isDragging);
    }
}


// GUI EVENTS NEGATIVE NAVIGATION
// ----------------------------------------------------------------------------------------------------------------

void Controller::ButtonPressAddNegative() {
    std::println("button pressed choose negative");

    auto execute = [this]() -> void {

        pfd::open_file fileOpener("Choose negative", "/", {"tiff images", "*.tif *.tiff *.TIFF *.TIF"}, pfd::opt::multiselect);
        std::vector<std::string> paths = fileOpener.result();

        if (paths.size() == 0) {
            std::println("didn't choose a file");
            return;
        }
        else {
            std::println("opened {} paths", paths.size());
            for (std::filesystem::path path : paths) {
                std::println("Trying to open negative at: {}", path.string());

                Negative* negative = model.addNegative(path);
                if (negative) {
                    int id = negative->getId();
                    ImageData thumbnail = negative->getThumbnail();
                    this->view.addThumbnail(createPreviewtexture(thumbnail), id);
                    this->updateEditSettings(*negative);
                }
            }
            return;
        }
    };

    auto undo = [this]() -> void {

    };

    this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
    this->view.LoadThumbnails();
    this->updatePreview(this->uiState.isDragging);
    this->view.updatePreviewSize();
    this->view.updatePreviewScale();
    this->view.updatePreviewPos();
    this->updatePreview(this->uiState.isDragging);
}

void Controller::ButtonPressRemoveNegative(int id) {
    std::println("button pressed remove negative");

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {
        std::filesystem::path path = negative->getPath();

        auto execute = [this, id]() -> void {
            this->model.removeNegativeById(id);
            this->view.removeThumbnail(id);
        };

        auto undo = [this, path, id]() -> void {
            std::println("Trying to open negative at: {}", path.string());
            Negative* negative = model.addNegative(path, id);
            if (negative) {
                ImageData thumbnail = negative->getThumbnail();
                this->view.addThumbnail(createPreviewtexture(thumbnail), id);
                this->updateEditSettings(*negative);
            }
        };

        this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
        this->view.LoadThumbnails();
    }
    this->updatePreview(this->uiState.isDragging);
    this->view.updatePreviewPos();
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
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {
        this->updateEditSettings(*negative);
    }
    this->updatePreview(this->uiState.isDragging);
    this->view.updatePreviewPos();
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
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {
        this->updateEditSettings(*negative);
    }
    this->updatePreview(this->uiState.isDragging);
    this->view.updatePreviewPos();
}

void Controller::ButtonPressThumbnail(int id) {
    std::println("thumbnail {} pressed", id);

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

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

        Negative* negative = this->model.getCurrentNegative();
        if (negative) {
            this->updateEditSettings(*negative);
        }
        this->updatePreview(this->uiState.isDragging);
        this->view.updatePreviewPos();
    }
}


// GUI EVENTS PRE-CONVERT
// ----------------------------------------------------------------------------------------------------------------

void Controller::ButtonPressSetScanGamma(float value) {
    std::println("scan gamma set to {}", value);
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {
        float previousScanGamma = negative->getScanGamma();

        auto execute = [negative, value]() {
            negative->setScanGamma(value);
        };

        auto undo = [negative, previousScanGamma]() {
            negative->setScanGamma(previousScanGamma);
        };

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, undo)
        );
        this->updatePreview(this->uiState.isDragging);
    };
}

void Controller::ButtonPressBorder(bool checked) {
    std::println("Border pressed");
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        bool previousHasNeutral = negative->getHasBorder();

        auto execute = [negative, checked]() {
            negative->setHasBorder(checked);
        };

        auto undo = [negative, previousHasNeutral]() {
            negative->setHasBorder(previousHasNeutral);
        };

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, undo)
        );

        this->updatePreview(this->uiState.isDragging);
    };
}

void Controller::ButtonPressSetBorder() {
    std::println("Set Border pressed");
    std::println("Setting Border");
    this->uiState.selectingBorder = !this->uiState.selectingBorder;
    this->uiState.readyToSelect = !this->uiState.readyToSelect;
}

void Controller::SetBorder(int x, int y) {
    std::println("Setting Border");

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        std::tuple<float, float, float> previousBorderData = negative->getBorder();

        auto execute = [negative, x, y]() {
            negative->setBorderByCoords(x, y);
        };

        auto undo = [negative, previousBorderData]() {
            negative->setBorder(std::get<0>(previousBorderData), std::get<1>(previousBorderData), std::get<2>(previousBorderData));
        };

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, undo)
        );

        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::ButtonPressDensest(bool checked) {
    std::println("Densest pressed");
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        bool previousHasNeutral = negative->getHasDensest();

        auto execute = [negative, checked]() {
            negative->setHasDensest(checked);
        };

        auto undo = [negative, previousHasNeutral]() {
            negative->setHasDensest(previousHasNeutral);
        };

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, undo)
        );

        this->updatePreview(this->uiState.isDragging);
    };
}

void Controller::ButtonPressSetDensest() {
    std::println("Set Densest pressed");
    this->uiState.selectingDensest = !this->uiState.selectingDensest;
    this->uiState.readyToSelect = !this->uiState.readyToSelect;
}

void Controller::SetDensest(int x, int y) {
    std::println("Setting Densest");

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {
        std::tuple<float, float, float> previousDensestData = negative->getDensest();

        auto execute = [negative, x, y]() {
            negative->setDensestByCoords(x, y);
        };

        auto undo = [negative, previousDensestData]() {
            negative->setDensest(std::get<0>(previousDensestData),
                                std::get<1>(previousDensestData),
                                std::get<2>(previousDensestData));
        };

        this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::ButtonPressScanArea(bool checked) {
    std::println("scan area pressed");
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        bool previousHasScanArea = negative->getHasScanArea();

        auto execute = [negative, checked]() {
            negative->setHasScanArea(checked); 
        };

        auto undo = [negative, previousHasScanArea]() {
            negative->setHasScanArea(previousHasScanArea);
        };

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, undo)
        );

        this->updatePreview(this->uiState.isDragging);
    };
}

void Controller::ButtonPressSetScanArea() {
    std::println("set scan area pressed");
    this->uiState.selectingScanArea = !this->uiState.selectingScanArea;
    this->uiState.readyToSelect = !this->uiState.readyToSelect;
    this->view.displaySelection = !this->view.displaySelection;
}

void Controller::ButtonPressSetScanAreaNumber() {
}

void Controller::ButtonPressConvert() {
    std::println("Button pressed convert");

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto execute = [this, negative]() {
            negative->convert();
            int id = negative->getId();
            negative->renderThumbnail();
            ImageData thumbnail = negative->getThumbnail();
            this->view.updateThumbnail(std::move(createPreviewtexture(thumbnail)), id);
        };

        auto undo = [this, negative]() {
            negative->resetConversion();
            int id = negative->getId();
            negative->renderThumbnail();
            ImageData thumbnail = negative->getThumbnail();
            this->view.updateThumbnail(std::move(createPreviewtexture(thumbnail)), id);
        };

        this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::ButtonPressResetConversion() {
    std::println("Reset conversion pressed");

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto execute = [this, negative]() {
            negative->resetConversion();
            int id = negative->getId();
            negative->renderThumbnail();
            ImageData thumbnail = negative->getThumbnail();
            this->view.updateThumbnail(std::move(createPreviewtexture(thumbnail)), id);
        };

        auto undo = [this, negative]() {
            negative->convert();
            int id = negative->getId();
            negative->renderThumbnail();
            ImageData thumbnail = negative->getThumbnail();
            this->view.updateThumbnail(std::move(createPreviewtexture(thumbnail)), id);
        };

        this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
        this->updatePreview(this->uiState.isDragging);
    }
}

// GUI EVENTS POST-CONVERT
// ----------------------------------------------------------------------------------------------------------------

void Controller::ButtonPressResetEdits() {
}

void Controller::SliderChangeSetDensity(float value) {
    std::println("Slider value was changed to {}", value);

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto setter = [negative](float v) -> float { return negative->setDensity(v); };
        auto getter = [negative]() -> float { return negative->getDensity(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits(this->uiState.isDragging);
        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::SliderChangeSetContrast(float value) {
    std::println("Contrast slider value was changed to {}", value);

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto setter = [negative](float v) -> float { return negative->setContrast(v); };
        auto getter = [negative]() -> float { return negative->getContrast(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits(this->uiState.isDragging);
        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::SliderChangeSetWhites(float value) {
    std::println("Whites slider value was changed to {}", value);

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto setter = [negative](float v) -> float { return negative->setWhites(v); };
        auto getter = [negative]() -> float { return negative->getWhites(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits(this->uiState.isDragging);
        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::SliderChangeSetHighlights(float value) {
    std::println("Highlights slider value was changed to {}", value);

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto setter = [negative](float v) -> float { return negative->setHighlights(v); };
        auto getter = [negative]() -> float { return negative->getHighlights(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits(this->uiState.isDragging);
        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::SliderChangeSetShadows(float value) {
    std::println("Shadows slider value was changed to {}", value);

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto setter = [negative](float v) -> float { return negative->setShadows(v); };
        auto getter = [negative]() -> float { return negative->getShadows(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits(this->uiState.isDragging);
        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::SliderChangeSetBlacks(float value) {
    std::println("Blacks slider value was changed to {}", value);

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto setter = [negative](float v) -> float { return negative->setBlacks(v); };
        auto getter = [negative]() -> float { return negative->getBlacks(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits(this->uiState.isDragging);
        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::SetNeutralBalance(int x, int y) {
    std::println("Setting Neutral balance");

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {
        std::tuple<float, float, float> previousNeutralData = negative->getNeutral();

        auto execute = [negative, x, y]() {
            negative->setNeutralByCoords(x, y);
        };

        auto undo = [negative, previousNeutralData]() {
            negative->setNeutral(std::get<0>(previousNeutralData),
                                std::get<1>(previousNeutralData),
                                std::get<2>(previousNeutralData));
        };

        this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::ButtonPressAutoWhiteBalance(bool checked) {
    std::println("AutoWB pressed");
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        bool previousHasNeutral = negative->getAutoWB();

        auto execute = [negative, checked]() {
            negative->setAutoWB(checked);
        };

        auto undo = [negative, previousHasNeutral]() {
            negative->setAutoWB(previousHasNeutral);
        };

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, undo)
        );

        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::ButtonPressNeutralBalance(bool checked) {
    std::println("Neutral Balance pressed");
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        bool previousHasNeutral = negative->getHasNeutral();

        auto execute = [negative, checked]() {
            negative->setHasNeutral(checked);
        };

        auto undo = [negative, previousHasNeutral]() {
            negative->setHasNeutral(previousHasNeutral);
        };

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, undo)
        );

        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::ButtonPressChooseNeutralBalance() {
    std::println("Neutral balanbce button pressed");
    this->uiState.selectingNeutral = !this->uiState.selectingNeutral;
    this->uiState.readyToSelect = !this->uiState.readyToSelect;
}

void Controller::SliderChangeSetRBalance(float value) {
    std::println("R balance slider value was changed to {}", value);

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto setter = [negative](float v) -> float { return negative->setRBalance(v); };
        auto getter = [negative]() -> float { return negative->getRBalance(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits(this->uiState.isDragging);
        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::SliderChangeSetGBalance(float value) {
    std::println("G balance slider value was changed to {}", value);

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto setter = [negative](float v) -> float { return negative->setGBalance(v); };
        auto getter = [negative]() -> float { return negative->getGBalance(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits(this->uiState.isDragging);
        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::SliderChangeSetBBalance(float value) {
    std::println("B balance slider value was changed to {}", value);

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto setter = [negative](float v) -> float { return negative->setBBalance(v); };
        auto getter = [negative]() -> float { return negative->getBBalance(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderEdits(this->uiState.isDragging);
        this->updatePreview(this->uiState.isDragging);
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
    if (!this->uiState.disableCallbacks) {
        Rml::Element* element = event.GetTargetElement();
        const std::string id = element->GetId();
        const std::string className = element->GetId();

        // BUTTONS (Click)
        if (event.GetId() == Rml::EventId::Click) {

            if (id == "convert") {
                this->ButtonPressConvert();
            }
            else if (id == "reset") {
                this->ButtonPressResetConversion();
            }
            else if (id == "import") {
                this->ButtonPressAddNegative();
            }
            else if (id == "export") {
                this->ButtonPressSavePositive();
            }
            else if (id == "remove") {
                Negative* negative = this->model.getCurrentNegative();
                if (negative) {
                    int id = negative->getId();
                    this->ButtonPressRemoveNegative(id);
                }
            }
            else if (id == "scanGamma") {
                // TODO
            }
            else if (id == "border") {
                bool has = element->HasAttribute("checked");
                this->ButtonPressBorder(has);
            }
            else if (id == "sampleBorder") {
                this->ButtonPressSetBorder();
            }
            else if (id == "densest") {
                bool has = element->HasAttribute("checked");
                this->ButtonPressDensest(has);
            }
            else if (id == "sampleDensest") {
                this->ButtonPressSetDensest();
            }
            else if (id == "scanArea") {
                this->uiState.usingScanArea = !this->uiState.usingScanArea;
                this->ButtonPressScanArea(this->uiState.usingScanArea);
            }
            else if (id == "setScanArea") {
                this->ButtonPressSetScanArea();
            }
            else if (id == "autoWB") {
                this->ButtonPressAutoWhiteBalance(true);
            }
            else if (id == "selectNeutral") {
                this->ButtonPressChooseNeutralBalance();
            }
            else if (id.contains("thumbnail")) {
                int thumbnailId = 0;
                size_t pos = id.find('_'); // find underscore
                if (pos != std::string::npos) {
                    thumbnailId = std::stoi(id.substr(pos + 1)); // substring after underscore
                }
                this->ButtonPressThumbnail(thumbnailId);
                std::println("pressed thumbnail {}", thumbnailId);
            }
        }

        // SLIDERS (Change)
        else if (event.GetId() == Rml::EventId::Change) {

            if (element->GetTagName() == "input") {

                this->uiState.isDragging = true;

                Rml::Variant* valueVar = element->GetAttribute("value");
                if (!valueVar) return;

                float value = valueVar->Get<float>();

                if (id == "density") {
                    this->SliderChangeSetDensity(value);
                }
                else if (id == "contrast") {
                    this->SliderChangeSetContrast(value);
                }
                else if (id == "whites") {
                    this->SliderChangeSetWhites(value);
                }
                else if (id == "highlights") {
                    this->SliderChangeSetHighlights(value);
                }
                else if (id == "shadows") {
                    this->SliderChangeSetShadows(value);
                }
                else if (id == "blacks") {
                    this->SliderChangeSetBlacks(value);
                }
                else if (id == "c-r") {
                    this->SliderChangeSetRBalance(value);
                }
                else if (id == "m-g") {
                    this->SliderChangeSetGBalance(value);
                }
                else if (id == "y-b") {
                    this->SliderChangeSetBBalance(value);
                }
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

        // Command Z
        if (keyPressed->system && keyPressed->code == sf::Keyboard::Key::Z) {
            if (keyPressed->shift) {
                this->redo();   // Shift + Cmd + Z
            }
            else {
                this->undo();   // Cmd + Z
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
        if (this->uiState.readyToSelect) {

            // Get the mouse position in the actual image coordinates
            std::tuple<int, int> correctedMousePos = this->view.previewCoordsToTextureCoords(mousePressed->position.x, mousePressed->position.y);
            int correctedMouseX = std::get<0>(correctedMousePos);
            int correctedMouseY = std::get<1>(correctedMousePos);
            
            // Selecting only a sample point
            if (this->uiState.selectingBorder || this->uiState.selectingDensest || this->uiState.selectingNeutral) {
                if (this->uiState.selectingBorder) {
                    this->SetBorder( correctedMouseX, correctedMouseY);
                }
                else if (this->uiState.selectingDensest) {
                    this->SetDensest( correctedMouseX, correctedMouseY);
                }
                else if (this->uiState.selectingNeutral) {
                    this->SetNeutralBalance( correctedMouseX, correctedMouseY);
                }
            }
            // Selecting a selection
            else {
                ImageArea& selectionArea = this->uiState.selectionArea;
                int selectionHandleBuffer = 10;
                // We are clicking on the top of the selection
                if (correctedMouseY > selectionArea.top - selectionHandleBuffer &&
                    correctedMouseY < selectionArea.top + selectionHandleBuffer &&
                    correctedMouseX > selectionArea.left &&
                    correctedMouseX < selectionArea.right) {
                    std::println("clicking top");
                    this->uiState.selectingTop = true;
                }
                // We are clicking on the bottom of the selection
                else if (correctedMouseY > selectionArea.bottom - selectionHandleBuffer &&
                    correctedMouseY < selectionArea.bottom + selectionHandleBuffer &&
                    correctedMouseX > selectionArea.left &&
                    correctedMouseX < selectionArea.right) {
                    std::println("clicking bottom");
                    this->uiState.selectingBottom = true;
                }
                // We are clicking on the left side of the selection
                else if (correctedMouseY > selectionArea.top &&
                    correctedMouseY < selectionArea.bottom &&
                    correctedMouseX > selectionArea.left - selectionHandleBuffer &&
                    correctedMouseX < selectionArea.left + selectionHandleBuffer) {
                    std::println("clicking right");
                    this->uiState.selectingLeft = true;
                }
                // We are clicking on the right side of the selection
                else if (correctedMouseY > selectionArea.top &&
                    correctedMouseY < selectionArea.bottom &&
                    correctedMouseX > selectionArea.right - selectionHandleBuffer &&
                    correctedMouseX < selectionArea.right + selectionHandleBuffer) {
                    std::println("clicking right");
                    this->uiState.selectingRight = true;
                }
                else {
                    this->uiState.selectingWhole = true;
                }
                std::println("starting dragging");
                this->uiState.selecting = true;
                this->uiState.selectionStart = { correctedMouseX, correctedMouseY };
            }
        }

        // Otherwise, hand the event over to the RmlUi context.
        RmlSFML::InputHandler(this->view.getRmlContext(), *event);
        return true;
    }

    // MOUSE MOVED
    else if (auto mouseMoved = event->getIf<sf::Event::MouseMoved>()) {

        if (this->uiState.selecting) {

        std::println("Dragging at ({},{})", mouseMoved->position.x, mouseMoved->position.y);
        std::tuple<int, int> correctedMousePos = this->view.previewCoordsToTextureCoords(mouseMoved->position.x, mouseMoved->position.y);
        int correctedMouseX = std::get<0>(correctedMousePos);
        int correctedMouseY = std::get<1>(correctedMousePos);

            if (this->uiState.selectingScanArea) {
                ImageArea& scanArea = this->uiState.selectionArea;

                if (this->uiState.selectingTop) {
                    scanArea.top = correctedMouseY;
                }
                else if (this->uiState.selectingBottom) {
                    scanArea.bottom = correctedMouseY;
                }
                else if (this->uiState.selectingLeft) {
                    scanArea.left = correctedMouseX;
                }
                else if (this->uiState.selectingRight) {
                    scanArea.right = correctedMouseX;
                }
                else if (this->uiState.selectingWhole) {
                    scanArea.left   = std::min(this->uiState.selectionStart.x, correctedMouseX);
                    scanArea.top    = std::min(this->uiState.selectionStart.y, correctedMouseY);
                    scanArea.right  = std::max(this->uiState.selectionStart.x, correctedMouseX);
                    scanArea.bottom = std::max(this->uiState.selectionStart.y, correctedMouseY);
                }
                this->view.setSelection(scanArea);
                Negative* negative = this->model.getCurrentNegative();
                if (negative) {
                    negative->setScanArea(scanArea);
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
        if (this->uiState.selecting) {
            std::println("Finished dragging");
            this->uiState.resetGeneralSelectionState();
        }
        if (this->uiState.isDragging) {
            this->uiState.isDragging = false;
            Negative* negative = this->model.getCurrentNegative();
            if (negative) {
                negative->renderEdits(false);
                int id = negative->getId();
                negative->renderThumbnail();
                ImageData thumbnail = negative->getThumbnail();
                this->view.updateThumbnail(createPreviewtexture(thumbnail), id);
            }
            this->updatePreview(false);
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
            this->view.getRmlContext()->Update();
            this->view.updatePreviewSize();
            this->view.updatePreviewScale();
            this->view.updatePreviewPos();
            this->updatePreview(false);
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