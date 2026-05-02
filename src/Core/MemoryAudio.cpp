/**
 * MemoryAudio.cpp - 从内存加载的音频类实现
 */

#include "MemoryAudio.h"
#include "Log.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

// ==================== 构造与析构 ====================

MemoryAudio::MemoryAudio()
    : mAudio(nullptr)
    , mPredecode(false)
{
}

MemoryAudio::~MemoryAudio()
{
    destroy();
}

// ==================== 移动语义 ====================

MemoryAudio::MemoryAudio(MemoryAudio&& other) noexcept
    : mAudio(other.mAudio)
    , mName(std::move(other.mName))
    , mPredecode(other.mPredecode)
{
    other.mAudio = nullptr;
    other.mPredecode = false;
}

MemoryAudio& MemoryAudio::operator=(MemoryAudio&& other) noexcept
{
    if (this != &other) {
        destroy();

        mAudio = other.mAudio;
        mName = std::move(other.mName);
        mPredecode = other.mPredecode;

        other.mAudio = nullptr;
        other.mPredecode = false;
    }
    return *this;
}

// ==================== 内存加载 ====================

bool MemoryAudio::loadFromMemory(void* mixer, const std::string& name,
                               const void* data, size_t dataSize, bool predecode)
{
    destroy();

    if (mixer == nullptr) {
        LOG_ERROR("MemoryAudio::loadFromMemory 失败: 混音器未初始化");
        return false;
    }

    if (data == nullptr || dataSize == 0) {
        LOG_ERROR("MemoryAudio::loadFromMemory 失败: 数据为空");
        return false;
    }

    SDL_IOStream* io = SDL_IOFromConstMem(data, dataSize);
    if (io == nullptr) {
        LOG_ERROR("MemoryAudio::loadFromMemory 创建IOStream失败: %s", SDL_GetError());
        return false;
    }

    mAudio = MIX_LoadAudio_IO(static_cast<MIX_Mixer*>(mixer), io, predecode, true);
    if (mAudio == nullptr) {
        LOG_ERROR("MemoryAudio::loadFromMemory 加载失败: %s", SDL_GetError());
        return false;
    }

    mName = name;
    mPredecode = predecode;

    LOG_DEBUG("MemoryAudio::loadFromMemory 成功: %s", name.c_str());
    return true;
}

// ==================== 实现基类接口 ====================

MIX_Audio* MemoryAudio::getRawAudio() const
{
    return mAudio;
}

const std::string& MemoryAudio::getName() const
{
    return mName;
}

bool MemoryAudio::isValid() const
{
    return mAudio != nullptr;
}

void MemoryAudio::setPredecode(bool predecode)
{
    mPredecode = predecode;
}

bool MemoryAudio::isPredecode() const
{
    return mPredecode;
}

void MemoryAudio::destroy()
{
    if (mAudio != nullptr) {
        MIX_DestroyAudio(mAudio);
        mAudio = nullptr;
    }
    mName.clear();
    mPredecode = false;
}
