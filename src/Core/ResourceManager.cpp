/**
 * ResourceManager.cpp - 游戏资源管理器的实现（使用工厂模式）
 */

#include "ResourceManager.h"
#include "Texture.h"
#include "Font.h"
#include "Audio.h"
#include "Log.h"
#include "BaseTexture.h"
#include "BaseFont.h"
#include "BaseAudio.h"
#include "TextureFactory.h"
#include "FontFactory.h"
#include "AudioFactory.h"
#include <SDL3/SDL.h>

// ==================== ResourceManager ====================

ResourceManager::ResourceManager(Renderer& renderer, Audio* audio)
    : mRenderer(renderer)
    , mAudio(audio)
{
}

// ==================== 纹理资源 ====================

BaseTexture* ResourceManager::loadTexture(const std::string& key, const std::string& filePath)
{
    if (mTextureCache.has(key)) {
        LOG_DEBUG("纹理 '%s' 已缓存，跳过加载", key.c_str());
        return mTextureCache.get(key);
    }

    auto texture = TextureFactory::createFromFile(mRenderer, filePath);
    if (!texture) {
        LOG_ERROR("纹理 '%s' 加载失败: %s", key.c_str(), filePath.c_str());
        return nullptr;
    }

    BaseTexture* ptr = texture.get();
    mTextureCache.add(key, std::move(texture));
    LOG_DEBUG("纹理 '%s' 加载成功", key.c_str());
    return ptr;
}

BaseTexture* ResourceManager::loadTextureFromMemory(const std::string& key, const void* data, size_t dataSize)
{
    if (mTextureCache.has(key)) {
        LOG_DEBUG("纹理 '%s' 已缓存，跳过加载", key.c_str());
        return mTextureCache.get(key);
    }

    auto texture = TextureFactory::createFromMemory(mRenderer, data, dataSize);
    if (!texture) {
        LOG_ERROR("纹理 '%s' 从内存加载失败", key.c_str());
        return nullptr;
    }

    BaseTexture* ptr = texture.get();
    mTextureCache.add(key, std::move(texture));
    LOG_DEBUG("纹理 '%s' 从内存加载成功", key.c_str());
    return ptr;
}

BaseTexture* ResourceManager::createTexture(const std::string& key, int width, int height,
                                            SDL_TextureAccess access)
{
    if (mTextureCache.has(key)) {
        LOG_DEBUG("纹理 '%s' 已缓存，跳过创建", key.c_str());
        return mTextureCache.get(key);
    }

    auto texture = TextureFactory::createBlank(mRenderer, width, height, access);
    if (!texture) {
        LOG_ERROR("纹理 '%s' 创建失败", key.c_str());
        return nullptr;
    }

    BaseTexture* ptr = texture.get();
    mTextureCache.add(key, std::move(texture));
    LOG_DEBUG("纹理 '%s' 创建成功", key.c_str());
    return ptr;
}

BaseTexture* ResourceManager::createTextureFromSurface(const std::string& key, SDL_Surface* surface)
{
    if (mTextureCache.has(key)) {
        LOG_DEBUG("纹理 '%s' 已缓存，跳过创建", key.c_str());
        return mTextureCache.get(key);
    }

    auto texture = TextureFactory::createFromSurface(mRenderer, surface);
    if (!texture) {
        LOG_ERROR("纹理 '%s' 从Surface创建失败", key.c_str());
        return nullptr;
    }

    BaseTexture* ptr = texture.get();
    mTextureCache.add(key, std::move(texture));
    LOG_DEBUG("纹理 '%s' 从Surface创建成功", key.c_str());
    return ptr;
}

BaseTexture* ResourceManager::getTexture(const std::string& key) const
{
    return mTextureCache.get(key);
}

Texture* ResourceManager::getLegacyTexture(const std::string& key) const
{
    return nullptr;  // 不再支持旧版纹理对象的返回
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
