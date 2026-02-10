#ifndef INPUT_HANDLERS_H
#define INPUT_HANDLERS_H

#include "Globals.h"
#include <AceButton.h>
#include "SamplerUI.h"  // [★] 샘플러 UI
#include "SampleLoader.h" // [★] 파일 로더

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
bool loadWavToPsram(String); // 그라뉼라용 (기존)
void enterDeepSleep(); 
int getStepsPerPage(); 
void saveProject(int);
void loadProject(int);
void drawInstrumentMenu(); 
extern int instSelectCursor; 

void handleButtonEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  uint8_t pin = button->getPin();
  bool isPressed = (eventType == AceButton::kEventPressed);
  bool isRepeat = (eventType == AceButton::kEventRepeatPressed);
  bool isLongPressed = (eventType == AceButton::kEventLongPressed);

  // [0] 악기 선택 메뉴 (SAMPLER / SYNTH 선택)
  if (currentMode == MODE_INSTRUMENT_SELECT) {
      if (isPressed) {
          if (pin == BTN_B) { 
              currentMode = MODE_DRUM; 
              drawDrumMachine(true); 
              return; 
          }
          if (pin == BTN_UP) { 
              instSelectCursor--; 
              if (instSelectCursor < 0) instSelectCursor = 2; 
              drawInstrumentMenu(); 
          }
          if (pin == BTN_DOWN) { 
              instSelectCursor++; 
              if (instSelectCursor > 2) instSelectCursor = 0; 
              drawInstrumentMenu(); 
          }
          if (pin == BTN_A) {
              // 악기 타입 변경 적용
              if (instSelectCursor == 0) trackType[drumCursorTrack] = 0; // SYNTH
              else if (instSelectCursor == 1) trackType[drumCursorTrack] = 1; // SAMPLER
              // 추후 EFFECTS 등 확장 가능
              
              currentMode = MODE_DRUM; 
              drawDrumMachine(true);
          }
      }
      return;
  }

  // [★NEW] 1. 샘플 로더 모드 (파일 브라우저에서 파일 선택)
  if (currentMode == MODE_SAMPLE_LOADER) {
      if (isPressed || isRepeat) {
          // B: 취소하고 에디터로 복귀
          if (pin == BTN_B) { 
              isSoundEditMode = true; 
              currentMode = MODE_DRUM; 
              drawSamplerEditor(); 
              return; 
          }
          // UP/DOWN: 파일 목록 이동
          if (pin == BTN_UP) { 
              fileCursor--; 
              if(fileCursor < 0) { fileCursor = 0; if(fileScrollTop > 0) fileScrollTop--; } 
              else if(fileCursor < fileScrollTop) fileScrollTop--; 
              drawFileBrowser(); 
          } 
          if (pin == BTN_DOWN) { 
              fileCursor++; 
              if(fileCursor >= fileCount) fileCursor = fileCount - 1; 
              else if(fileCursor >= fileScrollTop + 8) fileScrollTop++; 
              drawFileBrowser(); 
          } 
          // A: 파일 선택 및 로드
          if (pin == BTN_A && isPressed) { // Repeat 방지 권장
              String fname = fileList[fileCursor]; 
              // 현재 에디팅 중인 트랙에 로드 실행
              if (loadSampleToTrack(editTargetIndex, "/" + fname)) {
                  // 성공 시 에디터로 복귀
                  isSoundEditMode = true;
                  currentMode = MODE_DRUM;
                  drawSamplerEditor();
              } else {
                  // 실패 시 브라우저 유지 (에러 메시지 뜸)
                  drawFileBrowser();
              }
          }
      }
      return;
  }

  // [2. 사운드 에디터 모드]
  if (isSoundEditMode) {
      bool isSampler = (trackType[editTargetIndex] == 1);
      
      if (isPressed) {
          // 나가기
          if (pin == BTN_B || (pin == ENC1_SW)) { 
              isSoundEditMode = false; 
              if(currentMode == MODE_DRUM) drawDrumMachine(true); 
              else if(currentMode == MODE_TRACKER) drawTrackerUI(); 
              else drawFingerDrumUI(); 
              return; 
          }

          // 파라미터 이동
          if (pin == BTN_UP) { 
              editParamIndex--; 
              // 샘플러는 7번(END)까지, 신스는 6번(PitchMod)까지
              int maxIdx = isSampler ? 7 : 6;
              if(editParamIndex < 0) editParamIndex = maxIdx; 
              enc1.setCount(0); 
              if (isSampler) drawSamplerEditor(); else drawSoundEditor();
          }
          if (pin == BTN_DOWN) { 
              editParamIndex++; 
              int maxIdx = isSampler ? 7 : 6;
              if(editParamIndex > maxIdx) editParamIndex = 0; 
              enc1.setCount(0); 
              if (isSampler) drawSamplerEditor(); else drawSoundEditor();
          }

          // A 버튼 동작
          if (pin == BTN_A) { 
              // [★중요] LOAD 항목(5번)에서 A 누르면 파일 브라우저 진입
              if (isSampler && editParamIndex == 5) {
                  currentMode = MODE_SAMPLE_LOADER;
                  loadFileList(); // SD카드 파일 목록 읽기
                  drawFileBrowser();
                  return;
              }
              // 그 외엔 소리 들어보기
              triggerDrumVoice(editTargetIndex); 
          }
      }
      return; 
  }

  // ... (기타 모드: 피아노롤, 트래커 등 유지) ...
  if (currentMode == MODE_PIANO_ROLL && isPressed) {
      if (pin == BTN_Y || pin == BTN_B) { currentMode = MODE_TRACKER; drawTrackerUI(); return; }
      if (pin == BTN_X) { isSequencerPlaying = !isSequencerPlaying; return; }
      if (pin == BTN_LEFT) { pianoRollCursorStep--; if(pianoRollCursorStep < 0) pianoRollCursorStep = 63; drawPianoRoll(); }
      if (pin == BTN_RIGHT) { pianoRollCursorStep++; if(pianoRollCursorStep > 63) pianoRollCursorStep = 0; drawPianoRoll(); }
      if (pin == BTN_UP) { drumCursorTrack--; if(drumCursorTrack < 0) drumCursorTrack = 7; drawDrumMachine(true);} 
      if (pin == BTN_DOWN) { drumCursorTrack++; if(drumCursorTrack > 7) drumCursorTrack = 0; drawDrumMachine(true); }
      if (pin == BTN_A) {
          int8_t currentNote = trackerPattern[pianoRollCursorStep][trackerCursorTrack].note;
          if (currentNote == pianoRollCursorNote) { trackerPattern[pianoRollCursorStep][trackerCursorTrack].note = -1; } 
          else { trackerPattern[pianoRollCursorStep][trackerCursorTrack].note = pianoRollCursorNote; trackerPattern[pianoRollCursorStep][trackerCursorTrack].vol = 255; triggerTrackerVoice(trackerCursorTrack, pianoRollCursorNote); }
          drawPianoRoll();
      }
      return;
  }

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
  
  // 7. [드럼 머신]
  if (currentMode == MODE_DRUM) {
      // 7-1. 드럼 설정 팝업
      if (isDrumSettingsOpen) {
          if (isPressed) {
              if (pin == BTN_Y || pin == BTN_B) { isDrumSettingsOpen = false; drawDrumMachine(true); return; }
              // 커서
              if (pin == BTN_UP) { drumSettingsCursor--; if(drumSettingsCursor < 0) drumSettingsCursor = 5; drawDrumSettings(); }
              if (pin == BTN_DOWN) { drumSettingsCursor++; if(drumSettingsCursor > 5) drumSettingsCursor = 0; drawDrumSettings(); }
              
              if (pin == BTN_LEFT || pin == BTN_RIGHT) {
                  if (drumSettingsCursor == 0) { 
                      if (pin == BTN_RIGHT) { sequenceLength += 32; if(sequenceLength > 64) sequenceLength = 32; }
                      else { sequenceLength -= 32; if(sequenceLength < 32) sequenceLength = 64; }
                  }
                  else if (drumSettingsCursor == 1) { if (stepsPerBeat == 4) stepsPerBeat = 3; else stepsPerBeat = 4; }
                  else if (drumSettingsCursor == 2) { if (noteResolution == 4) noteResolution = 8; else noteResolution = 4; }
                  else if (drumSettingsCursor == 3) { 
                      if (pin == BTN_RIGHT) { drumProjectSlot++; if(drumProjectSlot > 15) drumProjectSlot = 0; } 
                      else { drumProjectSlot--; if(drumProjectSlot < 0) drumProjectSlot = 15; } 
                  }
                  drawDrumSettings();
              }
              // 프로젝트 저장/로드
              if (pin == BTN_A) {
                  if (drumSettingsCursor == 4) { saveProject(drumProjectSlot); drawDrumSettings(); }
                  else if (drumSettingsCursor == 5) { loadProject(drumProjectSlot); drawDrumSettings(); }
              }
          }
          return;
      }

      // 사운드 에디터 진입 (ENC1 클릭)
      if (pin == ENC1_SW && isPressed) { 
          isSoundEditMode = true; 
          editTargetIndex = drumCursorTrack; 
          enc1.setCount(0); 
          editParamIndex = 0; // 초기 커서
          
          // [★] 트랙 타입에 맞춰 적절한 에디터 열기
          if (trackType[editTargetIndex] == 1) drawSamplerEditor();
          else drawSoundEditor();
          
          return; 
      }

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

          // 좌우 이동 (헤더 -1 진입 허용)
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
              // [★] 헤더(-1) 선택 중 A버튼 -> 악기 선택 메뉴로 이동
              if (drumCursorStep == -1) {
                  currentMode = MODE_INSTRUMENT_SELECT;
                  instSelectCursor = 0; // 커서 초기화
                  drawInstrumentMenu();
                  return; 
              } 
              else {
                  // 노트 찍기
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