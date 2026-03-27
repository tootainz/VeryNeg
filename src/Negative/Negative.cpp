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
const float Negative::DRAGGING_SCALE = 0.3f;


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
    std::string dataName = std::filesystem::path(this->path)
        .replace_extension(".neg")
        .string();
    std::ifstream file(dataName);
    // No .neg file with this name exists
    if (!file) {
        std::println("failed to find negativeData file called {}", dataName);
        std::println("generating default data");
        file.open("assets/negativeDataTemplate.neg");
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
        file.open("assets/negativeDataTemplate.neg");
        this->negativeData = nlohmann::json::parse(file);
        file.close();
    }
    // Parsed succesfully but wrong version
    if (this->negativeData["version"] != this->NEGATIVEDATA_VERSION) {
        std::println("Incompatible NegativeData version");
        std::println("generating default data");
        file.open("assets/negativeDataTemplate.neg");
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
    std::unique_ptr<OIIO::ImageInput> input = OIIO::ImageInput::open(imagePath.string());

    if (!input) {
        std::println("Failed to open file");
        return false;
    }

    this->name = imagePath.stem();
    
    const OIIO::ImageSpec& spec = input->spec();
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
    this->renderEdits(false);

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

int Negative::getId() {
    return this->id;
}

std::filesystem::path Negative::getPath() {
    return this->path;
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

void Negative::renderEdits(bool dragging) {
    
    // Determine if this is used to render a smaller dragging preview or a final preview after dragging stops
    if (dragging) {
        this->editedDraggingPixels = this->convertedDraggingPixels;
    }
    else {
        this->editedPixels = this->convertedPixels;
    }
    std::vector<float>& targetPixels = dragging ? this->editedDraggingPixels : this->editedPixels;

    std::println("Editing the image");

    // Combined one loop
    float rBalance = exponentDampenerFixer(this->negativeData["edits"]["rBalance"]);
    float gBalance = exponentDampenerFixer(this->negativeData["edits"]["gBalance"]);
    float bBalance = exponentDampenerFixer(this->negativeData["edits"]["bBalance"]);
    
    float density = exponentDampenerFixer(this->negativeData["edits"]["density"]);

    float printDensity = this->negativeData["edits"]["density"];
    std::println("density is {}", printDensity);
    
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

        // Shadows and blacks
        r = shadowsFunction(r, shadows);
        g = shadowsFunction(g, shadows);
        b = shadowsFunction(b, shadows);

        r = blacksFunction(r, blacks);
        g = blacksFunction(g, blacks);
        b = blacksFunction(b, blacks);

        // Highlights and whites
        r = highlightsFunction(r, highlights);
        g = highlightsFunction(g, highlights);
        b = highlightsFunction(b, highlights);

        r = whitesFunction(r, whites);
        g = whitesFunction(g, whites);
        b = whitesFunction(b, whites);

        // Display gamma
        r = gammaFunction(r, 1.0f/2.2f);
        g = gammaFunction(g, 1.0f/2.2f);
        b = gammaFunction(b, 1.0f/2.2f);
    };

    iterateImageMutableMultiThread(targetPixels, applyEdits);

    // // Separate loops
    // // Color Balance
    // colorBalance(this->editedPixels, exponentDampenerFixer(this->negativeData["edits"]["rBalance"]), EditChannel::R);
    // colorBalance(this->editedPixels, exponentDampenerFixer(this->negativeData["edits"]["gBalance"]), EditChannel::G);
    // colorBalance(this->editedPixels, exponentDampenerFixer(this->negativeData["edits"]["bBalance"]), EditChannel::B);

    // // Exposure
    // curveExposure(this->editedPixels, exponentDampenerFixer(this->negativeData["edits"]["density"]), EditChannel::RGB);

    // // Contrast
    // contrast(this->editedPixels, exponentDampenerFixer(this->negativeData["edits"]["contrast"]), EditChannel::RGB);

    // shadows(this->editedPixels, this->negativeData["edits"]["shadows"]);
    // blacks(this->editedPixels, this->negativeData["edits"]["blacks"]);
    
    // highlights(this->editedPixels, this->negativeData["edits"]["highlights"]);
    // whites(this->editedPixels, this->negativeData["edits"]["whites"]);

    // // Apply display gamma
    // std::println("applying general display gamma correction");
    // gamma(this->editedPixels, 1.0/2.2, EditChannel::RGB);

    this->writeNegativeData();
}

void Negative::renderDragging() {
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

    this->editedDraggingPixels = this->convertedDraggingPixels;
}

void Negative::renderWorking() {

    // Copy the original workingPixels, we will be editing the convertedPixels vector
    this->convertedPixels = this->workingPixels;

    // 1. CONVERT TO LINEAR
    // If the scan has a baked in gamma, remove that and convert into linear
    float correctionGamma = 1.8;
    gamma(this->convertedPixels, correctionGamma, EditChannel::RGB);
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
        // Only sample the area indicated by this->scanArea

        // the blur will be stored in a separate vector, crop the blur vector to this->scanArea
        std::tuple<std::vector<float>, int, int> blurredCropResult = crop(this->convertedPixels, this->workingWidth, this->workingHeight, this->getScanArea());

        // Get the size of the cropped blur vector
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

    // Some handy prints for debugging
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

    // 3. BALANCE CHANNELS ACCORDING TO MEASURED VALUES

    // Balance the black point and white point densities to all match the red channel
    // This makes sure that the film border is actually white and the densest is actually black
    levelsR(this->convertedPixels, opaquestsR, transparentsR, densityToScan(brightestRDensity), densityToScan(darkestRDensity));
    levelsG(this->convertedPixels, opaquestsG, transparentsG, densityToScan(brightestRDensity), densityToScan(darkestRDensity));
    levelsB(this->convertedPixels, opaquestsB, transparentsB, densityToScan(brightestRDensity), densityToScan(darkestRDensity));

    // 4. INVERT

    // Perform the inversion and normalize to the highest and darkest values
    compromiseInvert(this->convertedPixels, darkestRDensity, brightestRDensity);

    // Handy measurements for debugging
    // TODO: remove from production

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

    // 4. WHITE BALANCE

    // Auto White balance
    //grayWorld(this->convertedPixels);

    // 5. SETUP FOR EDITING

    // Chache the result for quicker recovery in the future
    this->writeConversionCache();

    // Copy the converted pixels to edited pixels vector
    this->editedPixels = this->convertedPixels;

    // Also generate the dragging pixels for slider changing
    this->renderDragging();

    // Render Edits
    this->renderEdits(false);

    // Tell negativedata that this image is supposed to be converted
    this->negativeData["general"]["isConverted"] = true;

    // Save negativeData into file
    this->writeNegativeData();

    return;
}