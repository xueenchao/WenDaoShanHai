/**
 * TextureFactory.cpp - 纹理工厂类实现
 */

#include "TextureFactory.h"
#include "FileTexture.h"
#include "MemoryTexture.h"
#include "BlankTexture.h"
#include "SurfaceTexture.h"
#include "Log.h"

// ==================== 工厂方法 ====================

std::unique_ptr<BaseTexture> TextureFactory::createFromFile(Renderer& renderer, const std::string& filePath)
{
    auto texture = std::make_unique<FileTexture>();
    if (texture->loadFromFile(renderer, filePath)) {
        return texture;
    }
    LOG_ERROR("TextureFactory::createFromFile 失败: %s", filePath.c_str());
    return nullptr;
}

std::unique_ptr<BaseTexture> TextureFactory::createFromMemory(Renderer& renderer, const void* data, size_t dataSize)
{
    auto texture = std::make_unique<MemoryTexture>();
    if (texture->loadFromMemory(renderer, data, dataSize)) {
        return texture;
    }
    LOG_ERROR("TextureFactory::createFromMemory 失败");
    return nullptr;
}

std::unique_ptr<BaseTexture> TextureFactory::createBlank(Renderer& renderer, int width, int height,
                                                         SDL_TextureAccess access)
{
    auto texture = std::make_unique<BlankTexture>();
    if (texture->createBlank(renderer, width, height, access)) {
        return texture;
    }
    LOG_ERROR("TextureFactory::createBlank 失败");
    return nullptr;
}

std::unique_ptr<BaseTexture> TextureFactory::createFromSurface(Renderer& renderer, SDL_Surface* surface)
{
    auto texture = std::make_unique<SurfaceTexture>();
    if (texture->createFromSurface(renderer, surface)) {
        return texture;
    }
    LOG_ERROR("TextureFactory::createFromSurface 失败");
    return nullptr;
}

// ==================== 类型安全的工厂方法 ====================

std::unique_ptr<FileTexture> TextureFactory::createFileTexture(Renderer& renderer, const std::string& filePath)
{
    auto texture = std::make_unique<FileTexture>();
    if (texture->loadFromFile(renderer, filePath)) {
        return texture;
    }
    LOG_ERROR("TextureFactory::createFileTexture 失败: %s", filePath.c_str());
    return nullptr;
}

std::unique_ptr<MemoryTexture> TextureFactory::createMemoryTexture(Renderer& renderer, const void* data, size_t dataSize)
{
    auto texture = std::make_unique<MemoryTexture>();
    if (texture->loadFromMemory(renderer, data, dataSize)) {
        return texture;
    }
    LOG_ERROR("TextureFactory::createMemoryTexture 失败");
    return nullptr;
}

std::unique_ptr<BlankTexture> TextureFactory::createBlankTexture(Renderer& renderer, int width, int height,
                                                                 SDL_TextureAccess access)
{
    auto texture = std::make_unique<BlankTexture>();
    if (texture->createBlank(renderer, width, height, access)) {
        return texture;
    }
    LOG_ERROR("TextureFactory::createBlankTexture 失败");
    return nullptr;
}

std::unique_ptr<SurfaceTexture> TextureFactory::createSurfaceTexture(Renderer& renderer, SDL_Surface* surface)
{
    auto texture = std::make_unique<SurfaceTexture>();
    if (texture->createFromSurface(renderer, surface)) {
        return texture;
    }
    LOG_ERROR("TextureFactory::createSurfaceTexture 失败");
    return nullptr;
}
