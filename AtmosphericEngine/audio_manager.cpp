#include "audio_manager.hpp"

AudioManager::AudioManager(void) {
    ::InitAudioDevice();
}

AudioManager::~AudioManager(void) {
    for (auto [id, music] : musics) {
        ::UnloadMusicStream(music);
    }
    for (auto [id, sound] : sounds) {
        ::UnloadSound(sound);
    }
    ::CloseAudioDevice();
}

void AudioManager::Init(Application* app) {
    Server::Init(app);
}

void AudioManager::Process(float dt) {
    for (auto [id, music] : musics) {
        ::UpdateMusicStream(music);
    }
}

MusicID AudioManager::LoadMusic(const char* filename) {
    MusicID id = nextMusicId;
    musics[id] = ::LoadMusicStream(filename);
    nextMusicId++;
    return id;
}

void AudioManager::UnloadMusic(MusicID id) {
    ::UnloadMusicStream(musics[id]);
    musics.erase(id);
}

void AudioManager::PlayMusic(MusicID id) {
    if (musics.count(id) == 0) {
        return;
    }
    ::PlayMusicStream(musics[id]);
}

void AudioManager::StopMusic(MusicID id) {
    if (musics.count(id) == 0) {
        return;
    }
    ::StopMusicStream(musics[id]);
}

void AudioManager::SmoothStopMusic(MusicID id) {
    if (musics.count(id) == 0) {
        return;
    }
    // TODO: implement smooth stop
}

void AudioManager::StopAllMusics() {
    for (auto [id, music] : musics) {
        ::StopMusicStream(music);
    }
}

void AudioManager::SetMusicVolume(MusicID id, float volume) {
    if (musics.count(id) == 0) {
        return;
    }
    ::SetMusicVolume(musics[id], volume);
}

bool AudioManager::IsMusicPlaying(MusicID id) {
    if (musics.count(id) == 0) {
        return false;
    }
    return ::IsMusicStreamPlaying(musics[id]);
}

SoundID AudioManager::LoadSound(const char* filename) {
    SoundID id = nextSoundId;
    sounds[id] = ::LoadSound(filename);
    nextSoundId++;
    return id;
}

void AudioManager::UnloadSound(SoundID id) {
    ::UnloadSound(sounds[id]);
    sounds.erase(id);
}


void AudioManager::PlaySound(SoundID id) {
    if (sounds.count(id) == 0) {
        return;
    }
    ::PlaySound(sounds[id]);
}

void AudioManager::PlaySoundVariation(SoundID id, float pitchVariation, float volumeVariation) {
    if (sounds.count(id) == 0) {
        return;
    }
    // TODO: implement sound variation
    ::PlaySound(sounds[id]);
}

void AudioManager::StopSound(SoundID id) {
    if (sounds.count(id) == 0) {
        return;
    }
    ::StopSound(sounds[id]);
}

void AudioManager::StopAllSounds() {
    for (auto [id, sound] : sounds) {
        ::StopSound(sound);
    }
}

void AudioManager::SetSoundVolume(SoundID id, float volume) {
    if (sounds.count(id) == 0) {
        return;
    }
    ::SetSoundVolume(sounds[id], volume);
}

bool AudioManager::IsSoundPlaying(SoundID id) {
    if (sounds.count(id) == 0) {
        return false;
    }
    return ::IsSoundPlaying(sounds[id]);
}

void AudioManager::StopAll() {
    StopAllSounds();
    StopAllMusics();
}