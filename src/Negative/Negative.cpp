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
#include "../imageAlgorithms/grayWorld.hpp"
#include "../imageAlgorithms/multiply.hpp"
#include "../imageAlgorithms/curveExposure.hpp"
#include "../imageAlgorithms/compromiseInvert.hpp"
#include "../imageAlgorithms/boxFilter.hpp"
#include "../imageAlgorithms/crop.hpp"
#include "../imageAlgorithms/eyedropper.hpp"
#include "../imageAlgorithms/colorBalance.hpp"
#include "../imageAlgorithms/contrast.hpp"
#include "../imageAlgorithms/curveBlacksShadows.hpp"
#include "../imageAlgorithms/curveWhitesHighlights.hpp"


// STATIC HELPER FUNCTIONS
// ----------------------------------------------------------------------------------------------------------------
static float exponentDampenerFixer(float value) {
    return std::pow(1.2, value);
}


// STATIC DATA MEMBERS
// ----------------------------------------------------------------------------------------------------------------

int Negative::nextId = 0;
const std::string Negative::NEGATIVEDATA_VERSION = "0.2.0";
const std::string Negative::PRESETDATA_VERSION = "0.1.0";
const float Negative::DRAGGING_SCALE = 0.4f;


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
    this->renderDraggingConversion();
    return true;
}


// NEGATIVE DATA IO
// ----------------------------------------------------------------------------------------------------------------

void Negative::readNegativeData() {
    std::string defaultDataPath = "./resources/data_templates/negativeDataTemplate.neg";
    std::string dataName = std::filesystem::path(this->path)
        .replace_extension(".neg")
        .string();
    std::ifstream file(dataName);
    // No .neg file with this name exists
    if (!file) {
        std::println("failed to find negativeData file called {}", dataName);
        std::println("generating default data");
        file.open(defaultDataPath);
    }
    // Try to parse the .neg file
    try {
        this->negativeData = nlohmann::json::parse(file);
        std::println("read NegativeData succesfully");
        file.close();
    }
    // Error parsing the .neg file to json
    catch (const std::exception& e) {
        std::println("failed to parse negativeData file called {}", dataName);
        std::println("Error message: {}", e.what());
        std::println("generating default data");
        file.close();
        file.clear();
        file.open(defaultDataPath);
        this->negativeData = nlohmann::json::parse(file);
        file.close();
    }
    // Parsed succesfully but wrong version
    if (this->negativeData["version"] != this->NEGATIVEDATA_VERSION) {
        std::println("Incompatible NegativeData version");
        std::println("generating default data");
        file.open(defaultDataPath);
        this->negativeData = nlohmann::json::parse(file);
        file.close();
    }
}

void Negative::writeNegativeData() {
    std::string dataName = std::filesystem::path(this->path)
        .replace_extension(".neg")
        .string();
    std::ofstream file(dataName);
    file << this->negativeData << std::endl;
}


// PRESET DATA
// ----------------------------------------------------------------------------------------------------------------

std::unique_ptr<nlohmann::json> Negative::readPresetdata(std::filesystem::path path) {
    std::ifstream file(path.string());
    std::unique_ptr<nlohmann::json> presetData;
    // No preset file with this path exists
    if (!file) {
        std::println("failed to open file at path {}", path.string());
        return nullptr;
    }
    // Try to parse the preset file
    try {
        presetData = std::make_unique<nlohmann::json>(nlohmann::json::parse(file));
        std::println("read preset succesfully");
        file.close();
    }
    // Error parsing the preset file to json
    catch (const std::exception& e) {
        std::println("failed to read preset");
        std::println("Error message: {}", e.what());
        file.close();
        return nullptr;
    }
    // Parsed succesfully but wrong version
    if ((*presetData)["version"] != this->PRESETDATA_VERSION) {
        std::println("Incompatible PresetData version");
        return nullptr;
    }
    return presetData;
}


// HELPERS
// ----------------------------------------------------------------------------------------------------------------

std::tuple<float, float, float> Negative::samplePixels(int x, int y) {
    auto [r, g, b] = eyedropper(this->workingPixels, this->workingWidth, this->workingHeight, x, y, this->EYEDROPPER_SIZE);
    // Have to remove gamma from these pixels since the conversion expects gamma to be removed
    return {
        gammaFunction(r, this->getScanGamma()),
        gammaFunction(g, this->getScanGamma()),
        gammaFunction(b, this->getScanGamma())
    };
}

std::unique_ptr<std::vector<float>> Negative::renderFinal() {
    
    std::unique_ptr<std::vector<float>> finalPixels = std::make_unique<std::vector<float>>(this->originalPixels);
    
    this->renderConversion(*finalPixels, this->width, this->height, this->numberOfChannels, 1.0f);
    this->renderEdits(*finalPixels, this->numberOfChannels);

    return finalPixels;
}

void Negative::renderEdits(std::vector<float>& pixels, int channels) {

    std::println("Editing the image");

    // Collect all the edit values needed from negativedata

    // Color balance
    float rBalance = exponentDampenerFixer(this->negativeData["edits"]["rBalance"]);
    float gBalance = exponentDampenerFixer(this->negativeData["edits"]["gBalance"]);
    float bBalance = exponentDampenerFixer(this->negativeData["edits"]["bBalance"]);
    
    // Intensity
    float density = this->negativeData["edits"]["density"];
    float contrast = exponentDampenerFixer(this->negativeData["edits"]["contrast"]);
    
    float shadows = this->negativeData["edits"]["shadows"];
    float blacks = this->negativeData["edits"]["blacks"];
    float highlights = this->negativeData["edits"]["highlights"];
    float whites = this->negativeData["edits"]["whites"];

    auto applyEdits = [&](float& r, float& g, float& b) {
        // Color balance
        r = rationalCurveFunction(r, rBalance);
        g = rationalCurveFunction(g, gBalance);
        b = rationalCurveFunction(b, bBalance);

        // Exposure
        r = curveExposureFunction(r, density);
        g = curveExposureFunction(g, density);
        b = curveExposureFunction(b, density);

        // Contrast
        r = contrastFunction(r, contrast);
        g = contrastFunction(g, contrast);
        b = contrastFunction(b, contrast);

        // blacks and whites
        r = blacksFunction(r, blacks);
        g = blacksFunction(g, blacks);
        b = blacksFunction(b, blacks);

        r = whitesFunction(r, whites);
        g = whitesFunction(g, whites);
        b = whitesFunction(b, whites);

        // Shadows and highlights
        r = shadowsFunction(r, shadows);
        g = shadowsFunction(g, shadows);
        b = shadowsFunction(b, shadows);
        
        r = highlightsFunction(r, highlights);
        g = highlightsFunction(g, highlights);
        b = highlightsFunction(b, highlights);

        // Display gamma
        r = gammaFunction(r, 1.0f/2.2f);
        g = gammaFunction(g, 1.0f/2.2f);
        b = gammaFunction(b, 1.0f/2.2f);
    };

    iterateImageMutableMultiThread(pixels, applyEdits);
}

void Negative::renderConversion(std::vector<float>& pixels, int width, int height, int channels, float scale) {

    // 1. CONVERT TO LINEAR
    // If the scan has a baked in gamma, remove that and convert into linear
    float correctionGamma = this->negativeData["conversion"]["scanGamma"];
    gamma(pixels, correctionGamma, EditChannel::RGB);
    std::println("Transformed image into linear by correcting a gamma of {}", correctionGamma);

    std::println("starting negative conversion pipeline");

    // 2. DETERMINE THE DENSEST VALUES AND THE FILM BORDER VALUES

    // Store measured values
    float transparentsR;
    float transparentsG;
    float transparentsB;
    float opaquestsR;
    float opaquestsG;
    float opaquestsB;

    // Border and densest have not both been sampled by the user
    if (!this->negativeData["conversion"]["hasDensest"] || !this->negativeData["conversion"]["hasBorder"]) {

        // Sample the border and densest automatically
        // First blur the image slightly to remove noise and extremities
        // Only sample the area indicated by this->scanArea if we are usign scanArea, otherwise sample the whole image

        ImageArea scanArea;

        // Check if we are using scan area
        std::println("has scan area: {}", this->getHasScanArea());
        if (this->getHasScanArea()) {
            scanArea = this->getScanArea(scale);
            std::println("image dimensions are {} * {}", width, height);
            std::println("scanarea dimesnions are x {} - {}, y {} - {}", scanArea.left, scanArea.right, scanArea.top, scanArea.bottom);
        }
        else {
            scanArea = ImageArea{0, 0, width, height};
        }
        
        // the blur will be stored in a separate vector, crop the blur vector to this->scanArea
        auto [blurredPixels, blurredWidth, blurredHeight] = crop(pixels, width, height, scanArea);
        std::println("the dimensions after the crop are: w: {}, h: {}", blurredWidth, blurredHeight);

        int blurRadius = this->negativeData["conversion"]["samplingBlur"];
        blurRadius = blurRadius * scale;
        std::println("blur radius is: {}", blurRadius);

        // Perform the actual blur
        boxFilter(blurredPixels, blurredWidth, blurredHeight, blurRadius);

        // Measure brightest and darkest pixels from the blurred image
        // Brightest = most transparent = low density
        // Darkest = most opaque = high density

        std::println("getting the brightest and darkest pixels");

        // The border has not been sampled by the user
        if (!this->negativeData["conversion"]["hasBorder"]) {
            // Sampling border
            std::println("sampling border");
            transparentsR = std::get<0>(getBrightestPixel(blurredPixels, EditChannel::R));
            transparentsG = std::get<1>(getBrightestPixel(blurredPixels, EditChannel::G));
            transparentsB = std::get<2>(getBrightestPixel(blurredPixels, EditChannel::B));
        }
        // The border has been sampled by the user
        else {
            transparentsR = this->negativeData["conversion"]["border"]["r"];
            transparentsG = this->negativeData["conversion"]["border"]["g"];
            transparentsB = this->negativeData["conversion"]["border"]["b"];
        }

        // The densest has not been sampled by the user
        if (!this->negativeData["conversion"]["hasDensest"]) {
            std::println("sampling densest");
            // Sampling densest
            opaquestsR = std::get<0>(getDarkestPixel(blurredPixels, EditChannel::R));
            opaquestsG = std::get<1>(getDarkestPixel(blurredPixels, EditChannel::G));
            opaquestsB = std::get<2>(getDarkestPixel(blurredPixels, EditChannel::B));
        }
        // The border has been sampled by the user
        else {
            opaquestsR = this->negativeData["conversion"]["densest"]["r"];
            opaquestsG = this->negativeData["conversion"]["densest"]["g"];
            opaquestsB = this->negativeData["conversion"]["densest"]["b"];
        }

    }
    // Border and densest have both been sampled by the user
    else {
        // Get the results from negativeData
        // Border
        transparentsR = this->negativeData["conversion"]["border"]["r"];
        transparentsG = this->negativeData["conversion"]["border"]["g"];
        transparentsB = this->negativeData["conversion"]["border"]["b"];

        // Densest
        opaquestsR = this->negativeData["conversion"]["densest"]["r"];
        opaquestsG = this->negativeData["conversion"]["densest"]["g"];
        opaquestsB = this->negativeData["conversion"]["densest"]["b"];
    }

    // Save the finally used densest and border values
    this->setBorder(transparentsR, transparentsG, transparentsB);
    this->setDensest(opaquestsR, opaquestsG, opaquestsB);

    // Convert the measured values to actual density values
    // NOTE: Do i actually need this though?
    
    float brightestRDensity = scanToDensity(opaquestsR);
    float brightestGDensity = scanToDensity(opaquestsG);
    float brightestBDensity = scanToDensity(opaquestsB);

    float darkestRDensity = scanToDensity(transparentsR);
    float darkestGDensity = scanToDensity(transparentsG);
    float darkestBDensity = scanToDensity(transparentsB);

    // 3. BALANCE CHANNELS ACCORDING TO MEASURED VALUES

    // Balance the black point and white point densities to all match the red channel
    // This makes sure that the film border is actually white and the densest is actually black
    levelsR(pixels, opaquestsR, transparentsR, densityToScan(brightestRDensity), densityToScan(darkestRDensity));
    levelsG(pixels, opaquestsG, transparentsG, densityToScan(brightestRDensity), densityToScan(darkestRDensity));
    levelsB(pixels, opaquestsB, transparentsB, densityToScan(brightestRDensity), densityToScan(darkestRDensity));

    // 4. INVERT

    // Perform the inversion and normalize to the highest and darkest values
    compromiseInvert(pixels, darkestRDensity, brightestRDensity);

    return;
}

void Negative::initializeSharpnessPreview() {
    std::println("initializing sharpness preview");

    // Use OIIO::ImageBuf for ease of scaling resizing etc
    OIIO::ImageSpec originalSpec(this->width, this->height, this->numberOfChannels, OIIO::TypeDesc::FLOAT);
    OIIO::ImageBuf originalBuf(originalSpec, this->originalPixels.data());

    // The sharpness preview will be taken from the center of the image
    int x = this->width/2;
    int y = this->height/2;
    int halfSize = this->SHARPNESS_PREVIEW_SIZE/2;
    int xBegin = x - halfSize;
    int yBegin = y - halfSize;
    int xEnd = x + halfSize;
    int yEnd = y + halfSize;

    std::println("the sharpness preview area is defined by left {}, top {}, right {}, bottom {}", xBegin, yBegin, xEnd, yEnd);

    OIIO::ROI previewRoi(xBegin, xEnd, yBegin, yEnd);

    this->sharpnessPreviewOriginalPixels.resize(this->SHARPNESS_PREVIEW_SIZE*this->SHARPNESS_PREVIEW_SIZE*3);
    originalBuf.get_pixels(previewRoi, OIIO::TypeDesc::FLOAT, this->sharpnessPreviewOriginalPixels.data());

    std::println("initialized sharpness preview");
}

// INITIALIZER
// ----------------------------------------------------------------------------------------------------------------

bool Negative::initializeNegative(std::filesystem::path imagePath) {

    this->path = imagePath;
    
    // 1. Read the full original image
    std::unique_ptr<OIIO::ImageInput> input = OIIO::ImageInput::open(imagePath.string());
    if (!input) {
        std::println("Failed to open file");
        return false;
    }

    const OIIO::ImageSpec& spec = input->spec();

    this->name = imagePath.stem();
    this->width = spec.width;
    this->height = spec.height;
    this->numberOfChannels = spec.nchannels;

    // not RGB or Grayscale
    if (this->numberOfChannels != 3 && this->numberOfChannels != 1) {
        std::println("The image contains wrong amount of channels");
        return false;
    }

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

    // First determine the working scale, meaning the amount that it has to be scaled by to fit into the preview box as listed in PREVIEW_SIZE
    const float widthScale = this->PREVIEW_SIZE / (1.0f * this->width);
    const float heightScale = this->PREVIEW_SIZE / (1.0f * this->height);
    this->workingScale = std::min(widthScale, heightScale);

    std::println("the working scale for this image is {}", this->workingScale);
    std::println("Generating working pixels");

    // Then generate the workingPixels array with this info
    // Use OIIO::ImageBuf for ease of scaling resizing etc
    OIIO::ImageSpec originalSpec(this->width, this->height, this->numberOfChannels, OIIO::TypeDesc::FLOAT);
    OIIO::ImageBuf originalBuf(originalSpec, this->originalPixels.data());

    // Calculate new dimensions for the working image
    this->workingWidth = std::ceil(this->width*this->workingScale);
    this->workingHeight = std::ceil(this->height*this->workingScale);
    std::println("trying to resize resolution to width: {} height: {}", this->workingWidth, this->workingHeight);

    // Resize the working image
    OIIO::ROI roi(0, this->workingWidth, 0, this->workingHeight, 0, 1, /*chans:*/ 0, 3);
    OIIO::ImageBuf workingBuf = OIIO::ImageBufAlgo::resample(originalBuf, true, roi);
    std::println("resized resolution to width: {} height: {}", workingBuf.spec().width, workingBuf.spec().height);
    
    // Extract the working data from the ImageBuf
    this->workingPixels.resize(this->workingWidth*this->workingHeight*3);
    workingBuf.get_pixels(OIIO::ROI::All(), OIIO::TypeDesc::FLOAT, this->workingPixels.data());
    std::println("Working pixels generated");

    // Generate the sharpness preview
    this->initializeSharpnessPreview();

    // Generate convertedPixels array
    this->convertedPixels = this->workingPixels;
    // Generate dragging array
    this->renderDraggingConversion();

    // 3. Read saved NegativeData if exists
    this->readNegativeData();

    // set the scanArea to be everything if not defined by the negativeData
    if(!this->negativeData["conversion"]["hasScanArea"]) {
        this->setScanArea({0, 0, this->width, this->height}, 1.0f);
    }

    // Convert the image or read from cache if it is supposed to be converted
    if(this->negativeData["general"]["isConverted"] && !this->readConversionCache()) {
        this->renderWorkingConversion();
    }

    // Check and convert the Sharpnesspreview as well
    this->renderSharpnessPreviewConversion();


    // 5. apply edits from the saved data
    this->renderWorkingEdits();
    this->renderDraggingEdits();
    this->renderSharpnessPreviewEdits();

    // 6. Create a thumbnail
    this->renderThumbnail();

    return true;
}


// CONSTRUCTORS
// ----------------------------------------------------------------------------------------------------------------

Negative::Negative(std::filesystem::path imagePath) {
    this->id = this->nextId;
    this->nextId++;
    this->successfullyCreated = this->initializeNegative(imagePath);
    return;
}

Negative::Negative(std::filesystem::path imagePath, int id) {
    this->id = id;
    this->successfullyCreated = this->initializeNegative(imagePath);
    return;
}

bool Negative::wasCreated() {
    return this->successfullyCreated;
}

// GETTERS FOR THE UI
// ----------------------------------------------------------------------------------------------------------------

// An optimized version by ChatGPT mixed with my additions of the original getpreview by me that used OIIO::ImageBufs and many copies.
// I couldn't easily do that much betetr so i think its better to let AI optimize for me
ImageData Negative::getPreview(bool dragging) {
    std::println("generating preview");

    int srcChannels = this->numberOfChannels;
    int previewWidth = dragging ? this->workingWidth*this->DRAGGING_SCALE : this->workingWidth;
    int previewHeight = dragging ? this->workingHeight*this->DRAGGING_SCALE : this->workingHeight;

    const std::vector<float>& previewPixels = dragging ? this->editedDraggingPixels : this->editedPixels;

    std::vector<uint8_t> previewData(previewWidth * previewHeight * 4); // always RGBA

    for (int i = 0; i < previewWidth * previewHeight; ++i) {
        float r = (srcChannels > 0 ? previewPixels[i*srcChannels + 0] : 0.0f);
        float g = (srcChannels > 1 ? previewPixels[i*srcChannels + 1] : 0.0f);
        float b = (srcChannels > 2 ? previewPixels[i*srcChannels + 2] : 0.0f);
        float a = 1.0f; // fully opaque

        // Clamp and scale to 0–255
        previewData[i*4 + 0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 1.0f) * 255.0f);
        previewData[i*4 + 1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 1.0f) * 255.0f);
        previewData[i*4 + 2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 1.0f) * 255.0f);
        previewData[i*4 + 3] = static_cast<uint8_t>(std::clamp(a, 0.0f, 1.0f) * 255.0f);
    }

    std::println("generated preview data");

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

ImageData Negative::getSharpnessPreview() {
    std::println("generating sharpness preview");

    int srcChannels = this->numberOfChannels;
    int previewWidth = this->SHARPNESS_PREVIEW_SIZE;
    int previewHeight = this->SHARPNESS_PREVIEW_SIZE;

    const std::vector<float>& previewPixels = this->sharpnessPreviewEditedPixels;

    std::vector<uint8_t> previewData(previewWidth * previewHeight * 4); // always RGBA

    for (int i = 0; i < previewWidth * previewHeight; ++i) {
        float r = (srcChannels > 0 ? previewPixels[i*srcChannels + 0] : 0.0f);
        float g = (srcChannels > 1 ? previewPixels[i*srcChannels + 1] : 0.0f);
        float b = (srcChannels > 2 ? previewPixels[i*srcChannels + 2] : 0.0f);
        float a = 1.0f; // fully opaque

        // Clamp and scale to 0–255
        previewData[i*4 + 0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 1.0f) * 255.0f);
        previewData[i*4 + 1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 1.0f) * 255.0f);
        previewData[i*4 + 2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 1.0f) * 255.0f);
        previewData[i*4 + 3] = static_cast<uint8_t>(std::clamp(a, 0.0f, 1.0f) * 255.0f);
    }

    std::println("generated sharpness preview data");

    return {
        previewData,
        previewWidth,
        previewHeight
    };
}

int Negative::getId() {
    return this->id;
}

int Negative::getWorkingWidth() {
    return this->workingWidth;
}

int Negative::getWorkingHeight() {
    return this->workingHeight;
}

float Negative::getWorkingScale() {
    std::println("working scale inside getworkingscale function is: {}", this->workingScale);
    return this->workingScale;
}

std::filesystem::path Negative::getPath() {
    return this->path;
}

// EXPORTING
// ----------------------------------------------------------------------------------------------------------------

bool Negative::exportPositive(std::filesystem::path imagePath, bool jpeg) {

    std::println("Saving positive");

    std::unique_ptr<std::vector<float>> finalPixels = this->renderFinal();

    if (jpeg) {
        std::string filePath = std::format("{}.jpeg", imagePath.string());

        // Use OIIO::ImageBuf for ease of transforming the pixel data type
        OIIO::ImageSpec originalSpec(this->width, this->height, this->numberOfChannels, OIIO::TypeDesc::FLOAT);
        OIIO::ImageBuf originalBuf(originalSpec, finalPixels->data());

        // Save 8bit for jpeg
        if (!originalBuf.write(filePath, OIIO::TypeDesc::UINT8)) {
            std::println("Failed to save image");
            return false;
        }
        std::println("Saved positive successfully");
        return true;
    }
    else {
        std::string filePath = std::format("{}.tif", imagePath.string());

        // Use OIIO::ImageBuf for ease of transformign the pixel data type
        OIIO::ImageSpec originalSpec(this->width, this->height, this->numberOfChannels, OIIO::TypeDesc::FLOAT);
        OIIO::ImageBuf originalBuf(originalSpec, finalPixels->data());

        // For now we want to save images as 16bit
        if (!originalBuf.write(filePath, OIIO::TypeDesc::UINT16)) {
            std::println("Failed to save image");
            return false;
        }
        std::println("Saved positive successfully");
        return true;
    }
}


// SETTING EDIT SETTINGS PRE-CONVERT
// ----------------------------------------------------------------------------------------------------------------

float Negative::setScanGamma(float value) {
    this->negativeData["conversion"]["scanGamma"] = value;
    return this->getScanGamma();
}

void Negative::setHasScanArea(bool has) {
    this->negativeData["conversion"]["hasScanArea"] = has;
}

void Negative::setScanArea(ImageArea area, float scale) {
    std::println("setScanArea called");
    std::println("scale = {}", scale);
    std::println("incoming scanArea to negative has left {}, top {}, right {}, bottom {}", area.left, area.top, area.right, area.bottom);
    this->negativeData["conversion"]["scanArea"]["left"] = area.left * (1.0f/scale);
    this->negativeData["conversion"]["scanArea"]["top"] = area.top * (1.0f/scale);
    this->negativeData["conversion"]["scanArea"]["right"] = area.right * (1.0f/scale);
    this->negativeData["conversion"]["scanArea"]["bottom"] = area.bottom * (1.0f/scale);
}

void Negative::setHasBorder(bool has) {
    this->negativeData["conversion"]["hasBorder"] = has;
}

void Negative::setBorder(float r, float g, float b) {
    this->negativeData["conversion"]["border"]["r"] = r;
    this->negativeData["conversion"]["border"]["g"] = g;
    this->negativeData["conversion"]["border"]["b"] = b;
}

void Negative::setBorderByCoords(int x, int y) {
    std::tuple<float, float, float> sample = this->samplePixels(x, y);
    this->setBorder(std::get<0>(sample), std::get<1>(sample), std::get<2>(sample));
}

void Negative::setHasDensest(bool has) {
    this->negativeData["conversion"]["hasDensest"] = has;
}

void Negative::setDensest(float r, float g, float b) {
    this->negativeData["conversion"]["densest"]["r"] = r;
    this->negativeData["conversion"]["densest"]["g"] = g;
    this->negativeData["conversion"]["densest"]["b"] = b;
}

void Negative::setDensestByCoords(int x, int y) {
    std::tuple<float, float, float> sample = this->samplePixels(x, y);
    this->setDensest(std::get<0>(sample), std::get<1>(sample), std::get<2>(sample));
}

void Negative::convert() {
    this->renderWorkingConversion();
    this->renderSharpnessPreviewConversion();
}

void Negative::resetConversion() {
    this->convertedPixels = this->workingPixels;
}


// SETTING EDIT SETTINGS POST-CONVERT
// ----------------------------------------------------------------------------------------------------------------

void Negative::applyPreset(std::filesystem::path presetPath) {
    std::println("applying preset");
    std::unique_ptr<nlohmann::json> presetDataPointer = std::move(this->readPresetdata(presetPath));

    if (presetDataPointer) {
        std::println("preset exists");
        nlohmann::json presetData = *presetDataPointer;

        float density = presetData["density"];
        float contrast = presetData["contrast"];
        float whites = presetData["whites"];
        float highlights = presetData["highlights"];
        float shadows = presetData["shadows"];
        float blacks = presetData["blacks"];
        bool autoWB = presetData["autoWB"];
        float rBalance = presetData["rBalance"];
        float gBalance = presetData["gBalance"];
        float bBalance = presetData["bBalance"];
        float saturation = presetData["saturation"];
        float sharpening = presetData["sharpening"];

        this->setDensity(density);
        this->setContrast(contrast);
        this->setWhites(whites);
        this->setHighlights(highlights);
        this->setShadows(shadows);
        this->setBlacks(blacks);
        this->setAutoWB(autoWB);
        this->setRBalance(rBalance);
        this->setGBalance(gBalance);
        this->setBBalance(bBalance);
        this->setSaturation(saturation);
        this->setSharpening(sharpening);
    }
}

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

void Negative::setAutoWB(bool has) {
    this->negativeData["edits"]["autoWB"] = has;
}

void Negative::setHasNeutral(bool has)
{
    this->negativeData["edits"]["hasNeutralPoint"] = has;
}

void Negative::setNeutral(float r, float g, float b) {
    this->negativeData["edits"]["neutralPoint"]["r"] = r;
    this->negativeData["edits"]["neutralPoint"]["g"] = g;
    this->negativeData["edits"]["neutralPoint"]["b"] = b;
}

void Negative::setNeutralByCoords(int x, int y) {
    std::tuple<float, float, float> sample = this->samplePixels(x, y);
    this->setNeutral(std::get<0>(sample), std::get<1>(sample), std::get<2>(sample));
}

float Negative::setRBalance(float value) {
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

float Negative::setSaturation(float value) {
    this->negativeData["edits"]["saturation"] = value;
    return this->getSaturation();
}

float Negative::setSharpening(float value) {
    this->negativeData["edits"]["sharpening"] = value;
    return this->getSharpening();
}

// GETTING EDIT SETTINGS FROM NEGATIVEDATA PRE-CONVERT
// ----------------------------------------------------------------------------------------------------------------

float Negative::getScanGamma() {
    return this->negativeData["conversion"]["scanGamma"];
}

bool Negative::getHasBorder() {
    return this->negativeData["conversion"]["hasBorder"];
}

ImageArea Negative::getScanArea(float scale) {
    std::println("getScanArea called");
    std::println("scale = {}", scale);
    ImageArea scanArea;
    float left = this->negativeData["conversion"]["scanArea"]["left"];
    float top = this->negativeData["conversion"]["scanArea"]["top"];
    float right = this->negativeData["conversion"]["scanArea"]["right"];
    float bottom = this->negativeData["conversion"]["scanArea"]["bottom"];
    scanArea.left = left * scale;
    scanArea.top = top * scale;
    scanArea.right = right * scale;
    scanArea.bottom = bottom * scale;
    std::println("reading scanArea from negative has left {}, top {}, right {}, bottom {}", scanArea.left, scanArea.top, scanArea.right, scanArea.bottom);
    return scanArea;
}

std::tuple<float, float, float> Negative::getBorder() {
    float r = this->negativeData["conversion"]["border"]["r"];
    float g = this->negativeData["conversion"]["border"]["g"];
    float b = this->negativeData["conversion"]["border"]["b"];
    return { r, g, b };
}

bool Negative::getHasDensest() {
    return this->negativeData["conversion"]["hasDensest"];
}

std::tuple<float, float, float> Negative::getDensest() {
    float r = this->negativeData["conversion"]["densest"]["r"];
    float g = this->negativeData["conversion"]["densest"]["g"];
    float b = this->negativeData["conversion"]["densest"]["b"];
    return { r, g, b };
}

bool Negative::getHasScanArea() {
    return this->negativeData["conversion"]["hasScanArea"];
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

bool Negative::getAutoWB() {
    return this->negativeData["edits"]["autoWB"];
}

bool Negative::getHasNeutral() {
    return this->negativeData["edits"]["hasNeutralPoint"];
}

std::tuple<float, float, float> Negative::getNeutral()
{
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

float Negative::getSaturation() {
    return this->negativeData["edits"]["saturation"];
}

float Negative::getSharpening() {
    return this->negativeData["edits"]["sharpening"];
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

void Negative::renderDraggingConversion() {

    // Use OIIO::ImageBuf for ease of scaling resizing etc
    OIIO::ImageSpec convertedSpec(this->workingWidth, this->workingHeight, this->numberOfChannels, OIIO::TypeDesc::FLOAT);
    OIIO::ImageBuf convertedBuf(convertedSpec, this->convertedPixels.data());

    int draggingWidth = this->workingWidth*this->DRAGGING_SCALE;
    int draggingHeight = this->workingHeight*this->DRAGGING_SCALE;

    // Resize the dragging image
    OIIO::ROI roi(0, draggingWidth, 0, draggingHeight, 0, 1, /*chans:*/ 0, 3);
    OIIO::ImageBuf draggingBuf = OIIO::ImageBufAlgo::resample(convertedBuf, true, roi);
    
    // Extract the dragging data from the ImageBuf
    this->convertedDraggingPixels.resize(draggingWidth * draggingHeight * 3);
    draggingBuf.get_pixels(OIIO::ROI::All(), OIIO::TypeDesc::FLOAT, this->convertedDraggingPixels.data());
    std::println("Dragging pixels generated");

    // Copy the converted pixels to the edited pixels as well
    this->editedDraggingPixels = this->convertedDraggingPixels;
}

void Negative::renderWorkingConversion() {

    // Copy the original workingPixels, we will be editing the convertedPixels vector
    this->convertedPixels = this->workingPixels;

    // Perform the conversion

    this->renderConversion(this->convertedPixels, this->workingWidth, this->workingHeight, this->numberOfChannels, this->workingScale);

    // SETUP FOR EDITING

    // Chache the result for quicker recovery in the future
    this->writeConversionCache();

    // Copy the converted pixels to edited pixels vector
    this->editedPixels = this->convertedPixels;

    // Also generate the dragging pixels for slider changing
    this->renderDraggingConversion();

    // Render Edits
    this->renderEdits(this->editedPixels, this->numberOfChannels);

    // Tell negativedata that this image is supposed to be converted
    this->negativeData["general"]["isConverted"] = true;

    // Save negativeData into file
    this->writeNegativeData();

    return;
}

void Negative::renderDraggingEdits() {
    this->editedDraggingPixels = this->convertedDraggingPixels;
    this->renderEdits(this->editedDraggingPixels, this->numberOfChannels);
    this->writeNegativeData();
    return;
}

void Negative::renderSharpnessPreviewConversion() {

    this->sharpnessPreviewConvertedPixels = this->sharpnessPreviewOriginalPixels;

    if(this->negativeData["general"]["isConverted"]) {
        // 1. CONVERT TO LINEAR
        // If the scan has a baked in gamma, remove that and convert into linear
        float correctionGamma = this->negativeData["conversion"]["scanGamma"];
        gamma(this->sharpnessPreviewConvertedPixels, correctionGamma, EditChannel::RGB);

        // 2. GET THE DENSEST VALUES AND THE FILM BORDER VALUES

        // get the measured values from the initial conversion
        auto [rBorder, gBorder, bBorder] = this->getBorder();
        auto [rDensest, gDensest, bDensest] = this->getDensest();

        // Convert the measured values to actual density values
        // NOTE: Do i actually need this though?
        
        float brightestRDensity = scanToDensity(rDensest);
        float brightestGDensity = scanToDensity(gDensest);
        float brightestBDensity = scanToDensity(bDensest);

        float darkestRDensity = scanToDensity(rBorder);
        float darkestGDensity = scanToDensity(gBorder);
        float darkestBDensity = scanToDensity(bBorder);

        // 3. BALANCE CHANNELS ACCORDING TO MEASURED VALUES

        // Balance the black point and white point densities to all match the red channel
        // This makes sure that the film border is actually white and the densest is actually black
        levelsR(this->sharpnessPreviewConvertedPixels, rDensest, rBorder, densityToScan(brightestRDensity), densityToScan(darkestRDensity));
        levelsG(this->sharpnessPreviewConvertedPixels, gDensest, gBorder, densityToScan(brightestRDensity), densityToScan(darkestRDensity));
        levelsB(this->sharpnessPreviewConvertedPixels, bDensest, bBorder, densityToScan(brightestRDensity), densityToScan(darkestRDensity));

        // 4. INVERT

        // Perform the inversion and normalize to the highest and darkest values
        compromiseInvert(this->sharpnessPreviewConvertedPixels, darkestRDensity, brightestRDensity);
    }
    return;
}

void Negative::renderSharpnessPreviewEdits() {
    this->sharpnessPreviewEditedPixels = this->sharpnessPreviewConvertedPixels;
    this->renderEdits(this->sharpnessPreviewEditedPixels, this->numberOfChannels);
    return;
}

void Negative::renderWorkingEdits() {
    this->editedPixels = this->convertedPixels;
    this->renderEdits(this->editedPixels, this->numberOfChannels);
    this->writeNegativeData();
    return;
}