#pragma once

#include <vector>
#include <filesystem>

#include "../Negative/Negative.hpp"
#include "../Negative/ImageArea.hpp"


class Model {

private:

    // PRIVATE DATA MEMBERS

    /// A Vector that stores all the negatives loaded to the application
    std::vector<Negative> negatives;
    
    /// Stores the index of the negative that is currently selected and should be displayed
    int currentNegativeIndex;


public:

    // CONSTRUCTOR
    Model();

    void changeCurrentNegativeById(int id);
    void previousNegative();
    void nextNegative();

    Negative& addNegative(std::filesystem::path imagePath);
    void removeNegativeById(int id);
    
    Negative& getCurrentNegative();
};