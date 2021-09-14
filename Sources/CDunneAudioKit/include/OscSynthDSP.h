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

    // ensure this is always last in the list, to simplify parameter addressing
    OscSynthParameterRampDuration,
};

AK_API DSPRef akOscSynthCreateDSP(void);
