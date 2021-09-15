// Copyright AudioKit. All Rights Reserved.

#include "SustainPedalLogic.h"

namespace DunneCore
{

    SustainPedalLogic::SustainPedalLogic()
    {
        for (int i=0; i < kMidiNoteNumbers; i++) keyDown[i] = isPlaying[i] = false;
        pedalIsDown = false;
    }
    
    bool SustainPedalLogic::keyDownAction(unsigned noteNumber)
    {
        bool noteShouldStopBeforePlayingAgain = false;
        
        if (pedalIsDown && keyDown[noteNumber]) {
            noteShouldStopBeforePlayingAgain = true;
        } else {
            activeNotes.push_back(noteNumber);
            keyDown[noteNumber] = true;
        }

        printf("activeNotes:\n");
        for (int i = 0; i < activeNotes.size(); i++) {
            printf("activeNote %i = %i\n", i, activeNotes.at(i));
        }
        isPlaying[noteNumber] = true;
        return noteShouldStopBeforePlayingAgain;
    }
    
    bool SustainPedalLogic::keyUpAction(unsigned noteNumber)
    {
        bool noteShouldStop = false;
        
        if (!pedalIsDown)
        {
            noteShouldStop = true;
            isPlaying[noteNumber] = false;
        }
        activeNotes.erase(std::remove(activeNotes.begin(), activeNotes.end(), noteNumber), activeNotes.end());
        keyDown[noteNumber] = false;
        return noteShouldStop;
    }

    void SustainPedalLogic::pedalDown() { pedalIsDown = true; }
    
    void SustainPedalLogic::pedalUp() { pedalIsDown = false; }
    
    bool SustainPedalLogic::isNoteSustaining(unsigned noteNumber)
    {
        return isPlaying[noteNumber] && !keyDown[noteNumber];
    }

    bool SustainPedalLogic::isAnyKeyDown()
    {
        for (int i = 0; i < kMidiNoteNumbers; i++) if (keyDown[i]) return true;
        return false;
    }

    int SustainPedalLogic::firstKeyDown()
    {
        for (int i = 0; i < kMidiNoteNumbers; i++) if (keyDown[i]) return i;
        return -1;
    }

    int SustainPedalLogic::mostRecentKeyDown()
    {
        if (activeNotes.size() > 0) {
            return activeNotes.back();
        } else {
            return -1;
        }
    }
}
