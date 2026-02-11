#pragma once

#include <iostream>
#include <vector>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "../imageAlgorithms/getExtremePixels.hpp"
#include "../imageAlgorithms/levels.hpp"

struct ImageData {
    std::vector<uint8_t> data;
    int width;
    int height;
};

class Negative {

    public:
    std::vector<float> pixels;
    std::vector<float> editedPixels;
    int width;
    int height;
    int numberOfChannels;
    
    Negative(std::string imagePath) {
        auto input = OIIO::ImageInput::open(imagePath);

        if (!input) {
            std::cout << "Failed to open file" << std::endl;
            return;
        }
        
        const OIIO::ImageSpec& spec = input->spec();
        this->width = spec.width;
        this->height = spec.height;
        this->numberOfChannels = spec.nchannels;
        this->pixels.resize(this->width * this->height * this->numberOfChannels);
        input->read_image(0, 0, 0, this->numberOfChannels, OIIO::TypeDesc::FLOAT, &this->pixels[0]);
        this->editedPixels = this->pixels;

        // Prints handy knowledge about the image
        std::cout << "Read a file and created a Negative object" << std::endl;
        std::cout << "This image has the following data" << std::endl;
        std::string metadata = spec.serialize(OIIO::ImageSpec::SerialText, OIIO::ImageSpec::SerialDetailedHuman);
        std::cout << metadata << std::endl;
        
        input->close();
        return;
    }
    
    // Returns a preview to show in the GUI in the form of ImageData. This will be shown with SFML, The colors are assumed to be sRGB in the preview
    ImageData getPreview() {
        std::cout << "generating preview" << std::endl;

        // Use OIIO::ImageBuf for ease of scaling resizing etc
        OIIO::ImageSpec originalSpec(this->width, this->height, this->numberOfChannels, OIIO::TypeDesc::FLOAT);
        OIIO::ImageBuf originalBuf(originalSpec, this->editedPixels.data());

        // Make sure the data is in RGBA format since the preview will have to be in RGBA format for SFML
        if (this->numberOfChannels < 4) {
            std::cout << "changed channels to 4" << std::endl;
            // This is straight from OIIO, it adds an aplha channel to the image
            originalBuf = OIIO::ImageBufAlgo::channels(originalBuf, 4, { 0, 1, 2, -1 },
                                        { 0 /*ignore*/, 0 /*ignore*/, 0 /*ignore*/,
                                            1.0 },
                                        { "", "", "", "A" });
        }

        // Calculate new dimensions for the preview
        int previewWidth = this->width*0.3;
        int previewHeight = this->height*0.3;

        // Change the underlying bit depth of the image to 8bit
        OIIO::ImageBuf uint8Buf = OIIO::ImageBufAlgo::copy(originalBuf, OIIO::TypeDesc::UINT8);
        std::cout << "changed to uint8" << std::endl;

        // Resize the image for the preview
        OIIO::ROI roi(0, previewWidth, 0, previewHeight, 0, 1, /*chans:*/ 0, uint8Buf.nchannels());
        OIIO::ImageBuf previewBuf = OIIO::ImageBufAlgo::resample(uint8Buf, true, roi);
        std::cout << "resized resolution to width: " << previewWidth << " height: " << previewHeight << std::endl;
        
        // Extact the preview data from the ImageBuf
        std::vector<uint8_t> previewData;
        previewData.resize(previewWidth*previewHeight*4);
        previewBuf.get_pixels(OIIO::ROI::All(), OIIO::TypeDesc::UINT8, previewData.data());
        std::cout << "generated preview data" << std::endl;

        // Return an ImageData struct
        return {
            previewData,
            previewWidth,
            previewHeight
        };
    }

    // Renders the image with the given pipeline
    void render() {

        this->editedPixels = this->pixels;
        
        std::cout << "starting image processign pipeline" << std::endl;

        std::cout << "getting the brightest and darkest pixels" << std::endl;

        std::tuple<float, float, float> brightest = getBrightestPixel(this->pixels);
        std::tuple<float, float, float> darkest = getDarkestPixel(this->pixels);

        auto [r, g, b] = brightest;
        std::cout << "The brightest pixel in the image is: " << r << ", " << g << ", " << b << std::endl;
        auto [r2, g2, b2] = darkest;
        std::cout << "The darkest pixel in the image is: " << r2 << ", " << g2 << ", " << b2 << std::endl;

        std::cout << "applying levels adjustment" << std::endl;

        levelsRGB(this->editedPixels, r2, r, 0.0, 1.0);

        return;
    }
};