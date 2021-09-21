//
//  OscOscSynth.mm
//  
//
//  Created by Jeff Cooper on 9/14/21.
//

#import "OscSynthDSP.h"
#include <math.h>

#import "DSPBase.h"
#include "DunneCore/Synth/OscSynth.h"
#include "LinearParameterRamp.h"

struct OscSynthDSP : DSPBase, OscSynth
{
    // ramped parameters
    LinearParameterRamp masterVolumeRamp;
    LinearParameterRamp pitchBendRamp;
    LinearParameterRamp vibratoDepthRamp;
    LinearParameterRamp vibratoFreqRamp;
    LinearParameterRamp filterCutoffRamp;
    LinearParameterRamp filterStrengthRamp;
    LinearParameterRamp filterResonanceRamp;

    OscSynthDSP();
    void init(int channelCount, double sampleRate) override;
    void deinit() override;

    void setParameter(uint64_t address, float value, bool immediate) override;
    float getParameter(uint64_t address) override;

    void handleMIDIEvent(const AUMIDIEvent &midiEvent) override;
    void process(FrameRange) override;
};

DSPRef akOscSynthCreateDSP() {
    return new OscSynthDSP();
}

OscSynthDSP::OscSynthDSP() : DSPBase(/*inputBusCount*/0), OscSynth()
{
    masterVolumeRamp.setTarget(1.0, true);
    pitchBendRamp.setTarget(0.0, true);
    vibratoDepthRamp.setTarget(0.0, true);
    vibratoFreqRamp.setTarget(3.0, true);
    filterCutoffRamp.setTarget(1000.0, true);
    filterResonanceRamp.setTarget(1.0, true);
}

void OscSynthDSP::init(int channelCount, double sampleRate)
{
    DSPBase::init(channelCount, sampleRate);
    OscSynth::init(sampleRate);
}

void OscSynthDSP::deinit()
{
    DSPBase::deinit();
    OscSynth::deinit();
}

void OscSynthDSP::setParameter(uint64_t address, float value, bool immediate)
{
    switch (address) {
        case OscSynthParameterRampDuration:
            masterVolumeRamp.setRampDuration(value, sampleRate);
            pitchBendRamp.setRampDuration(value, sampleRate);
            vibratoDepthRamp.setRampDuration(value, sampleRate);
            vibratoFreqRamp.setRampDuration(value, sampleRate);
            filterCutoffRamp.setRampDuration(value, sampleRate);
            filterResonanceRamp.setRampDuration(value, sampleRate);
            break;

        case OscSynthParameterMasterVolume:
            masterVolumeRamp.setTarget(value, immediate);
            break;
        case OscSynthParameterPitchBend:
            pitchBendRamp.setTarget(value, immediate);
            break;
        case OscSynthParameterVibratoDepth:
            vibratoDepthRamp.setTarget(value, immediate);
            break;
        case OscSynthParameterVibratoFreq:
            vibratoFreqRamp.setTarget(value, immediate);
            break;
        case OscSynthParameterFilterCutoff:
            filterCutoffRamp.setTarget(value, immediate);
            break;
        case OscSynthParameterFilterStrength:
            filterStrengthRamp.setTarget(value, immediate);
            break;
        case OscSynthParameterFilterResonance:
            filterResonanceRamp.setTarget(pow(10.0, -0.05 * value), immediate);
            break;

        case OscSynthParameterAttackDuration:
            setAmpAttackDurationSeconds(value);
            break;
        case OscSynthParameterDecayDuration:
            setAmpDecayDurationSeconds(value);
            break;
        case OscSynthParameterSustainLevel:
            setAmpSustainFraction(value);
            break;
        case OscSynthParameterReleaseDuration:
            setAmpReleaseDurationSeconds(value);
            break;

        case OscSynthParameterFilterAttackDuration:
            setFilterAttackDurationSeconds(value);
            break;
        case OscSynthParameterFilterDecayDuration:
            setFilterDecayDurationSeconds(value);
            break;
        case OscSynthParameterFilterSustainLevel:
            setFilterSustainFraction(value);
            break;
        case OscSynthParameterFilterReleaseDuration:
            setFilterReleaseDurationSeconds(value);
            break;
        case OscSynthParameterWaveform:
            setWaveform(value);
            break;
        case OscSynthParameterIsMonophonic:
            isMonophonic = value > 0.5;
            break;
        case OscSynthParameterIsLegato:
            isLegato = value > 0.5;
            break;
    }
}

float OscSynthDSP::getParameter(uint64_t address)
{
    switch (address) {
        case OscSynthParameterRampDuration:
            return pitchBendRamp.getRampDuration(sampleRate);

        case OscSynthParameterMasterVolume:
            return masterVolumeRamp.getTarget();
        case OscSynthParameterPitchBend:
            return pitchBendRamp.getTarget();
        case OscSynthParameterVibratoDepth:
            return vibratoDepthRamp.getTarget();
        case OscSynthParameterVibratoFreq:
            return vibratoFreqRamp.getTarget();
        case OscSynthParameterFilterCutoff:
            return filterCutoffRamp.getTarget();
        case OscSynthParameterFilterStrength:
            return filterStrengthRamp.getTarget();
        case OscSynthParameterFilterResonance:
            return -20.0f * log10(filterResonanceRamp.getTarget());

        case OscSynthParameterAttackDuration:
            return getAmpAttackDurationSeconds();
        case OscSynthParameterDecayDuration:
            return getAmpDecayDurationSeconds();
        case OscSynthParameterSustainLevel:
            return getAmpSustainFraction();
        case OscSynthParameterReleaseDuration:
            return getAmpReleaseDurationSeconds();

        case OscSynthParameterFilterAttackDuration:
            return getFilterAttackDurationSeconds();
        case OscSynthParameterFilterDecayDuration:
            return getFilterDecayDurationSeconds();
        case OscSynthParameterFilterSustainLevel:
            return getFilterSustainFraction();
        case OscSynthParameterWaveform:
            return getWaveform();
        case OscSynthParameterIsMonophonic:
            return isMonophonic ? 1.0f : 0.0f;
        case OscSynthParameterIsLegato:
            return isLegato ? 1.0f : 0.0f;
    }
    return 0;
}

void OscSynthDSP::handleMIDIEvent(const AUMIDIEvent &midiEvent)
{
    if (midiEvent.length != 3) return;
    uint8_t status = midiEvent.data[0] & 0xF0;
    //uint8_t channel = midiEvent.data[0] & 0x0F; // works in omni mode.
    switch (status) {
        case MIDI_NOTE_OFF : {
            uint8_t note = midiEvent.data[1];
            if (note > 127) break;
            stopNote(note, false);
            break;
        }
        case MIDI_NOTE_ON : {
            uint8_t note = midiEvent.data[1];
            uint8_t veloc = midiEvent.data[2];
            if (note > 127 || veloc > 127) break;
            auto f = pow(2.0, (note - 69.0) / 12.0) * 440.0;
            playNote(note, veloc, f);
            break;
        }
        case MIDI_CONTINUOUS_CONTROLLER : {
            uint8_t num = midiEvent.data[1];
            if (num == 64) {
                uint8_t value = midiEvent.data[2];
                if (value <= 63) {
                    sustainPedal(false);
                } else {
                    sustainPedal(true);
                }
            }
            if (num == 123) { // all notes off
                stopAllVoices();
            }
            break;
        }
    }
}

void OscSynthDSP::process(FrameRange range)
{

    float *pLeft = (float *)outputBufferList->mBuffers[0].mData + range.start;
    float *pRight = (float *)outputBufferList->mBuffers[1].mData + range.start;

    memset(pLeft, 0, range.count * sizeof(float));
    memset(pRight, 0, range.count * sizeof(float));

    // process in chunks of maximum length CHUNKSIZE
    for (int frameIndex = 0; frameIndex < range.count; frameIndex += SYNTH_CHUNKSIZE) {
        int frameOffset = int(frameIndex + range.start);
        int chunkSize = range.count - frameIndex;
        if (chunkSize > SYNTH_CHUNKSIZE) chunkSize = SYNTH_CHUNKSIZE;

        // ramp parameters
        masterVolumeRamp.advanceTo(now + frameOffset);
        masterVolume = (float)masterVolumeRamp.getValue();
        pitchBendRamp.advanceTo(now + frameOffset);
        pitchOffset = (float)pitchBendRamp.getValue();
        vibratoDepthRamp.advanceTo(now + frameOffset);
        vibratoDepth = (float)vibratoDepthRamp.getValue();
        vibratoFreqRamp.advanceTo(now + frameOffset);
        vibratoFreq = (float)vibratoFreqRamp.getValue();
        filterCutoffRamp.advanceTo(now + frameOffset);
        cutoffMultiple = (float)filterCutoffRamp.getValue();
        filterStrengthRamp.advanceTo(now + frameOffset);
        cutoffEnvelopeStrength = (float)filterStrengthRamp.getValue();
        filterResonanceRamp.advanceTo(now + frameOffset);
        linearResonance = (float)filterResonanceRamp.getValue();

        // get data
        float *outBuffers[2];
        outBuffers[0] = (float *)outputBufferList->mBuffers[0].mData + frameOffset;
        outBuffers[1] = (float *)outputBufferList->mBuffers[1].mData + frameOffset;
        unsigned channelCount = outputBufferList->mNumberBuffers;
        OscSynth::render(channelCount, chunkSize, outBuffers);
    }
}

AK_REGISTER_DSP(OscSynthDSP, "osyn")
AK_REGISTER_PARAMETER(OscSynthParameterMasterVolume)
AK_REGISTER_PARAMETER(OscSynthParameterPitchBend)
AK_REGISTER_PARAMETER(OscSynthParameterVibratoDepth)
AK_REGISTER_PARAMETER(OscSynthParameterVibratoFreq)
AK_REGISTER_PARAMETER(OscSynthParameterFilterCutoff)
AK_REGISTER_PARAMETER(OscSynthParameterFilterStrength)
AK_REGISTER_PARAMETER(OscSynthParameterFilterResonance)
AK_REGISTER_PARAMETER(OscSynthParameterAttackDuration)
AK_REGISTER_PARAMETER(OscSynthParameterDecayDuration)
AK_REGISTER_PARAMETER(OscSynthParameterSustainLevel)
AK_REGISTER_PARAMETER(OscSynthParameterReleaseDuration)
AK_REGISTER_PARAMETER(OscSynthParameterFilterAttackDuration)
AK_REGISTER_PARAMETER(OscSynthParameterFilterDecayDuration)
AK_REGISTER_PARAMETER(OscSynthParameterFilterSustainLevel)
AK_REGISTER_PARAMETER(OscSynthParameterFilterReleaseDuration)
AK_REGISTER_PARAMETER(OscSynthParameterWaveform)
AK_REGISTER_PARAMETER(OscSynthParameterIsMonophonic)
AK_REGISTER_PARAMETER(OscSynthParameterIsLegato)
AK_REGISTER_PARAMETER(OscSynthParameterRampDuration)
