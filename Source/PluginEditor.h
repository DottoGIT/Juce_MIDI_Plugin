#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BinaryData.h"

/* Class responsible for creating and mentaining user interface */

class MIDI_Synth_PianoAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                              public juce::ComboBox::Listener,
                                              public juce::MidiKeyboardStateListener
{
public:
    
    ///////////// Initialization ////////////////
    MIDI_Synth_PianoAudioProcessorEditor (MIDI_Synth_PianoAudioProcessor&);
    ~MIDI_Synth_PianoAudioProcessorEditor() override;

    ///////////// Audio Processor Editor ////////
    void paint (juce::Graphics&) override;
    void resized() override;

    ///////////// ComponentListeners ////////////
    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    
private:
    
    ///////////// Component Initialization //////
    void initComponents();
    void initSoundFontSelector();
    void initKeyboard();

    ///////////// Variables /////////////////////
    MIDI_Synth_PianoAudioProcessor& audioProcessor;

    juce::ComboBox soundFontSelector;
    juce::MidiKeyboardComponent keyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MIDI_Synth_PianoAudioProcessorEditor)
};
