// Copyright AudioKit. All Rights Reserved.

#pragma once
#include <vector>

namespace DunneCore
{
    static const int kMidiNoteNumbers = 128;
    
    class SustainPedalLogic
    {
        bool keyDown[kMidiNoteNumbers];
        bool isPlaying[kMidiNoteNumbers];
        bool pedalIsDown;
        std::vector<uint8_t> activeNotes;
        
    public:
        SustainPedalLogic();
        
        // return true if given note should stop playing
        bool keyDownAction(unsigned noteNumber);
        bool keyUpAction(unsigned noteNumber);
        
        void pedalDown();
        bool isNoteSustaining(unsigned noteNumber);
        bool isAnyKeyDown();
        int firstKeyDown();
        int mostRecentKeyDown();
        void pedalUp();
    };

}
