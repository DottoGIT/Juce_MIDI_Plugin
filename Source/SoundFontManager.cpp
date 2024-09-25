#include "SoundFontManager.h"

///////////// Initialization ////////////////

SoundFontManager::~SoundFontManager()
{
    cleanupTemporaryFiles();
}

void SoundFontManager::generateSoundFiles()
{
    soundFontNames = {
        "our_piano.sf2",
        "test_sample.sf2"
    };

    for (const auto fontName : soundFontNames)
    {
        addSoundFont(fontName);
    }
}

std::shared_ptr<juce::File> SoundFontManager::getSoundFont(const juce::String& fontName) const
{
    int fontIndex = soundFontNames.indexOf(fontName);

    if (fontIndex == -1 || fontIndex >= soundFontFiles.size() || !soundFontFiles[fontIndex])
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Plugin Error", "Failed to find " + fontName + " inside plugin data");
        return nullptr;
    }

    return soundFontFiles[fontIndex];
}

///////////// Getters ///////////////////////

const juce::StringArray& SoundFontManager::getSoundFontNames() const
{
    return soundFontNames;
}

void SoundFontManager::addSoundFont(const juce::String& fontName)
{
    std::unique_ptr<juce::MemoryInputStream> fontResource = getResourceFromName(fontName);
    std::shared_ptr<juce::File> fontFile = makeTemporarySoundFontFileFromResource(fontResource.get(), fontName);

    soundFontFiles.push_back(fontFile);
}

std::unique_ptr<juce::MemoryInputStream> SoundFontManager::getResourceFromName(const juce::String& fontName)
{
    if (fontName == "our_piano.sf2")
    {
        const void* data = BinaryData::our_piano_sf2;
        int size = BinaryData::our_piano_sf2Size;

        return std::make_unique<juce::MemoryInputStream>(data, size, false);
    }
    else if (fontName == "test_sample.sf2")
    {
        const void* data = BinaryData::test_sample_sf2;
        int size = BinaryData::test_sample_sf2Size;
        return std::make_unique<juce::MemoryInputStream>(data, size, false);
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "File system error", "Failed to find font resources: " + fontName);
        return nullptr;
    }
}

std::shared_ptr<juce::File> SoundFontManager::makeTemporarySoundFontFileFromResource(const juce::MemoryInputStream* resource, const juce::String& tempFileName)
{
    juce::File tempDirectory = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("PianoPluginTemp");
    
    if (!tempDirectory.exists() && !tempDirectory.createDirectory())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "File system error", "Failed to create temporary directory to store soundfont: " + tempFileName);
        return nullptr;
    }

    std::shared_ptr<juce::File> tempFile = std::make_shared<juce::File>(tempDirectory.getChildFile(tempFileName));

    if (tempFile->replaceWithData(resource->getData(), resource->getDataSize()))
    {
        return tempFile;
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "File system error", "Soundfont failed to write into temporary file: " + tempFileName);
        return nullptr;
    }
}

void SoundFontManager::cleanupTemporaryFiles()
{
    for (auto& file : soundFontFiles)
    {
        if (file && file->exists())
        {
            file->deleteFile();
        }
    }
    soundFontFiles.clear();
}