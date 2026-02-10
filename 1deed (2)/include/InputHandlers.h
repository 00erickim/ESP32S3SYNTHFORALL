#ifndef INPUT_HANDLERS_H
#define INPUT_HANDLERS_H

#include "Globals.h"
#include <AceButton.h>

using namespace ace_button;

// [함수 전방 선언]
void initMixerUI();
void redrawMixerTrack(int);
void redrawPlayStatus();
void drawDrumMachine(bool);
void drawTrackerUI();
void drawGranularUI();
void drawFileBrowser();
void drawDroneUI();
void drawKeyboardUI();
void drawSoundEditor();
void drawPatternGenUI();
void drawPianoRoll();
void drawFingerDrumUI();
void drawDrumSettings(); // 추가된 설정 UI
void redrawStep(int);
void triggerDrumVoice(int);
void triggerTrackerVoice(int, int);
void applyEuclideanPattern(int, int);
void playKeyNote(int);
void stopKeyNote(int);
void loadFileList();
bool loadWavToPsram(String);
void enterDeepSleep(); 

void handleButtonEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  uint8_t pin = button->getPin();
  bool isPressed = (eventType == AceButton::kEventPressed);
  bool isRepeat = (eventType == AceButton::kEventRepeatPressed);
  bool isLongPressed = (eventType == AceButton::kEventLongPressed);

  // 1. [사운드 에디터 모드] (최우선 순위)
  if (isSoundEditMode) {
      if (isPressed) {
          if (pin == BTN_B || (pin == ENC1_SW)) { 
              isSoundEditMode = false; 
              if(currentMode == MODE_DRUM) drawDrumMachine(true); 
              else if(currentMode == MODE_TRACKER) drawTrackerUI(); 
              else drawFingerDrumUI();
              return; 
          }
          if (pin == BTN_UP) { editParamIndex--; if(editParamIndex < 0) editParamIndex = 6; enc1.setCount(0); drawSoundEditor(); }
          if (pin == BTN_DOWN) { editParamIndex++; if(editParamIndex > 6) editParamIndex = 0; enc1.setCount(0); drawSoundEditor(); }
          if (pin == BTN_A) { triggerDrumVoice(editTargetIndex); }
      }
      return; 
  }

  // 2. [피아노 롤 모드]
  if (currentMode == MODE_PIANO_ROLL && isPressed) {
      if (pin == BTN_Y || pin == BTN_B) { currentMode = MODE_TRACKER; drawTrackerUI(); return; }
      if (pin == BTN_X) { isSequencerPlaying = !isSequencerPlaying; return; }
      if (pin == BTN_LEFT) { pianoRollCursorStep--; if(pianoRollCursorStep < 0) pianoRollCursorStep = 63; drawPianoRoll(); }
      if (pin == BTN_RIGHT) { pianoRollCursorStep++; if(pianoRollCursorStep > 63) pianoRollCursorStep = 0; drawPianoRoll(); }
      if (pin == BTN_UP) { pianoRollCursorNote++; if(pianoRollCursorNote > 127) pianoRollCursorNote = 127; drawPianoRoll(); }
      if (pin == BTN_DOWN) { pianoRollCursorNote--; if(pianoRollCursorNote < 0) pianoRollCursorNote = 0; drawPianoRoll(); }
      if (pin == BTN_A) {
          int8_t currentNote = trackerPattern[pianoRollCursorStep][trackerCursorTrack].note;
          if (currentNote == pianoRollCursorNote) { trackerPattern[pianoRollCursorStep][trackerCursorTrack].note = -1; } 
          else { trackerPattern[pianoRollCursorStep][trackerCursorTrack].note = pianoRollCursorNote; trackerPattern[pianoRollCursorStep][trackerCursorTrack].vol = 255; triggerTrackerVoice(trackerCursorTrack, pianoRollCursorNote); }
          drawPianoRoll();
      }
      return;
  }

  // 3. [트래커 모드]
  if (currentMode == MODE_TRACKER && (isPressed || isRepeat)) {
      if (isPatternGenMode) {
          if (pin == BTN_B && isPressed) { isPatternGenMode = false; drawTrackerUI(); return; }
          if (pin == BTN_A && isPressed) { applyEuclideanPattern(trackerCursorStep, trackerCursorTrack); isPatternGenMode = false; drawTrackerUI(); return; }
          return; 
      }
      if (pin == BTN_Y && isPressed) { currentMode = MODE_PIANO_ROLL; pianoRollCursorStep = trackerCursorStep; int8_t n = trackerPattern[trackerCursorStep][trackerCursorTrack].note; pianoRollCursorNote = (n >= 0) ? n : 60; drawPianoRoll(); return; }
      if (pin == BTN_B && isPressed) { currentMode = MODE_MENU; initMixerUI(); return; } 
      if (pin == BTN_X && isPressed) { isSequencerPlaying = !isSequencerPlaying; return; } 
      if (pin == BTN_UP) { trackerCursorStep--; if(trackerCursorStep < 0) trackerCursorStep = 63; drawTrackerUI(); }
      if (pin == BTN_DOWN) { trackerCursorStep++; if(trackerCursorStep > 63) trackerCursorStep = 0; drawTrackerUI(); }
      if (pin == BTN_LEFT) { trackerCursorTrack--; if(trackerCursorTrack < 0) trackerCursorTrack = 15; drawTrackerUI(); }
      if (pin == BTN_RIGHT) { trackerCursorTrack++; if(trackerCursorTrack > 15) trackerCursorTrack = 0; drawTrackerUI(); }
      if (pin == BTN_A && isPressed) {
          int8_t currentNote = trackerPattern[trackerCursorStep][trackerCursorTrack].note;
          if (currentNote == -1) trackerPattern[trackerCursorStep][trackerCursorTrack].note = 60; 
          else if (currentNote >= 0) trackerPattern[trackerCursorStep][trackerCursorTrack].note = -2; 
          else trackerPattern[trackerCursorStep][trackerCursorTrack].note = -1; 
          drawTrackerUI();
      }
      return;
  }

  // 4. [메뉴(믹서) 모드]
  if (currentMode == MODE_MENU) {
      
      // 딥슬립 (A 길게 + 아래)
      if (pin == BTN_A && isLongPressed) {
          if (digitalRead(BTN_DOWN) == LOW) { 
              enterDeepSleep();
              return;
          }
      }

      if (isPressed) {
          if (pin == BTN_LEFT) { int old = currentMixerTrack; currentMixerTrack--; if(currentMixerTrack < 0) currentMixerTrack = 3; redrawMixerTrack(old); redrawMixerTrack(currentMixerTrack); }
          else if (pin == BTN_RIGHT) { int old = currentMixerTrack; currentMixerTrack++; if(currentMixerTrack > 3) currentMixerTrack = 0; redrawMixerTrack(old); redrawMixerTrack(currentMixerTrack); }
          else if (pin == BTN_X) { isSequencerPlaying = !isSequencerPlaying; redrawPlayStatus(); }
          else if (pin == BTN_A) {
              if (digitalRead(BTN_DOWN) == HIGH) {
                  if (currentMixerTrack == 0) { currentMode = MODE_DRUM; drawDrumMachine(true); }
                  else if (currentMixerTrack == 1) { if(isSampleLoaded) { currentMode = MODE_GRANULAR_PLAY; drawGranularUI(); } else { currentMode = MODE_GRANULAR_LOADER; loadFileList(); drawFileBrowser(); } }
                  else if (currentMixerTrack == 2) { currentMode = MODE_TRACKER; drawTrackerUI(); } 
                  else if (currentMixerTrack == 3) { currentMode = MODE_DRONE; drawDroneUI(); }
              }
          }
      }
      return;
  }

  // 5. [기타 모드들]
  if (currentMode == MODE_GRANULAR_PLAY && isPressed) { if (pin == BTN_B) { currentMode = MODE_MENU; initMixerUI(); return; } if (pin == BTN_A) { currentMode = MODE_GRANULAR_LOADER; loadFileList(); drawFileBrowser(); return; } if (pin == BTN_UP) { grainPitch += 0.1f; drawGranularUI(); } if (pin == BTN_DOWN) { grainPitch -= 0.1f; drawGranularUI(); } return; }
  
  if (currentMode == MODE_GRANULAR_LOADER && isPressed) { if (pin == BTN_B) { currentMode = MODE_MENU; initMixerUI(); return; } if (pin == BTN_UP) { fileCursor--; if(fileCursor < 0) { fileCursor = 0; if(fileScrollTop > 0) fileScrollTop--; } else if(fileCursor < fileScrollTop) fileScrollTop--; drawFileBrowser(); } if (pin == BTN_DOWN) { fileCursor++; if(fileCursor >= fileCount) fileCursor = fileCount - 1; else if(fileCursor >= fileScrollTop + 8) fileScrollTop++; drawFileBrowser(); } if (pin == BTN_A) { String fname = fileList[fileCursor]; if (loadWavToPsram("/" + fname)) { currentMode = MODE_GRANULAR_PLAY; isGranularActive = true; drawGranularUI(); } else { drawFileBrowser(); } } return; }
  
  if (currentMode == MODE_DRONE && isPressed) { if (pin == BTN_B) { currentMode = MODE_MENU; initMixerUI(); return; } if (pin == BTN_UP) { lfoDepth += 10; if(lfoDepth > 255) lfoDepth = 255; drawDroneUI(); } if (pin == BTN_DOWN) { lfoDepth -= 10; if(lfoDepth < 0) lfoDepth = 0; drawDroneUI(); } if (pin == BTN_RIGHT) { filterResonance += 10; if(filterResonance > 255) filterResonance = 255; drawDroneUI(); } if (pin == BTN_LEFT) { filterResonance -= 10; if(filterResonance < 0) filterResonance = 0; drawDroneUI(); } return; }
  
  // [드럼 머신] - 수정됨: 설정 팝업 및 페이지 이동 로직
  if (currentMode == MODE_DRUM) {
      
      if (isDrumSettingsOpen) {
          if (isPressed) {
              // 닫기
              if (pin == BTN_Y || pin == BTN_B) { 
                  isDrumSettingsOpen = false; 
                  drawDrumMachine(true); 
                  return; 
              }

              // [메뉴 이동] 위/아래
              if (pin == BTN_UP) { 
                  drumSettingsCursor--; 
                  if(drumSettingsCursor < 0) drumSettingsCursor = 2; // Loop
                  drawDrumSettings(); 
              }
              if (pin == BTN_DOWN) { 
                  drumSettingsCursor++; 
                  if(drumSettingsCursor > 2) drumSettingsCursor = 0; // Loop
                  drawDrumSettings(); 
              }

              // [값 조절] 좌/우 (또는 엔코더1/2)
              // 여기서는 버튼 좌우로 통합합니다.
              if (pin == BTN_LEFT || pin == BTN_RIGHT) {
                  
                  // 0: Sequence Length 조절
                  if (drumSettingsCursor == 0) {
                      if (pin == BTN_RIGHT) { sequenceLength += 16; if(sequenceLength > 64) sequenceLength = 16; }
                      else { sequenceLength -= 16; if(sequenceLength < 16) sequenceLength = 64; }
                  }
                  // 1: Gap (Visual) 조절
                  else if (drumSettingsCursor == 1) {
                      if (stepsPerBeat == 4) stepsPerBeat = 3; else stepsPerBeat = 4;
                  }
                  // 2: Div (Speed) 조절
                  else if (drumSettingsCursor == 2) {
                      if (noteResolution == 4) noteResolution = 8; else noteResolution = 4;
                  }
                  
                  drawDrumSettings();
              }
          }
          return;
      }

      // 2. 일반 드럼 모드
      if (pin == ENC1_SW && isPressed) { 
          isSoundEditMode = true; 
          editTargetIndex = drumCursorTrack; 
          enc1.setCount(0); 
          drawSoundEditor(); 
          return; 
      }

      bool changed = false; 
      int prevStep = drumCursorStep;

      if (isPressed) {
          // Y버튼: 설정 메뉴 열기
          if (pin == BTN_Y) { 
              isDrumSettingsOpen = true; 
              drawDrumSettings(); 
              return; 
          }

          if (pin == BTN_B) { currentMode = MODE_MENU; initMixerUI(); return; } 
          
          if (pin == BTN_X) { 
              isSequencerPlaying = !isSequencerPlaying; 
              if(isSequencerPlaying) changed = true; 
          }

          if (pin == BTN_UP) { drumCursorTrack--; if(drumCursorTrack < 0) drumCursorTrack = 7; changed = true; } 
          if (pin == BTN_DOWN) { drumCursorTrack++; if(drumCursorTrack > 7) drumCursorTrack = 0; changed = true; }
          
          // 좌우 이동 시 페이지 전환 로직
          if (pin == BTN_LEFT) { 
              drumCursorStep--; 
              if(drumCursorStep < 0) drumCursorStep = sequenceLength - 1; 
              
              // 페이지가 변경되면 전체 다시 그리기
              if ((prevStep / 16) != (drumCursorStep / 16)) drawDrumMachine(true);
              else { redrawStep(prevStep); redrawStep(drumCursorStep); drawDrumMachine(false); }
          }
          if (pin == BTN_RIGHT) { 
              drumCursorStep++; 
              if(drumCursorStep >= sequenceLength) drumCursorStep = 0; 
              
              if ((prevStep / 16) != (drumCursorStep / 16)) drawDrumMachine(true);
              else { redrawStep(prevStep); redrawStep(drumCursorStep); drawDrumMachine(false); }
          }
          
          if (pin == BTN_A) { 
              drumPattern[drumCursorTrack][drumCursorStep] = !drumPattern[drumCursorTrack][drumCursorStep]; 
              changed = true; 
          }
      }
      
      if (changed) { 
          redrawStep(drumCursorStep); 
          drawDrumMachine(false); 
      }
      return;
  }
  
  // [키보드]
  if (currentMode == MODE_KEYBOARD) { if (pin == BTN_B && isPressed) { currentMode = MODE_MENU; initMixerUI(); return; } int noteIdx = -1; if (pin == BTN_LEFT) noteIdx = 0; else if (pin == BTN_DOWN) noteIdx = 1; else if (pin == BTN_RIGHT) noteIdx = 2; else if (pin == BTN_UP) noteIdx = 3; else if (pin == BTN_A) noteIdx = 4; else if (pin == BTN_X) noteIdx = 6; if (noteIdx != -1) { if (isPressed) { playKeyNote(noteIdx); currentNote = noteIdx; drawKeyboardUI(); } else if (eventType == AceButton::kEventReleased) { stopKeyNote(noteIdx); } } }
}

#endif