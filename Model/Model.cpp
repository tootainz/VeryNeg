#include "Model.hpp"

#include <algorithm>

Model::Model() {}

void Model::renderWorking() {
    this->negatives[this->currentNegativeIndex].renderWorking();
}
void Model::renderEdits() {
    this->negatives[this->currentNegativeIndex].renderEdits();
}

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

void Model::exportPositive(std::string imagePath) {

}

void Model::setExposure(float value) {
    this->negatives[this->currentNegativeIndex].setExposure(value);
}

void Model::setRBalance(float value) {
    this->negatives[this->currentNegativeIndex].setRBalance(value);
}

void Model::setGBalance(float value) {
    this->negatives[this->currentNegativeIndex].setGBalance(value);
}

void Model::setBBalance(float value) {
    this->negatives[this->currentNegativeIndex].setBBalance(value);
}

float Model::getExposure() {
    return this->negatives[this->currentNegativeIndex].getExposure();
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