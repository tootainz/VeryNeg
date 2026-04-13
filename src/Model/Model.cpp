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
