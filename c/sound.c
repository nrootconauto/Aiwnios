#include <SDL.h>

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
static int8_t *WriteSample(int8_t *out,int8_t v) {
	int64_t big=v;
	int8_t *s8=NULL;
	int16_t *s16=NULL;
	int32_t *s32=NULL;
	int64_t *s64=NULL;
	union {
		int32_t i;
		float f; //iee754 single Precision if you are sane
	} f32u;
	union {
		int64_t i;
		double f; //iee754 single Precision if you are sane
	} f64u;
	if(SDL_AUDIO_ISFLOAT(have.format)) {
		if(SDL_AUDIO_BITSIZE(have.format)==32) {
			f32u.f=v*vol/127;
			big=f32u.i;
		} else if(SDL_AUDIO_BITSIZE(have.format)==64) {			
			f64u.f=v*vol/127;
			big=f64u.i;
		}
	} else {
    if(!SDL_AUDIO_ISSIGNED(have.format)) {
		if(v<=0) big=0;
		else big=0xFFffFFffFFffFFffull*vol;
	} else {
		//Bigest positive for type
		big=(1ull<<(SDL_AUDIO_BITSIZE(have.format)-1))-1;
		if(v>0) big*=vol;
		else if(v==0) big=0;
		else if(v<0) big*=-vol;
	}
	}
	switch(SDL_AUDIO_BITSIZE(have.format)) {
			case 8:
			s8=out;
			out++;
			break;
			case 16:
			s16=out;
			out+=2;
			break;
			case 32:
			s32=out;
			out+=4;
			break;
			case 64:
			s32=out;
			out+=8;
			break;
	}
	if(SDL_AUDIO_ISBIGENDIAN(have.format)) {
		if(s8) *s8=big;
		if(s16) *s16=__builtin_bswap16(big);
		if(s32) *s32=__builtin_bswap32(big);
		if(s64) *s64=__builtin_bswap64(big);
	} else {
		if(s8) *s8=big;
		if(s16) *s16=big;
		if(s32) *s32=big;
		if(s64) *s64=big;
	}
	return out;
}
static void AudioCB(void *ul, int8_t *out, int64_t len) {
  unsigned int i, i2;
  int64_t fpb = len / have.channels/(SDL_AUDIO_BITSIZE(have.format)/8);
  for (i = 0; i < fpb; i++) {
    double t = (double)++sample / have.freq;
    double amp = sin(2.0 * M_PI *t * freq);
    int64_t maxed = (amp > 0) ? 127 : -127;
    if (!freq)
      maxed = 0;
    for (i2 = 0; i2 != have.channels; i2++)
      out=WriteSample(out,maxed);
  }
}
void SndFreq(int64_t f) {
  freq = f;
  if(!f) {
    SDL_PauseAudioDevice(output,1);
  } else
    SDL_PauseAudioDevice(output,0);
}
void InitSound() {
  SDL_AudioSpec want;
  if(SDL_Init(SDL_INIT_AUDIO))
	return;
  memset(&want, 0, sizeof(SDL_AudioSpec));
  want.freq = 24000;
  want.format = AUDIO_F32;
  want.channels = 2;
  want.samples = 256;
  want.callback = (void *)&AudioCB;
  output = SDL_OpenAudioDevice(NULL, 0, &want, &have,
                               SDL_AUDIO_ALLOW_ANY_CHANGE);
  SDL_PauseAudioDevice(output, 0);
}

void DeinitSound() {
	SDL_PauseAudioDevice(output,1);
	SDL_CloseAudioDevice(output);
}
