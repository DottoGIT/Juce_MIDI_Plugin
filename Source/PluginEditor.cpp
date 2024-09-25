#include "PluginProcessor.h"
#include "PluginEditor.h"

///////////// Initialization ////////////////
MIDI_Synth_PianoAudioProcessorEditor::MIDI_Synth_PianoAudioProcessorEditor (MIDI_Synth_PianoAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), keyboardComponent(p.getKeyState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    initComponents();
    setSize(800, 200);
}

MIDI_Synth_PianoAudioProcessorEditor::~MIDI_Synth_PianoAudioProcessorEditor()
{
    audioProcessor.getKeyState().removeListener(this);
}

///////////// Audio Processor Editor ////////

void MIDI_Synth_PianoAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
}

void MIDI_Synth_PianoAudioProcessorEditor::resized()
{
    const int selectorHeight = 50;
    soundFontSelector.setBounds(0, 0, getWidth(), selectorHeight);
    keyboardComponent.setBounds(0, selectorHeight, getWidth(), getHeight() - selectorHeight);
}

///////////// ComponentListeners ////////////

void MIDI_Synth_PianoAudioProcessorEditor::handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    audioProcessor.noteOn(midiNoteNumber, static_cast<int>(velocity * 127), midiChannel);
}

void MIDI_Synth_PianoAudioProcessorEditor::handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float)
{
    audioProcessor.noteOff(midiNoteNumber, midiChannel);
}

void MIDI_Synth_PianoAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &soundFontSelector)
    {
        juce::String fontName = soundFontSelector.getText();
        if (!audioProcessor.loadSoundFont(fontName))
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Invalid .sf2 File",
                "Could not load the soundfont file to processor: " + fontName);
        }
    }
}

///////////// Component Initialization //////

void MIDI_Synth_PianoAudioProcessorEditor::initComponents()
{
    initSoundFontSelector();
    initKeyboard();
}

void MIDI_Synth_PianoAudioProcessorEditor::initSoundFontSelector()
{
    juce::StringArray soundFontNames = audioProcessor.getSoundFontManager().getSoundFontNames();

    addAndMakeVisible(soundFontSelector);
    soundFontSelector.addListener(this);
    soundFontSelector.addItemList(soundFontNames, 1);
    soundFontSelector.setSelectedItemIndex(0);
}

void MIDI_Synth_PianoAudioProcessorEditor::initKeyboard()
{
    addAndMakeVisible(keyboardComponent);
    audioProcessor.getKeyState().addListener(this);
}
