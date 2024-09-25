#pragma once

#include <JuceHeader.h>
#include <vector>
#include "memory"

/* Class responsible for giving access and managing sound fonts */
class SoundFontManager
{
public:
    ///////////// Initialization ////////////////
    SoundFontManager() {};
    ~SoundFontManager();
    void generateSoundFiles();

    ///////////// Getters ///////////////////////
    std::shared_ptr<juce::File> getSoundFont(const juce::String& fontName) const;
    const juce::StringArray& getSoundFontNames() const;

private:
    juce::StringArray soundFontNames;
    std::vector<std::shared_ptr<juce::File>> soundFontFiles;

    void addSoundFont(const juce::String& fontName);
    std::unique_ptr<juce::MemoryInputStream> getResourceFromName(const juce::String& name);
    std::shared_ptr<juce::File> makeTemporarySoundFontFileFromResource(const juce::MemoryInputStream* resource, const juce::String& name);
    void cleanupTemporaryFiles();
};