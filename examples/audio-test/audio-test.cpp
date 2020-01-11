#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "audio-test.hpp"
#include "rebounder.hpp"

using namespace blit;

/* setup */
void init() {

}

void ch_info(uint8_t ch) {
  switch(blit::audio::channels[ch].phase) {
    case blit::audio::ATTACK:
      fb.text("Attack", &minimal_font[0][0], point(0, 20 + (ch * 20)));
      break;
    case blit::audio::DECAY:
      fb.text("Decay", &minimal_font[0][0], point(0, 20 + (ch * 20)));
      break;
    case blit::audio::SUSTAIN:
      fb.text("Sustain", &minimal_font[0][0], point(0, 20 + (ch * 20)));
      break;
    case blit::audio::RELEASE:
      fb.text("Release", &minimal_font[0][0], point(0, 20 + (ch * 20)));
      break;
  }
  fb.text(std::to_string(blit::audio::channels[ch].adsr), &minimal_font[0][0], point(0, 28 + (ch * 20)));
}

void render(uint32_t time_ms) {

  fb.pen(rgba(0, 0, 0, 255));
  fb.clear();  

  fb.watermark();
  
  fb.pen(rgba(255, 255, 255));

  static uint16_t i = 0;
  i++;
  fb.pixel(point(i % 160, 0));

  for(auto j = 0; j < 3; j++) {
    fb.pixel(point(blit::audio::channels[j].pulse_width >> 8, (j * 2) + 10));
  }

  fb.pen(rgba(255, 255, 255, 255));
  fb.text(std::to_string(time_ms), &minimal_font[0][0], point(0, 10));

  ch_info(0);
  ch_info(1);
  ch_info(2);
  
}


// map of SID chip ADSR 4-bit values to millisecond timings
uint32_t  a_to_ms[] = {2,  8, 16, 24,  38,  56,  68,  80, 100, 250,  500,  800, 1000, 3000,  5000,  8000};
uint32_t dr_to_ms[] = {6, 24, 48, 72, 114, 168, 204, 240, 300, 750, 1500, 2400, 3000, 9000, 15000, 24000};
uint32_t s_to_vol[] = {0,4369,8738,13107,17476,21845,26214,30583,34952,39321,43690,48059,52428,56797,61166,65535};


void update(uint32_t time_ms) {
  static uint32_t last_time_ms = time_ms;
  static uint16_t tick = 0;

  tick++;
  uint16_t row = (tick >> 1) % (sizeof(song) / 25);
  
  for(auto i = 0; i < 3; i++) {
    uint8_t *sample = song + (row * 25) + (i * 7);

    uint32_t f = (sample[1] << 8) | sample[0];
    uint16_t voices = sample[4];

    uint32_t fhz = (f * 98525L) / 1677721L;

    blit::audio::channels[i].pulse_width = (((sample[3] & 0xf) << 8) | sample[2]) << 4;
    blit::audio::channels[i].frequency   = fhz;

    if(voices & 0b01110000) {
      // if any non-noise voice is active then disable noise channel      
      voices &= ~0b10000000;      
    }

    blit::audio::channels[i].voices      = voices & 0b11110000;
    blit::audio::channels[i].gate        = voices & 0b1;

    blit::audio::channels[i].attack_ms   = a_to_ms[(sample[5] & 0xf0) >> 4];
    blit::audio::channels[i].decay_ms    = dr_to_ms[sample[5] & 0xf];
    blit::audio::channels[i].sustain     = s_to_vol[(sample[6] & 0xf0) >> 4];
    blit::audio::channels[i].release_ms  = dr_to_ms[sample[6] & 0xf];

    blit::audio::channels[i].volume      = 0x7fff;
  }

  uint8_t volume = song[(row * 25) + 24] & 0x0f;
  uint8_t mute_3 = song[(row * 25) + 24] & 0b10000000;
  if(mute_3) {
    blit::audio::channels[2].volume      = 0x0000;
  }  

  blit::audio::volume      = s_to_vol[volume & 0x0f];


/*
  // seashore demo
  blit::audio::channels[0].frequency   = 4200;
  blit::audio::channels[0].voices      = blit::audio::audio_voice::NOISE;
  blit::audio::channels[0].gate        = 1;

  blit::audio::channels[0].attack_ms   = 0;
  blit::audio::channels[0].decay_ms    = 0;
  blit::audio::channels[0].sustain     = 0xffff;
  blit::audio::channels[0].release_ms  = 0;

  blit::audio::channels[0].volume      = (sin(float(tick) / 50.0f) * 6000) + 7000;
*/


  //blit::audio::channels[i].pw         = ((sample[3] & 0xf) << 8) | sample[2];
/*
  blit::audio::channels[0].frequency  = 440;
  blit::audio::channels[0].voices     = blit::audio::audio_voice::SINE;
  blit::audio::channels[0].attack_ms  = 500;
  blit::audio::channels[0].decay_ms   = 1000;
  blit::audio::channels[0].sustain    = 16000;
  blit::audio::channels[0].release_ms = 500;
  blit::audio::channels[0].volume     = 0x7fff;

  blit::audio::channels[0].gate       = sin(time_ms * 2 * M_PI / 5000) > 0;
*/

  last_time_ms = time_ms;
}