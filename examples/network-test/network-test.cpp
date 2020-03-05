#include "network-test.hpp"
#include "graphics/color.hpp"
#include <cmath>

using namespace blit;

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 120

#define NIC_DEVICE_ADDRESS  0x32    // All devices using 'this' protocol use this address

#define NIC_TYPE_REGISTER   0x00    // To set/request NIC type  (Set should be followed by get to check device is able to change!)
#define NIC_TYPE_P2P        0x00    // Single endpoint: wired, ir, rf.
#define NIC_TYPE_MESH       0x01    // Mesh network, i.e. ESP-Now
#define NIC_TYPE_ETH        0x02    // Ethernet (WiFi) - IP to PubSub service

//#define NIC_

uint8_t nicResult[7];
int blah;

void init() {
    set_screen_mode(ScreenMode::lores);

    //chip_id = useri2c_receive_8(NIC_DEVICE_ADDRESS, 0);
    //chip_id = i2c_recv(&hi2c4, NIC_DEVICE_ADDRESS, 0);
    blah = i2c_receive(NIC_DEVICE_ADDRESS, NIC_TYPE_REGISTER, *nicResult, 7);
}

//void i2c_receive(uint8_t address, uint8_t reg, uint8_t &result, uint8_t result_size);
//void i2c_send(uint8_t address, uint8_t reg, uint8_t &data, uint8_t data_size);

#define COL1 5
#define COL2 55
#define COL3 105

#define ROW1 38
#define ROW2 70
#define ROW3 102

void render(uint32_t time) {
    char text_buf[100] = {0};
    bool button_a = blit::buttons & blit::Button::A;
    bool button_b = blit::buttons & blit::Button::B;
    bool button_x = blit::buttons & blit::Button::X;
    bool button_y = blit::buttons & blit::Button::Y;
    bool button_m = blit::buttons & blit::Button::MENU;
    bool button_h = blit::buttons & blit::Button::HOME;
    bool dpad_l = blit::buttons & blit::Button::DPAD_LEFT;
    bool dpad_r = blit::buttons & blit::Button::DPAD_RIGHT;
    bool dpad_u = blit::buttons & blit::Button::DPAD_UP;
    bool dpad_d = blit::buttons & blit::Button::DPAD_DOWN;

    for(int b = 0; b < SCREEN_WIDTH; b++){
        for(int v = 0; v < SCREEN_HEIGHT; v++){
            screen.pen = blit::hsv_to_rgba(float(b) / (float)(SCREEN_WIDTH), 1.0f, float(v) / (float)(SCREEN_HEIGHT));
            screen.pixel(Point(b, v));
        }
    }

    screen.pen = button_a ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("A", minimal_font, Point(150, 15));

    screen.pen = button_b ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("B", minimal_font, Point(140, 25));

    screen.pen = button_x ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("X", minimal_font, Point(140, 5));

    screen.pen = button_y ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("Y", minimal_font, Point(130, 15));


    screen.pen = dpad_r ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("R", minimal_font, Point(25, 15));

    screen.pen = dpad_d ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("D", minimal_font, Point(15, 25));

    screen.pen = dpad_u ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("U", minimal_font, Point(15, 5));

    screen.pen = dpad_l ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("L", minimal_font, Point(5, 15));

    screen.pen = button_h ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("HOME", minimal_font, Point(85, 15));

    blit::LED = Pen(
        (float)((sin(blit::now() / 100.0f) + 1) / 2.0f),
        (float)((cos(blit::now() / 100.0f) + 1) / 2.0f),
        (float)((sin(blit::now() / 100.0f) + 1) / 2.0f)
    );

    snprintf(text_buf, 100, "%s", reinterpret_cast<char*>(nicResult));
    screen.text("NIC:", minimal_font, Point(COL1, ROW3));
    screen.text(text_buf, minimal_font, Point(COL1, ROW3+7));

    //snprintf(text_buf, 100, "%d", (int)(blit::battery * 1000.f));
    snprintf(text_buf, 100, "%d", blah);
    screen.text("Battery:", minimal_font, Point(COL3, ROW3));
    screen.text(text_buf, minimal_font, Point(COL3, ROW3+7));
}

void update(uint32_t time) {

}
