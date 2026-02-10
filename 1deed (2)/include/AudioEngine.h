#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <Arduino.h> 
#include "Globals.h"
#include <Mozzi.h>
#include <Oscil.h>
#include <ADSR.h>
#include <mozzi_midi.h> 
#include <mozzi_rand.h> 

#include <tables/sin2048_int8.h>      
#include <tables/saw2048_int8.h>      
#include <tables/square_no_alias_2048_int8.h> 
#include <tables/triangle2048_int8.h> 
#include <LowPassFilter.h> 

// #include "SoundDesign.h" <-- 삭제
#include "GranularSynth.h" 

// --- 악기 변수 ---
Oscil<2048, AUDIO_RATE> oscArp(SIN2048_DATA);
ADSR<CONTROL_RATE, AUDIO_RATE> envArp;

Oscil<2048, AUDIO_RATE> oscDrumVoices[8]; 
ADSR<CONTROL_RATE, AUDIO_RATE> envDrumVoices[8];
float voiceCurrentPitch[8]; 

Oscil<2048, AUDIO_RATE> oscSpaceArp(SAW2048_DATA); 
ADSR<CONTROL_RATE, AUDIO_RATE> envSpaceArp;
unsigned long lastArpTime = 0;
int arpNoteIndex = 0;
int arpNotes[] = {48, 52, 55, 59, 60, 64, 67, 71}; 

#define NUM_VOICES 8
Oscil<2048, AUDIO_RATE> oscKeys[NUM_VOICES];      
ADSR<CONTROL_RATE, AUDIO_RATE> envKeys[NUM_VOICES]; 
int activeNotes[NUM_VOICES]; 
bool voiceIsDrum[NUM_VOICES]; 
int voiceBasePitch[NUM_VOICES];
int voiceWaveform[NUM_VOICES]; 
LowPassFilter lpf; 
int keyNotes[] = {60, 62, 64, 65, 67, 69, 71, 72}; 

#define DELAY_MAX 2048
int delayBuffer[DELAY_MAX];
int delayIdx = 0;
unsigned long lastStepTime = 0; 
Oscil<2048, AUDIO_RATE> oscDrone(SAW2048_DATA);
Oscil<2048, AUDIO_RATE> oscLfo(SIN2048_DATA);

// 잡음 생성 및 일반 오실레이터 처리
inline long getSample(Oscil<2048, AUDIO_RATE>& osc, int waveType) {
    if (waveType == 4) return (long)((xorshift96() & 0xFF) - 128); 
    return osc.next();
}

void setDrumTable(Oscil<2048, AUDIO_RATE>& osc, int waveType) {
    switch(waveType) {
        case 0: osc.setTable(SIN2048_DATA); break;
        case 1: osc.setTable(TRIANGLE2048_DATA); break;
        case 2: osc.setTable(SAW2048_DATA); break;
        case 3: osc.setTable(SQUARE_NO_ALIAS_2048_DATA); break;
        case 4: osc.setTable(SIN2048_DATA); break; 
    }
}

// [드럼 트리거 함수]
void triggerDrumVoice(int trackIdx) {
    if(trackIdx < 0 || trackIdx > 7) return; 
    DrumSound& ds = drumSounds[trackIdx];
    setDrumTable(oscDrumVoices[trackIdx], ds.waveform);
    oscDrumVoices[trackIdx].setFreq(ds.pitch);
    int sustainLevel = (ds.volume * ds.sustain) >> 8;
    envDrumVoices[trackIdx].setADLevels(ds.volume, sustainLevel);
    envDrumVoices[trackIdx].setTimes(ds.attack, ds.decay, 50, ds.release);
    envDrumVoices[trackIdx].noteOn();
    voiceCurrentPitch[trackIdx] = ds.pitch;
}

// [트래커 트리거 함수]
void triggerTrackerVoice(int trackIdx, int note) {
    if (trackIdx < 0 || trackIdx >= 16) return; 
    
    if (trackIdx < 8) { // SYNTH
        int voiceIdx = trackIdx;
        if (note == -2) { 
            if (activeNotes[voiceIdx] != -1) {
                envKeys[voiceIdx].noteOff();
                activeNotes[voiceIdx] = -1;
            }
        } else if (note >= 0) { 
            activeNotes[voiceIdx] = note;
            oscKeys[voiceIdx].setFreq(mtof(note));
            envKeys[voiceIdx].noteOn();
            voiceIsDrum[voiceIdx] = false;
        }
    } else { // DRUM
        int drumVoiceIdx = trackIdx - 8;
        if (note >= 0) { 
             DrumSound& ds = drumSounds[drumVoiceIdx];
             setDrumTable(oscDrumVoices[drumVoiceIdx], ds.waveform);
             float pitchRatio = mtof(note) / mtof(60); 
             int finalPitch = ds.pitch * pitchRatio;
             oscDrumVoices[drumVoiceIdx].setFreq(finalPitch);
             int sustainLevel = (ds.volume * ds.sustain) >> 8;
             envDrumVoices[drumVoiceIdx].setADLevels(ds.volume, sustainLevel);
             envDrumVoices[drumVoiceIdx].setTimes(ds.attack, ds.decay, 50, ds.release);
             envDrumVoices[drumVoiceIdx].noteOn();
        }
    }
}

void setupAudio() {
  oscArp.setFreq(440); envArp.setADLevels(255, 0);
  // setupSfx(); <-- 삭제
  oscDrone.setFreq(110); oscLfo.setFreq(1.0f);

  for(int i=0; i<64; i++) {
      for(int j=0; j<16; j++) { 
          trackerPattern[i][j].note = -1; 
          trackerPattern[i][j].inst = 0;
          trackerPattern[i][j].vol = 255;
      }
  }

  // 드럼 사운드 초기화 (유지)
  drumSounds[0] = {5, 150, 0, 50, 60, 0, 255, 100};  
  drumSounds[1] = {0, 120, 0, 80, 200, 4, 255, 0};   
  drumSounds[2] = {0, 40, 0, 30, 800, 4, 200, 0};    
  drumSounds[3] = {10, 80, 0, 100, 500, 1, 255, 20}; 
  drumSounds[4] = {10, 200, 100, 300, 100, 0, 255, 80}; 
  drumSounds[5] = {0, 50, 0, 50, 1200, 2, 200, 0};   
  drumSounds[6] = {5, 200, 0, 100, 150, 3, 220, 0};  
  drumSounds[7] = {0, 30, 0, 20, 2000, 4, 180, 0};   

  for(int i=0; i<8; i++) {
      oscDrumVoices[i].setTable(SIN2048_DATA);
      envDrumVoices[i].setADLevels(255, 0);
  }

  fingerSounds[0] = {10, 200, 0, 100, 100, 0, 255, 0}; 
  
  for(int i=0; i<NUM_VOICES; i++) {
      oscKeys[i].setTable(SIN2048_DATA); 
      envKeys[i].setADLevels(255, 200);
      envKeys[i].setTimes(10, 100, 4294967295, 200); 
      activeNotes[i] = -1; voiceIsDrum[i] = false; voiceWaveform[i] = 0; 
  }
  lpf.setResonance(150);
}

// 기타 헬퍼 함수들
void playFingerDrum(int btnIndex) { /* 생략(기존 유지) */ }
void setSynthWaveform(int type) { const int8_t* table = SIN2048_DATA; if (type == 1) table = SAW2048_DATA; else if (type == 2) table = SQUARE_NO_ALIAS_2048_DATA; for(int i=0; i<NUM_VOICES; i++) oscKeys[i].setTable(table); }
void playKeyNote(int noteIdx) { int noteToPlay = keyNotes[noteIdx]; for(int i=0; i<NUM_VOICES; i++) { if(activeNotes[i] == noteToPlay && envKeys[i].playing()) { envKeys[i].noteOn(); return; } } for(int i=0; i<NUM_VOICES; i++) { if(!envKeys[i].playing()) { activeNotes[i] = noteToPlay; oscKeys[i].setFreq(mtof(noteToPlay)); envKeys[i].noteOn(); voiceIsDrum[i] = false; return; } } activeNotes[0] = noteToPlay; oscKeys[0].setFreq(mtof(noteToPlay)); envKeys[0].noteOn(); voiceIsDrum[0] = false; }
void stopKeyNote(int noteIdx) { int noteToStop = keyNotes[noteIdx]; for(int i=0; i<NUM_VOICES; i++) { if(activeNotes[i] == noteToStop) { envKeys[i].noteOff(); activeNotes[i] = -1; } } }
// playWallHit(), playFloorHit() 삭제됨

void updateControl() {
  envArp.update();
  

  for(int i=0; i<8; i++) {
      envDrumVoices[i].update();
      if(envDrumVoices[i].playing()) {
          if(drumSounds[i].pitchMod > 0) {
             float mod = (envDrumVoices[i].next() / 255.0f) * drumSounds[i].pitchMod * 2.0f;
             oscDrumVoices[i].setFreq(drumSounds[i].pitch + (int)mod);
          }
      }
  }

  if (isSequencerPlaying) {
    unsigned long interval = 60000 / (bpm * noteResolution); 
    if (millis() - lastStepTime >= interval) {
      lastStepTime = millis();
      currentStep = (currentStep + 1) % sequenceLength; 
      
      for(int trk=0; trk<8; trk++) {
          if (drumPattern[trk][currentStep]) triggerDrumVoice(trk);
      }
      if (synthPattern[0][currentStep % 16]) playFingerDrum(4); 
      
      for(int trk=0; trk<16; trk++) { 
          int note = trackerPattern[currentStep][trk].note;
          if (note != -1) triggerTrackerVoice(trk, note);
      } 
    }
  }
  
  if (currentMode == MODE_DRONE) { oscDrumVoices[3].setFreq(dronePitch); float speed = map(lfoRate, 0, 10000, 1, 200000) / 10.0f; oscLfo.setFreq(speed); long modulation = (long)oscLfo.next() * lfoDepth; int change = modulation >> 8; int cutoff = 100 + change; if (cutoff < 10) cutoff = 10; if (cutoff > 255) cutoff = 255; lpf.setCutoffFreq(cutoff); }
  if (currentMode == MODE_SPACE_VIEW) { envSpaceArp.update(); if (millis() - lastArpTime >= (unsigned long)arpSpeed) { lastArpTime = millis(); oscSpaceArp.setFreq(mtof(arpNotes[arpNoteIndex])); envSpaceArp.setTimes(10, 200, 0, 0); envSpaceArp.noteOn(); arpNoteIndex = (arpNoteIndex + 1) % 8; arpTriggered = true; } }
  for(int i=0; i<NUM_VOICES; i++) { envKeys[i].update(); if(voiceIsDrum[i] && envKeys[i].playing()) { oscKeys[i].setFreq(voiceBasePitch[i] + (envKeys[i].next() * 2)); } }
  lpf.setCutoffFreq(map(synthFilterVal, 0, 255, 20, 255)); 
}

MonoOutput updateAudio() {
  long drumSum = 0;
  for(int i=0; i<8; i++) {
      drumSum += getSample(oscDrumVoices[i], drumSounds[i].waveform) * envDrumVoices[i].next();
  }
  drumSum = (drumSum * trackVolume[0]) >> 8; 

  long grainSum = 0;
  if (isGranularActive && isSampleLoaded) grainSum = (long)getGranularSample();
  grainSum = (grainSum * trackVolume[1]) >> 8;

  long synthSum = 0;
  long voiceSum = 0; 
  for(int i=0; i<NUM_VOICES; i++) {
      int wave = (currentMode == MODE_KEYBOARD) ? 0 : voiceWaveform[i];
      voiceSum += getSample(oscKeys[i], wave) * envKeys[i].next(); 
  }
  synthSum += lpf.next(voiceSum) >> 3; 
  synthSum += (long)oscArp.next() * envArp.next(); 
  // synthSum += getSfxSample(); <-- 삭제됨
  synthSum = (synthSum * trackVolume[2]) >> 8;

  long droneSum = 0;
  if (currentMode == MODE_DRONE) droneSum += lpf.next(oscDrone.next()) * 150; 
  if (currentMode == MODE_SPACE_VIEW) { long rawSig = (long)oscSpaceArp.next() * envSpaceArp.next(); long delayedSig = delayBuffer[delayIdx]; long mixed = rawSig + ((delayedSig * reverbLevel) >> 8); delayBuffer[delayIdx] = mixed; delayIdx = (delayIdx + 1) % DELAY_MAX; droneSum += mixed; }
  droneSum = (droneSum * trackVolume[3]) >> 8;

  long totalOutput = (drumSum + grainSum + synthSum + droneSum) >> 2;
  if (totalOutput > 32000) totalOutput = 32000;
  if (totalOutput < -32000) totalOutput = -32000;

  return MonoOutput::from16Bit((int)(totalOutput));
}

#include <driver/i2s.h>
void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = AUDIO_RATE, 
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 128,
    .use_apll = true
  };
  i2s_pin_config_t pin_config = { .bck_io_num = I2S_BCK, .ws_io_num = I2S_WS, .data_out_num = I2S_DATA, .data_in_num = -1 };
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, AUDIO_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);
}
inline bool canBufferAudioOutput() { return true; }
void audioOutput(const AudioOutput f) {
  int16_t sample = f.l(); 
  uint32_t s32 = ((uint32_t)(uint16_t)sample << 16) | ((uint32_t)(uint16_t)sample & 0xFFFF);
  size_t w; i2s_write(I2S_NUM_0, &s32, 4, &w, portMAX_DELAY);
}
void AudioTask(void *pvParams) {
  setupI2S(); setupAudio(); startMozzi(MOZZI_CONTROL_RATE);
  while(true) { audioHook(); }
}

#endif