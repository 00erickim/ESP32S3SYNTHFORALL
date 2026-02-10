#include "Globals.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);
ESP32Encoder enc1, enc2;
SdFat sd;

// [딥슬립 데이터 보존 변수]
RTC_DATA_ATTR bool drumPattern[8][64] = {0}; 
RTC_DATA_ATTR bool synthPattern[8][16] = {0};
RTC_DATA_ATTR int trackVolume[4] = {200, 200, 200, 200}; 
RTC_DATA_ATTR int bpm = 120;

// [드럼 시퀀서 설정값 보존]
RTC_DATA_ATTR int sequenceLength = 32; 
RTC_DATA_ATTR int stepsPerBeat = 4;   
RTC_DATA_ATTR int noteResolution = 4; 
// [★추가됨] 트랙 타입 보존 (기본값 0 = SYNTH)
RTC_DATA_ATTR int trackType[8] = {0}; 

// [일반 변수 초기화]
int16_t* sampleBuffer = NULL;
uint32_t sampleLength = 0;
bool isSampleLoaded = false;
float grainPosition = 0.0f;
float grainSize = 5000.0f;
float grainPitch = 1.0f;
bool isGranularActive = false;

int currentMixerTrack = 0;
int currentMode = MODE_MENU; 
bool isAmbientPlaying = false; 

int currentStep = 0;
bool isSequencerPlaying = false;
int drumCursorTrack = 0;
int drumCursorStep = 0;

TrackerStep trackerPattern[64][16];
int trackerCursorStep = 0;
int trackerCursorTrack = 0;
int trackerScrollTop = 0;
int trackerPage = 0; 

int pianoRollCursorStep = 0;
int pianoRollCursorNote = 60;
int pianoRollScrollStep = 0;
int pianoRollScrollNote = 48;

DrumSound drumSounds[8]; 
SynthSound synthSounds[8]; 
DrumSound fingerSounds[8]; 

bool isSoundEditMode = false; 
int editTargetIndex = 0;
int editParamIndex = 0;

int synthWaveform = 0;
int synthFilterVal = 200;
int currentNote = -1;
int arpSpeed = 150;      
int reverbLevel = 50;    
bool arpTriggered = false;
int arpWaveformType = 0; 
int dronePitch = 110;
int lfoRate = 50;
int lfoDepth = 127;
int filterResonance = 180;

int fileCursor = 0;
int fileScrollTop = 0;

bool isPatternGenMode = false; 
int genLength = 16;
int genHits = 4;
int genNote = 60;

bool isDrumSettingsOpen = false;
int drumSettingsCursor = 0;
int drumProjectSlot = 0;