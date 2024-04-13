#include "beeper.h"
#include <string>
#include <stdexcept>
#include <cmath>

int Beeper::pos = 0;

Beeper::Beeper(int frequency, int duration) : mFrequency(frequency), mDuration(duration) {
    SDL_AudioSpec desired;
    desired.freq = 44100;
    desired.format = AUDIO_F32;
    desired.samples = 512;
    desired.channels = 1;
    desired.callback = Beeper::audioCallback;
    desired.userdata = this;

    mAudioDeviceId = SDL_OpenAudioDevice(nullptr, 0, &desired, &mObtained, 0);
    if (mAudioDeviceId == 0) {
        throw std::runtime_error(std::string("could not open audio device! SDL Error: ") + SDL_GetError());
    }
}

Beeper::~Beeper() {
    SDL_CloseAudioDevice(mAudioDeviceId);
}

void Beeper::play() const {
    SDL_PauseAudioDevice(mAudioDeviceId, 0);
    SDL_Delay(mDuration);
    SDL_PauseAudioDevice(mAudioDeviceId, 1);
}

void Beeper::audioCallback(void *userdata, Uint8 *stream, int len) {
    auto *beeper = static_cast<Beeper *>(userdata);
    auto *audioStream = (float *) stream;

    float period = static_cast<float>(beeper->mObtained.freq) / static_cast<float>(beeper->mFrequency);

    for (int i = 0; i < beeper->mObtained.samples; ++i) {
        audioStream[i] = sinf(2.0f * (float) M_PI * (1.0f / period) * (float) pos);
        ++pos;
        if (pos >= static_cast<int>(period)) {
            pos = 0;
        }
    }
}
