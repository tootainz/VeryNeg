#pragma once

#include <vector>
#include <filesystem>

#include <nlohmann/json.hpp>

#include "../Negative/Negative.hpp"
#include "../Negative/ImageArea.hpp"


/**

The Model class

The Model is a part of the classic Model, View, Controller or MVC architecture patter.
Stores all the basic functional data and state of the application.
All of the heavy lifting is done in individual Negative objects.
The Models main purpose is to keep a list of all Negatives that are open in the application
and manage the navigation between them and gettign the Negative currently being edited.

*/

class Model {

private:
    // PRIVATE DATA MEMBERS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // Negatives loaded to the application
    std::vector<Negative> negatives;
    
    // Index of the negative that is currently selected and should be displayed
    int currentNegativeIndex;

    // Place to store the held settings in the editor
    nlohmann::json heldSettings;


public:
    // PUBLIC METHODS
    // ------------------------------------------------------------------------------------------------------------------------------------
    
    // CONSTRUCTOR
    Model();

    // NEGATIVE NAVIGATION
    void changeCurrentNegativeById(int id);
    void previousNegative();
    void nextNegative();
    

    // NEGATIVE IO
    Negative* addNegative(std::filesystem::path imagePath);
    Negative* addNegative(std::filesystem::path imagePath, int id); // IMPORTANT! Call this only when undoing a removeNegativeById
    void removeNegativeById(int id);
    
    // GETTERS
    Negative* getCurrentNegative();
    Negative* getNegativeById(int id);
    std::vector<Negative>& getAllNegatives();
};