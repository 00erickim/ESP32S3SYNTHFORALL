// SpaceVisualizer.h
#ifndef SPACE_VISUALIZER_H
#define SPACE_VISUALIZER_H

#include <TFT_eSPI.h>
#include "Globals.h" 

#define NUM_STARS 60 // 별 개수 조정

// [★수정★] 흑백 밝기 정의 (알파값 느낌)
// 작을수록 어둡고, 클수록 밝게 설정
#define GRAY_TINY   0x2104 // 아주 작은 별 (거의 검은색에 가까운 회색)
#define GRAY_SMALL  0x528A // 작은 별 (어두운 회색)
#define GRAY_MEDIUM 0x9492 // 중간 별 (중간 회색)
#define GRAY_LARGE  0xFFFF // 큰 별 (완전 흰색)

struct Star {
  float x, y;
  float speed;
  // uint16_t color; // 컬러 삭제됨
  uint8_t size; // 0:tiny, 1:small, 2:medium, 3:large(cross)
  int flashIntensity; 
};

class SpaceVisualizer {
private:
    Star stars[NUM_STARS];
    TFT_eSprite* sprite; 
    int width, height;

public:
    SpaceVisualizer(TFT_eSPI* tft, int w, int h) {
        width = w; height = h;
        sprite = new TFT_eSprite(tft);
        sprite->setColorDepth(16);
        sprite->createSprite(width, height);
        initStars();
    }

    void initStars() { for(int i=0; i<NUM_STARS; i++) resetStar(i, true); }

    void resetStar(int i, bool randomY) {
        stars[i].x = random(width);
        stars[i].y = randomY ? random(height) : 0; 
        
        // 크기에 따른 속도 및 밝기 결정 (큰 별이 더 빠르고 밝음)
        int r = random(100);
        if (r < 40) { stars[i].size = 0; stars[i].speed = random(5, 15)/20.0; }      // 40%: Tiny (느림, 어둠)
        else if (r < 70) { stars[i].size = 1; stars[i].speed = random(15, 30)/20.0; } // 30%: Small
        else if (r < 90) { stars[i].size = 2; stars[i].speed = random(30, 50)/20.0; } // 20%: Medium
        else { stars[i].size = 3; stars[i].speed = random(50, 80)/20.0; }             // 10%: Large (빠름, 밝음)
        
        stars[i].flashIntensity = 0;
    }

    void flashRandomStar() {
        int idx = random(NUM_STARS);
        stars[idx].flashIntensity = 255; 
        // 플래시 터질 때는 무조건 가장 큰 크기로 변경
        stars[idx].size = 3;
    }

    void updateAndDraw() {
        sprite->fillSprite(TFT_BLACK); // 완전 검은 배경

        int fadeSpeed = map(reverbLevel, 0, 255, 25, 2);

        for(int i=0; i<NUM_STARS; i++) {
            stars[i].y += stars[i].speed;
            if (stars[i].y > height) resetStar(i, false);

            if (stars[i].flashIntensity > 0) {
                stars[i].flashIntensity -= fadeSpeed;
                if(stars[i].flashIntensity < 0) stars[i].flashIntensity = 0;
            }

            int x = (int)stars[i].x;
            int y = (int)stars[i].y;
            uint16_t c = TFT_BLACK;

            // [★핵심★] 크기에 따른 흑백 밝기 적용
            switch(stars[i].size) {
                case 0: c = GRAY_TINY; break;
                case 1: c = GRAY_SMALL; break;
                case 2: c = GRAY_MEDIUM; break;
                case 3: c = GRAY_LARGE; break;
            }

            // 플래시 효과 (가장 밝은 흰색 + 잔상 원)
            if (stars[i].flashIntensity > 20) {
                c = GRAY_LARGE; 
                sprite->fillCircle(x, y, 2 + (stars[i].flashIntensity/70), c);
            } 
            // 일반 그리기 (크기별 모양)
            else {
                if (stars[i].size == 0) sprite->drawPixel(x, y, c);                 // 1px 점
                else if (stars[i].size == 1) sprite->fillRect(x, y, 2, 2, c);       // 2x2 사각형
                else if (stars[i].size == 2) sprite->fillRect(x, y, 3, 3, c);       // 3x3 사각형
                else if (stars[i].size == 3) {                                      // 십자 모양
                    sprite->drawFastHLine(x-2, y, 5, c);
                    sprite->drawFastVLine(x, y-2, 5, c);
                }
            }
        }
        sprite->pushSprite(0, 0);
    }
};

#endif