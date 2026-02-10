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
void drawDrumSettings();
void redrawStep(int);
void triggerDrumVoice(int);
void triggerTrackerVoice(int, int);
void applyEuclideanPattern(int, int);
void playKeyNote(int);
void stopKeyNote(int);
void loadFileList();
bool loadWavToPsram(String);
void enterDeepSleep(); 
int getStepsPerPage(); 

// [★NEW] 새로 추가된 함수 및 변수 선언
void saveProject(int);
void loadProject(int);
void drawInstrumentMenu(); 
extern int instSelectCursor; // InstrumentMenu.h 등에 정의된 변수라고 가정

void handleButtonEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  uint8_t pin = button->getPin();
  bool isPressed = (eventType == AceButton::kEventPressed);
  bool isRepeat = (eventType == AceButton::kEventRepeatPressed);
  bool isLongPressed = (eventType == AceButton::kEventLongPressed);

  // [★NEW] 1. [악기 설정 메뉴 모드]
  if (currentMode == MODE_INSTRUMENT_SELECT) {
      if (isPressed) {
          if (pin == BTN_B) {
              // 뒤로 가기 -> 드럼 모드 복귀
              currentMode = MODE_DRUM;
              drawDrumMachine(true);
              return;
          }
          
          if (pin == BTN_UP) {
              instSelectCursor--;
              if (instSelectCursor < 0) instSelectCursor = 1; // 메뉴 항목 수에 따라 조정
              drawInstrumentMenu();
          }
          if (pin == BTN_DOWN) {
              instSelectCursor++;
              if (instSelectCursor > 1) instSelectCursor = 0; // 메뉴 항목 수에 따라 조정
              drawInstrumentMenu();
          }
          
          if (pin == BTN_A) {
              // TODO: 악기 선택 로직 구현 위치
              // 예: 악기 변경 후 드럼 모드로 복귀 등
              // currentMode = MODE_DRUM; 
              // drawDrumMachine(true);
          }
      }
      return;
  }

  // 2. [사운드 에디터 모드]
  if (isSoundEditMode) {
      if (isPressed) {
          if (pin == BTN_B || (pin == ENC1_SW)) { isSoundEditMode = false; if(currentMode == MODE_DRUM) drawDrumMachine(true); else if(currentMode == MODE_TRACKER) drawTrackerUI(); else drawFingerDrumUI(); return; }
          if (pin == BTN_UP) { editParamIndex--; if(editParamIndex < 0) editParamIndex = 6; enc1.setCount(0); drawSoundEditor(); }
          if (pin == BTN_DOWN) { editParamIndex++; if(editParamIndex > 6) editParamIndex = 0; enc1.setCount(0); drawSoundEditor(); }
          if (pin == BTN_A) { triggerDrumVoice(editTargetIndex); }
      }
      return; 
  }

  // 3. [피아노 롤 모드]
  if (currentMode == MODE_PIANO_ROLL && isPressed) {
      if (pin == BTN_Y || pin == BTN_B) { currentMode = MODE_TRACKER; drawTrackerUI(); return; }
      if (pin == BTN_X) { isSequencerPlaying = !isSequencerPlaying; return; }
      if (pin == BTN_LEFT) { pianoRollCursorStep--; if(pianoRollCursorStep < 0) pianoRollCursorStep = 63; drawPianoRoll(); }
      if (pin == BTN_RIGHT) { pianoRollCursorStep++; if(pianoRollCursorStep > 63) pianoRollCursorStep = 0; drawPianoRoll(); }
      if (pin == BTN_UP) { drumCursorTrack--; if(drumCursorTrack < 0) drumCursorTrack = 7; drawDrumMachine(true);} 
      if (pin == BTN_DOWN) { 
        drumCursorTrack++; if(drumCursorTrack > 7) drumCursorTrack = 0; 
        drawDrumMachine(true); 
        }
      if (pin == BTN_A) {
          int8_t currentNote = trackerPattern[pianoRollCursorStep][trackerCursorTrack].note;
          if (currentNote == pianoRollCursorNote) { trackerPattern[pianoRollCursorStep][trackerCursorTrack].note = -1; } 
          else { trackerPattern[pianoRollCursorStep][trackerCursorTrack].note = pianoRollCursorNote; trackerPattern[pianoRollCursorStep][trackerCursorTrack].vol = 255; triggerTrackerVoice(trackerCursorTrack, pianoRollCursorNote); }
          drawPianoRoll();
      }
      return;
  }

  // 4. [트래커 모드]
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

  // 5. [메뉴(믹서) 모드]
  if (currentMode == MODE_MENU) {
      if (pin == BTN_A && isLongPressed) { if (digitalRead(BTN_DOWN) == LOW) { enterDeepSleep(); return; } }
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

  // 6. [기타 모드]
  if (currentMode == MODE_GRANULAR_PLAY && isPressed) { if (pin == BTN_B) { currentMode = MODE_MENU; initMixerUI(); return; } if (pin == BTN_A) { currentMode = MODE_GRANULAR_LOADER; loadFileList(); drawFileBrowser(); return; } if (pin == BTN_UP) { grainPitch += 0.1f; drawGranularUI(); } if (pin == BTN_DOWN) { grainPitch -= 0.1f; drawGranularUI(); } return; }
  if (currentMode == MODE_GRANULAR_LOADER && isPressed) { if (pin == BTN_B) { currentMode = MODE_MENU; initMixerUI(); return; } if (pin == BTN_UP) { fileCursor--; if(fileCursor < 0) { fileCursor = 0; if(fileScrollTop > 0) fileScrollTop--; } else if(fileCursor < fileScrollTop) fileScrollTop--; drawFileBrowser(); } if (pin == BTN_DOWN) { fileCursor++; if(fileCursor >= fileCount) fileCursor = fileCount - 1; else if(fileCursor >= fileScrollTop + 8) fileScrollTop++; drawFileBrowser(); } if (pin == BTN_A) { String fname = fileList[fileCursor]; if (loadWavToPsram("/" + fname)) { currentMode = MODE_GRANULAR_PLAY; isGranularActive = true; drawGranularUI(); } else { drawFileBrowser(); } } return; }
  if (currentMode == MODE_DRONE && isPressed) { if (pin == BTN_B) { currentMode = MODE_MENU; initMixerUI(); return; } if (pin == BTN_UP) { lfoDepth += 10; if(lfoDepth > 255) lfoDepth = 255; drawDroneUI(); } if (pin == BTN_DOWN) { lfoDepth -= 10; if(lfoDepth < 0) lfoDepth = 0; drawDroneUI(); } if (pin == BTN_RIGHT) { filterResonance += 10; if(filterResonance > 255) filterResonance = 255; drawDroneUI(); } if (pin == BTN_LEFT) { filterResonance -= 10; if(filterResonance < 0) filterResonance = 0; drawDroneUI(); } return; }
  
  // [★UPDATE] 7. [드럼 머신] - 대폭 수정됨 (헤더 선택, 악기 메뉴, 프로젝트 저장/로드 추가)
  if (currentMode == MODE_DRUM) {
      // 7-1. 드럼 설정 팝업
      if (isDrumSettingsOpen) {
          if (isPressed) {
              if (pin == BTN_Y || pin == BTN_B) { isDrumSettingsOpen = false; drawDrumMachine(true); return; }
              // 커서 범위가 2에서 5로 확장됨
              if (pin == BTN_UP) { drumSettingsCursor--; if(drumSettingsCursor < 0) drumSettingsCursor = 5; drawDrumSettings(); }
              if (pin == BTN_DOWN) { drumSettingsCursor++; if(drumSettingsCursor > 5) drumSettingsCursor = 0; drawDrumSettings(); }
              
              if (pin == BTN_LEFT || pin == BTN_RIGHT) {
                  // 기존 설정
                  if (drumSettingsCursor == 0) { 
                      if (pin == BTN_RIGHT) { sequenceLength += 32; if(sequenceLength > 64) sequenceLength = 32; }
                      else { sequenceLength -= 32; if(sequenceLength < 32) sequenceLength = 64; }
                  }
                  else if (drumSettingsCursor == 1) { if (stepsPerBeat == 4) stepsPerBeat = 3; else stepsPerBeat = 4; }
                  else if (drumSettingsCursor == 2) { if (noteResolution == 4) noteResolution = 8; else noteResolution = 4; }
                  // [NEW] 프로젝트 슬롯 선택
                  else if (drumSettingsCursor == 3) { 
                      if (pin == BTN_RIGHT) { drumProjectSlot++; if(drumProjectSlot > 15) drumProjectSlot = 0; } 
                      else { drumProjectSlot--; if(drumProjectSlot < 0) drumProjectSlot = 15; } 
                  }
                  drawDrumSettings();
              }
              // [NEW] 저장 및 로드 실행
              if (pin == BTN_A) {
                  if (drumSettingsCursor == 4) { saveProject(drumProjectSlot); drawDrumSettings(); }
                  else if (drumSettingsCursor == 5) { loadProject(drumProjectSlot); drawDrumSettings(); }
              }
          }
          return;
      }

      // 사운드 에디터 진입
      if (pin == ENC1_SW && isPressed) { isSoundEditMode = true; editTargetIndex = drumCursorTrack; enc1.setCount(0); drawSoundEditor(); return; }

      bool changed = false; 
      int prevStep = drumCursorStep;

      // 7-2. 드럼 메인 그리드 조작
      if (isPressed) {
          if (pin == BTN_Y) { isDrumSettingsOpen = true; drawDrumSettings(); return; }
          if (pin == BTN_B) { currentMode = MODE_MENU; initMixerUI(); return; } 
          if (pin == BTN_X) { isSequencerPlaying = !isSequencerPlaying; if(isSequencerPlaying) changed = true; }
          
          if (pin == BTN_UP) { drumCursorTrack--; if(drumCursorTrack < 0) drumCursorTrack = 7; changed = true; } 
          if (pin == BTN_DOWN) { drumCursorTrack++; if(drumCursorTrack > 7) drumCursorTrack = 0; changed = true; }
          
          int stride = (noteResolution == 4) ? 2 : 1;
          int spp = getStepsPerPage();

          // [NEW] 좌우 이동 로직 수정 (-1 인덱스 진입 허용)
          if (pin == BTN_LEFT) { 
              if (drumCursorStep == -1) { drumCursorStep = sequenceLength - stride; } 
              else { drumCursorStep -= stride; if(drumCursorStep < 0) drumCursorStep = -1; }
              drawDrumMachine(false); 
          }
          
          if (pin == BTN_RIGHT) { 
              if (drumCursorStep == -1) { drumCursorStep = 0; } 
              else { drumCursorStep += stride; if(drumCursorStep >= sequenceLength) drumCursorStep = -1; }
              drawDrumMachine(false); 
          }
          
          if (pin == BTN_A) { 
              // [★NEW] 헤더(-1) 선택 중 A버튼 -> 악기 선택 메뉴로 이동
              if (drumCursorStep == -1) {
                  currentMode = MODE_INSTRUMENT_SELECT;
                  instSelectCursor = 0; // 커서 초기화
                  drawInstrumentMenu();
                  return; 
              } 
              // 노트 영역일 때: 노트 찍기
              else {
                  drumPattern[drumCursorTrack][drumCursorStep] = !drumPattern[drumCursorTrack][drumCursorStep]; 
                  changed = true; 
              }
          }
      }
      
      if (changed) { redrawStep(drumCursorStep); drawDrumMachine(false); }
      return;
  }
  
  // 8. [키보드 모드]
  if (currentMode == MODE_KEYBOARD) { if (pin == BTN_B && isPressed) { currentMode = MODE_MENU; initMixerUI(); return; } int noteIdx = -1; if (pin == BTN_LEFT) noteIdx = 0; else if (pin == BTN_DOWN) noteIdx = 1; else if (pin == BTN_RIGHT) noteIdx = 2; else if (pin == BTN_UP) noteIdx = 3; else if (pin == BTN_A) noteIdx = 4; else if (pin == BTN_X) noteIdx = 6; if (noteIdx != -1) { if (isPressed) { playKeyNote(noteIdx); currentNote = noteIdx; drawKeyboardUI(); } else if (eventType == AceButton::kEventReleased) { stopKeyNote(noteIdx); } } }
}

#endif