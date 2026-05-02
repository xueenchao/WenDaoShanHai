/**
 * AudioFactory.cpp - 音频工厂类实现
 */

#include "AudioFactory.h"
#include "FileAudio.h"
#include "MemoryAudio.h"
#include "Log.h"

// ==================== 工厂方法 ====================

std::unique_ptr<BaseAudio> AudioFactory::createFromFile(void* mixer,
                                                         const std::string& name,
                                                         const std::string& filePath,
                                                         bool predecode)
{
    auto audio = std::make_unique<FileAudio>();
    if (audio->loadFromFile(mixer, name, filePath, predecode)) {
        return audio;
    }
    LOG_ERROR("AudioFactory::createFromFile 失败: %s", name.c_str());
    return nullptr;
}

std::unique_ptr<BaseAudio> AudioFactory::createFromMemory(void* mixer,
                                                          const std::string& name,
                                                          const void* data,
                                                          size_t dataSize,
                                                          bool predecode)
{
    auto audio = std::make_unique<MemoryAudio>();
    if (audio->loadFromMemory(mixer, name, data, dataSize, predecode)) {
        return audio;
    }
    LOG_ERROR("AudioFactory::createFromMemory 失败: %s", name.c_str());
    return nullptr;
}

// ==================== 类型安全的工厂方法 ====================

std::unique_ptr<FileAudio> AudioFactory::createFileAudio(void* mixer,
                                                         const std::string& name,
                                                         const std::string& filePath,
                                                         bool predecode)
{
    auto audio = std::make_unique<FileAudio>();
    if (audio->loadFromFile(mixer, name, filePath, predecode)) {
        return audio;
    }
    LOG_ERROR("AudioFactory::createFileAudio 失败: %s", name.c_str());
    return nullptr;
}

std::unique_ptr<MemoryAudio> AudioFactory::createMemoryAudio(void* mixer,
                                                             const std::string& name,
                                                             const void* data,
                                                             size_t dataSize,
                                                             bool predecode)
{
    auto audio = std::make_unique<MemoryAudio>();
    if (audio->loadFromMemory(mixer, name, data, dataSize, predecode)) {
        return audio;
    }
    LOG_ERROR("AudioFactory::createMemoryAudio 失败: %s", name.c_str());
    return nullptr;
}
