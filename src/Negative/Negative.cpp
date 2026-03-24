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
#include "../imageAlgorithms/gamma.hpp"
#include "../imageAlgorithms/myExposure.hpp"
#include "../imageAlgorithms/grayWorld.hpp"
#include "../imageAlgorithms/multiply.hpp"
#include "../imageAlgorithms/compromiseInvert.hpp"
#include "../imageAlgorithms/boxFilter.hpp"
#include "../imageAlgorithms/crop.hpp"
#include "../imageAlgorithms/eyedropper.hpp"


// STATIC DATA MEMBERS
// ----------------------------------------------------------------------------------------------------------------

int Negative::nextId = 0;


// PREVIEW CACHING
// ----------------------------------------------------------------------------------------------------------------

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


// NEGATIVE DATA IO
// ----------------------------------------------------------------------------------------------------------------

void Negative::readNegativeData() {
    std::string dataName = this->path.replace_extension(".neg").string();
    std::ifstream file(dataName);
    if (!file) {
        std::println("failed to find negativeData file called {}", dataName);
        std::println("generating default data");
        file.open("assets/negativeDataTemplate.neg");
    }
    this->negativeData = nlohmann::json::parse(file);
}

void Negative::writeNegativeData() {
    std::string dataName = this->path.replace_extension(".neg").string();
    std::ofstream file(dataName);
    file << this->negativeData << std::endl;
}


// HELPERS
// ----------------------------------------------------------------------------------------------------------------

std::tuple<float, float, float> Negative::samplePixels(int x, int y) {
    return eyedropper(this->workingPixels, this->workingWidth, this->workingHeight, x, y, this->EYEDROPPER_SIZE);
}


// INITIALIZER
// ----------------------------------------------------------------------------------------------------------------

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

    const int widthScale = ceilDiv(this->width, this->PREVIEW_SIZE);
    const int heightScale = ceilDiv(this->height, this->PREVIEW_SIZE);
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

    
    // 3. Read saved NegativeData if exists
    this->readNegativeData();

    if(this->negativeData["general"]["isConverted"] && !this->readConversionCache()) {
        this->renderWorking();
    }
    
    if(!this->negativeData["conversion"]["hasScanArea"]) {
        this->setScanArea({0, 0, this->workingWidth, this->workingHeight});
    }

    // 5. apply edits from the saved data
    this->renderEdits();

    // 6. Create a thumbnail
    this->renderThumbnail();

    return true;
}


// CONSTRUCTORS
// ----------------------------------------------------------------------------------------------------------------

Negative::Negative(std::filesystem::path imagePath) {
    this->id = this->nextId;
    this->nextId++;
    this->initializeNegative(imagePath);
    return;
}


// GETTERS FOR THE UI
// ----------------------------------------------------------------------------------------------------------------

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

ImageData Negative::getThumbnail() {
    // Return an ImageData struct
    return {
        this->thumbnailPixels,
        this->THUMBNAIL_SIZE,
        this->THUMBNAIL_SIZE
    };
}

int Negative::getId() {
    return this->id;
}


// EXPORTING
// ----------------------------------------------------------------------------------------------------------------

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


// SETTING EDIT SETTINGS PRE-CONVERT
// ----------------------------------------------------------------------------------------------------------------

float Negative::setScanGamma(float value) {
    this->negativeData["conversion"]["scanGamma"] = value;
    return this->getScanGamma();
}

void Negative::setScanArea(ImageArea area) {
    this->negativeData["conversion"]["scanArea"]["left"] = area.left;
    this->negativeData["conversion"]["scanArea"]["top"] = area.top;
    this->negativeData["conversion"]["scanArea"]["right"] = area.right;
    this->negativeData["conversion"]["scanArea"]["bottom"] = area.bottom;
}

void Negative::setBorder(float r, float g, float b) {
    this->negativeData["conversion"]["border"]["r"] = r;
    this->negativeData["conversion"]["border"]["g"] = g;
    this->negativeData["conversion"]["border"]["b"] = b;
    this->negativeData["conversion"]["hasBorder"] = true;
}

void Negative::setBorderByCoords(int x, int y) {
    std::tuple<float, float, float> sample = this->samplePixels(x, y);
    this->setBorder(std::get<0>(sample), std::get<1>(sample), std::get<2>(sample));
}

void Negative::setDensest(float r, float g, float b) {
    this->negativeData["conversion"]["densest"]["r"] = r;
    this->negativeData["conversion"]["densest"]["g"] = g;
    this->negativeData["conversion"]["densest"]["b"] = b;
    this->negativeData["conversion"]["hasDensest"] = true;
}

void Negative::setDensestByCoords(int x, int y) {
    std::tuple<float, float, float> sample = this->samplePixels(x, y);
    this->setDensest(std::get<0>(sample), std::get<1>(sample), std::get<2>(sample));
}

void Negative::convert() {
    this->renderWorking();
}

void Negative::resetConversion() {
    this->convertedPixels = this->workingPixels;
}


// SETTING EDIT SETTINGS POST-CONVERT
// ----------------------------------------------------------------------------------------------------------------

float Negative::setDensity(float value) {
    std::println("exposure was set to: {}", value);
    this->negativeData["edits"]["density"] = value;
    return this->getDensity();
}

float Negative::setContrast(float value) {
    this->negativeData["edits"]["contrast"] = value;
    return this->getContrast();
}

float Negative::setWhites(float value) {
    this->negativeData["edits"]["whites"] = value;
    return this->getWhites();
}

float Negative::setHighlights(float value) {
    this->negativeData["edits"]["highlights"] = value;
    return this->getHighlights();
}

float Negative::setShadows(float value) {
    this->negativeData["edits"]["shadows"] = value;
    return this->getShadows();
}

float Negative::setBlacks(float value) {
    this->negativeData["edits"]["blacks"] = value;
    return this->getBlacks();
}

void Negative::setNeutral(float r, float g, float b) {
    this->negativeData["edits"]["neutralPoint"]["r"] = r;
    this->negativeData["edits"]["neutralPoint"]["g"] = g;
    this->negativeData["edits"]["neutralPoint"]["b"] = b;
        this->negativeData["conversion"]["hasNeutral"] = true;
}

void Negative::setNeutralByCoords(int x, int y) {
    std::tuple<float, float, float> sample = this->samplePixels(x, y);
    this->setNeutral(std::get<0>(sample), std::get<1>(sample), std::get<2>(sample));
}

float Negative::setRBalance(float value)
{
    this->negativeData["edits"]["rBalance"] = value;
    return this->getRBalance();
}

float Negative::setGBalance(float value) {
    this->negativeData["edits"]["gBalance"] = value;
    return this->getGBalance();
}

float Negative::setBBalance(float value) {
    this->negativeData["edits"]["bBalance"] = value;
    return this->getBBalance();
}


// GETTING EDIT SETTINGS FROM NEGATIVEDATA PRE-CONVERT
// ----------------------------------------------------------------------------------------------------------------

float Negative::getScanGamma() {
    return this->negativeData["conversion"]["scanGamma"];
}

ImageArea Negative::getScanArea() {
    ImageArea scanArea;
    scanArea.left = this->negativeData["conversion"]["scanArea"]["left"];
    scanArea.top = this->negativeData["conversion"]["scanArea"]["top"];
    scanArea.right = this->negativeData["conversion"]["scanArea"]["right"];
    scanArea.bottom = this->negativeData["conversion"]["scanArea"]["bottom"];
    return scanArea;
}

std::tuple<float, float, float> Negative::getBorder() {
    float r = this->negativeData["conversion"]["border"]["r"];
    float g = this->negativeData["conversion"]["border"]["g"];
    float b = this->negativeData["conversion"]["border"]["b"];
    return { r, g, b };
}

std::tuple<float, float, float> Negative::getDensest() {
    float r = this->negativeData["conversion"]["densest"]["r"];
    float g = this->negativeData["conversion"]["densest"]["g"];
    float b = this->negativeData["conversion"]["densest"]["b"];
    return { r, g, b };
}


// GETTING EDIT SETTINGS FROM NEGATIVEDATA POST-CONVERT
// ----------------------------------------------------------------------------------------------------------------

float Negative::getDensity() {
    return this->negativeData["edits"]["density"];
}

float Negative::getContrast() {
    return this->negativeData["edits"]["contrast"];
}

float Negative::getWhites() {
    return this->negativeData["edits"]["whites"];
}

float Negative::getHighlights() {
    return this->negativeData["edits"]["highlights"];
}

float Negative::getShadows() {
    return this->negativeData["edits"]["shadows"];
}

float Negative::getBlacks() {
    return this->negativeData["edits"]["blacks"];
}

std::tuple<float, float, float> Negative::getNeutral() {
    float r = this->negativeData["edits"]["neutralPoint"]["r"];
    float g = this->negativeData["edits"]["neutralPoint"]["g"];
    float b = this->negativeData["edits"]["neutralPoint"]["b"];
    return { r, g, b };
}

float Negative::getRBalance() {
    return this->negativeData["edits"]["rBalance"];
}

float Negative::getGBalance() {
    return this->negativeData["edits"]["gBalance"];
}

float Negative::getBBalance() {
    return this->negativeData["edits"]["bBalance"];
}


// RENDERING METHODS
// ----------------------------------------------------------------------------------------------------------------

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
    OIIO::ROI roi(0, this->THUMBNAIL_SIZE, 0, this->THUMBNAIL_SIZE, 0, 1, /*chans:*/ 0, uint8Buf.nchannels());
    OIIO::ImageBuf thumbnailBuf = OIIO::ImageBufAlgo::resample(uint8Buf, true, roi);
    
    // Extact the preview data from the ImageBuf
    this->thumbnailPixels.resize(this->THUMBNAIL_SIZE*this->THUMBNAIL_SIZE*4);
    thumbnailBuf.get_pixels(OIIO::ROI::All(), OIIO::TypeDesc::UINT8, this->thumbnailPixels.data());

    std::println("generated thumbnail data");
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
    multiply(this->editedPixels, this->negativeData["edits"]["density"], EditChannel::RGB);

    // Apply display gamma
    // std::println("applying general display gamma correction");
    // gamma(this->editedPixels, 1.0/2.2, EditChannel::RGB);

    this->writeNegativeData();
}

void Negative::renderWorking() {

    this->convertedPixels = this->workingPixels;

    gamma(this->convertedPixels, 1.8, EditChannel::RGB);
    
    std::println("starting negative conversion pipeline");

    float transparentsR;
    float transparentsG;
    float transparentsB;

    float opaquestsR;
    float opaquestsG;
    float opaquestsB;

    if (!this->negativeData["conversion"]["hasDensest"] || !this->negativeData["conversion"]["hasBorder"]) {

        // First blur the image slightly to remove noise and extremities

        // Only sample the area indicated by this->scanArea
        std::tuple<std::vector<float>, int, int> blurredCropResult = crop(this->convertedPixels, this->workingWidth, this->workingHeight, this->getScanArea());

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

        if (!this->negativeData["conversion"]["hasBorder"]) {
            // sampling border
            std::println("sampling border");
            transparentsR = std::get<0>(getBrightestPixel(blurredPixels, EditChannel::R));
            transparentsG = std::get<1>(getBrightestPixel(blurredPixels, EditChannel::G));
            transparentsB = std::get<2>(getBrightestPixel(blurredPixels, EditChannel::B));
        }
        else {
            transparentsR = this->negativeData["conversion"]["border"]["r"];
            transparentsG = this->negativeData["conversion"]["border"]["g"];
            transparentsB = this->negativeData["conversion"]["border"]["b"];
        }

        if (!this->negativeData["conversion"]["hasDensest"]) {
            std::println("sampling densest");
            // sampling densest
            opaquestsR = std::get<0>(getDarkestPixel(blurredPixels, EditChannel::R));
            opaquestsG = std::get<1>(getDarkestPixel(blurredPixels, EditChannel::G));
            opaquestsB = std::get<2>(getDarkestPixel(blurredPixels, EditChannel::B));
        }
        else {
            opaquestsR = this->negativeData["conversion"]["densest"]["r"];
            opaquestsG = this->negativeData["conversion"]["densest"]["g"];
            opaquestsB = this->negativeData["conversion"]["densest"]["b"];
        }

    }
    else {
        transparentsR = this->negativeData["conversion"]["border"]["r"];
        transparentsG = this->negativeData["conversion"]["border"]["g"];
        transparentsB = this->negativeData["conversion"]["border"]["b"];

        opaquestsR = this->negativeData["conversion"]["densest"]["r"];
        opaquestsG = this->negativeData["conversion"]["densest"]["g"];
        opaquestsB = this->negativeData["conversion"]["densest"]["b"];
    }

    // auto eyeropperResults = eyedropper(this->originalPixels, this->width, this->height, 10, 10, 10);
    // std::println("the eyedropper at (10,10) with size 10 gives an average R of {}", std::get<0>(eyeropperResults));

    std::println("transparentsRMeasurement: {}", transparentsR);
    std::println("transparentsGMeasurement: {}", transparentsG);
    std::println("transparentsBMeasurement: {}", transparentsB);

    std::println("opaquestsRMeasurement: {}", opaquestsR);
    std::println("opaquestsGMeasurement: {}", opaquestsG);
    std::println("opaquestsBMeasurement: {}", opaquestsB);

    // Convert the measured values to actual density values
    float brightestRDensity = scanToDensity(opaquestsR);
    float brightestGDensity = scanToDensity(opaquestsG);
    float brightestBDensity = scanToDensity(opaquestsB);

    std::println("brightestRDensity: {}", brightestRDensity);
    std::println("brightestGDensity: {}", brightestGDensity);
    std::println("brightestBDensity: {}", brightestBDensity);

    float darkestRDensity = scanToDensity(transparentsR);
    float darkestGDensity = scanToDensity(transparentsG);
    float darkestBDensity = scanToDensity(transparentsB);

    std::println("darkestRDensity: {}", darkestRDensity);
    std::println("darkestGDensity: {}", darkestGDensity);
    std::println("darkestBDensity: {}", darkestBDensity);

    // Balance the black point and white point densities to all match the red channel
    levelsR(this->convertedPixels, opaquestsR, transparentsR, densityToScan(brightestRDensity), densityToScan(darkestRDensity));
    levelsG(this->convertedPixels, opaquestsG, transparentsG, densityToScan(brightestRDensity), densityToScan(darkestRDensity));
    levelsB(this->convertedPixels, opaquestsB, transparentsB, densityToScan(brightestRDensity), densityToScan(darkestRDensity));

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
    //grayWorld(this->convertedPixels);

    this->writeConversionCache();

    this->editedPixels = this->convertedPixels;

    this->negativeData["conversion"]["isConverted"] = true;

    this->writeNegativeData();

    return;
}
