/**
 * ResourceManager.cpp - 游戏资源管理器的实现
 */

#include "ResourceManager.h"
#include "Texture.h"
#include "Font.h"
#include "Audio.h"
#include "Log.h"
#include <SDL3/SDL.h>

// ==================== ResourceManager ====================

ResourceManager::ResourceManager(Renderer& renderer, Audio* audio)
    : mRenderer(renderer)
    , mAudio(audio)
{
}

// ==================== 纹理资源 ====================

Texture* ResourceManager::loadTexture(const std::string& key, const std::string& filePath)
{
    // 已存在则直接返回
    if (mTextureCache.has(key)) {
        LOG_DEBUG("纹理 '%s' 已缓存，跳过加载", key.c_str());
        return mTextureCache.get(key);
    }

    auto texture = std::make_unique<Texture>();
    if (!texture->loadFromFile(mRenderer, filePath)) {
        LOG_ERROR("纹理 '%s' 加载失败: %s", key.c_str(), filePath.c_str());
        return nullptr;
    }

    Texture* ptr = texture.get();
    mTextureCache.add(key, std::move(texture));
    LOG_DEBUG("纹理 '%s' 加载成功", key.c_str());
    return ptr;
}

Texture* ResourceManager::getTexture(const std::string& key) const
{
    return mTextureCache.get(key);
}

void ResourceManager::unloadTexture(const std::string& key)
{
    mTextureCache.remove(key);
}

// ==================== 字体资源 ====================

Font* ResourceManager::loadFont(const std::string& key, const std::string& filePath, float fontSize)
{
    if (mFontCache.has(key)) {
        LOG_DEBUG("字体 '%s' 已缓存，跳过加载", key.c_str());
        return mFontCache.get(key);
    }

    auto font = std::make_unique<Font>();
    if (!font->load(filePath, fontSize)) {
        LOG_ERROR("字体 '%s' 加载失败: %s", key.c_str(), filePath.c_str());
        return nullptr;
    }

    Font* ptr = font.get();
    mFontCache.add(key, std::move(font));
    LOG_DEBUG("字体 '%s' 加载成功 (大小: %.0f)", key.c_str(), fontSize);
    return ptr;
}

Font* ResourceManager::getFont(const std::string& key) const
{
    return mFontCache.get(key);
}

void ResourceManager::unloadFont(const std::string& key)
{
    mFontCache.remove(key);
}

// ==================== 音频资源 ====================

bool ResourceManager::loadAudio(const std::string& key, const std::string& filePath)
{
    if (mAudio == nullptr) {
        LOG_ERROR("Audio 模块未初始化，无法加载音频 '%s'", key.c_str());
        return false;
    }

    return mAudio->loadAudio(key, filePath);
}

void ResourceManager::unloadAudio(const std::string& key)
{
    if (mAudio != nullptr) {
        mAudio->unloadAudio(key);
    }
}

// ==================== 全局管理 ====================

void ResourceManager::clearAll()
{
    mTextureCache.clear();
    mFontCache.clear();
    LOG_INFO("所有资源已清空");
}

size_t ResourceManager::getTotalResourceCount() const
{
    return mTextureCache.size() + mFontCache.size();
}
