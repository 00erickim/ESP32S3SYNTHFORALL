#ifndef N64_H
#define N64_H

#include <Arduino.h>

class N64Controller {
public:
    N64Controller(int pin);
    void begin();
    bool update();

    // 버튼 상태
    bool A() { return _data[0] & 0x80; }
    bool B() { return _data[0] & 0x40; }
    bool Z() { return _data[0] & 0x20; }
    bool Start() { return _data[0] & 0x10; }
    bool D_up() { return _data[0] & 0x08; }
    bool D_down() { return _data[0] & 0x04; }
    bool D_left() { return _data[0] & 0x02; }
    bool D_right() { return _data[0] & 0x01; }
    bool L() { return _data[1] & 0x20; }
    bool R() { return _data[1] & 0x10; }
    bool C_up() { return _data[1] & 0x08; }
    bool C_down() { return _data[1] & 0x04; }
    bool C_left() { return _data[1] & 0x02; }
    bool C_right() { return _data[1] & 0x01; }

    int8_t axis_x() { return (int8_t)_data[2]; }
    int8_t axis_y() { return (int8_t)_data[3]; }

private:
    int _pin;
    uint8_t _data[4];
    void send_bit(bool bit);
};

#endif