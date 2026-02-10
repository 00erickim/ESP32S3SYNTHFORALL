#include "N64.h"

N64Controller::N64Controller(int pin) : _pin(pin) {
    memset(_data, 0, 4);
}

void N64Controller::begin() {
    pinMode(_pin, INPUT_PULLUP);
    digitalWrite(_pin, HIGH);
}

void N64Controller::send_bit(bool bit) {
    if (bit) { // Bit 1: 1us Low, 3us High
        pinMode(_pin, OUTPUT); digitalWrite(_pin, LOW);
        delayMicroseconds(1);
        pinMode(_pin, INPUT_PULLUP); // High-Z (Pull-up)
        delayMicroseconds(3);
    } else { // Bit 0: 3us Low, 1us High
        pinMode(_pin, OUTPUT); digitalWrite(_pin, LOW);
        delayMicroseconds(3);
        pinMode(_pin, INPUT_PULLUP);
        delayMicroseconds(1);
    }
}

bool N64Controller::update() {
    uint8_t temp_data[4] = {0, 0, 0, 0};
    unsigned long t;

    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);

    // 1. Get Status (0x01) 전송
    for(int i=0; i<7; i++) send_bit(0);
    send_bit(1);
    
    // Stop Bit
    pinMode(_pin, OUTPUT); digitalWrite(_pin, LOW); delayMicroseconds(1);
    pinMode(_pin, INPUT_PULLUP);

    // 2. 응답 시작 대기 (Low 신호 포착)
    t = micros();
    while (digitalRead(_pin) == HIGH) {
        if (micros() - t > 500) { portEXIT_CRITICAL(&mux); return false; }
    }

    // 3. 32비트 데이터 읽기
    for (int i = 0; i < 32; i++) {
        // 비트 시작(Low) 대기
        t = micros();
        while (digitalRead(_pin) == HIGH) {
            if (micros() - t > 500) { portEXIT_CRITICAL(&mux); return false; }
        }
        
        // Low 구간 길이 측정
        unsigned long low_start = micros();
        while (digitalRead(_pin) == LOW) {
            if (micros() - low_start > 200) { portEXIT_CRITICAL(&mux); return false; }
        }
        unsigned long low_duration = micros() - low_start;

        // N64 표준: 0은 3us Low, 1은 1us Low. 따라서 2us 기준으로 판별
        if (low_duration < 2) { 
            temp_data[i / 8] |= (0x80 >> (i % 8));
        }
    }

    portEXIT_CRITICAL(&mux);
    memcpy(_data, temp_data, 4);
    return true;
}