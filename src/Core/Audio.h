/**
 * Audio.h - 音频管理类
 *
 * 封装 SDL3_mixer 的音频加载、播放与管理功能。
 * SDL3_mixer 使用 Mixer（混音器）→ Audio（音频数据）→ Track（播放轨道）的三层模型：
 *
 *   - Mixer：混音器，负责最终的音频输出，相当于"音频引擎"
 *   - Audio：音频数据（从文件加载的音效/音乐数据），相当于"音频文件"
 *   - Track：播放轨道，每播放一次音频就创建一个 Track，支持暂停/恢复/停止
 *
 * 可以用 Tag（标签）给轨道分组，方便批量控制（如"bgm"标签控制所有背景音乐）。
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <string>
#include <unordered_map>
#include <memory>

class BaseAudio;

class Audio {
public:
    // ==================== 构造与析构 ====================

    /**
     * 构造函数：初始化音频系统参数
     * @param frequency  采样率（Hz），常用值：44100（CD音质）、48000（DVD音质）
     * @param channels   声道数：1=单声道，2=立体声
     */
    Audio(int frequency = 44100, int channels = 2);

    /**
     * 析构函数：自动关闭音频系统并释放所有资源
     */
    ~Audio();

    // 禁用拷贝
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    // ==================== 音频系统生命周期 ====================

    /**
     * 初始化音频系统
     * 会创建混音器并打开音频设备
     * @return 成功返回 true，失败返回 false
     */
    bool init();

    /**
     * 关闭音频系统
     * 释放所有已加载的音频数据和混音器
     */
    void shutdown();

    /**
     * 判断音频系统是否已初始化
     * @return 已初始化返回 true
     */
    bool isInitialized() const;

    // ==================== 音频文件加载与卸载 ====================

    /**
     * 从文件加载音频数据，并指定一个名称用于后续引用（使用工厂模式）
     * 支持的格式取决于 SDL3_mixer 配置，通常包括 WAV、OGG、MP3、FLAC 等
     * @param name     音频名称（用于后续播放时引用）
     * @param filePath 音频文件路径
     * @param predecode 是否预解码到内存（true=加载时全部解码，播放无延迟，但占用更多内存）
     *                  短音效建议 true，长音乐建议 false（流式播放，节省内存）
     * @return 加载成功返回 true，失败返回 false
     */
    bool loadAudio(const std::string& name, const std::string& filePath, bool predecode = true);

    /**
     * 从内存数据加载音频（使用工厂模式）
     * @param name     音频名称
     * @param data     音频数据的内存指针
     * @param dataSize 数据大小（字节）
     * @param predecode 是否预解码
     * @return 加载成功返回 true，失败返回 false
     */
    bool loadAudioFromMemory(const std::string& name, const void* data, size_t dataSize,
                             bool predecode = true);

    /**
     * 卸载指定名称的音频数据
     * @param name 音频名称
     */
    void unloadAudio(const std::string& name);

    /**
     * 卸载所有已加载的音频数据
     */
    void unloadAllAudio();

    // ==================== 获取音频对象 ====================

    /**
     * 获取音频对象指针（工厂模式版本）
     * @param name 音频名称
     * @return BaseAudio 指针，未找到返回 nullptr
     */
    BaseAudio* getAudio(const std::string& name) const;

    /**
     * 获取音频对象指针（向后兼容版本）
     * @param name 音频名称
     * @return MIX_Audio 指针，未找到返回 nullptr
     */
    MIX_Audio* getRawAudio(const std::string& name) const;

    // ==================== 音频播放控制 ====================

    /**
     * 播放指定名称的音频（快捷方法）
     * 内部会创建一个新的 Track 并播放
     * @param name  音频名称（通过 loadAudio 加载时指定的名称）
     * @param loops 循环次数：0=播放一次，1=循环一次（共播放两次），-1=无限循环
     * @return 播放成功返回 true，失败返回 false
     */
    bool play(const std::string& name, int loops = 0);

    /**
     * 播放指定名称的音频，并设置标签（用于分组控制）
     * 例如：给背景音乐打上 "bgm" 标签，音效打上 "sfx" 标签
     * @param name  音频名称
     * @param tag   轨道标签（用于分组控制）
     * @param loops 循环次数
     * @return 播放成功返回 true，失败返回 false
     */
    bool playWithTag(const std::string& name, const std::string& tag, int loops = 0);

    /**
     * 停止所有正在播放的音频轨道
     * @param fadeOutMs 淡出时间（毫秒），0 表示立即停止
     */
    void stopAll(int fadeOutMs = 0);

    /**
     * 停止指定标签的所有轨道
     * @param tag       标签名称
     * @param fadeOutMs 淡出时间（毫秒）
     */
    void stopByTag(const std::string& tag, int fadeOutMs = 0);

    /**
     * 暂停所有正在播放的音频轨道
     */
    void pauseAll();

    /**
     * 暂停指定标签的所有轨道
     * @param tag 标签名称
     */
    void pauseByTag(const std::string& tag);

    /**
     * 恢复所有暂停的音频轨道
     */
    void resumeAll();

    /**
     * 恢复指定标签的所有轨道
     * @param tag 标签名称
     */
    void resumeByTag(const std::string& tag);

    // ==================== 音量控制 ====================

    /**
     * 设置混音器总音量（主音量）
     * @param gain 音量增益 (0.0=静音, 1.0=正常音量, 可以超过1.0放大)
     */
    void setMasterVolume(float gain);

    /**
     * 获取混音器总音量
     * @return 当前音量增益
     */
    float getMasterVolume() const;

    /**
     * 设置指定标签的音量
     * @param tag  标签名称
     * @param gain 音量增益
     */
    void setTagVolume(const std::string& tag, float gain);

private:
    /**
     * 内部方法：查找已加载的音频数据
     * @param name 音频名称
     * @return BaseAudio 指针，未找到返回 nullptr
     */
    BaseAudio* findAudio(const std::string& name) const;

    MIX_Mixer* mMixer;    // SDL3_mixer 混音器指针

    // 音频数据存储表：名称 → 智能指针
    std::unordered_map<std::string, std::unique_ptr<BaseAudio>> mAudioMap;

    int mFrequency;       // 采样率
    int mChannels;        // 声道数
};

#endif // AUDIO_H
