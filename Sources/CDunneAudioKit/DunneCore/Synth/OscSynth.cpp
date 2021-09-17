//
//  OscSynth.cpp
//  
//
//  Created by Jeff Cooper on 9/14/21.

#include "OscSynth.h"
#include "FunctionTable.h"
#include "OscVoice.h"
#include "WaveStack.h"
#include "SustainPedalLogic.h"

#include <math.h>
#include <list>
#include <random>

using std::unique_ptr;

#define MAX_VOICE_COUNT 32      // number of voices
#define MIDI_NOTENUMBERS 128    // MIDI offers 128 distinct note numbers

struct OscSynth::InternalData
{
    std::mt19937 gen{0};

    /// array of voice resources
    unique_ptr<DunneCore::OscVoice> voice[MAX_VOICE_COUNT];

    DunneCore::WaveStack waveform1;      // WaveStacks are shared by all voice oscillators
    DunneCore::FunctionTableOscillator vibratoLFO;             // one vibrato LFO shared by all voices
    DunneCore::SustainPedalLogic pedalLogic;

    // simple parameters
    DunneCore::OscVoiceParameters voiceParameters;
    DunneCore::ADSREnvelopeParameters ampEGParameters;
    DunneCore::ADSREnvelopeParameters filterEGParameters;

    DunneCore::EnvelopeSegmentParameters segParameters[8];
    DunneCore::EnvelopeParameters envParameters;
};

OscSynth::OscSynth()
: eventCounter(0)
, masterVolume(1.0f)
, pitchOffset(0.0f)
, vibratoDepth(0.0f)
, cutoffMultiple(1024.0f)
, cutoffEnvelopeStrength(0.0f)
, linearResonance(1.0f)
, isMonophonic(false)
, isLegato(false)
, data(new InternalData)
{
    for (int i=0; i < MAX_VOICE_COUNT; i++)
    {
        data->voice[i] = unique_ptr<DunneCore::OscVoice>(new DunneCore::OscVoice(&data->gen));
        data->voice[i]->ampEG.pParameters = &data->ampEGParameters;
        data->voice[i]->filterEG.pParameters = &data->filterEGParameters;
    }

    data->voiceParameters.osc1.waveform = DunneCore::sinusoid;
    data->voiceParameters.osc1.phases = 1;
    data->voiceParameters.osc1.frequencySpread = 25.0f;
    data->voiceParameters.osc1.panSpread = 0.95f;
    data->voiceParameters.osc1.pitchOffset = 0.0f;
    data->voiceParameters.osc1.mixLevel = 1.0f;

    data->voiceParameters.filterStages = 2;

    data->segParameters[0].initialLevel = 0.0f;   // attack: ramp quickly to 0.2
    data->segParameters[0].finalLevel = 0.2f;
    data->segParameters[0].seconds = 0.01f;
    data->segParameters[1].initialLevel = 0.2f;   // hold at 0.2 for 1 sec
    data->segParameters[1].finalLevel = 0.2;
    data->segParameters[1].seconds = 1.0f;
    data->segParameters[2].initialLevel = 0.2f;   // decay: fall to 0.0 in 0.5 sec
    data->segParameters[2].finalLevel = 0.0f;
    data->segParameters[2].seconds = 0.5f;
    data->segParameters[3].initialLevel = 0.0f;   // sustain pump up: up to 1.0 in 0.1 sec
    data->segParameters[3].finalLevel = 1.0f;
    data->segParameters[3].seconds = 0.1f;
    data->segParameters[4].initialLevel = 1.0f;   // sustain pump down: down to 0 again in 0.5 sec
    data->segParameters[4].finalLevel = 0.0f;
    data->segParameters[4].seconds = 0.5f;
    data->segParameters[5].initialLevel = 0.0f;   // release: from wherever we leave off
    data->segParameters[5].finalLevel = 0.0f;     // down to 0
    data->segParameters[5].seconds = 0.5f;        // in 0.5 sec
}

OscSynth::~OscSynth()
{
}

int OscSynth::init(double sampleRate)
{
    DunneCore::FunctionTable waveform;
    int length = 1 << DunneCore::WaveStack::maxBits;
    waveform.init(length);
    switch (data->voiceParameters.osc1.waveform) {
        case DunneCore::sinusoid:
            waveform.sinusoid(1.0f);
            break;
        case DunneCore::square:
            waveform.square(1.0f);
            break;
        case DunneCore::triangle:
            waveform.triangle(1.0f);
            break;
        case DunneCore::sawtooth:
            waveform.sawtooth(1.0f);
            break;
        case DunneCore::hammond:
            waveform.hammond(1.0f);
            break;
    }
    data->waveform1.initStack(waveform.waveTable);

    data->ampEGParameters.updateSampleRate((float)(sampleRate/SYNTH_CHUNKSIZE));
    data->filterEGParameters.updateSampleRate((float)(sampleRate/SYNTH_CHUNKSIZE));

    data->vibratoLFO.waveTable.sinusoid();
    data->vibratoLFO.init(sampleRate/SYNTH_CHUNKSIZE, 5.0f);

    data->envParameters.init((float)(sampleRate/SYNTH_CHUNKSIZE), 6, data->segParameters, 3, 0, 5);

    for (int i=0; i < MAX_VOICE_COUNT; i++)
    {
        data->voice[i]->init(sampleRate, &data->waveform1, &data->voiceParameters, &data->envParameters);
    }

    return 0;   // no error
}

void OscSynth::deinit()
{
}

DunneCore::OscVoice *OscSynth::voicePlayingNote(unsigned noteNumber)
{
    for (int i=0; i < MAX_VOICE_COUNT; i++)
    {
        if (isMonophonic) {
            if (data->voice[i]->noteNumber > 0) return data->voice[i].get();
        } else {
            if (data->voice[i]->noteNumber == noteNumber) return data->voice[i].get();
        }
    }
    return 0;
}

void OscSynth::playNote(unsigned noteNumber, unsigned velocity, float noteFrequency)
{
    eventCounter++;
    bool anotherKeyWasDown = data->pedalLogic.isAnyKeyDown();
    data->pedalLogic.keyDownAction(noteNumber);
    play(noteNumber, velocity, noteFrequency, anotherKeyWasDown);
}

void OscSynth::stopNote(unsigned noteNumber, bool immediate)
{
    eventCounter++;
    if (immediate || data->pedalLogic.keyUpAction(noteNumber))
        stop(noteNumber, immediate);
}

void OscSynth::sustainPedal(bool down)
{
    eventCounter++;
    if (down) data->pedalLogic.pedalDown();
    else {
        for (int nn=0; nn < MIDI_NOTENUMBERS; nn++)
        {
            if (data->pedalLogic.isNoteSustaining(nn))
                stop(nn, false);
        }
        data->pedalLogic.pedalUp();
    }
}

void OscSynth::play(unsigned noteNumber, unsigned velocity, float noteFrequency, bool anotherKeyWasDown)
{

    if (isMonophonic)
    {
        if (isLegato && anotherKeyWasDown)
        {
            // is our one and only voice playing some note?
            DunneCore::OscVoice *pVoice = data->voice[0].get();
            if (pVoice->noteNumber >= 0)
            {
                pVoice->restartNewNoteLegato(eventCounter, noteNumber, noteFrequency);
            }
            else
            {
                pVoice->start(eventCounter, noteNumber, noteFrequency, velocity / 127.0f);
            }
            lastPlayedNoteNumber = noteNumber;
            return;
        }
        else
        {
            // monophonic but not legato: always start a new note
            DunneCore::OscVoice *pVoice = data->voice[0].get();
            if (pVoice->noteNumber >= 0)
                pVoice->restart(eventCounter, noteNumber, noteFrequency, velocity / 127.0f);
            else
                pVoice->start(eventCounter, noteNumber, noteFrequency, velocity / 127.0f);
            
            lastPlayedNoteNumber = noteNumber;
            return;
        }
    } else {
    // is any voice already playing this note?
        DunneCore::OscVoice *pVoice = voicePlayingNote(noteNumber);
        if (pVoice)
        {
            // re-start the note
            pVoice->restart(eventCounter, velocity / 127.0f);
            return;
        }

        // find a free voice (with noteNumber < 0) to play the note
        int polyphony = isMonophonic ? 1 : MAX_VOICE_COUNT;
        for (int i=0; i < polyphony; i++)
        {
            auto pVoice = data->voice[i].get();
            if (pVoice->noteNumber < 0)
            {
                // found a free voice: assign it to play this note
                pVoice->start(eventCounter, noteNumber, noteFrequency, velocity / 127.0f);
                lastPlayedNoteNumber = noteNumber;
                return;
            }
        }

        // all oscillators in use: find "stalest" voice to steal
        unsigned greatestDiffOfAll = 0;
        DunneCore::OscVoice *pStalestVoiceOfAll = 0;
        unsigned greatestDiffInRelease = 0;
        DunneCore::OscVoice *pStalestVoiceInRelease = 0;
        for (int i=0; i < MAX_VOICE_COUNT; i++)
        {
            auto pVoice = data->voice[i].get();
            unsigned diff = eventCounter - pVoice->event;
            if (pVoice->ampEG.isReleasing())
            {
                if (diff > greatestDiffInRelease)
                {
                    greatestDiffInRelease = diff;
                    pStalestVoiceInRelease = pVoice;
                }
            }
            if (diff > greatestDiffOfAll)
            {
                greatestDiffOfAll = diff;
                pStalestVoiceOfAll = pVoice;
            }
        }

        if (pStalestVoiceInRelease != 0)
        {
            // We have a stalest note in its release phase: restart that one
            pStalestVoiceInRelease->restart(eventCounter, noteNumber, noteFrequency, velocity / 127.0f);
        }
        else
        {
            // No notes in release phase: restart the "stalest" one we could find
            pStalestVoiceOfAll->restart(eventCounter, noteNumber, noteFrequency, velocity / 127.0f);
        }
    }
}

void OscSynth::stop(unsigned noteNumber, bool immediate)
{
    DunneCore::OscVoice *pVoice = voicePlayingNote(noteNumber);
    if (pVoice == 0) {
        return;
    }

    if (immediate) {
        pVoice->stop(eventCounter);
    } else if (isMonophonic) {
        int fallbackKey = data->pedalLogic.mostRecentKeyDown();
        auto noteFrequency = pow(2.0, (fallbackKey - 69.0) / 12.0) * 440.0;
        if (fallbackKey < 0) {
            pVoice->release(eventCounter);
        } else if (isLegato) {
            pVoice->restartNewNoteLegato(eventCounter, (unsigned)fallbackKey, noteFrequency);
        } else if (fallbackKey == pVoice->noteNumber) {
            return;
        } else {
            unsigned velocity = 100;
            if (pVoice->noteNumber >= 0) {
                pVoice->restartMonophonic(eventCounter, fallbackKey, noteFrequency, velocity / 127.0f);
            } else {
                pVoice->start(eventCounter, fallbackKey, noteFrequency, velocity / 127.0f);
            }
        }
    } else {
        pVoice->release(eventCounter);
    }
}

void OscSynth::render(unsigned channelCount, unsigned sampleCount, float *outBuffers[])
{
    float *pOutLeft = outBuffers[0];
    float *pOutRight = outBuffers[1];
    data->vibratoLFO.setFrequency(vibratoFreq);
    data->vibratoLFO.advanceLFO();

    for (int i=0; i < MAX_VOICE_COUNT; i++)
    {
        auto pVoice = data->voice[i].get();

        float pitchDev = pitchOffset + vibratoDepth * data->vibratoLFO.getSample(pVoice->phaseOffset);
        float phaseDeltaMultiplier = pow(2.0f, pitchDev / 12.0);

        int nn = pVoice->noteNumber;
        if (nn >= 0)
        {
            if (pVoice->prepToGetSamples(masterVolume, phaseDeltaMultiplier, cutoffMultiple, cutoffEnvelopeStrength, linearResonance) ||
                pVoice->getSamples(sampleCount, pOutLeft, pOutRight))
            {
                stopNote(nn, true);
            }
        }
    }
}

void OscSynth::setAmpAttackDurationSeconds(float value)
{
    data->ampEGParameters.setAttackDurationSeconds(value);
    for (int i = 0; i < MAX_VOICE_COUNT; i++) data->voice[i]->updateAmpAdsrParameters();
}
float OscSynth::getAmpAttackDurationSeconds(void)
{
    return data->ampEGParameters.getAttackDurationSeconds();
}
void  OscSynth::setAmpDecayDurationSeconds(float value)
{
    data->ampEGParameters.setDecayDurationSeconds(value);
    for (int i = 0; i < MAX_VOICE_COUNT; i++) data->voice[i]->updateAmpAdsrParameters();
}
float OscSynth::getAmpDecayDurationSeconds(void)
{
    return data->ampEGParameters.getDecayDurationSeconds();
}
void  OscSynth::setAmpSustainFraction(float value)
{
    data->ampEGParameters.sustainFraction = value;
    for (int i = 0; i < MAX_VOICE_COUNT; i++) data->voice[i]->updateAmpAdsrParameters();
}
float OscSynth::getAmpSustainFraction(void)
{
    return data->ampEGParameters.sustainFraction;
}
void  OscSynth::setAmpReleaseDurationSeconds(float value)
{
    data->ampEGParameters.setReleaseDurationSeconds(value);
    for (int i = 0; i < MAX_VOICE_COUNT; i++) data->voice[i]->updateAmpAdsrParameters();
}

float OscSynth::getAmpReleaseDurationSeconds(void)
{
    return data->ampEGParameters.getReleaseDurationSeconds();
}

void  OscSynth::setFilterAttackDurationSeconds(float value)
{
    data->filterEGParameters.setAttackDurationSeconds(value);
    for (int i = 0; i < MAX_VOICE_COUNT; i++) data->voice[i]->updateFilterAdsrParameters();
}
float OscSynth::getFilterAttackDurationSeconds(void)
{
    return data->filterEGParameters.getAttackDurationSeconds();
}
void  OscSynth::setFilterDecayDurationSeconds(float value)
{
    data->filterEGParameters.setDecayDurationSeconds(value);
    for (int i = 0; i < MAX_VOICE_COUNT; i++) data->voice[i]->updateFilterAdsrParameters();
}
float OscSynth::getFilterDecayDurationSeconds(void)
{
    return data->filterEGParameters.getDecayDurationSeconds();
}
void  OscSynth::setFilterSustainFraction(float value)
{
    data->filterEGParameters.sustainFraction = value;
    for (int i = 0; i < MAX_VOICE_COUNT; i++) data->voice[i]->updateFilterAdsrParameters();
}
float OscSynth::getFilterSustainFraction(void)
{
    return data->filterEGParameters.sustainFraction;
}
void  OscSynth::setFilterReleaseDurationSeconds(float value)
{
    data->filterEGParameters.setReleaseDurationSeconds(value);
    for (int i = 0; i < MAX_VOICE_COUNT; i++) data->voice[i]->updateFilterAdsrParameters();
}
float OscSynth::getFilterReleaseDurationSeconds(void)
{
    return data->filterEGParameters.getReleaseDurationSeconds();
}

void OscSynth::setWaveform(float value)
{
    data->voiceParameters.osc1.waveform = (DunneCore::OscWaveform)value;
}
float OscSynth::getWaveform(void)
{
    return (float)data->voiceParameters.osc1.waveform;
}
