#pragma once

#include <RmlUi/Core.h>
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <memory>
#include <string>

// ================= SfmlTextureDecorator =================
class SfmlTextureDecorator : public Rml::Decorator
{
public:
    // Store references to the texture and render target
    SfmlTextureDecorator(sf::Texture& tex, sf::RenderTarget& target)
        : texture(tex), render_target(target) {}

    // GenerateElementData is unused, return 0
    Rml::DecoratorDataHandle GenerateElementData(Rml::Element* /*element*/, Rml::BoxArea /*area*/) const override
    {
        return 0;
    }

    void ReleaseElementData(Rml::DecoratorDataHandle /*handle*/) const override {}

    void RenderElement(Rml::Element* element, Rml::DecoratorDataHandle /*handle*/) const override
    {
        const Rml::Box& box = element->GetBox();
        Rml::Vector2f pos = element->GetAbsoluteOffset(Rml::BoxArea::Border);
        Rml::Vector2f size = box.GetSize(Rml::BoxArea::Border);

        sf::Sprite sprite(texture);
        sprite.setPosition(sf::Vector2f(pos.x, pos.y));
        sprite.setScale(sf::Vector2f(size.x / texture.getSize().x, size.y / texture.getSize().y));

        render_target.draw(sprite);
    }

private:
    sf::Texture& texture;
    sf::RenderTarget& render_target;
};

// ================= SfmlTextureDecoratorInstancer =================
class SfmlTextureDecoratorInstancer : public Rml::DecoratorInstancer
{
public:
    SfmlTextureDecoratorInstancer(
        std::unordered_map<std::string, sf::Texture>& textures,
        sf::RenderTarget& target)
        : texture_map(textures), render_target(target)
    {
        // Register a "texture" property for RCSS
        texture_prop = RegisterProperty("texture", "")
                           .AddParser("string")
                           .GetId();
        RegisterShorthand("decorator", "texture", Rml::ShorthandType::FallThrough);
    }

    Rml::SharedPtr<Rml::Decorator> InstanceDecorator(
        const Rml::String& /*name*/,
        const Rml::PropertyDictionary& props,
        const Rml::DecoratorInstancerInterface&) override
    {
        std::string tex_name = props.GetProperty(texture_prop)->Get<std::string>();
        auto it = texture_map.find(tex_name);
        if (it == texture_map.end())
            return nullptr;

        // Pass reference to the decorator, lifetime safe as long as map lives
        return Rml::MakeShared<SfmlTextureDecorator>(it->second, render_target);
    }

private:
    Rml::PropertyId texture_prop;
    std::unordered_map<std::string, sf::Texture>& texture_map;
    sf::RenderTarget& render_target;
};