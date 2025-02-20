#include <SDL.h>
#include <math.h>

static SDL_AudioDeviceID output;
static int64_t sample, freq;
static SDL_AudioSpec have;
static double vol = .2;
void AiwniosSetVolume(double v) {
  if (v > 100.)
    v = 100;
  vol = v / 100;
}
double AiwniosGetVolume() {
  return vol * 100;
}
static int8_t *WriteSample(int8_t *out, int8_t v) {
  int64_t big = v;
  size_t bitsz = (size_t)SDL_AUDIO_BITSIZE(have.format) >> 3;
  union {
    int32_t i;
    float f; // iee754 single Precision if you are sane
  } f32u;
  union {
    int64_t i;
    double f; // iee754 single Precision if you are sane
  } f64u;
  if (SDL_AUDIO_ISFLOAT(have.format)) {
    if (SDL_AUDIO_BITSIZE(have.format) == 32) {
      f32u.f = v * vol / 127;
      big = f32u.i;
    } else if (SDL_AUDIO_BITSIZE(have.format) == 64) {
      f64u.f = v * vol / 127;
      big = f64u.i;
    }
  } else {
    if (SDL_AUDIO_ISSIGNED(have.format)) {
      // Bigest positive for type
      big = (1ull << (SDL_AUDIO_BITSIZE(have.format) - 1)) - 1;
      big *= !!v * copysign(vol, v);
    } else {
      big = (v > 0) * -1ull * vol;
    }
  }
  if (SDL_AUDIO_ISBIGENDIAN(have.format))
    switch (bitsz) {
    case 2: {
      uint16_t b = __builtin_bswap16(big);
      __builtin_memcpy(&big, &b, 2);
    } break;
    case 4: {
      uint32_t b = __builtin_bswap32(big);
      __builtin_memcpy(&big, &b, 4);
    } break;
    case 8: {
      uint64_t b = __builtin_bswap64(big);
      __builtin_memcpy(&big, &b, 8);
    } break;
    }
  __builtin_memcpy(out, &big, bitsz);
  out += bitsz;
  return out;
}
static void AudioCB(void *ul, int8_t *out, int64_t len) {
  unsigned int i, i2;
  int64_t fpb = len / have.channels / (SDL_AUDIO_BITSIZE(have.format) / 8);
  for (i = 0; i < fpb; i++) {
    double t = (double)++sample / have.freq;
    double amp = sin(2.0 * M_PI * t * freq);
    int64_t maxed = copysign(127, amp);
    if (!freq)
      maxed = 0;
    for (i2 = 0; i2 != have.channels; i2++)
      out = WriteSample(out, maxed);
  }
}
void SndFreq(int64_t f) {
  freq = f;
  if (!f) {
    SDL_PauseAudioDevice(output, 1);
  } else
    SDL_PauseAudioDevice(output, 0);
}
static int audio_init = 0;
void InitSound() {
  SDL_AudioSpec want;
  if (0 > SDL_Init(SDL_INIT_AUDIO))
    return;
  audio_init = 1;
  want = (SDL_AudioSpec){0};
  want.freq = 24000;
  want.format = AUDIO_F32;
  want.channels = 2;
  want.samples = 256;
  want.callback = (void *)&AudioCB;
  output =
      SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
  SDL_PauseAudioDevice(output, 0);
}

void DeinitSound() {
  if (!audio_init)
    return;
  SDL_PauseAudioDevice(output, 1);
  SDL_CloseAudioDevice(output);
}
