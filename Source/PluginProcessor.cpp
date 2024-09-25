#include "PluginProcessor.h"
#include "PluginEditor.h"


///////////// Initialization ////////////////

MIDI_Synth_PianoAudioProcessor::MIDI_Synth_PianoAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
         : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    soundFontManager = SoundFontManager();
    soundFontManager.generateSoundFiles();

    settings = new_fluid_settings();
    synth = new_fluid_synth(settings);

    fluid_synth_set_polyphony(synth, NUMBER_OF_VOICES);
    fluid_synth_set_gain(synth, GAIN);
}

MIDI_Synth_PianoAudioProcessor::~MIDI_Synth_PianoAudioProcessor()
{
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

///////////// AudioProcessor ////////////////

void MIDI_Synth_PianoAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    fluid_synth_set_sample_rate(synth, (float)sampleRate);
    midiCollector.reset(sampleRate); 
}

void MIDI_Synth_PianoAudioProcessor::releaseResources()
{
    fluid_synth_system_reset(synth);
}

void MIDI_Synth_PianoAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    for (const juce::MidiMessageMetadata& metadata : midiMessages)
    {
        const juce::MidiMessage& message = metadata.getMessage();
        processMidiMessage(message);
    }

    applyDynamicGainAdjustment(buffer, midiMessages);

    juce::MidiBuffer incomingMidi;
    midiCollector.removeNextBlockOfMessages(incomingMidi, buffer.getNumSamples());
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);
    keyboardState.processNextMidiBuffer(incomingMidi, 0, buffer.getNumSamples(), true);

    const juce::ScopedLock l(lock);
    if (synth != nullptr)
    {
        fluid_synth_write_float(synth,
            buffer.getNumSamples(),
            buffer.getWritePointer(0), 0, 1,
            buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr, 0, 1);
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MIDI_Synth_PianoAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

///////////// Editor Processor //////////////

juce::AudioProcessorEditor* MIDI_Synth_PianoAudioProcessor::createEditor()
{
    return new MIDI_Synth_PianoAudioProcessorEditor(*this);
}


///////////// MidiKeyboardStateListener /////

void MIDI_Synth_PianoAudioProcessor::noteOn(int note, int velocity, int channel)
{
    fluid_synth_noteon(synth, channel, note, velocity);
}

void MIDI_Synth_PianoAudioProcessor::noteOff(int note, int channel)
{
    fluid_synth_noteoff(synth, channel, note);
}

void MIDI_Synth_PianoAudioProcessor::processMidiMessage(const juce::MidiMessage& message)
{
    if (message.isNoteOn()) {
        noteOn(message.getNoteNumber(), message.getVelocity());
    }
    else if (message.isNoteOff()) {
        noteOff(message.getNoteNumber());
    }
    else if (message.isController()) {
        sendContinuousControllerMessage(message.getControllerNumber(), message.getControllerValue());
    }
    else if (message.isPitchWheel()) {
        sendPitchBendMessage(message.getPitchWheelValue());
    }
    else if (message.isChannelPressure()) {
        sendChannelPressure(message.getChannelPressureValue());
    }
}


///////////// Setters ///////////////////////

bool MIDI_Synth_PianoAudioProcessor::loadSoundFont(const juce::String& fontName)
{
    if (fontName == loadedSoundFontName)
    {
        return true;
    }

    const juce::ScopedLock l(lock);

    std::shared_ptr<juce::File> fontToLoad = soundFontManager.getSoundFont(fontName);

    if (fontToLoad == nullptr)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "SoundFont Error", "Could not find the SoundFont: " + fontName);
        return false;
    }

    if (fluid_synth_sfcount(synth) > 0)
    {
        int err = fluid_synth_sfunload(synth, (unsigned int)sfontID, true);
        if (err == -1)
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "SoundFont Unload Error", "Failed to unload the previous SoundFont.");
            return false;
        }
    }

    sfontID = fluid_synth_sfload(synth, fontToLoad->getFullPathName().toRawUTF8(), true);
    if (sfontID == -1)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "SoundFont Load Error", "Failed to load the SoundFont: " + fontName);
        return false;
    }

    loadedSoundFontName = fontName;
    return true;
}


///////////// Getters ///////////////////////

bool MIDI_Synth_PianoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MIDI_Synth_PianoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MIDI_Synth_PianoAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

void MIDI_Synth_PianoAudioProcessor::sendContinuousControllerMessage(int control, int value, int channel)
{
    fluid_synth_cc(synth, channel, control, value);
}

void MIDI_Synth_PianoAudioProcessor::sendPitchBendMessage(int value, int channel)
{
    fluid_synth_pitch_bend(synth, channel, value);
}

void MIDI_Synth_PianoAudioProcessor::sendChannelPressure(int value, int channel)
{
    fluid_synth_channel_pressure(synth, channel, value);
}

float MIDI_Synth_PianoAudioProcessor::calculateGainFactorToCancelVelocity(int velocity)
{
    if (velocity == 0) return 1.0f;
    float normalizedVelocity = static_cast<float>(velocity) / 127.0f;
    float gainFactor = 1.0f / (normalizedVelocity * normalizedVelocity);
    return gainFactor;
}

///////////// Gain Adjustments //////////////

void MIDI_Synth_PianoAudioProcessor::applyDynamicGainAdjustment(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    for (const auto metadata : midiMessages)
    {
        const auto& message = metadata.getMessage();
        if (message.isNoteOn())
        {
            int velocity = message.getVelocity();
            float gainFactor = calculateGainFactorToCancelVelocity(velocity);
            buffer.applyGain(gainFactor);
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MIDI_Synth_PianoAudioProcessor();
}
