//
//  OscVoice.h
//  
//
//  Created by Jeff Cooper on 9/14/21.
//

#pragma once
#include <math.h>

#include "EnsembleOscillator.h"
#include "ADSREnvelope.h"
#include "CoreEnvelope.h"
#include "MultiStageFilter.h"

namespace DunneCore
{

    enum OscWaveform {
        sinusoid = 0,
        square = 1,
        triangle = 2,
        sawtooth = 3,
        hammond = 4
    };

    struct OscParameters
    {
        OscWaveform waveform;   // 0 = sin, 1 = sqr, 2 = tri, 3 = saw, 4 = hammond
        int phases;             // 1 to 10, or 0 to disable oscillator
        float frequencySpread;  // cents
        float panSpread;        // fraction 0 = no spread, 1 = max spread
        float pitchOffset;      // semitones, relative to MIDI note
        float mixLevel;         // fraction
    };

    struct OscVoiceParameters
    {
        OscParameters osc1;
        /// 1 to 4, or 0 to disable filter
        int filterStages;
    };

    struct OscVoice
    {
        OscVoiceParameters *pParameters;

        EnsembleOscillator osc1;
        MultiStageFilter leftFilter, rightFilter;            // two filters (left/right)
        ADSREnvelope ampEG, filterEG;

        unsigned event = 0;      // last "event number" associated with this voice
        int noteNumber = -1;     // MIDI note number, or -1 if not playing any note
        float noteFrequency = 0; // note frequency in Hz
        float noteVolume = 0;    // fraction 0.0 - 1.0, based on MIDI velocity

        // temporary holding variables
        int newNoteNumber;  // holds new note number while damping note before restarting
        float newNoteVol;   // holds new note volume while damping note before restarting
        float tempGain;     // product of global volume, note volume, and amp EG

        float phaseOffset = (float)rand() / RAND_MAX;   // generate a random number 0-1 to offset lfo phase per voice

        OscVoice(std::mt19937* gen) : noteNumber(-1), osc1(gen) {}

        void init(double sampleRate,
                  WaveStack *pOsc1Stack,
                  OscVoiceParameters *pParameters,
                  EnvelopeParameters *pEnvParameters);

        void updateAmpAdsrParameters() { ampEG.updateParams(); }
        void updateFilterAdsrParameters() { filterEG.updateParams(); }

        void start(unsigned evt, unsigned noteNumber, float frequency, float volume);
        void restart(unsigned evt, float volume);
        void restart(unsigned evt, unsigned noteNumber, float frequency, float volume);
        void release(unsigned evt);
        void stop(unsigned evt);

        // return true if amp envelope is finished
        bool prepToGetSamples(float masterVol,
                              float phaseDeltaMultiplier,
                              float cutoffMultiple,
                              float cutoffStrength,
                              float resLinear);
        bool getSamples(int sampleCount, float *leftOuput, float *rightOutput);
    };

}
