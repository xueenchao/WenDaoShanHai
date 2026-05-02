/**
 * AudioFactory.h - 音频工厂类
 *
 * 简单工厂模式实现，负责创建各种类型的音频对象。
 * 封装音频的创建过程，提高代码的可扩展性和可维护性。
 *
 * 支持创建的音频类型：
 *   - FileAudio：从文件加载音频
 *   - MemoryAudio：从内存加载音频
 */

#ifndef AUDIOFACTORY_H
#define AUDIOFACTORY_H

#include <memory>
#include <string>

// 前向声明
struct MIX_Audio;
class BaseAudio;
class FileAudio;
class MemoryAudio;

class AudioFactory {
public:
    // ==================== 工厂方法 ====================

    /**
     * 从文件加载音频
     * @param mixer SDL3_mixer 混音器指针
     * @param name 音频名称
     * @param filePath 文件路径
     * @param predecode 是否预解码
     * @return 音频对象的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<BaseAudio> createFromFile(void* mixer,
                                                    const std::string& name,
                                                    const std::string& filePath,
                                                    bool predecode = true);

    /**
     * 从内存加载音频
     * @param mixer SDL3_mixer 混音器指针
     * @param name 音频名称
     * @param data 音频数据的内存指针
     * @param dataSize 数据大小（字节）
     * @param predecode 是否预解码
     * @return 音频对象的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<BaseAudio> createFromMemory(void* mixer,
                                                      const std::string& name,
                                                      const void* data,
                                                      size_t dataSize,
                                                      bool predecode = true);

    // ==================== 类型安全的工厂方法 ====================

    /**
     * 创建 FileAudio 对象并从文件加载
     * @param mixer SDL3_mixer 混音器指针
     * @param name 音频名称
     * @param filePath 文件路径
     * @param predecode 是否预解码
     * @return FileAudio 的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<FileAudio> createFileAudio(void* mixer,
                                                    const std::string& name,
                                                    const std::string& filePath,
                                                    bool predecode = true);

    /**
     * 创建 MemoryAudio 对象并从内存加载
     * @param mixer SDL3_mixer 混音器指针
     * @param name 音频名称
     * @param data 音频数据的内存指针
     * @param dataSize 数据大小（字节）
     * @param predecode 是否预解码
     * @return MemoryAudio 的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<MemoryAudio> createMemoryAudio(void* mixer,
                                                        const std::string& name,
                                                        const void* data,
                                                        size_t dataSize,
                                                        bool predecode = true);

private:
    // 私有构造函数，防止创建工厂实例
    AudioFactory() = delete;
    ~AudioFactory() = delete;
    AudioFactory(const AudioFactory&) = delete;
    AudioFactory& operator=(const AudioFactory&) = delete;
};

#endif // AUDIOFACTORY_H
