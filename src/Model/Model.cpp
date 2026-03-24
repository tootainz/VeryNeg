#include "Model.hpp"

#include <algorithm>


// CONSTRUCTOR
// ----------------------------------------------------------------------------------------------------------------

Model::Model() {}


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

Negative& Model::addNegative(std::filesystem::path imagePath) {
    this->negatives.push_back(Negative(imagePath));
    if (negatives.size() == 1) currentNegativeIndex = 0;
    else currentNegativeIndex += 1;
    return this->negatives[this->currentNegativeIndex];
}

Negative& Model::addNegative(std::filesystem::path imagePath, int id) {
    this->negatives.push_back(Negative(imagePath, id));
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
        std::println("negative with id {} was deleted", id);
    }

    this->currentNegativeIndex = std::max(0, this->currentNegativeIndex - 1);
}


// GETTERS
// ----------------------------------------------------------------------------------------------------------------

std::optional<Negative*> Model::getCurrentNegative() {
    if (this->negatives.size() <= 0) {
        return std::nullopt;
    }
    else {
        return &this->negatives[this->currentNegativeIndex];
    }
}

std::optional<Negative*> Model::getNegativeById(int id) {
    auto iterator = std::find_if(this->negatives.begin(), this->negatives.end(), [&id](Negative& negative) {
        return negative.getId() == id;
    });

    // Check if we actually found the negative
    if (iterator != this->negatives.end()) {
        return &*iterator;
    }
    else {
        return std::nullopt;
    }
}