/**
 * Audio.cpp - 音频管理类的实现（使用工厂模式）
 *
 * 封装 SDL3_mixer 的三层模型（Mixer → Audio → Track）。
 */

#include "Audio.h"
#include "Log.h"
#include "BaseAudio.h"
#include "AudioFactory.h"

// ==================== 构造与析构 ====================

Audio::Audio(int frequency, int channels)
    : mMixer(nullptr)
    , mFrequency(frequency)
    , mChannels(channels)
{
}

Audio::~Audio()
{
    // 析构时自动关闭音频系统
    shutdown();
}

// ==================== 音频系统生命周期 ====================

bool Audio::init()
{
    if (mMixer != nullptr) {
        // 已经初始化过了，直接返回
        return true;
    }

    // 初始化 SDL3_mixer 库
    if (!MIX_Init()) {
        LOG_ERROR("init MIX_Init 失败: %s", SDL_GetError());
        return false;
    }

    // 构建音频规格
    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_F32;    // 使用 32位浮点格式（现代音频推荐）
    spec.channels = mChannels;      // 声道数
    spec.freq = mFrequency;         // 采样率

    // 创建混音器
    mMixer = MIX_CreateMixer(&spec);

    if (mMixer == nullptr) {
        LOG_ERROR("init 创建Mixer失败: %s", SDL_GetError());
        return false;
    }

    return true;
}

void Audio::shutdown()
{
    if (mMixer == nullptr) {
        return;
    }

    // 先停止所有播放
    MIX_StopAllTracks(mMixer, 0);

    // 释放所有已加载的音频数据
    unloadAllAudio();

    // 销毁混音器
    MIX_DestroyMixer(mMixer);
    mMixer = nullptr;

    // 关闭 SDL3_mixer 库
    MIX_Quit();
}

bool Audio::isInitialized() const
{
    return mMixer != nullptr;
}

// ==================== 音频文件加载与卸载 ====================

bool Audio::loadAudio(const std::string& name, const std::string& filePath, bool predecode)
{
    if (mMixer == nullptr) {
        LOG_ERROR("loadAudio 失败: 音频系统尚未初始化");
        return false;
    }

    // 如果同名音频已存在，先释放旧的
    if (findAudio(name) != nullptr) {
        unloadAudio(name);
    }

    // 使用工厂从文件加载音频
    auto audio = AudioFactory::createFromFile(mMixer, name, filePath, predecode);
    if (!audio) {
        LOG_ERROR("loadAudio 失败 (%s): %s", filePath.c_str(), SDL_GetError());
        return false;
    }

    // 存入音频表
    mAudioMap[name] = std::move(audio);
    return true;
}

bool Audio::loadAudioFromMemory(const std::string& name, const void* data, size_t dataSize,
                                bool predecode)
{
    if (mMixer == nullptr) {
        LOG_ERROR("loadAudioFromMemory 失败: 音频系统尚未初始化");
        return false;
    }

    if (data == nullptr || dataSize == 0) {
        LOG_ERROR("loadAudioFromMemory 失败: 数据为空");
        return false;
    }

    if (findAudio(name) != nullptr) {
        unloadAudio(name);
    }

    // 使用工厂从内存加载音频
    auto audio = AudioFactory::createFromMemory(mMixer, name, data, dataSize, predecode);
    if (!audio) {
        LOG_ERROR("loadAudioFromMemory 加载失败: %s", SDL_GetError());
        return false;
    }

    mAudioMap[name] = std::move(audio);
    return true;
}

void Audio::unloadAudio(const std::string& name)
{
    auto it = mAudioMap.find(name);
    if (it != mAudioMap.end()) {
        // 音频数据会被智能指针自动销毁
        mAudioMap.erase(it);
    }
}

void Audio::unloadAllAudio()
{
    mAudioMap.clear();
}

// ==================== 获取音频对象 ====================

BaseAudio* Audio::getAudio(const std::string& name) const
{
    return findAudio(name);
}

MIX_Audio* Audio::getRawAudio(const std::string& name) const
{
    BaseAudio* audio = findAudio(name);
    if (audio) {
        return audio->getRawAudio();
    }
    return nullptr;
}

// ==================== 音频播放控制 ====================

bool Audio::play(const std::string& name, int loops)
{
    if (mMixer == nullptr) {
        return false;
    }

    BaseAudio* audio = findAudio(name);
    if (audio == nullptr) {
        LOG_ERROR("play 失败: 未找到音频 '%s'", name.c_str());
        return false;
    }

    MIX_Track* track = MIX_CreateTrack(mMixer);
    if (track == nullptr) {
        LOG_ERROR("play 创建Track失败: %s", SDL_GetError());
        return false;
    }

    if (!MIX_SetTrackAudio(track, audio->getRawAudio())) {
        LOG_ERROR("play 绑定Audio到Track失败: %s", SDL_GetError());
        MIX_DestroyTrack(track);
        return false;
    }

    if (loops != 0) {
        MIX_SetTrackLoops(track, loops);
    }

    if (!MIX_PlayTrack(track, 0)) {
        LOG_ERROR("play 播放失败: %s", SDL_GetError());
        MIX_DestroyTrack(track);
        return false;
    }

    return true;
}

bool Audio::playWithTag(const std::string& name, const std::string& tag, int loops)
{
    if (mMixer == nullptr) {
        return false;
    }

    BaseAudio* audio = findAudio(name);
    if (audio == nullptr) {
        LOG_ERROR("playWithTag 失败: 未找到音频 '%s'", name.c_str());
        return false;
    }

    MIX_Track* track = MIX_CreateTrack(mMixer);
    if (track == nullptr) {
        return false;
    }

    if (!MIX_SetTrackAudio(track, audio->getRawAudio())) {
        MIX_DestroyTrack(track);
        return false;
    }

    MIX_TagTrack(track, tag.c_str());

    if (loops != 0) {
        MIX_SetTrackLoops(track, loops);
    }

    if (!MIX_PlayTrack(track, 0)) {
        MIX_DestroyTrack(track);
        return false;
    }

    return true;
}

void Audio::stopAll(int fadeOutMs)
{
    if (mMixer == nullptr) {
        return;
    }

    if (fadeOutMs > 0) {
        MIX_StopAllTracks(mMixer, fadeOutMs);
    } else {
        MIX_StopAllTracks(mMixer, 0);
    }
}

void Audio::stopByTag(const std::string& tag, int fadeOutMs)
{
    if (mMixer == nullptr) {
        return;
    }

    if (fadeOutMs > 0) {
        MIX_StopTag(mMixer, tag.c_str(), fadeOutMs);
    } else {
        MIX_StopTag(mMixer, tag.c_str(), 0);
    }
}

void Audio::pauseAll()
{
    if (mMixer == nullptr) {
        return;
    }
    MIX_PauseAllTracks(mMixer);
}

void Audio::pauseByTag(const std::string& tag)
{
    if (mMixer == nullptr) {
        return;
    }
    MIX_PauseTag(mMixer, tag.c_str());
}

void Audio::resumeAll()
{
    if (mMixer == nullptr) {
        return;
    }
    MIX_ResumeAllTracks(mMixer);
}

void Audio::resumeByTag(const std::string& tag)
{
    if (mMixer == nullptr) {
        return;
    }
    MIX_ResumeTag(mMixer, tag.c_str());
}

// ==================== 音量控制 ====================

void Audio::setMasterVolume(float gain)
{
    if (mMixer == nullptr) {
        return;
    }
    MIX_SetMixerGain(mMixer, gain);
}

float Audio::getMasterVolume() const
{
    if (mMixer == nullptr) {
        return 0.0f;
    }
    return MIX_GetMixerGain(mMixer);
}

void Audio::setTagVolume(const std::string& tag, float gain)
{
    if (mMixer == nullptr) {
        return;
    }
    MIX_SetTagGain(mMixer, tag.c_str(), gain);
}

// ==================== 内部方法 ====================

BaseAudio* Audio::findAudio(const std::string& name) const
{
    auto it = mAudioMap.find(name);
    if (it != mAudioMap.end()) {
        return it->second.get();
    }
    return nullptr;
}
