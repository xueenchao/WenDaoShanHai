/**
 * FileAudio.cpp - 从文件加载的音频类实现
 */

#include "FileAudio.h"
#include "Log.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

// ==================== 构造与析构 ====================

FileAudio::FileAudio()
    : mAudio(nullptr)
    , mPredecode(false)
{
}

FileAudio::~FileAudio()
{
    destroy();
}

// ==================== 移动语义 ====================

FileAudio::FileAudio(FileAudio&& other) noexcept
    : mAudio(other.mAudio)
    , mName(std::move(other.mName))
    , mFilePath(std::move(other.mFilePath))
    , mPredecode(other.mPredecode)
{
    other.mAudio = nullptr;
    other.mPredecode = false;
}

FileAudio& FileAudio::operator=(FileAudio&& other) noexcept
{
    if (this != &other) {
        destroy();

        mAudio = other.mAudio;
        mName = std::move(other.mName);
        mFilePath = std::move(other.mFilePath);
        mPredecode = other.mPredecode;

        other.mAudio = nullptr;
        other.mPredecode = false;
    }
    return *this;
}

// ==================== 文件加载 ====================

bool FileAudio::loadFromFile(void* mixer, const std::string& name,
                           const std::string& filePath, bool predecode)
{
    destroy();

    if (mixer == nullptr) {
        LOG_ERROR("FileAudio::loadFromFile 失败: 混音器未初始化");
        return false;
    }

    mAudio = MIX_LoadAudio(static_cast<MIX_Mixer*>(mixer), filePath.c_str(), predecode);
    if (mAudio == nullptr) {
        LOG_ERROR("FileAudio::loadFromFile 失败 (%s): %s",
                 filePath.c_str(), SDL_GetError());
        return false;
    }

    mName = name;
    mFilePath = filePath;
    mPredecode = predecode;

    LOG_DEBUG("FileAudio::loadFromFile 成功: %s", name.c_str());
    return true;
}

const std::string& FileAudio::getFilePath() const
{
    return mFilePath;
}

// ==================== 实现基类接口 ====================

MIX_Audio* FileAudio::getRawAudio() const
{
    return mAudio;
}

const std::string& FileAudio::getName() const
{
    return mName;
}

bool FileAudio::isValid() const
{
    return mAudio != nullptr;
}

void FileAudio::setPredecode(bool predecode)
{
    mPredecode = predecode;
}

bool FileAudio::isPredecode() const
{
    return mPredecode;
}

void FileAudio::destroy()
{
    if (mAudio != nullptr) {
        MIX_DestroyAudio(mAudio);
        mAudio = nullptr;
    }
    mName.clear();
    mFilePath.clear();
    mPredecode = false;
}
