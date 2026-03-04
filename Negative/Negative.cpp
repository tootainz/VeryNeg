#include "Negative.hpp"

#include <iostream>
#include <print>
#include <format>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "../imageAlgorithms/getExtremePixels.hpp"
#include "../imageAlgorithms/levels.hpp"
#include "../imageAlgorithms/crudeInversion.hpp"
#include "../imageAlgorithms/densityInvert.hpp"
#include "../imageAlgorithms/gamma.hpp"
#include "../imageAlgorithms/myExposure.hpp"
#include "../imageAlgorithms/charCurves.hpp"
#include "../imageAlgorithms/grayWorld.hpp"
#include "../imageAlgorithms/multiply.hpp"

Negative::Negative(std::string imagePath) {
    this->initializeNegative(imagePath);
    return;
}

Negative::Negative() {};

bool Negative::initializeNegative(std::string imagePath) {
    auto input = OIIO::ImageInput::open(imagePath);

    if (!input) {
        std::println("Failed to open file");
        return false;
    }
    
    const OIIO::ImageSpec& spec = input->spec();
    this->width = spec.width;
    this->height = spec.height;
    this->numberOfChannels = spec.nchannels;
    this->pixels.resize(this->width * this->height * this->numberOfChannels);
    input->read_image(0, 0, 0, this->numberOfChannels, OIIO::TypeDesc::FLOAT, &this->pixels[0]);
    this->convertedPixels = this->pixels;
    this->editedPixels = this->pixels;
    std::println("Opened negative successfully");

    // Prints handy knowledge about the image
    std::println("Read a file and created a Negative object");
    std::println("This image has the following data");
    std::string metadata = spec.serialize(OIIO::ImageSpec::SerialText, OIIO::ImageSpec::SerialDetailedHuman);
    std::println("{}", metadata);
    
    input->close();
    return true;
}

bool Negative::savePositive(std::string imagePath) {
    std::println("Saving positive");
    std::string fileName = std::format("{}.tif", imagePath);

    // Use OIIO::ImageBuf for ease of transformign the pixel data type
    OIIO::ImageSpec originalSpec(this->width, this->height, this->numberOfChannels, OIIO::TypeDesc::FLOAT);
    OIIO::ImageBuf originalBuf(originalSpec, this->editedPixels.data());

    // For now we want to save images as 16bit
    if (!originalBuf.write(fileName, OIIO::TypeDesc::UINT16)) {
        std::println("Failed to save image");
        return false;
    }
    std::println("Saved positive successfully");
    
    return true;
}

// Returns a preview to show in the GUI in the form of ImageData. This will be shown with SFML, The colors are assumed to be sRGB in the preview
ImageData Negative::getPreview() {
    std::println("generating preview");

    // Use OIIO::ImageBuf for ease of scaling resizing etc
    OIIO::ImageSpec originalSpec(this->width, this->height, this->numberOfChannels, OIIO::TypeDesc::FLOAT);
    OIIO::ImageBuf originalBuf(originalSpec, this->editedPixels.data());

    // Make sure the data is in RGBA format since the preview will have to be in RGBA format for SFML
    if (this->numberOfChannels < 4) {
        std::println("changed channels to 4");
        // This is straight from OIIO, it adds an aplha channel to the image
        originalBuf = OIIO::ImageBufAlgo::channels(originalBuf, 4, { 0, 1, 2, -1 },
                                    { 0 /*ignore*/, 0 /*ignore*/, 0 /*ignore*/,
                                        1.0 },
                                    { "", "", "", "A" });
    }

    // Calculate new dimensions for the preview
    int previewWidth = this->width*0.4;
    int previewHeight = this->height*0.4;

    // Change the underlying bit depth of the image to 8bit
    OIIO::ImageBuf uint8Buf = OIIO::ImageBufAlgo::copy(originalBuf, OIIO::TypeDesc::UINT8);
    std::println("changed to uint8");

    // Resize the image for the preview
    OIIO::ROI roi(0, previewWidth, 0, previewHeight, 0, 1, /*chans:*/ 0, uint8Buf.nchannels());
    OIIO::ImageBuf previewBuf = OIIO::ImageBufAlgo::resample(uint8Buf, true, roi);
    std::println("resized resolution to width: {} height: {}", previewWidth, previewHeight);
    
    // Extact the preview data from the ImageBuf
    std::vector<uint8_t> previewData;
    previewData.resize(previewWidth*previewHeight*4);
    previewBuf.get_pixels(OIIO::ROI::All(), OIIO::TypeDesc::UINT8, previewData.data());

    std::println("generated preview data");

    // Return an ImageData struct
    return {
        previewData,
        previewWidth,
        previewHeight
    };
}

void Negative::setExposure(float value) {
    this->exposure = value;
}

void Negative::renderEdits() {

    this->editedPixels = this->convertedPixels;

    std::println("Editing the image");

    std::println("Editing exposure");
    myExposure(this->editedPixels, this->exposure, EditChannel::RGB);
}

void Negative::render() {

    this->convertedPixels = this->pixels;
    
    std::println("starting negative conversion pipeline");

    std::println("getting the brightest and darkest pixels");

    // R
    std::tuple<float, float, float> transparentsR = getBrightestPixel(this->convertedPixels, EditChannel::R);
    std::tuple<float, float, float> opaquestsR = getDarkestPixel(this->convertedPixels, EditChannel::R);
    // G
    std::tuple<float, float, float> transparentsG = getBrightestPixel(this->convertedPixels, EditChannel::G);
    std::tuple<float, float, float> opaquestsG = getDarkestPixel(this->convertedPixels, EditChannel::G);
    // B
    std::tuple<float, float, float> transparentsB = getBrightestPixel(this->convertedPixels, EditChannel::B);
    std::tuple<float, float, float> opaquestsB = getDarkestPixel(this->convertedPixels, EditChannel::B);

    std::println("transparentsRMeasurement: {}", std::get<0>(transparentsR));
    std::println("transparentsGMeasurement: {}", std::get<1>(transparentsG));
    std::println("transparentsBMeasurement: {}", std::get<2>(transparentsB));

    std::println("opaquestsRMeasurement: {}", std::get<0>(opaquestsR));
    std::println("opaquestsGMeasurement: {}", std::get<1>(opaquestsG));
    std::println("opaquestsBMeasurement: {}", std::get<2>(opaquestsB));

    std::println("Performing a density inversion");
    densityInvert(this->convertedPixels);

    grayWorld(this->convertedPixels);



    // float brightestRDensity = scanToDensity(std::get<0>(opaquestsR));
    // float brightestGDensity = scanToDensity(std::get<1>(opaquestsG));
    // float brightestBDensity = scanToDensity(std::get<2>(opaquestsB));

    // std::println("brightestRDensity: {}", brightestRDensity);
    // std::println("brightestGDensity: {}", brightestGDensity);
    // std::println("brightestBDensity: {}", brightestBDensity);

    // float brightestRLogExposure = charCurveInverse(brightestRDensity, FilmStock::Gold_200, EditChannel::R);
    // float brightestGLogExposure = charCurveInverse(brightestGDensity, FilmStock::Gold_200, EditChannel::G);
    // float brightestBLogExposure = charCurveInverse(brightestBDensity, FilmStock::Gold_200, EditChannel::B);

    // std::println("brightestRLogExposure: {}", brightestRLogExposure);
    // std::println("brightestGLogExposure: {}", brightestGLogExposure);
    // std::println("brightestBLogExposure: {}", brightestBLogExposure);

    // float correctedBDensityHighest = brightestBDensity;
    // float correctedGDensityHighest = charCurveFunction(brightestBLogExposure, FilmStock::Gold_200, EditChannel::G);
    // float correctedRDensityHighest = charCurveFunction(brightestBLogExposure, FilmStock::Gold_200, EditChannel::R);

    // float darkestRDensity = scanToDensity(std::get<0>(transparentsR));
    // float darkestGDensity = scanToDensity(std::get<1>(transparentsG));
    // float darkestBDensity = scanToDensity(std::get<2>(transparentsB));

    // std::println("darkestRDensity: {}", darkestRDensity);
    // std::println("darkestGDensity: {}", darkestGDensity);
    // std::println("darkestBDensity: {}", darkestBDensity);

    // float darkestRLogExposure = charCurveInverse(darkestRDensity, FilmStock::Gold_200, EditChannel::R);
    // float darkestGLogExposure = charCurveInverse(darkestGDensity, FilmStock::Gold_200, EditChannel::G);
    // float darkestBLogExposure = charCurveInverse(darkestBDensity, FilmStock::Gold_200, EditChannel::B);

    // std::println("darkestRLogExposure: {}", darkestRLogExposure);
    // std::println("darkestGLogExposure: {}", darkestGLogExposure);
    // std::println("darkestBLogExposure: {}", darkestBLogExposure);

    // float correctedRDensityDarkest = darkestRDensity;
    // float correctedGDensityDarkest = charCurveFunction(darkestRLogExposure, FilmStock::Gold_200, EditChannel::G);
    // float correctedBDensityDarkest = charCurveFunction(darkestRLogExposure, FilmStock::Gold_200, EditChannel::B);

    // levelsR(this->convertedPixels, std::get<0>(opaquestsR), std::get<0>(transparentsR), densityToScan(correctedRDensityHighest), densityToScan(correctedRDensityDarkest));
    // levelsG(this->convertedPixels, std::get<1>(opaquestsG), std::get<1>(transparentsG), densityToScan(correctedGDensityHighest), densityToScan(correctedGDensityDarkest));
    // levelsB(this->convertedPixels, std::get<2>(opaquestsB), std::get<2>(transparentsB), densityToScan(correctedBDensityHighest), densityToScan(correctedBDensityDarkest));

    // charCurveInvert(this->convertedPixels, brightestBLogExposure, darkestRLogExposure, EditChannel::RGB);

    // // R
    // std::tuple<float, float, float> brightestAfterR = getBrightestPixel(this->convertedPixels, EditChannel::R);
    // std::tuple<float, float, float> darkestAfterR = getDarkestPixel(this->convertedPixels, EditChannel::R);
    // // G
    // std::tuple<float, float, float> brightestAfterG = getBrightestPixel(this->convertedPixels, EditChannel::G);
    // std::tuple<float, float, float> darkestAfterG = getDarkestPixel(this->convertedPixels, EditChannel::G);
    // // B
    // std::tuple<float, float, float> brightestAfterB = getBrightestPixel(this->convertedPixels, EditChannel::B);
    // std::tuple<float, float, float> darkestAfterB = getDarkestPixel(this->convertedPixels, EditChannel::B);

    // std::println("brightestRMeasurement after conversion: {}", std::get<0>(brightestAfterR));
    // std::println("brightestGMeasurement after conversion: {}", std::get<1>(brightestAfterG));
    // std::println("brightestBMeasurement after conversion: {}", std::get<2>(brightestAfterB));

    // std::println("darkestRMeasurement after conversion: {}", std::get<0>(darkestAfterR));
    // std::println("darkestGMeasurement after conversion: {}", std::get<1>(darkestAfterG));
    // std::println("darkestBMeasurement after conversion: {}", std::get<2>(darkestAfterB));

    // std::println("applying levels adjustment for the g channel");
    // levelsG(this->convertedPixels, std::get<1>(darkestG), std::get<1>(brightestG), std::get<0>(darkestR), std::get<0>(brightestR));

    // std::println("applying levels adjustment for the b channel");
    // levelsB(this->convertedPixels, std::get<2>(darkestB), std::get<2>(brightestB), std::get<0>(darkestR), std::get<0>(brightestR));


    // std::println("applying gamma correction to the b channel");
    // gamma(this->convertedPixels, 0.7f, EditChannel::B);

    // std::println("applying gamma correction to the g channel");
    // gamma(this->convertedPixels, 0.88f, EditChannel::G);

    // // R
    // std::tuple<float, float, float> brightestR1 = getBrightestPixel(this->convertedPixels, EditChannel::R);
    // std::tuple<float, float, float> darkestR1 = getDarkestPixel(this->convertedPixels, EditChannel::R);
    // // G
    // std::tuple<float, float, float> brightestG1 = getBrightestPixel(this->convertedPixels, EditChannel::G);
    // std::tuple<float, float, float> darkestG1 = getDarkestPixel(this->convertedPixels, EditChannel::G);
    // // B
    // std::tuple<float, float, float> brightestB1 = getBrightestPixel(this->convertedPixels, EditChannel::B);
    // std::tuple<float, float, float> darkestB1 = getDarkestPixel(this->convertedPixels, EditChannel::B);

    // std::println("applying levels adjustment for the r channel");
    // levelsR(this->convertedPixels, std::get<0>(darkestR1), std::get<0>(brightestR1), 0.0, 1.0);

    // std::println("applying levels adjustment for the g channel");
    // levelsG(this->convertedPixels, std::get<1>(darkestG1), std::get<1>(brightestG1), 0.0, 1.0);

    // std::println("applying levels adjustment for the b channel");
    // levelsB(this->convertedPixels, std::get<2>(darkestB1), std::get<2>(brightestB1), 0.0, 1.0);

    // std::println("applying gamma correction to the b channel");
    // gamma(this->convertedPixels, 0.65, EditChannel::B);

    // std::println("applying gamma correction to the g channel");
    // gamma(this->convertedPixels, 0.85, EditChannel::G);
    
    // std::println("Performing a crude inversion");
    // crudeInversion(this->convertedPixels);

    // std::println("applying general display gamma correction");
    // gamma(this->convertedPixels, 1.0/2.2, EditChannel::RGB);

    this->editedPixels = this->convertedPixels;

    return;
}