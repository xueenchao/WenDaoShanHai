/**
 * MemoryAudio.h - 从内存加载的音频类
 *
 * 工厂模式的具体产品类，负责从内存加载音频数据。
 * 适用于从自定义资源包或网络数据中加载音频。
 */

#ifndef MEMORYAUDIO_H
#define MEMORYAUDIO_H

#include "BaseAudio.h"
#include <string>

struct MIX_Audio;

class MemoryAudio : public BaseAudio {
public:
    // ==================== 构造与析构 ====================
    MemoryAudio();
    ~MemoryAudio() override;

    // 禁用拷贝
    MemoryAudio(const MemoryAudio&) = delete;
    MemoryAudio& operator=(const MemoryAudio&) = delete;

    // 支持移动语义
    MemoryAudio(MemoryAudio&& other) noexcept;
    MemoryAudio& operator=(MemoryAudio&& other) noexcept;

    // ==================== 内存加载 ====================

    /**
     * 从内存加载音频
     * @param mixer SDL3_mixer 混音器指针
     * @param name 音频名称
     * @param data 音频数据的内存指针
     * @param dataSize 数据大小（字节）
     * @param predecode 是否预解码
     * @return 加载成功返回 true
     */
    bool loadFromMemory(void* mixer, const std::string& name,
                        const void* data, size_t dataSize, bool predecode = true);

    // ==================== 实现基类接口 ====================
    MIX_Audio* getRawAudio() const override;
    const std::string& getName() const override;
    bool isValid() const override;
    void setPredecode(bool predecode) override;
    bool isPredecode() const override;
    void destroy() override;

private:
    MIX_Audio* mAudio;
    std::string mName;
    bool mPredecode;
};

#endif // MEMORYAUDIO_H
