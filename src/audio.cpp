// audio.cpp — synteza i odtwarzanie SE
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "audio.h"

namespace rpg {

AudioBank g_audio;

void AudioBank::init() {
    InitAudioDevice();
    ready_ = IsAudioDeviceReady();
    if (!ready_) { TraceLog(LOG_WARNING, "AUDIO: brak urzadzenia, gra bez dzwieku"); return; }
    auto sq  = [](float t, float f) { return std::fmod(t * f, 1.0f) < 0.5f ? 1.0f : -1.0f; };
    auto trw = [](float t, float f) { float p = std::fmod(t * f, 1.0f); return 4.0f * std::fabs(p - 0.5f) - 1.0f; };
    auto saw = [](float t, float f) { return 2.0f * std::fmod(t * f, 1.0f) - 1.0f; };
    auto sn  = [](float t, float f) { return std::sin(2.0f * PI * f * t); };
    auto dec = [](float t, float d, float k) { float x = 1.0f - t / d; return x <= 0 ? 0.0f : std::pow(x, k); };

    snd_[SE_CURSOR] = synth(0.045f, [=](float t) { return sq(t, 1320.f) * dec(t, 0.045f, 1.f) * 0.16f; });
    snd_[SE_OK]     = synth(0.13f, [=](float t) { return sq(t, t < 0.05f ? 880.f : 1320.f) * dec(t, 0.13f, 0.7f) * 0.16f; });
    snd_[SE_CANCEL] = synth(0.13f, [=](float t) { return sq(t, t < 0.05f ? 660.f : 440.f) * dec(t, 0.13f, 0.7f) * 0.15f; });
    snd_[SE_BUZZER] = synth(0.22f, [=](float t) { return (saw(t, 110.f) * 0.6f + sq(t, 55.f) * 0.4f) * dec(t, 0.22f, 0.5f) * 0.22f; });
    snd_[SE_BLIP]   = synth(0.03f, [=](float t) { return sq(t, 620.f) * dec(t, 0.03f, 1.f) * 0.07f; });
    Rng n1(11), n2(22), n3(33), n4(44), n5(55);
    snd_[SE_SWITCH] = synth(0.14f, [=](float t) mutable {
        float n = n1.f() * 2.f - 1.f;
        return n * dec(t, 0.02f, 2.f) * 0.35f + sq(t, 240.f - 600.f * t) * dec(t, 0.08f, 2.f) * 0.18f; });
    snd_[SE_PLATE]  = synth(0.18f, [=](float t) mutable {
        float n = n2.f() * 2.f - 1.f;
        return sn(t, 95.f - 40.f * t) * dec(t, 0.18f, 1.5f) * 0.55f + n * dec(t, 0.03f, 2.f) * 0.2f; });
    float lp1 = 0.f;
    snd_[SE_DOOR]   = synth(0.5f, [=](float t) mutable {
        lp1 += ((n3.f() * 2.f - 1.f) - lp1) * 0.08f;
        return lp1 * dec(t, 0.5f, 1.f) * 1.4f + sn(t, 150.f - 110.f * t) * dec(t, 0.5f, 1.2f) * 0.3f; });
    snd_[SE_CHEST]  = synth(0.3f, [=](float t) {
        const float nt[3] = {523.25f, 659.25f, 783.99f};
        int i = std::min(2, (int)(t / 0.07f));
        return trw(t, nt[i]) * dec(t, 0.3f, 0.6f) * 0.3f; });
    snd_[SE_ITEM]   = synth(0.75f, [=](float t) {
        const float nt[4] = {523.25f, 659.25f, 783.99f, 1046.5f};
        int i = std::min(3, (int)(t / 0.085f));
        return (sq(t, nt[i]) * 0.5f + trw(t, nt[i] * 0.5f) * 0.5f) * dec(t, 0.75f, 0.8f) * 0.17f; });
    float lp2 = 0.f;
    snd_[SE_RUMBLE] = synth(1.0f, [=](float t) mutable {
        lp2 += ((n4.f() * 2.f - 1.f) - lp2) * 0.03f;
        return lp2 * (0.6f + 0.4f * sn(t, 9.f)) * dec(t, 1.0f, 0.7f) * 2.6f; });
    float lp3 = 0.f;
    snd_[SE_PUSH]   = synth(0.3f, [=](float t) mutable {
        lp3 += ((n5.f() * 2.f - 1.f) - lp3) * 0.06f;
        return lp3 * dec(t, 0.3f, 0.6f) * 1.6f + sn(t, 60.f) * dec(t, 0.3f, 1.f) * 0.2f; });
    snd_[SE_CRYSTAL] = synth(0.8f, [=](float t) {
        return (sn(t, 1568.f) + sn(t, 2093.f) * 0.6f + sn(t, 3136.f) * 0.3f) * dec(t, 0.8f, 2.f) * 0.15f; });
    snd_[SE_SAVE]   = synth(0.3f, [=](float t) { return sn(t, t < 0.1f ? 784.f : 1175.f) * dec(t, 0.3f, 1.f) * 0.3f; });
}

void AudioBank::play(int id, float pitch, float vol) {
    if (!ready_ || id < 0 || id >= SE_COUNT) return;
    SetSoundPitch(snd_[id], pitch);
    SetSoundVolume(snd_[id], vol);
    PlaySound(snd_[id]);
}

void AudioBank::shutdown() {
    if (!ready_) return;
    for (int i = 0; i < SE_COUNT; ++i) UnloadSound(snd_[i]);
    CloseAudioDevice();
}

Sound AudioBank::synth(float dur, std::function<float(float)> fn) {
    const int rate = 44100;
    const int n = (int)(dur * rate);
    short* data = (short*)MemAlloc((unsigned int)(n * sizeof(short)));
    for (int i = 0; i < n; ++i) {
        float t = (float)i / rate;
        float env = std::min(1.0f, t / 0.002f) * std::min(1.0f, (dur - t) / 0.004f);
        data[i] = (short)(clampf(fn(t), -1.f, 1.f) * env * 30000.0f);
    }
    Wave w;
    w.frameCount = (unsigned int)n; w.sampleRate = rate; w.sampleSize = 16; w.channels = 1; w.data = data;
    Sound s = LoadSoundFromWave(w);
    MemFree(data);
    return s;
}

}  // namespace rpg
