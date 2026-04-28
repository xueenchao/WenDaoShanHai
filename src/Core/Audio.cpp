/**
 * Audio.cpp - 音频管理类的实现
 *
 * 封装 SDL3_mixer 的三层模型（Mixer → Audio → Track）。
 */

#include "Audio.h"
#include "Log.h"

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
    // 传入 nullptr 表示自动打开默认音频设备
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

    // 使用 SDL3_mixer 从文件加载音频数据
    // predecode: true=立即全部解码到内存，false=流式解码（播放时按需读取）
    MIX_Audio* audio = MIX_LoadAudio(mMixer, filePath.c_str(), predecode);

    if (audio == nullptr) {
        LOG_ERROR("loadAudio 失败 (%s): %s", filePath.c_str(), SDL_GetError());
        return false;
    }

    // 存入音频表
    mAudioMap[name] = audio;
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

    // 从内存创建 IOStream，然后通过 IOStream 加载音频
    SDL_IOStream* io = SDL_IOFromConstMem(data, dataSize);
    if (io == nullptr) {
        LOG_ERROR("loadAudioFromMemory 创建IOStream失败: %s", SDL_GetError());
        return false;
    }

    // closeio = true 表示加载完成后自动关闭 IOStream
    MIX_Audio* audio = MIX_LoadAudio_IO(mMixer, io, predecode, true);

    if (audio == nullptr) {
        LOG_ERROR("loadAudioFromMemory 加载失败: %s", SDL_GetError());
        return false;
    }

    mAudioMap[name] = audio;
    return true;
}

void Audio::unloadAudio(const std::string& name)
{
    auto it = mAudioMap.find(name);
    if (it != mAudioMap.end()) {
        // 销毁音频数据
        MIX_DestroyAudio(it->second);
        mAudioMap.erase(it);
    }
}

void Audio::unloadAllAudio()
{
    // 逐个销毁所有音频数据
    for (auto& pair : mAudioMap) {
        MIX_DestroyAudio(pair.second);
    }
    mAudioMap.clear();
}

// ==================== 音频播放控制 ====================

bool Audio::play(const std::string& name, int loops)
{
    if (mMixer == nullptr) {
        return false;
    }

    MIX_Audio* audio = findAudio(name);
    if (audio == nullptr) {
        LOG_ERROR("play 失败: 未找到音频 '%s'", name.c_str());
        return false;
    }

    // 创建一条新的播放轨道
    MIX_Track* track = MIX_CreateTrack(mMixer);
    if (track == nullptr) {
        LOG_ERROR("play 创建Track失败: %s", SDL_GetError());
        return false;
    }

    // 将音频数据绑定到轨道
    if (!MIX_SetTrackAudio(track, audio)) {
        LOG_ERROR("play 绑定Audio到Track失败: %s", SDL_GetError());
        MIX_DestroyTrack(track);
        return false;
    }

    // 设置循环次数
    // loops = 0 表示播放一次（不循环），-1 表示无限循环
    if (loops != 0) {
        MIX_SetTrackLoops(track, loops);
    }

    // 开始播放
    // options 传 0（空属性集）表示使用默认播放选项
    if (!MIX_PlayTrack(track, 0)) {
        LOG_ERROR("play 播放失败: %s", SDL_GetError());
        MIX_DestroyTrack(track);
        return false;
    }

    // 注意：播放完成后轨道不会自动销毁
    // SDL3_mixer 会在轨道播放结束后将其标记为完成状态
    // 对于简单的使用场景，这不会造成问题
    // 如果需要精细管理，可以维护一个 Track 列表并定期清理已完成的轨道

    return true;
}

bool Audio::playWithTag(const std::string& name, const std::string& tag, int loops)
{
    if (mMixer == nullptr) {
        return false;
    }

    MIX_Audio* audio = findAudio(name);
    if (audio == nullptr) {
        LOG_ERROR("playWithTag 失败: 未找到音频 '%s'", name.c_str());
        return false;
    }

    // 创建轨道
    MIX_Track* track = MIX_CreateTrack(mMixer);
    if (track == nullptr) {
        return false;
    }

    // 绑定音频数据
    if (!MIX_SetTrackAudio(track, audio)) {
        MIX_DestroyTrack(track);
        return false;
    }

    // 给轨道打上标签
    // 标签用于分组控制，例如 "bgm" 标签可以统一控制所有背景音乐
    MIX_TagTrack(track, tag.c_str());

    // 设置循环次数
    if (loops != 0) {
        MIX_SetTrackLoops(track, loops);
    }

    // 开始播放
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
        // 带淡出效果的停止
        // fade_out_frames 参数实际上是毫秒数（根据 SDL3_mixer 文档）
        MIX_StopAllTracks(mMixer, fadeOutMs);
    } else {
        // 立即停止
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
    // gain: 0.0 = 静音, 1.0 = 正常, >1.0 = 放大
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

MIX_Audio* Audio::findAudio(const std::string& name) const
{
    auto it = mAudioMap.find(name);
    if (it != mAudioMap.end()) {
        return it->second;
    }
    return nullptr;
}
