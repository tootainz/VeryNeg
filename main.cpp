#include <iostream>
#include <filesystem>
#include <vector>

#include <OpenImageIO/imageio.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

#include <RmlUi/Core.h>
#include <RmlUi_Platform_SFML.h>
#include <RmlUi_Renderer_GL2.h>
#include "rmlui-backend/RmlUi_Backend.h"

#include "Negative/Negative.hpp"
#include "View/View.hpp"
#include "Controller/Controller.hpp"
#include "Model/Model.hpp"

int main() {
    // SETUP
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 0; // 0 = no anti-aliasing, higher values = more smoothing
    sf::RenderWindow window(sf::VideoMode({1000, 1000}), "Very Negative Image Editor", sf::State::Windowed, settings);
    Model model;
    View view(window);
    Controller controller(window, view, model);

    Backend::Initialize("VeryNeg", 800, 800, true);

    Rml::SetRenderInterface(Backend::GetRenderInterface());
    Rml::SetSystemInterface(Backend::GetSystemInterface());


    if (!Rml::Initialise()) {
        std::cout << "RmlUi failed to initialise\n";
        return -1;
    }

    Rml::Context* context = Rml::CreateContext("default", Rml::Vector2i(800, 800));
    context->SetDimensions(Rml::Vector2i(800, 800));
    bool success = Rml::LoadFontFace("assets/oceert_pixel.otf");
    Rml::ElementDocument* document = context->LoadDocument("assets/hello.rml");
    if (document)
        document->Show();

    // MAIN LOOP
    while (Backend::ProcessEvents(context))
    {
        Backend::BeginFrame();

        context->Update();
        context->Render();   // REQUIRED

        Backend::PresentFrame();
    }

    // MAIN LOOP
    controller.mainLoop();
}