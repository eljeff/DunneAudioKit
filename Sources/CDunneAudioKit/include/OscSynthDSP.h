//
//  OscSynthDSP.h
//  
//
//  Created by Jeff Cooper on 9/14/21.

#pragma once

#import <AVFoundation/AVFoundation.h>
#import "Interop.h"

typedef NS_ENUM(AUParameterAddress, OscSynthParameter)
{
    // ramped parameters

    OscSynthParameterMasterVolume,
    OscSynthParameterPitchBend,
    OscSynthParameterVibratoDepth,
    OscSynthParameterVibratoFreq,
    OscSynthParameterFilterCutoff,
    OscSynthParameterFilterStrength,
    OscSynthParameterFilterResonance,

    // simple parameters

    OscSynthParameterAttackDuration,
    OscSynthParameterDecayDuration,
    OscSynthParameterSustainLevel,
    OscSynthParameterReleaseDuration,
    OscSynthParameterFilterAttackDuration,
    OscSynthParameterFilterDecayDuration,
    OscSynthParameterFilterSustainLevel,
    OscSynthParameterFilterReleaseDuration,

    OscSynthParameterWaveform,
    OscSynthParameterIsMonophonic,
    OscSynthParameterIsLegato,

    // ensure this is always last in the list, to simplify parameter addressing
    OscSynthParameterRampDuration,
};

CF_EXTERN_C_BEGIN
DSPRef akOscSynthCreateDSP(void);
CF_EXTERN_C_END
