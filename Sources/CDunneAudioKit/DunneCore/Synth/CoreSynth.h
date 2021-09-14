// Copyright AudioKit. All Rights Reserved.

#ifdef __cplusplus
#import <memory>

#define SYNTH_CHUNKSIZE 16            // process samples in "chunks" this size

namespace DunneCore
{
    struct SynthVoice;
}

class CoreSynth
{
public:
    CoreSynth();
    ~CoreSynth();
    
    /// returns system error code, nonzero only if a problem occurs
    int init(double sampleRate);
    
    /// call this to un-load all samples and clear the keymap
    void deinit();
    
    void playNote(unsigned noteNumber, unsigned velocity, float noteFrequency);
    void stopNote(unsigned noteNumber, bool immediate);
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

    void  setOsc1Mix(float value);
    float getOsc1Mix(void);
    void  setOsc1Phases(float value);
    float getOsc1Phases(void);
    void  setOsc1FreqSpread(float value);
    float getOsc1FreqSpread(void);
    void  setOsc1PanSpread(float value);
    float getOsc1PanSpread(void);
    void  setOsc1PitchOffset(float value);
    float getOsc1PitchOffset(void);

    void  setOsc2Mix(float value);
    float getOsc2Mix(void);

    void  setOsc3Mix(float value);
    float getOsc3Mix(void);
    
    void render(unsigned channelCount, unsigned sampleCount, float *outBuffers[]);
    
protected:
 
    struct InternalData;
    std::unique_ptr<InternalData> data;
    
    /// "event" counter for voice-stealing (reallocation)
    unsigned eventCounter;
    
    // performance parameters
    float masterVolume, pitchOffset, vibratoDepth;
    
    // filter parameters
    
    /// multiple of note frequency - 1.0 means cutoff at fundamental
    float cutoffMultiple;
    
    /// how much filter EG adds on top of cutoffMultiple
    float cutoffEnvelopeStrength;
    
    /// resonance [-20 dB, +20 dB] becomes linear [10.0, 0.1]
    float linearResonance;
    
    void play(unsigned noteNumber, unsigned velocity, float noteFrequency);
    void stop(unsigned noteNumber, bool immediate);
    
    DunneCore::SynthVoice *voicePlayingNote(unsigned noteNumber);
};

#endif

