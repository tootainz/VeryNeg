#include "Model.hpp"

#include <algorithm>

// CONSTRUCTOR
// ----------------------------------------------------------------------------------------------------------------

Model::Model() {}


// RENDER FUNCTIONS FOR IMAGE DATA
// ----------------------------------------------------------------------------------------------------------------

void Model::renderWorking() {
    this->negatives[this->currentNegativeIndex].renderWorking();
}

void Model::renderEdits() {
    this->negatives[this->currentNegativeIndex].renderEdits();
}

void Model::renderFinal() {
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
    std::println("current negative is: {}", this->currentNegativeIndex);
}

Negative& Model::addNegative(std::filesystem::path imagePath) {
    this->negatives.push_back(Negative(imagePath));
    if (negatives.size() == 1) currentNegativeIndex = 0;
    else currentNegativeIndex += 1;
    return this->negatives[this->currentNegativeIndex];
}

void Model::removeNegative(int i) {
}


// EXPORT
// ----------------------------------------------------------------------------------------------------------------

void Model::exportPositive(std::string imagePath) {
}


// PRE-CONVERT
// ----------------------------------------------------------------------------------------------------------------

float Model::setScanGamma(float value) {
    this->negatives[this->currentNegativeIndex].setScanGamma(value);
    return this->getScanGamma();
}

void Model::setBorder() {
}

void Model::setDensest() {
}

void Model::setScanArea() {
}

void Model::convert() {
    this->renderWorking();
}

void Model::resetConversion() {
    this->negatives[this->currentNegativeIndex].resetWorking();
}


// POST-CONVERT
// ----------------------------------------------------------------------------------------------------------------

float Model::setDensity(float value) {
    this->negatives[this->currentNegativeIndex].setDensity(value);
    return this->getDensity();
}

float Model::setContrast(float value) {
    this->negatives[this->currentNegativeIndex].setContrast(value);
    return this->getContrast();
}

float Model::setWhites(float value) {
    this->negatives[this->currentNegativeIndex].setWhites(value);
    return this->getWhites();
}

float Model::setHighlights(float value) {
    this->negatives[this->currentNegativeIndex].setHighlights(value);
    return this->getHighlights();
}

float Model::setShadows(float value) {
    this->negatives[this->currentNegativeIndex].setShadows(value);
    return this->getShadows();
}

float Model::setBlacks(float value) {
    this->negatives[this->currentNegativeIndex].setBlacks(value);
    return this->getBlacks();
}

void Model::autoWhiteBalance() {
}

void Model::chooseNeutralBalance() {
}

float Model::setRBalance(float value) {
    this->negatives[this->currentNegativeIndex].setRBalance(value);
    return this->getRBalance();
}

float Model::setGBalance(float value) {
    this->negatives[this->currentNegativeIndex].setGBalance(value);
    return this->getGBalance();
}

float Model::setBBalance(float value) {
    this->negatives[this->currentNegativeIndex].setBBalance(value);
    return this->getBBalance();
}


// GETTERS
// ----------------------------------------------------------------------------------------------------------------

float Model::getScanGamma() {
    return this->negatives[this->currentNegativeIndex].getScanGamma();
}

float Model::getDensity() {
    return this->negatives[this->currentNegativeIndex].getDensity();
}

float Model::getContrast() {
    return this->negatives[this->currentNegativeIndex].getContrast();
}

float Model::getWhites() {
    return this->negatives[this->currentNegativeIndex].getWhites();
}

float Model::getHighlights() {
    return this->negatives[this->currentNegativeIndex].getHighlights();
}

float Model::getShadows() {
    return this->negatives[this->currentNegativeIndex].getShadows();
}

float Model::getBlacks() {
    return this->negatives[this->currentNegativeIndex].getBlacks();
}

float Model::getRBalance() {
    return this->negatives[this->currentNegativeIndex].getRBalance();
}

float Model::getGBalance() {
    return this->negatives[this->currentNegativeIndex].getGBalance();
}

float Model::getBBalance() {
    return this->negatives[this->currentNegativeIndex].getBBalance();
}

ImageData Model::getPreview() {
    return this->negatives[this->currentNegativeIndex].getPreview();
}

ImageData Model::getThumbnail(int id) {
    return ImageData();
}

int Model::getCurrentNegativeId() {
    return this->negatives[this->currentNegativeIndex].getId();
}
