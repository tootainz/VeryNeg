#include "Model.hpp"

#include <algorithm>
#include <filesystem>
#include <print>


// CONSTRUCTOR
// ----------------------------------------------------------------------------------------------------------------

Model::Model() {
    std::filesystem::create_directories("./cache");
}


// NEGATIVE NAVIGATION
// ----------------------------------------------------------------------------------------------------------------

void Model::changeCurrentNegativeById(int id) {

    auto iterator = std::find_if(this->negatives.begin(), this->negatives.end(), [&id](Negative& negative) {
        return negative.getId() == id;
    });

    if (iterator != this->negatives.end()) {
        int index = std::distance(this->negatives.begin(), iterator);
        this->currentNegativeIndex = index;
    }
    std::println("current negative is: {}", this->currentNegativeIndex);
}

void Model::previousNegative() {
    this->currentNegativeIndex = std::max(0, this->currentNegativeIndex - 1);
    std::println("current negative is: {}", this->currentNegativeIndex);
}

void Model::nextNegative() {
    int size = this->negatives.size();
    this->currentNegativeIndex = std::min(size - 1, this->currentNegativeIndex + 1);
    this->currentNegativeIndex = std::max(0, this->currentNegativeIndex); // make sure the index is not negative
    std::println("current negative is: {}", this->currentNegativeIndex);
}


// NEGATIVE IO
// ----------------------------------------------------------------------------------------------------------------

// Returns a raw pointer since std::Optional<T&> is apparently not allowed
// The return value can either be nulptr or contain the negative.
// NEVER call delete on these pointers, they do not own the negative
Negative* Model::addNegative(std::filesystem::path imagePath) {
    
    this->negatives.push_back(Negative(imagePath));

    if (negatives.size() == 1) currentNegativeIndex = 0;
    else currentNegativeIndex += 1;

    if (this->getCurrentNegative()->wasCreated()) {
        return this->getCurrentNegative();
    }
    else {
        this->removeNegativeById(this->getCurrentNegative()->getId());
        this->currentNegativeIndex = std::max(0, currentNegativeIndex - 1);
        return nullptr;
    }
}

// Returns a raw pointer since std::Optional<T&> is apparently not allowed
// The return value can either be nulptr or contain the negative.
// NEVER call delete on these pointers, they do not own the negative
Negative* Model::addNegative(std::filesystem::path imagePath, int id) {

    this->negatives.push_back(Negative(imagePath, id));

    if (negatives.size() == 1) currentNegativeIndex = 0;
    else currentNegativeIndex += 1;

    if (this->getCurrentNegative()->wasCreated()) {
        return this->getCurrentNegative();
    }
    else {
        this->removeNegativeById(this->getCurrentNegative()->getId());
        this->currentNegativeIndex = std::max(0, currentNegativeIndex - 1);
        return nullptr;
    }
}

void Model::removeNegativeById(int id) {
    auto iterator = std::find_if(this->negatives.begin(), this->negatives.end(), [&id](Negative& negative) {
        return negative.getId() == id;
    });

    // Check if we actually found the negative
    if (iterator != this->negatives.end()) {
        // Remove the negative
        this->negatives.erase(iterator);
        std::println("negative with id {} was deleted", id);
    }

    this->currentNegativeIndex = std::max(0, this->currentNegativeIndex - 1);
}

void Model::holdSettings(bool hold) {
    if (hold) {
        std::println("settings held");
        this->heldNegative = this->getCurrentNegative();
    }
    else {
        std::println("settings unheld");
        this->heldNegative = nullptr;
    }
}

void Model::applyHoldPreConvert() {

    Negative* negative = this->getCurrentNegative();
    if (negative && this->heldNegative) {

        std::println("applying held pre convert settings");

        // Get held settings
        float scanGamma = this->heldNegative->getScanGamma();

        ImageArea scanArea = this->heldNegative->getScanArea(this->heldNegative->getWorkingScale());
        bool hasScanArea = this->heldNegative->getHasScanArea();

        bool hasDensest = this->heldNegative->getHasDensest();
        bool hasBorder = this->heldNegative->getHasBorder();
        auto [densestR, densestG, densestB] = this->heldNegative->getDensest();
        auto [borderR, borderG, borderB] = this->heldNegative->getBorder();

        bool isConverted = this->heldNegative->getIsConverted();

        // Apply held settings
        negative->setScanGamma(scanGamma);

        negative->setHasScanArea(hasScanArea);
        if (hasScanArea) {
            negative->setScanArea(scanArea, this->heldNegative->getWorkingScale());
        }

        negative->setHasDensest(hasDensest);
        if (hasDensest) {
            negative->setDensest(densestR, densestG, densestB);
        }
        negative->setHasBorder(hasBorder);
        if (hasBorder) {
            negative->setBorder(borderR, borderG, borderB);
        }

        if (isConverted) negative->convert();
    }
}

void Model::applyHoldPostConvert() {
    this->applyHoldColor();
    this->applyHoldIntensity();
    this->applyHoldSharpening();
}

void Model::applyHoldColor() {
    Negative* negative = this->getCurrentNegative();
    if (negative && this->heldNegative) {

        // Get held settings
        bool autoWB = this->heldNegative->getAutoWB();
        bool hasNeutral = this->heldNegative->getHasNeutral();
        auto [neutralR, neutralG, neutralB] = this->heldNegative->getNeutral();

        float rBalance = this->heldNegative->getRBalance();
        float gBalance = this->heldNegative->getGBalance();
        float bBalance = this->heldNegative->getBBalance();

        float saturation = this->heldNegative->getSaturation();

        // Apply held settings
        negative->setAutoWB(autoWB);

        negative->setHasNeutral(hasNeutral);
        if (hasNeutral) {
            negative->setNeutral(neutralR, neutralG, neutralB);
        }

        negative->setRBalance(rBalance);
        negative->setGBalance(gBalance);
        negative->setBBalance(bBalance);

        negative->setSaturation(saturation);
    }
}

void Model::applyHoldIntensity() {
    Negative* negative = this->getCurrentNegative();
    if (negative && this->heldNegative) {

        // Get held settings
        float density = this->heldNegative->getDensity();
        float contrast = this->heldNegative->getContrast();
        float whites = this->heldNegative->getWhites();
        float highlights = this->heldNegative->getHighlights();
        float shadows = this->heldNegative->getShadows();
        float blacks = this->heldNegative->getBlacks();

        // Get held settings
        negative->setDensity(density);
        negative->setContrast(contrast);
        negative->setWhites(whites);
        negative->setHighlights(highlights);
        negative->setShadows(shadows);
        negative->setBlacks(blacks);
    }
}

void Model::applyHoldSharpening() {
    Negative* negative = this->getCurrentNegative();
    if (negative && this->heldNegative) {

        // Get held settings
        float sharpeningAmount = this->heldNegative->getSharpeningAmount();
        float sharpeningDiameter = this->heldNegative->getSharpeningDiameter();

        // Apply held settings
        negative->setSharpeningAmount(sharpeningAmount);
        negative->setSharpeningDiameter(sharpeningDiameter);
    }
}

// GETTERS
// ----------------------------------------------------------------------------------------------------------------

// Returns a raw pointer since std::Optional<T&> is apparently not allowed
// The return value can either be nulptr or contain the negative.
// NEVER call delete on these pointers, they do not own the negative
Negative* Model::getCurrentNegative() {
    if (this->negatives.size() <= 0) {
        return nullptr;
    }
    else {
        return &this->negatives[this->currentNegativeIndex];
    }
}

// Returns a raw pointer since std::Optional<T&> is apparently not allowed
// The return value can either be nulptr or contain the negative.
// NEVER call delete on these pointers, they do not own the negative
Negative* Model::getNegativeById(int id) {
    auto iterator = std::find_if(this->negatives.begin(), this->negatives.end(), [&id](Negative& negative) {
        return negative.getId() == id;
    });

    // Check if we actually found the negative
    if (iterator != this->negatives.end()) {
        return &*iterator;
    }
    else {
        return nullptr;
    }
}

std::vector<Negative>& Model::getAllNegatives() {
    return this->negatives;
}


// CLEANUP
// ----------------------------------------------------------------------------------------------------------------

void Model::cleanCache(){
    std::filesystem::remove_all("./cache");
}
