/*! \file audio.cpp
    \brief Audio engine
*/
#include "engine.hpp"

#include "audio.hpp"

namespace blit {

  namespace audio {

    uint32_t sample_rate = 22050;
    uint32_t frame_ms = (1000 << 16) / 22050;
    const uint32_t precision = 16;

    uint16_t volume = 0xffff;
    uint16_t sine_voice[256] = {32767,33571,34375,35177,35979,36778,37575,38369,39160,39946,40729,41506,42279,43045,43806,44560,45306,46046,46777,47499,48213,48918,49613,50297,50971,51635,52286,52926,53554,54170,54772,55361,55937,56498,57046,57578,58096,58599,59086,59557,60012,60450,60872,61277,61665,62035,62388,62723,63040,63338,63619,63880,64123,64347,64552,64738,64904,65052,65179,65288,65376,65445,65495,65524,65534,65524,65495,65445,65376,65288,65179,65052,64904,64738,64552,64347,64123,63880,63619,63338,63040,62723,62388,62035,61665,61277,60872,60450,60012,59557,59086,58599,58096,57578,57046,56498,55937,55361,54772,54170,53554,52926,52286,51635,50971,50297,49613,48918,48213,47499,46777,46046,45306,44560,43806,43045,42279,41506,40729,39946,39160,38369,37575,36778,35979,35177,34375,33571,32767,31963,31159,30357,29555,28756,27959,27165,26374,25588,24805,24028,23255,22489,21728,20974,20228,19488,18757,18035,17321,16616,15921,15237,14563,13899,13248,12608,11980,11364,10762,10173,9597,9036,8488,7956,7438,6935,6448,5977,5522,5084,4662,4257,3869,3499,3146,2811,2494,2196,1915,1654,1411,1187,982,796,630,482,355,246,158,89,39,10,0,10,39,89,158,246,355,482,630,796,982,1187,1411,1654,1915,2196,2494,2811,3146,3499,3869,4257,4662,5084,5522,5977,6448,6935,7438,7956,8488,9036,9597,10173,10762,11364,11980,12608,13248,13899,14563,15237,15921,16616,17321,18035,18757,19488,20228,20974,21728,22489,23255,24028,24805,25588,26374,27165,27959,28756,29555,30357,31159,31963};

    audio_channel channels[CHANNEL_COUNT];

    uint16_t get_audio_frame() {
      int32_t sample = 0;  // used to combine channel output

      for(auto &channel : channels) {
        bool overflow = false;

        // gate bit has changed, reset note timer
        if((channel.flags & 0b1) != (channel.gate & 0b1)) {
          channel.frames = 0;
          if(channel.gate) {
            channel.phase = adsr_phase::ATTACK;
          } else {
            channel.phase = adsr_phase::RELEASE;
          }
        }

        // always run the voice position counter,
        // so that waveforms are synchronised even if voices
        // are turned on/off briefly

        // increment the voice position counter. this provides an 
        // Q8 fixed point value representing how far through 
        // the current voice pattern we are
        channel.voice_offset += ((channel.frequency * 256) << 8) / sample_rate;

        if(channel.voice_offset & 0xffff0000) {
          overflow = true;
          channel.voice_offset &= 0xffff;
        }

        // check if any voices are active for this channel
        if(channel.voices) {
          uint32_t channel_sample = 0xffff;
          
          if(channel.voices & audio_voice::NOISE) {
              // if the voice offset overflows then generate a new random
              // noise sample
            if(overflow) {
              channel.noise = blit::random() & 0xffff;
            }

            channel_sample &= channel.noise;
          }

          if(channel.voices & audio_voice::SAW) {
            channel_sample &= channel.voice_offset;
          }

          if(channel.voices & audio_voice::TRIANGLE) {          
            channel_sample &= channel.voice_offset < 0x7fff ? channel.voice_offset: 0xffff - channel.voice_offset;
          }

          if(channel.voices & audio_voice::SQUARE) {
            channel_sample &= (channel.voice_offset < channel.pulse_width) ? 0xffff : 0;
          }

          if(channel.voices & audio_voice::SINE) {
            // the sine_voice sample contains 256 samples in total
            // so we'll just use the top 8 most significant bits of
            // the voice position to index into it
            channel_sample &= sine_voice[(channel.voice_offset >> 8) & 0xff];
          }
          uint32_t sustain_level = (channel.volume * channel.sustain) / 0xffff;

          uint32_t attack_frames = (sample_rate * channel.attack_ms) / 1000;
          uint32_t decay_frames = (sample_rate * channel.decay_ms) / 1000;
          uint32_t release_frames = (sample_rate * channel.release_ms) / 1000;

          uint32_t attack_per_frame = 0;
          uint32_t decay_per_frame = 0;
          uint32_t release_per_frame = 0;

          switch(channel.phase) {
            case adsr_phase::ATTACK:
              attack_per_frame = (channel.volume << precision) / attack_frames;
              channel.adsr += attack_per_frame;

              if(channel.adsr > (channel.volume << precision)){
                channel.adsr = channel.volume << precision;
              }
              if(channel.frames > attack_frames){
                channel.phase = adsr_phase::DECAY;
              }
              break;
            case adsr_phase::DECAY:
              decay_per_frame = ((channel.volume - sustain_level) << precision) / decay_frames;
              channel.adsr -= decay_per_frame;

              if(channel.adsr < (sustain_level << precision)){
                channel.adsr = sustain_level << precision;
              }
              if(channel.frames > (attack_frames + decay_frames)){
                channel.phase = adsr_phase::SUSTAIN;
              }
              break;
            case adsr_phase::SUSTAIN:
              channel.adsr = sustain_level << precision;
              break;
            case adsr_phase::RELEASE:
              release_per_frame = (sustain_level << precision) / release_frames;
              if(channel.adsr > release_per_frame){
                channel.adsr -= release_per_frame;
              }
              else {
                channel.adsr = 0;
              }
              break;
          }

          // Why does shifting down by our original precision distort
          // the merry heck out of audio?
          channel_sample *= (channel.adsr >> (precision + 1));
          channel_sample >>= 16;

          int64_t channel_sample_int = channel_sample - 0x7fff;

          // combine channel sample into the final sample
          sample += channel_sample_int;
        }

        channel.frames++;

        // copy the current gate value into the status flags
        channel.flags &= ~0b0000001;
        channel.flags |= channel.gate & 0b0000001;
      }

      // apply global volume
      sample = (sample * volume) >> 16;

      // clip result to 16-bit and convert to unsigned
      sample += 0x7fff;
      sample = sample <= 0x0000 ? 0x0000 : (sample > 0xffff ? 0xffff : sample);      

      return sample;
    }
  }
}