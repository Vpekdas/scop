#pragma once

#include "SDL3/SDL_audio.h"
#include <filesystem>

class AudioPlayer
{
    public:
    AudioPlayer(const std::filesystem::path& path);
    ~AudioPlayer();

    void Play();
    void Stop();

    inline bool GetIsPlaying() const
    {
        return _isPlaying;
    }

    double GetPlaybackTime() const;

    private:
    SDL_AudioSpec _spec{};
    Uint8* _wavData = nullptr;
    Uint32 _wavDataLen = 0;
    SDL_AudioStream* _stream = nullptr;
    bool _isPlaying = false;
};