/**
 * FileAudio.h - 从文件加载的音频类
 *
 * 工厂模式的具体产品类，负责从文件系统加载音频。
 * 支持 WAV、OGG、MP3、FLAC 等音频格式。
 */

#ifndef FILEAUDIO_H
#define FILEAUDIO_H

#include "BaseAudio.h"
#include <string>

struct MIX_Audio;
class AudioSystem;

class FileAudio : public BaseAudio {
public:
    // ==================== 构造与析构 ====================
    FileAudio();
    ~FileAudio() override;

    // 禁用拷贝
    FileAudio(const FileAudio&) = delete;
    FileAudio& operator=(const FileAudio&) = delete;

    // 支持移动语义
    FileAudio(FileAudio&& other) noexcept;
    FileAudio& operator=(FileAudio&& other) noexcept;

    // ==================== 文件加载 ====================

    /**
     * 从文件加载音频
     * @param mixer SDL3_mixer 混音器指针
     * @param name 音频名称
     * @param filePath 文件路径
     * @param predecode 是否预解码到内存
     * @return 加载成功返回 true
     */
    bool loadFromFile(void* mixer, const std::string& name,
                      const std::string& filePath, bool predecode = true);

    /**
     * 获取加载的文件路径
     * @return 文件路径
     */
    const std::string& getFilePath() const;

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
    std::string mFilePath;
    bool mPredecode;
};

#endif // FILEAUDIO_H
