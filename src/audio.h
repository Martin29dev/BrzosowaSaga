// audio.h — proceduralnie syntezowane efekty dźwiękowe
// Brzozowa Saga (RPG Maker MV-like, raylib + C++17)
#pragma once
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "common.h"

namespace rpg {

// ================================================================ AUDIO ====
enum SeId { SE_CURSOR, SE_OK, SE_CANCEL, SE_BUZZER, SE_BLIP, SE_SWITCH, SE_PLATE, SE_DOOR,
            SE_CHEST, SE_ITEM, SE_RUMBLE, SE_PUSH, SE_CRYSTAL, SE_SAVE, SE_COUNT };

class AudioBank {
public:
    void init();
    void play(int id, float pitch = 1.0f, float vol = 1.0f);
    void shutdown();
private:
    static Sound synth(float dur, std::function<float(float)> fn);
    Sound snd_[SE_COUNT] = {};
    bool ready_ = false;
};
extern AudioBank g_audio;

}  // namespace rpg
