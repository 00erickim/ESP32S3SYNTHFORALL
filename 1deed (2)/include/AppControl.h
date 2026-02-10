#ifndef APP_CONTROL_H
#define APP_CONTROL_H

#include "Globals.h"
#include "SamplerUI.h" // [★중요] 샘플러 UI 포함

// 함수 전방 선언
void redrawMixerTrack(int);
void drawSoundEditor();
void drawPatternGenUI();
void drawTrackerUI();
void drawGranularUI();
void drawDroneUI();
void drawKeyboardUI();
void setSynthWaveform(int);
void redrawStep(int);
void drawDrumMachine(bool);
void drawPianoRoll();
void drawFileBrowser(); 

void updateAppLogic() {
    
    // [0. 사운드 에디터 모드 (엔코더 처리)]
    if (isSoundEditMode) {
        long val = enc1.getCount(); 
        if (val != 0) {
            // 현재 트랙이 샘플러인지 확인
            bool isSampler = (trackType[editTargetIndex] == 1);
            DrumSound* targetDS = &drumSounds[editTargetIndex];

            // 0~4번 파라미터는 공통 (ADSR + Pitch)
            if (editParamIndex == 0) { int newVal = targetDS->attack + val; if(newVal < 0) newVal = 0; if(newVal > 500) newVal = 500; targetDS->attack = newVal; }
            else if (editParamIndex == 1) { int newVal = targetDS->decay + (val * 10); if(newVal < 10) newVal = 10; if(newVal > 2000) newVal = 2000; targetDS->decay = newVal; }
            else if (editParamIndex == 2) { int newVal = targetDS->sustain + (val * 5); if(newVal < 0) newVal = 0; if(newVal > 255) newVal = 255; targetDS->sustain = newVal; }
            else if (editParamIndex == 3) { int newVal = targetDS->release + (val * 10); if(newVal < 0) newVal = 0; if(newVal > 2000) newVal = 2000; targetDS->release = newVal; }
            else if (editParamIndex == 4) { int newVal = targetDS->pitch + (val * 10); if(newVal < 20) newVal = 20; if(newVal > 2000) newVal = 2000; targetDS->pitch = newVal; }
            
            // 5번 파라미터: [신스] 파형선택 / [샘플러] 파일로드(버튼)
            else if (editParamIndex == 5) { 
                if (!isSampler) {
                    // 신스 모드일 때만 파형 변경
                    int newVal = targetDS->waveform + val;
                    if(newVal < 0) newVal = 4; 
                    if(newVal > 4) newVal = 0; 
                    targetDS->waveform = newVal; 
                }
                // 샘플러일 때는 버튼 기능이므로 엔코더 회전 무시
            }
            
            // 6번 파라미터: [신스] Pitch Mod / [샘플러] Sample Start
            else if (editParamIndex == 6) { 
                if (isSampler) {
                    // Start 지점 (0 ~ 990)
                    int newVal = targetDS->sampleStart + (val * 10);
                    if(newVal < 0) newVal = 0; 
                    if(newVal >= targetDS->sampleEnd) newVal = targetDS->sampleEnd - 10; // End보다 커질 수 없음
                    targetDS->sampleStart = newVal;
                } else {
                    // Pitch Mod
                    int newVal = targetDS->pitchMod + (val * 5); 
                    if(newVal < 0) newVal = 0; 
                    if(newVal > 500) newVal = 500; 
                    targetDS->pitchMod = newVal; 
                }
            }

            // 7번 파라미터: [샘플러 전용] Sample End
            else if (editParamIndex == 7 && isSampler) {
                 int newVal = targetDS->sampleEnd + (val * 10);
                 if(newVal <= targetDS->sampleStart) newVal = targetDS->sampleStart + 10; // Start보다 작아질 수 없음
                 if(newVal > 1000) newVal = 1000;
                 targetDS->sampleEnd = newVal;
            }

            enc1.setCount(0); 
            
            // 화면 갱신
            if (isSampler) drawSamplerEditor();
            else drawSoundEditor(); 
        }
        return; 
    }

    // [1. 파일 브라우저 모드 (샘플러 로딩용)]
    if (currentMode == MODE_SAMPLE_LOADER) {
        // 엔코더로 파일 목록 스크롤 (버튼 외에 편의성 제공)
        long scroll = enc1.getCount();
        if (scroll != 0) {
            fileCursor += scroll;
            // 범위 체크는 InputHandlers와 공유하거나 중복 구현 필요 (간략화)
             if(fileCursor < 0) { fileCursor = 0; if(fileScrollTop > 0) fileScrollTop--; } 
             // ... (스크롤 로직은 InputHandlers의 버튼 로직과 동일하게 동작해야 함)
            enc1.setCount(0);
            drawFileBrowser();
        }
        return;
    }

    // [2. 패턴 생성기 모드]
    if (isPatternGenMode) {
        long lenChange = enc1.getCount();
        if (lenChange != 0) { genLength += lenChange; if (genLength < 1) genLength = 1; if (genLength > 64) genLength = 64; enc1.setCount(0); drawPatternGenUI(); }
        long hitChange = enc2.getCount();
        if (hitChange != 0) { genHits += hitChange; if (genHits < 0) genHits = 0; if (genHits > genLength) genHits = genLength; enc2.setCount(0); drawPatternGenUI(); }
        return; 
    }

    // [3. 모드별 엔코더 기능]
    if (currentMode == MODE_MENU) {
        long volChange = enc1.getCount();
        if (volChange != 0) { trackVolume[currentMixerTrack] += volChange * 5; if (trackVolume[currentMixerTrack] < 0) trackVolume[currentMixerTrack] = 0; if (trackVolume[currentMixerTrack] > 255) trackVolume[currentMixerTrack] = 255; enc1.setCount(0); redrawMixerTrack(currentMixerTrack); }
    }
    else if (currentMode == MODE_TRACKER) {
        long val = enc1.getCount();
        if (val != 0) { int8_t& note = trackerPattern[trackerCursorStep][trackerCursorTrack].note; if (note == -1) note = 60; else if (note >= 0) { note += val; if (note < 0) note = 0; if (note > 127) note = 127; } enc1.setCount(0); drawTrackerUI(); }
        // ... (나머지 트래커 로직 유지) ...
        if (digitalRead(ENC1_SW) == LOW) { delay(50); if(digitalRead(ENC1_SW) == LOW) { while(digitalRead(ENC1_SW) == LOW); isSoundEditMode = true; editTargetIndex = trackerCursorTrack % 8; enc1.setCount(0); drawSoundEditor(); } }
        if (digitalRead(ENC2_SW) == LOW) { delay(50); if(digitalRead(ENC2_SW) == LOW) { while(digitalRead(ENC2_SW) == LOW); isPatternGenMode = true; genNote = 60; int8_t curN = trackerPattern[trackerCursorStep][trackerCursorTrack].note; if (curN >= 0) genNote = curN; enc1.setCount(0); enc2.setCount(0); drawPatternGenUI(); } }
    }
    else if (currentMode == MODE_GRANULAR_PLAY) { 
        long posChange = enc1.getCount(); if (posChange != 0) { grainPosition += posChange * 0.005f; if (grainPosition < 0.0f) grainPosition = 0.0f; if (grainPosition > 1.0f) grainPosition = 1.0f; enc1.setCount(0); drawGranularUI(); } 
        long sizeChange = enc2.getCount(); if (sizeChange != 0) { grainSize += sizeChange * 500; if (grainSize < 100) grainSize = 100; if (grainSize > 500000) grainSize = 500000; enc2.setCount(0); drawGranularUI(); } 
    }
    else if (currentMode == MODE_DRONE) { 
        long pitchChange = enc1.getCount(); if (pitchChange != 0) { dronePitch += pitchChange * 50; if (dronePitch < 20) dronePitch = 20; if (dronePitch > 20000) dronePitch = 20000; enc1.setCount(0); drawDroneUI(); } 
        long rateChange = enc2.getCount(); if (rateChange != 0) { lfoRate += rateChange * 100; if (lfoRate < 0) lfoRate = 0; if (lfoRate > 10000) lfoRate = 10000; enc2.setCount(0); drawDroneUI(); } 
        // ... (드론 UI 애니메이션 유지) ...
        static unsigned long lastDraw = 0; if (millis() - lastDraw > 30) { lastDraw = millis(); float uiSpeed = map(constrain(lfoRate, 0, 1000), 0, 1000, 1, 500); int movingBar = 120 + (sin(millis() * uiSpeed * 0.001) * lfoDepth / 2); tft.fillRect(20, 50, 200, 20, TFT_DARKGREY); tft.fillRect(movingBar - 5, 50, 10, 20, TFT_GREEN); } 
    }
    else if (currentMode == MODE_KEYBOARD) { 
        long wave = enc1.getCount(); if (wave != synthWaveform) { if(wave < 0) { wave = 2; enc1.setCount(2); } if(wave > 2) { wave = 0; enc1.setCount(0); } synthWaveform = wave; setSynthWaveform(synthWaveform); drawKeyboardUI(); } 
        long filter = enc2.getCount(); if (filter < 0) { filter = 0; enc2.setCount(0); } if (filter > 255) { filter = 255; enc2.setCount(255); } if (filter != synthFilterVal) { synthFilterVal = filter; drawKeyboardUI(); } 
    }

    // [BPM 조절 (공통)]
    if (!isSoundEditMode && !isPatternGenMode && (currentMode == MODE_DRUM || currentMode == MODE_FINGER_DRUM || currentMode == MODE_TRACKER || currentMode == MODE_PIANO_ROLL)) { 
        long bpmChange = enc2.getCount(); 
        if (bpmChange != 0) { bpm += bpmChange; if (bpm < 40) bpm = 40; if (bpm > 300) bpm = 300; enc2.setCount(0); if(currentMode == MODE_DRUM) drawDrumMachine(false); } 
    }

    // [시퀀서 화면 갱신]
    if (isSequencerPlaying && !isSoundEditMode && !isPatternGenMode) {
        if (currentMode == MODE_DRUM) { static int lastStep = -1; if (currentStep != lastStep) { if(lastStep != -1) redrawStep(lastStep); redrawStep(currentStep); drawDrumMachine(false); lastStep = currentStep; } }
        else if (currentMode == MODE_TRACKER || currentMode == MODE_PIANO_ROLL) { static int lastTrkStep = -1; if (currentStep != lastTrkStep) { if (currentMode == MODE_TRACKER) drawTrackerUI(); else drawPianoRoll(); lastTrkStep = currentStep; } }
    }
}
#endif