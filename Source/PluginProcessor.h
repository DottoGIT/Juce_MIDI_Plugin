#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include "Fluidlite/include/fluidlite.h"
#include "SoundFontManager.h"

constexpr int __CHANNEL__ = 1;
constexpr int NUMBER_OF_VOICES = 255;
constexpr double GAIN = 1.0f;

/* Class responsible for processing audio output */

class MIDI_Synth_PianoAudioProcessor : public juce::AudioProcessor
{
public:
    ///////////// Initialization ////////////////
    MIDI_Synth_PianoAudioProcessor();
    ~MIDI_Synth_PianoAudioProcessor() override;

    ///////////// AudioProcessor ////////////////
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    ///////////// Editor Processor //////////////
    juce::AudioProcessorEditor* createEditor() override;

    ///////////// Midi //////////////////////////
    void noteOn(int note, int velocity, int channel = __CHANNEL__);
    void noteOff(int note, int channel = __CHANNEL__);
    void processMidiMessage(const juce::MidiMessage& message);

    ///////////// Setters ///////////////////////
    void changeProgramName(int index, const juce::String& newName) override {}
    void setCurrentProgram(int index) override {}
    void setStateInformation(const void* data, int sizeInBytes) override {}

    bool loadSoundFont(const juce::String& fontName);


    ///////////// Getters ///////////////////////
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    const juce::String getProgramName(int index) override { return {}; }
    void getStateInformation(juce::MemoryBlock& destData) override {}

   juce::MidiKeyboardState& getKeyState() { return keyboardState; }
    SoundFontManager& getSoundFontManager() { return soundFontManager; }

private:

    SoundFontManager soundFontManager;

    juce::CriticalSection lock;
    fluid_settings_t* settings;
    fluid_synth_t* synth;
    juce::MidiKeyboardState keyboardState;

    juce::String loadedSoundFontName;
    juce::MidiMessageCollector midiCollector;
    int sfontID = -1;


    ///////////// MIDI Controls /////////////////
    void sendContinuousControllerMessage(int control, int value, int channel = __CHANNEL__);
    void sendPitchBendMessage(int value, int channel = __CHANNEL__);
    void sendChannelPressure(int value, int channel = __CHANNEL__);

    ///////////// Gain Adjustments //////////////
    float calculateGainFactorToCancelVelocity(int velocity);
    void applyDynamicGainAdjustment(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MIDI_Synth_PianoAudioProcessor)
};
