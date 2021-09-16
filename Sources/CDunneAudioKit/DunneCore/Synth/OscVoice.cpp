//
//  OscVoice.cpp
//  
//  A single-oscillator synth voice
//  Created by Jeff Cooper on 9/14/21.
//

#include "OscVoice.h"
#include <stdio.h>

namespace DunneCore
{

    void OscVoice::init(double sampleRate,
                          WaveStack *pOsc1Stack,
                          OscVoiceParameters *pParams,
                          EnvelopeParameters *pEnvParameters)
    {
        samplingRate = sampleRate;
        
        pParameters = pParams;
        event = 0;
        noteNumber = -1;

        osc1.init(sampleRate, pOsc1Stack);
        osc1.setPhases(pParameters->osc1.phases);
        osc1.setFreqSpread(pParameters->osc1.frequencySpread);
        osc1.setPanSpread(pParameters->osc1.panSpread);

        leftFilter.init(sampleRate);
        rightFilter.init(sampleRate);
        leftFilter.setStages(pParameters->filterStages);
        rightFilter.setStages(pParameters->filterStages);

        ampEG.init();
        filterEG.init();
    }

    void OscVoice::start(unsigned evt, unsigned noteNum, float frequency, float volume)
    {
        printf("voice start\n");
        event = evt;
        noteVolume = volume;
        osc1.setFrequency(frequency * pow(2.0f, pParameters->osc1.pitchOffset / 12.0f));
        ampEG.start();
        filterEG.start();

        noteFrequency = frequency;
        noteNumber = noteNum;
    }

    void OscVoice::restart(unsigned evt, float volume)
    {
        printf("voice restart (same note)\n");
        event = evt;
        newNoteNumber = -1;
        newNoteVol = volume;
        ampEG.restart();
    }

    void OscVoice::restart(unsigned evt, unsigned noteNum, float frequency, float volume)
    {
        printf("voice restart (new note) %i\n", noteNum);
        event = evt;
        newNoteNumber = noteNum;
        newNoteVol = volume;
        noteFrequency = frequency;
        ampEG.restart();
    }
    void OscVoice::restartMonophonic(unsigned evt, unsigned noteNum, float frequency, float volume)
    {
        printf("voice restartMonophonic (new note) %i\n", noteNum);
        event = evt;
        newNoteNumber = noteNum;
        noteNumber = noteNum;
        newNoteVol = volume;
        noteFrequency = frequency;
        ampEG.restart();
    }

    void OscVoice::restartNewNoteLegato(unsigned evt, unsigned noteNum, float frequency)
    {
        printf("voice restartNewNoteLegato\n");
        event = evt;
        osc1.setFrequency(frequency * pow(2.0f, pParameters->osc1.pitchOffset / 12.0f));
        noteNumber = noteNum;
        noteFrequency = frequency;
    }

    void OscVoice::release(unsigned evt)
    {
        printf("voice release\n");
        event = evt;
        ampEG.release();
        filterEG.release();
    }

    void OscVoice::stop(unsigned evt)
    {
        printf("voice stop\n");
        event = evt;
        noteNumber = -1;
        ampEG.reset();
        filterEG.reset();
    }

    bool OscVoice::prepToGetSamples(float masterVolume,
                                      float phaseDeltaMultiplier,
                                      float cutoffMultiple,
                                      float cutoffStrength,
                                      float resLinear)
    {
        if (ampEG.isIdle()) return true;

        if (ampEG.isPreStarting())
        {
            float ampeg = ampEG.getSample();
            tempGain = masterVolume * noteVolume * ampeg;
            if (!ampEG.isPreStarting())
            {
                noteVolume = newNoteVol;
                tempGain = masterVolume * noteVolume * ampeg;

                if (newNoteNumber >= 0)
                {
                    // restarting a "stolen" voice with a new note number
                    printf("restarting stolen voice newNote %i note %i\n", newNoteNumber, noteNumber);
                    osc1.setFrequency(noteFrequency * pow(2.0f, pParameters->osc1.pitchOffset / 12.0f));
                    noteNumber = newNoteNumber;
                }
                ampEG.start();
                filterEG.start();
            }
        }
        else
            tempGain = masterVolume * noteVolume * ampEG.getSample();

        // standard ADSR EG
        double cutoffFrequency = noteFrequency * (1.0f + cutoffMultiple + cutoffStrength * filterEG.getSample());

        leftFilter.setParameters(cutoffFrequency, resLinear);
        rightFilter.setParameters(cutoffFrequency, resLinear);

        osc1.phaseDeltaMultiplier = phaseDeltaMultiplier;

        return false;
    }

    bool OscVoice::getSamples(int sampleCount, float *leftOutput, float *rightOutput)
    {
        for (int i=0; i < sampleCount; i++)
        {
            float leftSample = 0.0f;
            float rightSample = 0.0f;
            osc1.getSamples(&leftSample, &rightSample, pParameters->osc1.mixLevel);

            if (pParameters->filterStages == 0)
            {
                *leftOutput++ += tempGain * leftSample;
                *rightOutput++ += tempGain * rightSample;
            }
            else
            {
                *leftOutput++ += leftFilter.process(tempGain * leftSample);
                *rightOutput++ += rightFilter.process(tempGain * rightSample);
            }
        }
        return false;
    }

}
