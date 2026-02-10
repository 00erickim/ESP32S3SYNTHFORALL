#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "Globals.h"

// [프로젝트 데이터 구조체]
// 패턴 뿐만 아니라 사운드 설정도 같이 저장합니다.
struct ProjectData {
    char header[4];      // "PRJ1"
    int bpm;
    int sequenceLength;
    int stepsPerBeat;
    int noteResolution;
    int trackVolumes[4];
    bool drumPattern[8][64];
    DrumSound drumSounds[8]; // 사운드 파라미터도 저장
};

const int MAX_FILES = 50; 
String fileList[MAX_FILES];
int fileCount = 0;
String msgText = "";   
unsigned long msgTime = 0;

// [★함수] 슬롯 파일 존재 여부 확인 (UI 표시용)
bool checkSlotExists(int slotIndex) {
    char filename[32];
    sprintf(filename, "/PROJECT_%02d.PRJ", slotIndex + 1);
    return sd.exists(filename);
}

// [함수] 프로젝트 저장
void saveProject(int slotIndex) {
    char filename[32];
    sprintf(filename, "/PROJECT_%02d.PRJ", slotIndex + 1);

    ProjectData pData;
    memcpy(pData.header, "PRJ1", 4);
    pData.bpm = bpm;
    pData.sequenceLength = sequenceLength;
    pData.stepsPerBeat = stepsPerBeat;
    pData.noteResolution = noteResolution;
    for(int i=0; i<4; i++) pData.trackVolumes[i] = trackVolume[i];
    memcpy(pData.drumPattern, drumPattern, sizeof(drumPattern));
    memcpy(pData.drumSounds, drumSounds, sizeof(drumSounds)); // 사운드 저장

    if (sd.exists(filename)) sd.remove(filename);
    
    File32 file = sd.open(filename, O_WRITE | O_CREAT);
    if (file) {
        file.write((uint8_t*)&pData, sizeof(ProjectData));
        file.close();
        msgText = "Saved Slot " + String(slotIndex+1);
    } else {
        msgText = "Save Failed!";
    }
    msgTime = millis();
}

// [함수] 프로젝트 불러오기
void loadProject(int slotIndex) {
    char filename[32];
    sprintf(filename, "/PROJECT_%02d.PRJ", slotIndex + 1);

    if (!sd.exists(filename)) {
        msgText = "Empty Slot!";
        msgTime = millis();
        return;
    }

    File32 file = sd.open(filename, O_READ);
    if (file) {
        ProjectData pData;
        file.read((uint8_t*)&pData, sizeof(ProjectData));
        file.close();

        if (strncmp(pData.header, "PRJ1", 4) == 0) {
            bpm = pData.bpm;
            sequenceLength = pData.sequenceLength;
            stepsPerBeat = pData.stepsPerBeat;
            noteResolution = pData.noteResolution;
            for(int i=0; i<4; i++) trackVolume[i] = pData.trackVolumes[i];
            memcpy(drumPattern, pData.drumPattern, sizeof(drumPattern));
            memcpy(drumSounds, pData.drumSounds, sizeof(drumSounds)); // 사운드 복원
            
            msgText = "Loaded Slot " + String(slotIndex+1);
        } else {
            msgText = "Invalid File!";
        }
    } else {
        msgText = "Load Error!";
    }
    msgTime = millis();
}

// WAV 파일 로딩 관련 (기존 유지)
void loadFileList() {
    fileCount = 0;
    File32 root = sd.open("/");
    if (!root) return;
    File32 file;
    while (file.openNext(&root, O_RDONLY)) {
        if (fileCount >= MAX_FILES) { file.close(); break; }
        char name[32]; file.getName(name, sizeof(name));
        if (name[0] != '.') {
            if (file.isDirectory()) fileList[fileCount] = "[D] " + String(name);
            else fileList[fileCount] = String(name);
            fileCount++;
        }
        file.close();
    }
    root.close();
}

void drawFileBrowser() {
    tft.fillScreen(TFT_BLACK); tft.setTextSize(2); tft.setTextColor(TFT_CYAN, TFT_BLACK); tft.setTextDatum(TC_DATUM);
    tft.drawString("LOAD WAV", 120, 10); tft.drawFastHLine(20, 35, 200, TFT_WHITE);
    tft.setTextSize(1); tft.setTextDatum(TL_DATUM);
    if (fileCount == 0) { tft.setTextColor(TFT_ORANGE, TFT_BLACK); tft.drawCentreString("No Files Found", 120, 100, 2); return; }
    int displayCount = 8;
    for (int i = 0; i < displayCount; i++) {
        int idx = fileScrollTop + i; if (idx >= fileCount) break; int y = 45 + (i * 20);
        if (idx == fileCursor) { tft.fillRect(10, y, 220, 18, TFT_DARKGREY); tft.setTextColor(TFT_YELLOW, TFT_DARKGREY); tft.drawString("> " + fileList[idx], 15, y + 2); } 
        else { tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.drawString("  " + fileList[idx], 15, y + 2); }
    }
}
#endif