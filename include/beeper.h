#ifndef C8_EMU_BEEPER_H
#define C8_EMU_BEEPER_H

#include <SDL.h>

class Beeper {
public:
    Beeper(int frequency, int duration);

    ~Beeper();

    void play() const;

private:
    int mFrequency;
    int mDuration;

    SDL_AudioDeviceID mAudioDeviceId;
    SDL_AudioSpec mObtained{};

    static int pos;

    static void audioCallback(void *userdata, Uint8 *stream, int len);
};

#endif //C8_EMU_BEEPER_H
