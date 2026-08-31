#include <OpenImageIO/imageio.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <RmlUi/Core.h>
#include <RmlUi_Platform_SFML.h>
#include <RmlUi_Renderer_GL2.h>

#include "RmlUi_Backend/RmlUi_Backend.hpp"

#include "Negative/Negative.hpp"
#include "View/View.hpp"
#include "Controller/Controller.hpp"
#include "Model/Model.hpp"
#include "debug_print.hpp"


/*

VeryNeg v0.4.0
By Joonatan Koponen

VeryNeg is an image editor for converting scanned film negative files
to beautiful positive digital photographs.

VeryNeg performs a fully automatic conversion but also allows manual setting of some parameters
You can also apply edits to the image after the conversion to edit color balance, density, sharpness etc.

Currently the only supported files are RGB 16bit tiff file scans from a flatbed scanner

*/ 

int main() {

    // SETUP
    // ------------------------------------------------------------------------------------------------------

    // SFML window
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 0; // 0 = no anti-aliasing, 4 = standard, 8 = better'
    settings.stencilBits = 8;
    sf::RenderWindow window(sf::VideoMode({1600, 950}), "VeryNeg", sf::State::Windowed, settings);

    // RmlUi
    RmlBackend::Initialize(window, true);
    Rml::SetRenderInterface(RmlBackend::GetRenderInterface());
    Rml::SetSystemInterface(RmlBackend::GetSystemInterface());
    if (!Rml::Initialise()) {
        DEBUG_PRINT("RmlUi failed to initialise");
        return -1;
    }

    // Application logic following MVC pattern
    Model model;
    if (!model.getWasConstructed()) {
        return -1;
    }
    View view(window);
    Controller controller(window, view, model);


    // MAIN LOOP
    // ------------------------------------------------------------------------------------------------------

    controller.mainLoop();


    // CLEANUP
    // ------------------------------------------------------------------------------------------------------

    controller.cleanup();
    Rml::Shutdown();
    RmlBackend::Shutdown();

    return 1;
}
