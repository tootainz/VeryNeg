#pragma once

#include "RmlUi_Backend.h"
#include <RmlUi_Platform_SFML.h>
#include <RmlUi_Renderer_GL2.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Profiling.h>
#include <RmlUi/Debugger/Debugger.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdint>

#if SFML_VERSION_MAJOR >= 3
	#define SFML_COORDINATE(x, y) {x, y}
#else
	#define SFML_COORDINATE(x, y) x, y
#endif

/**
    Custom render interface example for the SFML/GL2 backend.

    Overloads the OpenGL2 render interface to load textures through SFML's built-in texture loading functionality.
 */
class RenderInterface_GL2_SFML : public RenderInterface_GL2 {
public:
	// -- Inherited from Rml::RenderInterface --

	void RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) override
	{
		if (texture)
		{
			sf::Texture::bind((sf::Texture*)texture);
			texture = RenderInterface_GL2::TextureEnableWithoutBinding;
		}

		RenderInterface_GL2::RenderGeometry(handle, translation, texture);
	}

	Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override
	{
		Rml::FileInterface* file_interface = Rml::GetFileInterface();
		Rml::FileHandle file_handle = file_interface->Open(source);
		if (!file_handle)
			return false;

		file_interface->Seek(file_handle, 0, SEEK_END);
		size_t buffer_size = file_interface->Tell(file_handle);
		file_interface->Seek(file_handle, 0, SEEK_SET);

		using Rml::byte;
		Rml::UniquePtr<byte[]> buffer(new byte[buffer_size]);
		file_interface->Read(buffer.get(), buffer_size, file_handle);
		file_interface->Close(file_handle);

		sf::Image image;
		if (!image.loadFromMemory(buffer.get(), buffer_size))
			return false;

		// Convert colors to premultiplied alpha, which is necessary for correct alpha compositing.
		for (unsigned int x = 0; x < image.getSize().x; x++)
		{
			for (unsigned int y = 0; y < image.getSize().y; y++)
			{
				sf::Color color = image.getPixel(SFML_COORDINATE(x, y));
				color.r = static_cast<std::uint8_t>((color.r * color.a) / 255);
				color.g = static_cast<std::uint8_t>((color.g * color.a) / 255);
				color.b = static_cast<std::uint8_t>((color.b * color.a) / 255);
				image.setPixel(SFML_COORDINATE(x, y), color);
			}
		}

		sf::Texture* texture = new sf::Texture();
		texture->setSmooth(true);

		if (!texture->loadFromImage(image))
		{
			delete texture;
			return false;
		}

		texture_dimensions = Rml::Vector2i(texture->getSize().x, texture->getSize().y);
		return (Rml::TextureHandle)texture;
	}

	Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions_i) override
	{
		const auto source_dimensions = Rml::Vector2<unsigned int>(source_dimensions_i);

#if SFML_VERSION_MAJOR >= 3
		sf::Texture* texture = new sf::Texture(sf::Vector2u{source_dimensions.x, source_dimensions.y});
#else
		sf::Texture* texture = new sf::Texture();
		if (!texture->create(source_dimensions.x, source_dimensions.y))
		{
			delete texture;
			return false;
		}
#endif
		texture->setSmooth(true);
		texture->update(source.data(), SFML_COORDINATE(source_dimensions.x, source_dimensions.y), SFML_COORDINATE(0, 0));
		return (Rml::TextureHandle)texture;
	}

	void ReleaseTexture(Rml::TextureHandle texture_handle) override { delete (sf::Texture*)texture_handle; }
};