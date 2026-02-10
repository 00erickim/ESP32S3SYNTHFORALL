#ifndef TRACKER_UI_H
#define TRACKER_UI_H

#include "Globals.h"

// 디자인 상수
const int ROW_H = 15;
const int HDR_H = 20;
const int ROW_NUM_W = 24; // 행 번호 너비
const int COL_W = 27;     // 트랙 너비 (27 * 8 = 216) -> Total 240
const int VISIBLE_ROWS = 8; 

// 노트 이름을 문자로 변환
String getNoteName(int note) {
    if (note == -1) return "---";
    if (note == -2) return "OFF";
    
    const char* notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int octave = (note / 12) - 1; 
    int n = note % 12;
    
    // 공간 절약을 위해 '#' 생략하거나 짧게 표시 (C#4 -> C4#, C-4)
    // 여기서는 3글자로 맞춤: "C-4", "F#4"
    String s = String(notes[n]);
    if (s.length() == 1) s += "-"; // C -> C-
    s += String(octave);
    return s;
}

void drawTrackerUI() {
    // 0. 페이지 계산
    trackerPage = trackerCursorTrack / 8;
    int trackOffset = trackerPage * 8; // 0 or 8

    // 1. 스프라이트 배경
    img.fillSprite(TFT_BLACK);

    // 2. 헤더 (트랙 번호)
    img.fillRect(0, 0, 240, HDR_H, 0x2124); 
    img.setTextColor(TFT_CYAN, 0x2124);
    img.setTextDatum(MC_DATUM);
    
    // 타이틀: SYNTH (1-8) or DRUM (9-16)
    String title = (trackerPage == 0) ? "SYNTH (1-8)" : "DRUM (9-16)";
    img.drawString(title, ROW_NUM_W + 108, HDR_H/2); // 중앙 정렬

    // 3. 그리드 그리기
    if (trackerCursorStep < trackerScrollTop) trackerScrollTop = trackerCursorStep;
    if (trackerCursorStep >= trackerScrollTop + VISIBLE_ROWS) trackerScrollTop = trackerCursorStep - VISIBLE_ROWS + 1;

    int yStart = HDR_H;
    
    for(int i=0; i<VISIBLE_ROWS; i++) {
        int stepIdx = trackerScrollTop + i;
        if (stepIdx >= 64) break;

        int y = yStart + (i * ROW_H);
        
        bool isPlayingStep = (isSequencerPlaying && stepIdx == currentStep);
        uint16_t rowBgColor = isPlayingStep ? 0x2124 : TFT_BLACK; 
        
        // 행 번호
        img.fillRect(0, y, ROW_NUM_W, ROW_H, TFT_BLACK); 
        img.setTextColor((stepIdx % 4 == 0) ? TFT_WHITE : TFT_DARKGREY, rowBgColor);
        char hexBuf[4]; sprintf(hexBuf, "%02X", stepIdx);
        img.setTextDatum(MR_DATUM);
        img.drawString(hexBuf, ROW_NUM_W - 2, y + ROW_H/2);

        // 트랙 데이터 그리기 (8개 컬럼)
        for(int t=0; t<8; t++) {
            int actualTrackIdx = trackOffset + t;
            int x = ROW_NUM_W + (t * COL_W);
            
            bool isCursor = (stepIdx == trackerCursorStep && actualTrackIdx == trackerCursorTrack);
            
            // 배경
            if (isCursor) img.fillRect(x, y, COL_W, ROW_H, TFT_WHITE);
            else img.fillRect(x, y, COL_W, ROW_H, rowBgColor);
            
            // 데이터
            TrackerStep& ts = trackerPattern[stepIdx][actualTrackIdx];
            String txt = getNoteName(ts.note);
            
            uint16_t txtColor;
            if (isCursor) txtColor = TFT_BLACK; 
            else if (ts.note == -1) txtColor = 0x3186; 
            else if (ts.note == -2) txtColor = TFT_RED; 
            else txtColor = TFT_GREEN; 
            
            img.setTextColor(txtColor, isCursor ? TFT_WHITE : rowBgColor);
            img.setTextDatum(MC_DATUM);
            img.drawString(txt, x + COL_W/2, y + ROW_H/2);
            
            // 구분선
            if (!isCursor) img.drawFastVLine(x + COL_W - 1, y, ROW_H, 0x10A2); // 아주 연한 선
        }
        
        if (stepIdx % 4 == 3) img.drawFastHLine(0, y + ROW_H - 1, 240, 0x3186);
    }
    
    // 재생바
    if (isSequencerPlaying) {
        if (currentStep >= trackerScrollTop && currentStep < trackerScrollTop + VISIBLE_ROWS) {
             int relY = HDR_H + ((currentStep - trackerScrollTop) * ROW_H);
             img.drawRect(0, relY, 240, ROW_H, TFT_YELLOW); // 전체 테두리 강조
        }
    }
    
    img.pushSprite(0, 0);
}


// [유클리드 패턴 계산 및 UI]
void drawPatternGenUI() {
    int boxW = 180;
    int boxH = 100;
    int boxX = (240 - boxW) / 2;
    int boxY = (135 - boxH) / 2;
    
    // 팝업 배경
    img.fillSprite(TFT_BLACK); // 배경 지우고 다시 그리는 게 깔끔함 (또는 drawTrackerUI 위에 덧칠)
    drawTrackerUI(); // 배경에 트래커 깔기
    
    // 박스
    img.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
    img.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
    
    // 타이틀
    img.setTextColor(TFT_CYAN, TFT_BLACK);
    img.setTextDatum(TC_DATUM);
    img.drawString("EUCLIDEAN GEN", 120, boxY + 5);
    
    // 파라미터 표시
    img.setTextDatum(TL_DATUM);
    img.setTextColor(TFT_WHITE, TFT_BLACK);
    
    // ENC1: Length, ENC2: Hits
    String strLen = "LEN (Enc1): " + String(genLength);
    String strHits = "HITS (Enc2): " + String(genHits);
    
    img.drawString(strLen, boxX + 15, boxY + 30);
    img.drawString(strHits, boxX + 15, boxY + 50);
    
    // 미리보기 (Visualizer)
    int vizX = boxX + 10;
    int vizY = boxY + 75;
    int vizW = 160;
    int stepW = vizW / genLength;
    if (stepW < 2) stepW = 2;
    
    for(int i=0; i<genLength; i++) {
        // 유클리드 공식: (i * hits) % length < hits
        // 더 고른 분포를 위한 Bjorklund 알고리즘 대신 간단한 Bresenham 라인 알고리즘 변형 사용
        // (hits * i) / length
        
        bool isHit = false;
        // 간단한 방식:
        // isHit = ((i * genHits) % genLength) < genHits; 
        // -> 이 방식은 뭉치는 경향이 있음.
        
        // 더 나은 방식 (정수 연산):
        int acc = (i * genHits) % genLength;
        // 첫 박자는 무조건? 유클리드 정의상 분산됨.
        // 가장 간단한 Spread:
        if (genHits > 0) {
           // (current * hits) / length != (prev * hits) / length
           // 하지만 그냥 간단히 가겠습니다.
           if ( ((i * genHits) % genLength) < genHits ) isHit = true; 
        }
        
        // UI 그리기
        int x = vizX + (i * stepW);
        if (isHit) img.fillRect(x, vizY, stepW-1, 8, TFT_GREEN);
        else img.drawRect(x, vizY, stepW-1, 8, TFT_DARKGREY);
    }
    
    img.pushSprite(0, 0);
}

// 패턴 적용 함수
void applyEuclideanPattern(int startStep, int trackIdx) {
    for(int i=0; i<genLength; i++) {
        int targetStep = (startStep + i) % 64;
        
        // 간단 유클리드 공식 적용
        bool isHit = ((i * genHits) % genLength) < genHits;
        
        if (isHit) {
            trackerPattern[targetStep][trackIdx].note = genNote; // Note On
            // 볼륨 등 초기화
            trackerPattern[targetStep][trackIdx].inst = 0; 
            trackerPattern[targetStep][trackIdx].vol = 255; 
        } else {
            // 기존 노트 지우기 (선택사항 - 덮어쓰기 모드라면 지우는 게 맞음)
            trackerPattern[targetStep][trackIdx].note = -1; 
        }
    }
}

// [피아노 롤 UI]
const int PR_KEY_W = 30;
const int PR_NOTE_H = 11;
const int PR_VISIBLE_NOTES = 12; // 1 Octave
const int PR_STEP_W = 13; 
const int PR_VISIBLE_STEPS = 16; 

void drawPianoRoll() {
    // 1. 배경
    img.fillSprite(TFT_BLACK);
    
    // 2. 스크롤 계산 (Step)
    if (pianoRollCursorStep < pianoRollScrollStep) pianoRollScrollStep = pianoRollCursorStep;
    if (pianoRollCursorStep >= pianoRollScrollStep + PR_VISIBLE_STEPS) pianoRollScrollStep = pianoRollCursorStep - PR_VISIBLE_STEPS + 1;
    
    // 3. 스크롤 계산 (Note)
    // 커서가 화면 밖으로 나가면 스크롤 이동
    if (pianoRollCursorNote < pianoRollScrollNote) pianoRollScrollNote = pianoRollCursorNote;
    if (pianoRollCursorNote >= pianoRollScrollNote + PR_VISIBLE_NOTES) pianoRollScrollNote = pianoRollCursorNote - PR_VISIBLE_NOTES + 1;

    // 4. 건반 및 그리드 그리기 (위에서 아래로 높은음->낮은음? 보통 피아노롤은 아래가 낮은음)
    // 여기서는 화면 y=0이 높은음, y=bottom이 낮은음으로 배치하거나 반대로.
    // 보통 y=bottom이 낮은음(Low Pitch)이 직관적.
    // 따라서 루프는 0~11을 돌리되, 그리는 y좌표는 (11-i)*H 로 계산.
    
    for(int i=0; i<PR_VISIBLE_NOTES; i++) {
        int noteIdx = pianoRollScrollNote + i; // 실제 노트 번호 (ex: 60, 61...)
        int y = (PR_VISIBLE_NOTES - 1 - i) * PR_NOTE_H; // 아래에서 위로 쌓음
        
        // 4-1. 좌측 건반
        bool isBlackKey = false;
        int n = noteIdx % 12;
        if(n==1 || n==3 || n==6 || n==8 || n==10) isBlackKey = true;
        
        uint16_t keyColor = isBlackKey ? TFT_BLACK : TFT_WHITE;
        uint16_t keyTextColor = isBlackKey ? TFT_WHITE : TFT_BLACK;
        
        // 현재 커서 노트 강조
        if (noteIdx == pianoRollCursorNote) {
            keyColor = TFT_YELLOW;
            keyTextColor = TFT_BLACK;
        }
        
        img.fillRect(0, y, PR_KEY_W, PR_NOTE_H, keyColor);
        img.drawRect(0, y, PR_KEY_W, PR_NOTE_H, TFT_DARKGREY);
        
        // 노트 이름 (C4, D#4 등) - C, F, A 정도만 표시하거나 옥타브 시작만 표시
        if (n == 0) { // C 노트
            img.setTextColor(keyTextColor);
            img.setTextDatum(MC_DATUM);
            img.drawString("C" + String(noteIdx/12 - 1), PR_KEY_W/2, y + PR_NOTE_H/2);
        }
        
        // 4-2. 그리드 배경
        img.drawFastHLine(PR_KEY_W, y, 240-PR_KEY_W, 0x18E3); // 어두운 라인
        img.drawFastHLine(PR_KEY_W, y + PR_NOTE_H - 1, 240-PR_KEY_W, 0x18E3);
        
        // 4-3. 노트 데이터 그리기
        for(int s=0; s<PR_VISIBLE_STEPS; s++) {
            int stepIdx = pianoRollScrollStep + s;
            if (stepIdx >= 64) break;
            
            int x = PR_KEY_W + (s * PR_STEP_W);
            
            // 재생 바 (세로선)
            if (isSequencerPlaying && stepIdx == currentStep) {
                img.fillRect(x, 0, PR_STEP_W, 135, 0x2124); // 흐린 배경 하이라이트
            }
            
            // 그리드 세로선 (4박자마다 진하게)
            if (stepIdx % 4 == 0) img.drawFastVLine(x, 0, 135, 0x3186);
            else img.drawFastVLine(x, 0, 135, 0x10A2);

            // 노트 존재 확인
            int8_t trkNote = trackerPattern[stepIdx][trackerCursorTrack].note;
            
            // 노트 찍기 (현재 행의 노트와 일치하면)
            if (trkNote == noteIdx) {
                img.fillRect(x+1, y+1, PR_STEP_W-2, PR_NOTE_H-2, TFT_GREEN);
            }
            
            // 커서 (Step, Note)
            if (stepIdx == pianoRollCursorStep && noteIdx == pianoRollCursorNote) {
                img.drawRect(x, y, PR_STEP_W, PR_NOTE_H, TFT_RED);
            }
        }
    }
    
    // 상단 정보 (현재 트랙 등)
    img.setTextColor(TFT_CYAN, TFT_BLACK);
    img.setTextDatum(TR_DATUM);
    img.drawString("TRK " + String(trackerCursorTrack + 1), 240, 0);
    
    img.pushSprite(0, 0);
}

#endif