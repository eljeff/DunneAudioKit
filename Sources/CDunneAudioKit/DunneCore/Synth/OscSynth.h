//
//  OscSynth.h
//
//  Created by Jeff Cooper on 9/14/21.
//
// Copyright AudioKit. All Rights Reserved.

#ifdef __cplusplus
#import <memory>

#define SYNTH_CHUNKSIZE 16            // process samples in "chunks" this size

namespace DunneCore
{
    struct OscVoice;
}

class OscSynth
{
public:
    OscSynth();
    ~OscSynth();

    /// returns system error code, nonzero only if a problem occurs
    int init(double sampleRate);

    /// call this to un-load all samples and clear the keymap
    void deinit();

    void playNote(unsigned noteNumber, unsigned velocity, float noteFrequency);
    void stopNote(unsigned noteNumber, bool immediate);
    void stopAllVoices();
    void restartVoices();
    void sustainPedal(bool down);

    void  setAmpAttackDurationSeconds(float value);
    float getAmpAttackDurationSeconds(void);
    void  setAmpDecayDurationSeconds(float value);
    float getAmpDecayDurationSeconds(void);
    void  setAmpSustainFraction(float value);
    float getAmpSustainFraction(void);
    void  setAmpReleaseDurationSeconds(float value);
    float getAmpReleaseDurationSeconds(void);

    void  setFilterAttackDurationSeconds(float value);
    float getFilterAttackDurationSeconds(void);
    void  setFilterDecayDurationSeconds(float value);
    float getFilterDecayDurationSeconds(void);
    void  setFilterSustainFraction(float value);
    float getFilterSustainFraction(void);
    void  setFilterReleaseDurationSeconds(float value);
    float getFilterReleaseDurationSeconds(void);

    void render(unsigned channelCount, unsigned sampleCount, float *outBuffers[]);

protected:

    struct InternalData;
    std::unique_ptr<InternalData> data;

    /// "event" counter for voice-stealing (reallocation)
    unsigned eventCounter;

    // performance parameters
    float masterVolume, pitchOffset, vibratoDepth, vibratoFreq;

    // parameters for mono-mode only

    // default false
    bool isMonophonic;

    // true if notes shouldn't retrigger in mono mode
    bool isLegato;

    // mono-mode state
    unsigned lastPlayedNoteNumber;
    float lastPlayedNoteFrequency;

    // filter parameters

    /// multiple of note frequency - 1.0 means cutoff at fundamental
    float cutoffMultiple;

    /// how much filter EG adds on top of cutoffMultiple
    float cutoffEnvelopeStrength;

    /// resonance [-20 dB, +20 dB] becomes linear [10.0, 0.1]
    float linearResonance;

    // temporary state
    bool stoppingAllVoices;

    void play(unsigned noteNumber, unsigned velocity, float noteFrequency, bool anotherKeyWasDown);
    void stop(unsigned noteNumber, bool immediate);

    DunneCore::OscVoice *voicePlayingNote(unsigned noteNumber);

    void  setWaveform(float value);
    float getWaveform(void);
};

#endif


