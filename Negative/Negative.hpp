#pragma once

#include <iostream>
#include <vector>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "../imageAlgorithms/getExtremePixels.hpp"
#include "../imageAlgorithms/levels.hpp"
#include "../imageAlgorithms/crudeInversion.hpp"
#include "../imageAlgorithms/densityInvert.hpp"
#include "../imageAlgorithms/gamma.hpp"

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
        this->initializeNegative(imagePath);
        return;
    }

    Negative() {};

    bool initializeNegative(std::string imagePath) {
        auto input = OIIO::ImageInput::open(imagePath);

        if (!input) {
            std::cout << "Failed to open file" << std::endl;
            return false;
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
        return true;
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

        std::cout << "Performing a density inversion" << std::endl;
        densityInvert(this->editedPixels);

        std::cout << "getting the brightest and darkest pixels" << std::endl;

        // // RGB
        // std::tuple<float, float, float> brightest = getBrightestPixel(this->pixels, MaxChannel::RGB);
        // std::tuple<float, float, float> darkest = getDarkestPixel(this->pixels, MaxChannel::RGB);

        // R
        std::tuple<float, float, float> brightestR = getBrightestPixel(this->editedPixels, MaxChannel::R);
        std::tuple<float, float, float> darkestR = getDarkestPixel(this->editedPixels, MaxChannel::R);
        // G
        std::tuple<float, float, float> brightestG = getBrightestPixel(this->editedPixels, MaxChannel::G);
        std::tuple<float, float, float> darkestG = getDarkestPixel(this->editedPixels, MaxChannel::G);
        // B
        std::tuple<float, float, float> brightestB = getBrightestPixel(this->editedPixels, MaxChannel::B);
        std::tuple<float, float, float> darkestB = getDarkestPixel(this->editedPixels, MaxChannel::B);
    
        std::cout << "applying levels adjustment for the r channel" << std::endl;
        levelsR(this->editedPixels, std::get<0>(darkestR), std::get<0>(brightestR), 0.0, 1.0);

        std::cout << "applying levels adjustment for the g channel" << std::endl;
        levelsG(this->editedPixels, std::get<1>(darkestG), std::get<1>(brightestG), 0.0, 1.0);

        std::cout << "applying levels adjustment for the b channel" << std::endl;
        levelsB(this->editedPixels, std::get<2>(darkestB), std::get<2>(brightestB), 0.0, 1.0);

        // std::cout << "applying gamma correction to the b channel" << std::endl;
        // gamma(this->editedPixels, 0.65, GammaChannel::B);

        // std::cout << "applying gamma correction to the g channel" << std::endl;
        // gamma(this->editedPixels, 0.85, GammaChannel::G);
        
        // std::cout << "Performing a crude inversion" << std::endl;
        // crudeInversion(this->editedPixels);

        // std::cout << "applying general display gamma correction" << std::endl;
        // gamma(this->editedPixels, 1.7, GammaChannel::RGB);

        return;
    }
};