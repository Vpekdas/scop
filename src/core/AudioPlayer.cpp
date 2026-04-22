#include "AudioPlayer.hpp"
#include "SDL3/SDL_log.h"

AudioPlayer::AudioPlayer(const std::filesystem::path& path)
{
    if (!SDL_LoadWAV(path.string().c_str(), &_spec, &_wavData, &_wavDataLen))
    {
        SDL_Log("Couldn't load .wav file: %s", SDL_GetError());
        return;
    }

    _stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &_spec, nullptr, nullptr);
    if (!_stream)
    {
        SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
        SDL_free(_wavData);
        _wavData = nullptr;
    }
}

AudioPlayer::~AudioPlayer()
{
    SDL_DestroyAudioStream(_stream);
    SDL_free(_wavData);
}

void AudioPlayer::Play()
{

    if (!_isPlaying)
    {
        if (SDL_GetAudioStreamQueued(_stream) == 0)
        {
            if (!SDL_PutAudioStreamData(_stream, _wavData, _wavDataLen))
            {
                SDL_Log("Failed to put audio data: %s", SDL_GetError());
                return;
            }
        }

        if (!SDL_ResumeAudioStreamDevice(_stream))
        {
            SDL_Log("Failed to start audio: %s", SDL_GetError());
            return;
        }

        _isPlaying = true;
    }
}
void AudioPlayer::Stop()
{

    if (!SDL_PauseAudioStreamDevice(_stream))
    {
        SDL_Log("Failed to start audio: %s", SDL_GetError());
        return;
    }

    _isPlaying = false;
}

double AudioPlayer::GetPlaybackTime() const
{

    int queued = SDL_GetAudioStreamQueued(_stream);

    int bytesPerSample = SDL_AUDIO_BITSIZE(_spec.format) / 8;
    int bytesPerFrame = bytesPerSample * _spec.channels;

    int totalFrames = _wavDataLen / bytesPerFrame;
    int queuedFrames = queued / bytesPerFrame;

    int playedFrames = totalFrames - queuedFrames;

    return (double)playedFrames / _spec.freq;
}