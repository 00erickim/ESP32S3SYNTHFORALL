#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ESP32Encoder.h>
#include <SdFat.h>

// ... (핀 정의는 기존과 동일) ...
#define ENC1_A    2   
#define ENC1_B    1
#define ENC1_SW   40  
#define ENC2_A    18  
#define ENC2_B    42
#define ENC2_SW   41  

#define BTN_UP    4   
#define BTN_DOWN  5   
#define BTN_LEFT  7   
#define BTN_RIGHT 6   

#define BTN_A     8   
#define BTN_B     9   
#define BTN_X     3   
#define BTN_Y     10  

#define I2S_BCK   16
#define I2S_WS    15
#define I2S_DATA  17
#define SD_CS_PIN 46

// [모드 ENUM 정의]
enum AppMode { 
    MODE_MENU, 
    MODE_SPACE_VIEW, 
    MODE_DRUM, 
    MODE_KEYBOARD, 
    MODE_FINGER_DRUM, 
    MODE_DRONE, 
    MODE_GRANULAR_LOADER, 
    MODE_GRANULAR_PLAY, 
    MODE_TRACKER, 
    MODE_PIANO_ROLL,
    MODE_PROJECT_MGR,
    MODE_INSTRUMENT_SELECT // [★NEW] 악기 선택 메뉴 모드 추가
};

// ... (이하 내용은 기존 Globals.h와 동일하므로 그대로 유지) ...
// [EXTERN 선언]
extern TFT_eSPI tft;
extern TFT_eSprite img; 
extern ESP32Encoder enc1, enc2;
extern SdFat sd;

extern int trackVolume[4];    
extern int currentMixerTrack; 

extern int16_t* sampleBuffer;  
extern uint32_t sampleLength;  
extern bool isSampleLoaded;    
extern float grainPosition;    
extern float grainSize;        
extern float grainPitch;       
extern bool isGranularActive; 

struct TrackerStep {
    int8_t note;    
    uint8_t inst;   
    uint8_t vol;    
};
extern TrackerStep trackerPattern[64][16];
extern int trackerCursorStep;
extern int trackerCursorTrack;
extern int trackerPage; 
extern int trackerScrollTop;

extern int pianoRollCursorStep;
extern int pianoRollCursorNote;
extern int pianoRollScrollStep;
extern int pianoRollScrollNote;

extern bool drumPattern[8][64]; 
extern bool synthPattern[8][16]; 

extern int currentStep;         
extern bool isSequencerPlaying; 
extern int bpm;                 
extern int drumCursorTrack;     
extern int drumCursorStep;

extern int sequenceLength;     
extern int stepsPerBeat;       
extern int noteResolution;     
extern bool isDrumSettingsOpen; 
extern int drumSettingsCursor; 
extern int drumProjectSlot;

extern int trackType[8]; // 0: SYNTH, 1: SAMPLER

struct DrumSound {
    int attack; int decay; int sustain; int release;
    int pitch; int waveform; int volume; int pitchMod;  
};

struct SynthSound {
    int attack; int decay; int sustain; int release;
    int waveform; int filter; int res;      
};

extern DrumSound drumSounds[8]; 
extern SynthSound synthSounds[8]; 
extern DrumSound fingerSounds[8]; 

extern bool isSoundEditMode;    
extern int editTargetIndex;     
extern int editParamIndex;      

extern int arpSpeed;
extern int reverbLevel;
extern bool arpTriggered;
extern int arpWaveformType; 
extern int synthWaveform;   
extern int synthFilterVal;  
extern int currentNote;     

extern int dronePitch;      
extern int lfoRate;         
extern int lfoDepth;        
extern int filterResonance; 

extern int fileCursor;
extern int fileScrollTop;

extern bool isPatternGenMode;
extern int genLength;
extern int genHits;
extern int genNote;

extern int currentMode;
extern bool isAmbientPlaying;

#endif