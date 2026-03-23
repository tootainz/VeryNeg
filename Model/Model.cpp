#include "Model.hpp"

#include <algorithm>


Model::Model() {}

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

void Model::removeNegativeById(int id) {
    auto iterator = std::find_if(this->negatives.begin(), this->negatives.end(), [&id](Negative& negative) {
        return negative.getId() == id;
    });

    // Check if we actually found the negative
    if (iterator != this->negatives.end()) {
        // Remove the negative
        this->negatives.erase(iterator);
    }
}

Negative& Model::getCurrentNegative() {
    return this->negatives[this->currentNegativeIndex];
}
