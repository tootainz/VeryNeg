#include "Controller.hpp"

#include <print>
#include <filesystem>

#include <RmlUi/Core.h>
#include <RmlUi_Platform_SFML.h>
#include <RmlUi_Renderer_GL2.h>

#include "../RmlUi_Backend/RmlUi_Backend.hpp"
#include "../../libraries/portable-file-dialogs.h"

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
    Rml::Context* rmlContextUi = this->view.getRmlContextUi();
    rmlContextUi->AddEventListener("click", this);
    rmlContextUi->AddEventListener("change", this);
    Rml::Context* rmlContextPopups = this->view.getRmlContextPopups();
    rmlContextPopups->AddEventListener("click", this);
    rmlContextPopups->AddEventListener("change", this);
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

void Controller::cleanup() {
    this->model.cleanCache();
}

// UPDATE GUI
// ----------------------------------------------------------------------------------------------------------------

void Controller::updatePreview(bool dragging) {
    std::println("updatign preview");
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

void Controller::updateSharpnessPreview() {
    std::println("updatign sharpness preview");
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {
        std::println("preview negative exists");
        std::unique_ptr<sf::Texture> previewTexture = std::move(createPreviewtexture(negative->getSharpnessPreview()));
        std::println("preview successfully recovered from model");
        this->view.setSharpeningPreviewTexture(std::move(previewTexture));
    }
    else {
        std::println("preview negative doesnt exist");
        // Clear the preview
        this->view.setSharpeningPreviewTexture(std::make_unique<sf::Texture>());
    }
}

void Controller::updateEditSettings(Negative& negative) {
    this->uiState.disableCallbacks = true;

    float scanGamma = negative.getScanGamma();
    ImageArea scanArea = negative.getScanArea(negative.getWorkingScale());
    bool hasScanArea = negative.getHasScanArea();
    bool hasDensest = negative.getHasDensest();
    bool hasBorder = negative.getHasBorder();

    float density = negative.getDensity();
    float contrast = negative.getContrast();
    float whites = negative.getWhites();
    float highlights = negative.getHighlights();
    float shadows = negative.getShadows();
    float blacks = negative.getBlacks();

    bool autoWB = negative.getAutoWB();
    float rBalance = negative.getRBalance();
    float gBalance = negative.getGBalance();
    float bBalance = negative.getBBalance();
    float saturation = negative.getSaturation();

    float sharpeningAmount = negative.getSharpeningAmount();
    float sharpeningDiameter = negative.getSharpeningDiameter();

    this->view.setSliderValue("scanGamma", scanGamma);
    this->view.setSelection(scanArea);
    this->view.setCheckboxValue("scanArea", hasScanArea);
    this->uiState.hasScanArea = hasScanArea;
    this->uiState.selectionArea = scanArea;
    this->view.setCheckboxValue("border", hasBorder);
    this->uiState.hasBorder = hasBorder;
    this->view.setCheckboxValue("densest", hasDensest);
    this->uiState.hasDensest = hasDensest;

    this->view.setSliderValue("density", density);
    this->view.setSliderValue("contrast", contrast);
    this->view.setSliderValue("whites", whites);
    this->view.setSliderValue("highlights", highlights);
    this->view.setSliderValue("shadows", shadows);
    this->view.setSliderValue("blacks", blacks);

    this->view.setCheckboxValue("autoWB", autoWB);
    this->uiState.autoWB = autoWB;
    this->view.setSliderValue("c-r", rBalance);
    this->view.setSliderValue("m-g", gBalance);
    this->view.setSliderValue("y-b", bBalance);

    this->view.setSliderValue("sharpeningAmount", sharpeningAmount);
    this->view.setSliderValue("sharpeningDiameter", sharpeningDiameter);

    // reset held checkboxes
    this->view.setCheckboxValue("holdConversion", false);
    this->view.setCheckboxValue("holdEdits", false);
    this->view.setCheckboxValue("holdColor", false);
    this->view.setCheckboxValue("holdIntensity", false);
    this->view.setCheckboxValue("holdSharpening", false);

    this->uiState.disableCallbacks = false;
}

// UNDO REDO
// ----------------------------------------------------------------------------------------------------------------

void Controller::undo() {
    if (this->history.undo()) {
        Negative* negative = this->model.getCurrentNegative();
        if (negative) {
            this->updateEditSettings(*negative);
            negative->renderWorkingEdits();
            negative->renderSharpnessPreviewEdits();
        }
        this->updatePreview(false);
        this->updateSharpnessPreview();
    }
}

void Controller::redo() {
    if (this->history.redo()) {
        Negative* negative = this->model.getCurrentNegative();
        if (negative) {
            this->updateEditSettings(*negative);
            negative->renderWorkingEdits();
            negative->renderSharpnessPreviewEdits();
        }
        this->updatePreview(false);
        this->updateSharpnessPreview();
    }
}

// GUI EVENTS HOLDING
// ----------------------------------------------------------------------------------------------------------------

void Controller::applyHeld() {
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {
        std::println("applying held settings");
        if (this->uiState.heldConvert) {
            this->model.applyHoldPreConvert();
        }
        if (this->uiState.heldEdits) {
            this->model.applyHoldPostConvert();
        }
        else {
            if (this->uiState.heldColor) {
                this->model.applyHoldColor();
            }
            if (this->uiState.heldIntensity) {
                this->model.applyHoldIntensity();
            }
            if (this->uiState.heldSharpening) {
                this->model.applyHoldSharpening();
            }
        }
        this->updateEditSettings(*negative);
        negative->renderWorkingEdits();
        negative->renderSharpnessPreviewEdits();
        this->updatePreview(false);
        this->updateSharpnessPreview();
        negative->renderThumbnail();
        int id = negative->getId();
        ImageData thumbnail = negative->getThumbnail();
        this->view.updateThumbnail(std::move(createPreviewtexture(thumbnail)), id);
    }
}

// GUI EVENTS SLIDER RESET
// ----------------------------------------------------------------------------------------------------------------

void Controller::SliderReset(std::string name) {
    this->view.setSliderValue(name, 0.0f);
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
    this->updatePreview(false);
    this->updateSharpnessPreview();
    this->view.updatePreviewSize();
    this->view.updatePreviewScale();
    this->view.updatePreviewPos();
    this->updatePreview(false);
}

void Controller::ButtonPressRemoveNegative(int id) {
    std::println("button pressed remove negative");

    Negative* negative = this->model.getNegativeById(id);
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
    if (!this->model.getCurrentNegative()) { // negatives are empty

    }
    this->updatePreview(false);
    this->updateSharpnessPreview();
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
    this->updatePreview(false);
    this->updateSharpnessPreview();
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
    this->updatePreview(false);
    this->updateSharpnessPreview();
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
        this->updatePreview(false);
        this->updateSharpnessPreview();
        this->view.updatePreviewPos();
    }
}

void Controller::ButtonPressRotateClock() {
    std::println("Rotate clockwise pressed");

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto execute = [this, negative]() {
            negative->rotateClockwise();
            this->view.setPreviewOrientation(negative->getOrientation());
        };
        
        auto undo = [this, negative]() {
            negative->rotateCounterClockwise();
            this->view.setPreviewOrientation(negative->getOrientation());
        };

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, undo)
        );
        
    }
}

void Controller::ButtonPressRotateCounterClock() {
    std::println("Rotate counter clockwise pressed");

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto execute = [this, negative]() {
            negative->rotateCounterClockwise();
            this->view.setPreviewOrientation(negative->getOrientation());
        };
        
        auto undo = [this, negative]() {
            negative->rotateClockwise();
            this->view.setPreviewOrientation(negative->getOrientation());
        };

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, undo)
        );
        
    }
}

void Controller::ButtonPressFlipHorizontal() {
    std::println("flip horizontal pressed");

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto execute = [this, negative]() {
            negative->flipHorizontal();
            this->view.setPreviewOrientation(negative->getOrientation());
        };

        // Undo is same as execute since we are just flipping

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, execute)
        );
        
    }
}

void Controller::ButtonPressFlipVertical() {
    std::println("flip vertical pressed");

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto execute = [this, negative]() {
            negative->flipHorizontal();
            this->view.setPreviewOrientation(negative->getOrientation());
        };

        // Undo is same as execute since we are just flipping

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, execute)
        );
        
    }
}

// GUI EVENTS PRE-CONVERT
// ----------------------------------------------------------------------------------------------------------------

void Controller::CheckboxPressHoldConvert(bool checked) {
    std::println("hold convert pressed");
    std::println("value is {}", checked);
    bool previousChecked = !checked;

    auto execute = [this, checked]() {
        if (checked) {
            this->model.holdSettings(true);
        }
        else {
            if (!this->uiState.somethingIsHolding()) {
                this->model.holdSettings(false);
            }
        }
        this->uiState.heldConvert = checked;
    };

    auto undo = [this, previousChecked]() {
        if (previousChecked) {
            this->model.holdSettings(true);
        }
        else {
            if (!this->uiState.somethingIsHolding()) {
                this->model.holdSettings(false);
            }
        }
        this->uiState.heldConvert = previousChecked;
    };

    this->history.addCommand(
        std::make_unique<Command_Lambda>(execute, undo)
    );
}

void Controller::OptionPressSetScanGamma(float value)
{
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
        this->updatePreview(false);
    };
}

void Controller::CheckboxPressBorder(bool checked) {
    std::println("Border pressed");
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        bool previousHasBorder = negative->getHasBorder();

        auto execute = [this, negative, checked]() {
            this->uiState.hasBorder = checked;
            negative->setHasBorder(checked);
        };

        auto undo = [this, negative, previousHasBorder]() {
            this->uiState.hasBorder = previousHasBorder;
            negative->setHasBorder(previousHasBorder);
        };

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, undo)
        );

        this->updatePreview(false);
    }
}

void Controller::ButtonPressSetBorder() {
    std::println("Set Border pressed");
    if (!this->uiState.hasBorder) {
        // Update the view to mach UiState since this didn't come directly from the main checkbox
        this->view.setCheckboxValue("border", true);
        // Update the UiState and model
        this->CheckboxPressBorder(true);
    }
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
        
        this->updatePreview(false);
    }
}

void Controller::CheckboxPressDensest(bool checked) {
    std::println("Densest pressed");
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        bool previousHasBorder = negative->getHasDensest();

        auto execute = [this, negative, checked]() {
            this->uiState.hasDensest = checked;
            negative->setHasDensest(checked);
        };

        auto undo = [this, negative, previousHasBorder]() {
            this->uiState.hasDensest = previousHasBorder;
            negative->setHasDensest(previousHasBorder);
        };

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, undo)
        );
        
        this->updatePreview(false);
    }
}

void Controller::ButtonPressSetDensest() {
    std::println("Set Densest pressed");
    if (!this->uiState.hasDensest) {
        // Update the view to mach UiState since this didn't come directly from the main checkbox
        this->view.setCheckboxValue("densest", true);
        // Update the UiState and model
        this->CheckboxPressDensest(true);
    }
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
        
        this->updatePreview(false);
    }
}

void Controller::CheckboxPressScanArea(bool checked) {
    std::println("scan area pressed");
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        bool previousHasScanArea = negative->getHasScanArea();

        auto execute = [this, negative, checked]() {
            this->uiState.hasScanArea = checked;
            negative->setHasScanArea(checked); 
        };

        auto undo = [this, negative, previousHasScanArea]() {
            this->uiState.hasScanArea = previousHasScanArea;
            negative->setHasScanArea(previousHasScanArea);
        };

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, undo)
        );

        std::println("checked for scan area is: {}", checked);
        std::println("uistate scan area is: {}", this->uiState.hasScanArea);
                
        this->updatePreview(false);
    };
}

void Controller::ButtonPressSetScanArea() {
    std::println("set scan area pressed");
    if (!this->uiState.hasScanArea) {
        // Update the view to mach UiState since this didn't come directly from scanArea checkbox
        this->view.setCheckboxValue("scanArea", true);
        // Update the UiState and model
        this->CheckboxPressScanArea(true);
    }
    this->uiState.selectingScanArea = !this->uiState.selectingScanArea;
    this->uiState.readyToSelect = !this->uiState.readyToSelect;
    this->view.setSelection(this->uiState.selectionArea);
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
            negative->renderSharpnessPreviewConversion();
            int id = negative->getId();
            negative->renderThumbnail();
            ImageData thumbnail = negative->getThumbnail();
            this->view.updateThumbnail(std::move(createPreviewtexture(thumbnail)), id);
        };

        auto undo = [this, negative]() {
            negative->resetConversion();
            negative->renderSharpnessPreviewConversion();
            int id = negative->getId();
            negative->renderThumbnail();
            ImageData thumbnail = negative->getThumbnail();
            this->view.updateThumbnail(std::move(createPreviewtexture(thumbnail)), id);
        };

        this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
        this->updatePreview(false);
        this->updateSharpnessPreview();
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
        this->updatePreview(false);
        this->updateSharpnessPreview();
    }
}

// GUI EVENTS POST-CONVERT
// ----------------------------------------------------------------------------------------------------------------

void Controller::CheckboxPressHoldEdits(bool checked) {
    std::println("hold edits pressed");
    bool previousChecked = !checked;

    auto execute = [this, checked]() {
        if (checked) {
            this->model.holdSettings(true);
        }
        else {
            if (!this->uiState.somethingIsHolding()) {
                this->model.holdSettings(false);
            }
        }
        this->uiState.heldEdits = checked;
    };

    auto undo = [this, previousChecked]() {
        if (previousChecked) {
            this->model.holdSettings(true);
        }
        else {
            if (!this->uiState.somethingIsHolding()) {
                this->model.holdSettings(false);
            }
        }
        this->uiState.heldEdits = previousChecked;
    };

    this->history.addCommand(
        std::make_unique<Command_Lambda>(execute, undo)
    );
}

void Controller::ButtonPressResetEdits()
{
}

void Controller::OptionPressSetPreset(std::string name) {
    Negative* negative = this->model.getCurrentNegative();
    std::filesystem::path path = std::format("./resources/presets/{}.json", name);
    if (negative) {
        negative->applyPreset(path);
        this->updateEditSettings(*negative);
        negative->renderWorkingEdits();
        negative->renderSharpnessPreviewEdits();
        this->updatePreview(false);
        this->updateSharpnessPreview();
    }
}

void Controller::CheckboxPressHoldIntensity(bool checked) {
    std::println("hold intensity pressed");
    bool previousChecked = !checked;

    auto execute = [this, checked]() {
        if (checked) {
            this->model.holdSettings(true);
        }
        else {
            if (!this->uiState.somethingIsHolding()) {
                this->model.holdSettings(false);
            }
        }
        this->uiState.heldIntensity = checked;
    };

    auto undo = [this, previousChecked]() {
        if (previousChecked) {
            this->model.holdSettings(true);
        }
        else {
            if (!this->uiState.somethingIsHolding()) {
                this->model.holdSettings(false);
            }
        }
        this->uiState.heldIntensity = previousChecked;
    };

    this->history.addCommand(
        std::make_unique<Command_Lambda>(execute, undo)
    );
}

void Controller::ButtonPressResetIntensity()
{
}

void Controller::SliderChangeSetDensity(float value)
{
    std::println("Slider value was changed to {}", value);

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto setter = [negative](float v) -> float { return negative->setDensity(v); };
        auto getter = [negative]() -> float { return negative->getDensity(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderDraggingEdits();
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
        negative->renderDraggingEdits();
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
        negative->renderDraggingEdits();
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
        negative->renderDraggingEdits();
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
        negative->renderDraggingEdits();
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
        negative->renderDraggingEdits();
        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::CheckboxPressHoldColor(bool checked) {
    std::println("hold edits pressed");
    bool previousChecked = !checked;

    auto execute = [this, checked]() {
        if (checked) {
            this->model.holdSettings(true);
        }
        else {
            if (!this->uiState.somethingIsHolding()) {
                this->model.holdSettings(false);
            }
        }
        this->uiState.heldColor = checked;
    };

    auto undo = [this, previousChecked]() {
        if (previousChecked) {
            this->model.holdSettings(true);
        }
        else {
            if (!this->uiState.somethingIsHolding()) {
                this->model.holdSettings(false);
            }
        }
        this->uiState.heldColor = previousChecked;
    };

    this->history.addCommand(
        std::make_unique<Command_Lambda>(execute, undo)
    );
}

void Controller::ButtonPressResetColor()
{
}

void Controller::CheckboxPressAutoWhiteBalance(bool checked)
{
    std::println("AutoWB pressed");
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        bool previousAutoWB = negative->getAutoWB();

        auto execute = [this, negative, checked]() {
            this->uiState.autoWB = checked;
            negative->setHasAutoWB(checked);
            negative->autoWB();
        };

        auto undo = [this, negative, previousAutoWB]() {
            this->uiState.autoWB = previousAutoWB;
            negative->setHasAutoWB(previousAutoWB);
        };

        this->history.addCommand(
            std::make_unique<Command_Lambda>(execute, undo)
        );
        this->updateEditSettings(*negative);
        negative->renderWorkingEdits();
        negative->renderSharpnessPreviewEdits();
        this->updatePreview(false);
        this->updateSharpnessPreview();
    }
}

void Controller::ButtonPressSetNeutralSample() {
    this->uiState.selectingNeutral = !this->uiState.selectingNeutral;
    this->uiState.readyToSelect = !this->uiState.readyToSelect;
}

void Controller::SetNeutralSample(int x, int y) {
    std::println("Setting Neutral balance");

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto execute = [negative, x, y]() {
            negative->setNeutralByCoords(x, y);
        };

        // FIX ME
        auto undo = [negative]() {
        };

        this->history.addCommand(std::make_unique<Command_Lambda>(execute, undo));
        this->updatePreview(false);
        this->updateSharpnessPreview();
    }
}

void Controller::SliderChangeSetRBalance(float value) {
    std::println("R balance slider value was changed to {}", value);

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto setter = [negative](float v) -> float { return negative->setRBalance(v); };
        auto getter = [negative]() -> float { return negative->getRBalance(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
        negative->renderDraggingEdits();
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
        negative->renderDraggingEdits();
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
        negative->renderDraggingEdits();
        this->updatePreview(this->uiState.isDragging);
    }
}

void Controller::CheckboxPressHoldSharpening(bool checked) {
    std::println("hold sharpening pressed");
    bool previousChecked = !checked;

    auto execute = [this, checked]() {
        if (checked) {
            this->model.holdSettings(true);
        }
        else {
            if (!this->uiState.somethingIsHolding()) {
                this->model.holdSettings(false);
            }
        }
        this->uiState.heldSharpening = checked;
    };

    auto undo = [this, previousChecked]() {
        if (previousChecked) {
            this->model.holdSettings(true);
        }
        else {
            if (!this->uiState.somethingIsHolding()) {
                this->model.holdSettings(false);
            }
        }
        this->uiState.heldSharpening = previousChecked;
    };

    this->history.addCommand(
        std::make_unique<Command_Lambda>(execute, undo)
    );
}

void Controller::ButtonPressResetSharpening()
{
}

void Controller::SliderChangeSetSharpeningAmount(float value) {
    std::println("Sharpening amount slider value was changed to {}", value);

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto setter = [negative](float v) -> float { return negative->setSharpeningAmount(v); };
        auto getter = [negative]() -> float { return negative->getSharpeningAmount(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
    }
}

void Controller::SliderChangeSetSharpeningDiameter(float value) {
    std::println("Sharpening diameter slider value was changed to {}", value);

    Negative* negative = this->model.getCurrentNegative();
    if (negative) {

        auto setter = [negative](float v) -> float { return negative->setSharpeningDiameter(v); };
        auto getter = [negative]() -> float { return negative->getSharpeningDiameter(); };

        this->history.addCommand(std::make_unique<Command_SetValue>(this->model, value, setter, getter));
    }
}

// EXPORTING
// ----------------------------------------------------------------------------------------------------------------

void Controller::ButtonPressExport() {
    std::println("button pressed export");
    this->view.setPopUp("exportSettings", true);
}

void Controller::ButtonPressExportCurrent() {
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {
        pfd::save_file fileSaver("Choose positive location", "/");
        std::filesystem::path path = fileSaver.result();
        std::println("save path is {}", path.string());
        negative->exportPositive(path, this->uiState.exportFileFormat, "sRGB");
    }
}

void Controller::ButtonPressExportAll() {
    std::vector<Negative>& negatives = this->model.getAllNegatives();
    if (negatives.size() > 0) {
        pfd::save_file fileSaver("Choose positive location", "/");
        std::filesystem::path path = fileSaver.result();
        std::println("save path is {}", path.string());
        for (auto& negative : negatives) {

            std::filesystem::path currentPath = path;
            std::string name = currentPath.filename();
            std::string newName = std::format("{}_{}", name, negative.getId());
            currentPath.replace_filename(newName);

            negative.exportPositive(currentPath, this->uiState.exportFileFormat, "sRGB");
        }
    }
}

void Controller::ButtonPressExportCancel() {
    this->view.setPopUp("exportSettings", false);
}

void Controller::OptionPressImageFormat(std::string format) {
    std::println("Export image format is {}", format);
    this->uiState.exportFileFormat = format;
}

// EVENT LISTENER
// ----------------------------------------------------------------------------------------------------------------

void Controller::ProcessEvent(Rml::Event& event) {
    if (!this->uiState.disableCallbacks) {
        Rml::Element* element = event.GetTargetElement();
        const std::string id = element->GetId();

        // BUTTONS & CHECKBOXES (Click)
        if (event.GetId() == Rml::EventId::Click) {

            auto checkedAttribute = element->GetAttribute("checked");
            bool checked = false;
            if (checkedAttribute) {
                checked = true;
            }
            // This is required since the value is still false, but will turn true during this press
            checked = !checked;

            // IMPORT EXPORT REMOVE
            if (id == "import") {
                this->ButtonPressAddNegative();
            }
            else if (id == "export") {
                this->ButtonPressExport();
            }
            else if (id == "exportCancel") {
                this->ButtonPressExportCancel();
            }
            else if (id == "exportCurrent") {
                this->ButtonPressExportCurrent();
            }
            else if (id == "exportAll") {
                this->ButtonPressExportAll();
            }
            else if (id == "remove") {
                Negative* negative = this->model.getCurrentNegative();
                if (negative) {
                    int id = negative->getId();
                    this->ButtonPressRemoveNegative(id);
                }
            }

            // FILM ROLL
            else if (id.contains("thumbnail")) {
                int thumbnailId = 0;
                size_t pos = id.find('_'); // find underscore
                if (pos != std::string::npos) {
                    thumbnailId = std::stoi(id.substr(pos + 1)); // substring after underscore
                }
                this->ButtonPressThumbnail(thumbnailId);
                std::println("pressed thumbnail {}", thumbnailId);
            }
            else if (id == "next") {
                this->ButtonPressNextNegative();
            }
            else if (id == "previous") {
                this->ButtonPressPreviousNegative();
            }

            // ORIENTATION
            else if (id == "rotateClock") {
                this->ButtonPressRotateClock();
            }
            else if (id == "rotateCounterClock") {
                this->ButtonPressRotateCounterClock();
            }
            else if (id == "flipHorizontal") {
                this->ButtonPressFlipHorizontal();
            }
            else if (id == "flipVertical") {
                this->ButtonPressFlipVertical();
            }

            // PRE-CONVERT
            else if (id == "convert") {
                this->ButtonPressConvert();
            }
            else if (id == "holdConversion") {
                this->CheckboxPressHoldConvert(checked);
            }
            else if (id == "resetConversion") {
                this->ButtonPressResetConversion();
            }
            else if (id == "border") {
                this->CheckboxPressBorder(checked);
            }
            else if (id == "sampleBorder") {
                this->ButtonPressSetBorder();
            }
            else if (id == "densest") {
                this->CheckboxPressDensest(checked);
            }
            else if (id == "sampleDensest") {
                this->ButtonPressSetDensest();
            }
            else if (id == "scanArea") {
                this->CheckboxPressScanArea(checked);
            }
            else if (id == "setScanArea") {
                this->ButtonPressSetScanArea();
            }

            // POST-CONVERT
            else if (id == "holdEdits") {
                this->CheckboxPressHoldEdits(checked);
            }
            else if (id == "resetEdits") {
                this->ButtonPressResetEdits();
            }

            else if (id == "holdColor") {
                this->CheckboxPressHoldColor(checked);
            }
            else if (id == "resetColor") {
                this->ButtonPressResetColor();
            }
            else if (id == "autoWB") {
                this->CheckboxPressAutoWhiteBalance(checked);
            }
            else if (id == "selectNeutral") {
                this->ButtonPressSetNeutralSample();
            }

            else if (id == "holdIntensity") {
                this->CheckboxPressHoldIntensity(checked);
            }
            else if (id == "resetIntensity") {
                this->ButtonPressResetIntensity();
            }

            else if (id == "holdSharpening") {
                this->CheckboxPressHoldSharpening(checked);
            }
            else if (id == "resetSharpening") {
                this->ButtonPressResetSharpening();
            }
        }

        // SLIDERS (double click)
        // THIS IS BROKEN ATM
        if (event.GetId() == Rml::EventId::Dblclick) {
            std::println("double click");
            // Reset sliders
            if (element->GetTagName() == "input" && element->GetAttribute("type")->Get("") == "range") {
                std::println("slider double click");
                this->SliderReset(id);
            }
        }

        // CHANGE
        else if (event.GetId() == Rml::EventId::Change) {

            Rml::Variant* valueVar = element->GetAttribute("value");
            if (!valueVar) return;

            float value = valueVar->Get<float>(0.0f);
            std::string stringValue = valueVar->Get<std::string>("standard");

            // DROP-DOWNS
            if (element->GetTagName() == "select") {

                    if (id == "scanGamma") {
                        this->OptionPressSetScanGamma(value);
                    }
                    else if (id == "preset") {
                        this->OptionPressSetPreset(stringValue);
                    }
                    else if (id == "imageFormat") {
                        this->OptionPressImageFormat(stringValue);
                    }
            }
            // SLIDERS
            else if (element->GetTagName() == "input") {

                this->uiState.isDragging = true;

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
                else if (id == "sharpeningAmount") {
                    this->SliderChangeSetSharpeningAmount(value);
                }
                else if (id == "sharpeningDiameter") {
                    this->SliderChangeSetSharpeningDiameter(value);
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
        RmlSFML::InputHandler(this->view.getRmlContextPopups(), *event);
        RmlSFML::InputHandler(this->view.getRmlContextUi(), *event);

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
        // Command C
        else if (keyPressed->system && keyPressed->code == sf::Keyboard::Key::C) {
            this->CheckboxPressHoldConvert(true);
            this->CheckboxPressHoldEdits(true);
            this->view.setCheckboxValue("holdConversion", true);
            this->view.setCheckboxValue("holdEdits", true);
        }
        // Command V
        else if (keyPressed->system && keyPressed->code == sf::Keyboard::Key::V) {
            this->applyHeld();
        }
        return true;
    }
    else {
        return false;
    }
}

bool Controller::handleMouseEvents(std::optional<sf::Event> event) {

    int previewWidth = 0;
    int previewHeight = 0;
    Negative* negative = this->model.getCurrentNegative();
    if (negative) {
        previewWidth = negative->getWorkingWidth();
        previewHeight = negative->getWorkingHeight();
    }

    // MOUSE PRESSED
    if (auto mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {

        // Make sure we accept only clicks on the preview area
        ImageArea previewArea = this->view.getPreviewArea();
        bool clickingPreviewX = mousePressed->position.x > previewArea.left && mousePressed->position.x < previewArea.right;
        bool clickingPreviewY = mousePressed->position.y > previewArea.top && mousePressed->position.y < previewArea.bottom;
        bool clickingPreview = clickingPreviewX && clickingPreviewY;

        if (this->uiState.readyToSelect && clickingPreview) {

            // Get the mouse position in the actual image coordinates
            auto [textureMouseX, textureMouseY] = this->view.previewCoordsToTextureCoords(mousePressed->position.x, mousePressed->position.y);
            int correctedMouseX = std::max(0, std::min(textureMouseX, previewWidth));
            int correctedMouseY = std::max(0, std::min(textureMouseY, previewHeight));
            
            // Selecting only a sample point
            if (this->uiState.selectingBorder || this->uiState.selectingDensest || this->uiState.selectingNeutral) {
                if (this->uiState.selectingBorder) {
                    this->SetBorder( correctedMouseX, correctedMouseY);
                }
                else if (this->uiState.selectingDensest) {
                    this->SetDensest( correctedMouseX, correctedMouseY);
                }
                else if (this->uiState.selectingNeutral) {
                    this->SetNeutralSample( correctedMouseX, correctedMouseY);
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
        RmlSFML::InputHandler(this->view.getRmlContextPopups(), *event);
        RmlSFML::InputHandler(this->view.getRmlContextUi(), *event);
        return true;
    }

    // MOUSE MOVED
    else if (auto mouseMoved = event->getIf<sf::Event::MouseMoved>()) {

        if (this->uiState.selecting) {

        auto [correctedMouseX, correctedMouseY] = this->view.previewCoordsToTextureCoords(mouseMoved->position.x, mouseMoved->position.y);

            if (this->uiState.selectingScanArea) {
                ImageArea& scanArea = this->uiState.selectionArea;
                int safetyPadding = 1;

                if (this->uiState.selectingTop) {
                    scanArea.top = std::max(0, std::min(scanArea.bottom-safetyPadding, correctedMouseY));
                }
                else if (this->uiState.selectingBottom) {
                    scanArea.bottom = std::max(scanArea.top+safetyPadding, std::min(previewHeight, correctedMouseY));
                }
                else if (this->uiState.selectingLeft) {
                    scanArea.left = std::max(0, std::min(scanArea.right-safetyPadding, correctedMouseX));
                }
                else if (this->uiState.selectingRight) {
                    scanArea.right = std::max(scanArea.left+safetyPadding, std::min(previewWidth, correctedMouseX));
                }
                else if (this->uiState.selectingWhole) {
                    int left   = std::min(this->uiState.selectionStart.x, correctedMouseX);
                    int top    = std::min(this->uiState.selectionStart.y, correctedMouseY);
                    int right  = std::max(this->uiState.selectionStart.x, correctedMouseX);
                    int bottom = std::max(this->uiState.selectionStart.y, correctedMouseY);
                    scanArea.left = std::max(0, std::min(left, previewWidth));
                    scanArea.top = std::max(0, std::min(top, previewHeight));
                    scanArea.right = std::max(0, std::min(right, previewWidth));
                    scanArea.bottom = std::max(0, std::min(bottom, previewHeight));
                }
                this->view.setSelection(scanArea);
                Negative* negative = this->model.getCurrentNegative();
                if (negative) {
                    std::println("setting scanArea to left {}, top {}, right {}, bottom {}", scanArea.left, scanArea.top, scanArea.right, scanArea.bottom);
                    float scale = negative->getWorkingScale();
                    std::println("setScanArea scale = {}", scale);
                    negative->setScanArea(scanArea, scale);
                }
            }
        }

        // Hand the event over to the RmlUi context.
        RmlSFML::InputHandler(this->view.getRmlContextPopups(), *event);
        RmlSFML::InputHandler(this->view.getRmlContextUi(), *event);
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
                negative->renderWorkingEdits();
                negative->renderSharpnessPreviewEdits();
                int id = negative->getId();
                negative->renderThumbnail();
                ImageData thumbnail = negative->getThumbnail();
                this->view.updateThumbnail(createPreviewtexture(thumbnail), id);
            }

            this->updatePreview(false);
            this->updateSharpnessPreview();
        }

        // Hand the event over to the RmlUi context.
        RmlSFML::InputHandler(this->view.getRmlContextPopups(), *event);
        RmlSFML::InputHandler(this->view.getRmlContextUi(), *event);
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
            RmlBackend::Resize(this->view.getRmlContextPopups());
            RmlBackend::Resize(this->view.getRmlContextUi());
            this->view.getRmlContextPopups()->Update();
            this->view.getRmlContextUi()->Update();
            this->view.updatePreviewSize();
            this->view.updatePreviewScale();
            this->view.updatePreviewPos();
            this->view.updateFilmRollRenderArea();
            this->view.updateSettingsRenderArea();
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
            RmlSFML::InputHandler(this->view.getRmlContextPopups(), *event);
            RmlSFML::InputHandler(this->view.getRmlContextUi(), *event);
        }
    }
}