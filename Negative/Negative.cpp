#include "Negative.hpp"

#include <iostream>
#include <print>
#include <format>
#include <filesystem>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "../imageAlgorithms/getExtremePixels.hpp"
#include "../imageAlgorithms/levels.hpp"
#include "../imageAlgorithms/crudeInversion.hpp"
#include "../imageAlgorithms/densityInvert.hpp"
#include "../imageAlgorithms/gamma.hpp"
#include "../imageAlgorithms/myExposure.hpp"
#include "../imageAlgorithms/grayWorld.hpp"
#include "../imageAlgorithms/multiply.hpp"
#include "../imageAlgorithms/compromiseInvert.hpp"
#include "../imageAlgorithms/boxFilter.hpp"
#include "../imageAlgorithms/crop.hpp"

const int PREVIEW_SIZE = 800;
const int THUMBNAIL_SIZE = 500;

int Negative::nextId = 0;

Negative::Negative(std::filesystem::path imagePath) {
    this->id = this->nextId;
    this->nextId++;
    this->initializeNegative(imagePath);
    return;
}

Negative::Negative() {};

bool Negative::initializeNegative(std::filesystem::path imagePath) {

    this->path = imagePath;
    
    // 1. Read the full original image
    auto input = OIIO::ImageInput::open(imagePath.string());

    if (!input) {
        std::println("Failed to open file");
        return false;
    }

    this->name = imagePath.stem();
    
    const OIIO::ImageSpec& spec = input->spec();
    this->width = spec.width;
    this->height = spec.height;
    this->numberOfChannels = spec.nchannels;
    this->originalPixels.resize(this->width * this->height * this->numberOfChannels);
    input->read_image(0, 0, 0, this->numberOfChannels, OIIO::TypeDesc::FLOAT, &this->originalPixels[0]);
    std::println("Opened negative successfully");

    // Prints handy knowledge about the image
    std::println("Read a file and created a Negative object");
    std::println("This image has the following data");
    std::string metadata = spec.serialize(OIIO::ImageSpec::SerialText, OIIO::ImageSpec::SerialDetailedHuman);
    std::println("{}", metadata);
    
    input->close();

    // 2. Generate the working image

    // First determine the working scale, meaning the amount that it has to be divided by to fit into the preview box as listed in PREVIEW_SIZE
    auto ceilDiv = [](int a, int b) {
        return a / b + (a % b != 0);
    };

    const int widthScale = ceilDiv(this->width, PREVIEW_SIZE);
    const int heightScale = ceilDiv(this->height, PREVIEW_SIZE);
    this->workingScale = std::max(widthScale, heightScale);

    std::println("the working scale for this image is {}", this->workingScale);
    std::println("Generating working pixels");

    // Then generate the workingPixels array with this info
    // Use OIIO::ImageBuf for ease of scaling resizing etc
    OIIO::ImageSpec originalSpec(this->width, this->height, this->numberOfChannels, OIIO::TypeDesc::FLOAT);
    OIIO::ImageBuf originalBuf(originalSpec, this->originalPixels.data());

    // Calculate new dimensions for the working image
    this->workingWidth = this->width*(1.0f/this->workingScale);
    this->workingHeight = this->height*(1.0f/this->workingScale);
    std::println("trying to resize resolution to width: {} height: {}", this->workingWidth, this->workingHeight);

    // Resize the working image
    OIIO::ROI roi(0, this->workingWidth, 0, this->workingHeight, 0, 1, /*chans:*/ 0, 3);
    OIIO::ImageBuf workingBuf = OIIO::ImageBufAlgo::resample(originalBuf, true, roi);
    std::println("resized resolution to width: {} height: {}", workingBuf.spec().width, workingBuf.spec().height);
    
    // Extract the working data from the ImageBuf
    this->workingPixels.resize(this->workingWidth*this->workingHeight*3);
    workingBuf.get_pixels(OIIO::ROI::All(), OIIO::TypeDesc::FLOAT, this->workingPixels.data());
    std::println("Working pixels generated");

    this->convertedPixels = this->workingPixels;

    this->scanArea = ImageArea{0, 0, this->workingWidth, this->workingHeight};

    // 3. Read saved NegativeData if exists
    this->readNegativeData();
    
    // 4. Check if the image has been converted and if a cached conversion exists
    if(this->negativeData["conversion"]["isConverted"] && !this->readConversionCache()) {
        this->renderWorking();
    }

    // 5. apply edits from the saved data
    this->renderEdits();

    // 6. Create a thumbnail
    this->renderThumbnail();

    return true;
}

bool Negative::exportPositive(std::filesystem::path imagePath) {
    std::println("Saving positive");
    std::string filePath = std::format("{}.tif", imagePath.string());

    // Use OIIO::ImageBuf for ease of transformign the pixel data type
    OIIO::ImageSpec originalSpec(this->width, this->height, this->numberOfChannels, OIIO::TypeDesc::FLOAT);
    OIIO::ImageBuf originalBuf(originalSpec, this->editedPixels.data());

    // For now we want to save images as 16bit
    if (!originalBuf.write(filePath, OIIO::TypeDesc::UINT16)) {
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
    OIIO::ImageSpec workingSpec(this->workingWidth, this->workingHeight, this->numberOfChannels, OIIO::TypeDesc::FLOAT);
    OIIO::ImageBuf workingBuf(workingSpec, this->editedPixels.data());

    // Make sure the data is in RGBA format since the preview will have to be in RGBA format for SFML
    if (this->numberOfChannels < 4) {
        std::println("changed channels to 4");
        // This is straight from OIIO, it adds an aplha channel to the image
        workingBuf = OIIO::ImageBufAlgo::channels(workingBuf, 4, { 0, 1, 2, -1 },
                                    { 0 /*ignore*/, 0 /*ignore*/, 0 /*ignore*/,
                                        1.0 },
                                    { "", "", "", "A" });
    }

    // Calculate new dimensions for the preview
    int previewWidth = this->workingWidth;
    int previewHeight = this->workingHeight;

    // Change the underlying bit depth of the image to 8bit
    OIIO::ImageBuf uint8Buf = OIIO::ImageBufAlgo::copy(workingBuf, OIIO::TypeDesc::UINT8);
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
    this->negativeData["edits"]["exposure"] = value;
}

void Negative::setRBalance(float value) {
    this->negativeData["edits"]["rBalance"] = value;
}
void Negative::setGBalance(float value) {
    this->negativeData["edits"]["gBalance"] = value;
}
void Negative::setBBalance(float value) {
    this->negativeData["edits"]["bBalance"] = value;
}

bool Negative::writeConversionCache() {
    std::println("Saving cahched conversion");
    std::string fileName = std::format("{}_chache.tif", this->name);

    // Use OIIO::ImageBuf for ease of transformign the pixel data type
    OIIO::ImageSpec convertedSpec(this->workingWidth, this->workingHeight, this->numberOfChannels, OIIO::TypeDesc::FLOAT);
    OIIO::ImageBuf convertedBuf(convertedSpec, this->convertedPixels.data());

    // Save the converted pixels vector
    if (!convertedBuf.write(fileName, OIIO::TypeDesc::FLOAT)) {
        std::println("Failed to save image");
        return false;
    }
    std::println("Saved positive successfully");
    
    return true;
}

bool Negative::readConversionCache() {

    std::println("trying to open cache");
    std::string fileName = std::format("{}_chache.tif", this->name);
    auto input = OIIO::ImageInput::open(fileName);

    if (!input) {
        std::println("Failed to open/find cache");
        return false;
    }

    input->read_image(0, 0, 0, this->numberOfChannels, OIIO::TypeDesc::FLOAT, &this->convertedPixels[0]);
    std::println("loaded cahce");
    input->close();
    return true;
}

void Negative::readNegativeData() {
    std::string dataName = this->path.replace_extension(".neg").string();
    std::ifstream file(dataName);
    if (!file) {
        std::println("failed to find negativeData file called {}", dataName);
        std::println("generating default data");
        file.open("negativeDataTemplate.neg");
    }
    this->negativeData = nlohmann::json::parse(file);
}

void Negative::writeNegativeData() {
    std::string dataName = this->path.replace_extension(".neg").string();
    std::ofstream file(dataName);
    file << this->negativeData << std::endl;
}

void Negative::renderThumbnail() {
    std::println("generating thumbnail");

    // Use OIIO::ImageBuf for ease of scaling resizing etc
    OIIO::ImageSpec workingSpec(this->workingWidth, this->workingHeight, this->numberOfChannels, OIIO::TypeDesc::FLOAT);
    OIIO::ImageBuf workingBuf(workingSpec, this->editedPixels.data());

    // Make sure the data is in RGBA format since it will have to be in RGBA format for SFML
    if (this->numberOfChannels < 4) {
        // This is straight from OIIO, it adds an aplha channel to the image
        workingBuf = OIIO::ImageBufAlgo::channels(workingBuf, 4, { 0, 1, 2, -1 },
                                    { 0 /*ignore*/, 0 /*ignore*/, 0 /*ignore*/,
                                        1.0 },
                                    { "", "", "", "A" });
    }

    // Change the underlying bit depth of the image to 8bit
    OIIO::ImageBuf uint8Buf = OIIO::ImageBufAlgo::copy(workingBuf, OIIO::TypeDesc::UINT8);

    // Resize the image for the preview
    OIIO::ROI roi(0, THUMBNAIL_SIZE, 0, THUMBNAIL_SIZE, 0, 1, /*chans:*/ 0, uint8Buf.nchannels());
    OIIO::ImageBuf thumbnailBuf = OIIO::ImageBufAlgo::resample(uint8Buf, true, roi);
    
    // Extact the preview data from the ImageBuf
    this->thumbnailPixels.resize(THUMBNAIL_SIZE*THUMBNAIL_SIZE*4);
    thumbnailBuf.get_pixels(OIIO::ROI::All(), OIIO::TypeDesc::UINT8, this->thumbnailPixels.data());

    std::println("generated thumbnail data");
}

ImageData Negative::getThumbnail() {
    // Return an ImageData struct
    return {
        this->thumbnailPixels,
        THUMBNAIL_SIZE,
        THUMBNAIL_SIZE
    };
}

void Negative::renderEdits() {

    this->editedPixels = this->convertedPixels;

    std::println("Editing the image");

    // gamma(this->editedPixels, this->rBalance, EditChannel::R);
    // gamma(this->editedPixels, this->gBalance, EditChannel::G);
    // gamma(this->editedPixels, this->bBalance, EditChannel::B);

    multiply(this->editedPixels, this->negativeData["edits"]["rBalance"], EditChannel::R);
    multiply(this->editedPixels, this->negativeData["edits"]["gBalance"], EditChannel::G);
    multiply(this->editedPixels, this->negativeData["edits"]["bBalance"], EditChannel::B);

    // std::println("Editing exposure");
    multiply(this->editedPixels, this->negativeData["edits"]["exposure"], EditChannel::RGB);

    // Apply display gamma
    std::println("applying general display gamma correction");
    gamma(this->editedPixels, 1.0/2.2, EditChannel::RGB);

    this->writeNegativeData();
}

void Negative::renderWorking() {

    this->convertedPixels = this->workingPixels;

    gamma(this->convertedPixels, 1.8, EditChannel::RGB);
    
    std::println("starting negative conversion pipeline");

    // First blur the image slightly to remove noise and extremities

    // Only sample the area indicated by this->scanArea
    std::tuple<std::vector<float>, int, int> blurredCropResult = crop(this->convertedPixels, this->workingWidth, this->workingHeight, this->scanArea);

    std::vector<float> blurredPixels = std::get<0>(blurredCropResult);
    int blurredWidth = std::get<1>(blurredCropResult);
    int blurredHeight = std::get<2>(blurredCropResult);
    std::println("the dimensions after the crop are: w: {}, h: {}", blurredWidth, blurredHeight);

    // Perform the actual blur
    boxFilter(blurredPixels, blurredWidth, blurredHeight, 20);

    // Measure brightest and darkest pixels from the blurred image
    // Brightest = most transparent = low density
    // Darkest = most opaque = high density

    std::println("getting the brightest and darkest pixels");

    // R
    std::tuple<float, float, float> transparentsR = getBrightestPixel(blurredPixels, EditChannel::R);
    std::tuple<float, float, float> opaquestsR = getDarkestPixel(blurredPixels, EditChannel::R);
    // G
    std::tuple<float, float, float> transparentsG = getBrightestPixel(blurredPixels, EditChannel::G);
    std::tuple<float, float, float> opaquestsG = getDarkestPixel(blurredPixels, EditChannel::G);
    // B
    std::tuple<float, float, float> transparentsB = getBrightestPixel(blurredPixels, EditChannel::B);
    std::tuple<float, float, float> opaquestsB = getDarkestPixel(blurredPixels, EditChannel::B);

    std::println("transparentsRMeasurement: {}", std::get<0>(transparentsR));
    std::println("transparentsGMeasurement: {}", std::get<1>(transparentsG));
    std::println("transparentsBMeasurement: {}", std::get<2>(transparentsB));

    std::println("opaquestsRMeasurement: {}", std::get<0>(opaquestsR));
    std::println("opaquestsGMeasurement: {}", std::get<1>(opaquestsG));
    std::println("opaquestsBMeasurement: {}", std::get<2>(opaquestsB));

    // Store the measured values
    this->negativeData["conversion"]["blackPoint"]["r"] = std::get<0>(transparentsR);
    this->negativeData["conversion"]["blackPoint"]["g"] = std::get<1>(transparentsG);
    this->negativeData["conversion"]["blackPoint"]["b"] = std::get<2>(transparentsB);
    
    this->negativeData["conversion"]["whitePoint"]["r"] = std::get<0>(opaquestsR);
    this->negativeData["conversion"]["whitePoint"]["g"] = std::get<1>(opaquestsG);
    this->negativeData["conversion"]["whitePoint"]["b"] = std::get<2>(opaquestsB);

    // Convert the measured values to actual density values
    float brightestRDensity = scanToDensity(std::get<0>(opaquestsR));
    float brightestGDensity = scanToDensity(std::get<1>(opaquestsG));
    float brightestBDensity = scanToDensity(std::get<2>(opaquestsB));

    std::println("brightestRDensity: {}", brightestRDensity);
    std::println("brightestGDensity: {}", brightestGDensity);
    std::println("brightestBDensity: {}", brightestBDensity);

    float darkestRDensity = scanToDensity(std::get<0>(transparentsR));
    float darkestGDensity = scanToDensity(std::get<1>(transparentsG));
    float darkestBDensity = scanToDensity(std::get<2>(transparentsB));

    std::println("darkestRDensity: {}", darkestRDensity);
    std::println("darkestGDensity: {}", darkestGDensity);
    std::println("darkestBDensity: {}", darkestBDensity);

    // Balance the black point and white point densities to all match the red channel
    levelsR(this->convertedPixels, std::get<0>(opaquestsR), std::get<0>(transparentsR), densityToScan(brightestRDensity), densityToScan(darkestRDensity));
    levelsG(this->convertedPixels, std::get<1>(opaquestsG), std::get<1>(transparentsG), densityToScan(brightestRDensity), densityToScan(darkestRDensity));
    levelsB(this->convertedPixels, std::get<2>(opaquestsB), std::get<2>(transparentsB), densityToScan(brightestRDensity), densityToScan(darkestRDensity));

    // Perform the inversion and normalize to the highest and darkest values
    compromiseInvert(this->convertedPixels, darkestRDensity, brightestRDensity);

    std::tuple<float, float, float> brightestAfterR = getBrightestPixel(this->convertedPixels, EditChannel::R);
    std::tuple<float, float, float> darkestAfterR = getDarkestPixel(this->convertedPixels, EditChannel::R);

    std::tuple<float, float, float> brightestAfterG = getBrightestPixel(this->convertedPixels, EditChannel::G);
    std::tuple<float, float, float> darkestAfterG = getDarkestPixel(this->convertedPixels, EditChannel::G);

    std::tuple<float, float, float> brightestAfterB = getBrightestPixel(this->convertedPixels, EditChannel::B);
    std::tuple<float, float, float> darkestAfterB = getDarkestPixel(this->convertedPixels, EditChannel::B);

    std::println("brightestRMeasurement after conversion: {}", std::get<0>(brightestAfterR));
    std::println("brightestGMeasurement after conversion: {}", std::get<1>(brightestAfterG));
    std::println("brightestBMeasurement after conversion: {}", std::get<2>(brightestAfterB));

    std::println("darkestRMeasurement after conversion: {}", std::get<0>(darkestAfterR));
    std::println("darkestGMeasurement after conversion: {}", std::get<1>(darkestAfterG));
    std::println("darkestBMeasurement after conversion: {}", std::get<2>(darkestAfterB));

    // Auto White balance
    grayWorld(this->convertedPixels);

    this->writeConversionCache();

    this->editedPixels = this->convertedPixels;

    this->negativeData["conversion"]["isConverted"] = true;

    this->writeNegativeData();

    return;
}