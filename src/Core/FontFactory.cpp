/**
 * FontFactory.cpp - 字体工厂类实现
 */

#include "FontFactory.h"
#include "TrueTypeFont.h"
#include "MemoryFont.h"
#include "Log.h"

// ==================== 工厂方法 ====================

std::unique_ptr<BaseFont> FontFactory::createFromFile(const std::string& filePath, float fontSize)
{
    auto font = std::make_unique<TrueTypeFont>();
    if (font->loadFromFile(filePath, fontSize)) {
        return font;
    }
    LOG_ERROR("FontFactory::createFromFile 失败: %s", filePath.c_str());
    return nullptr;
}

std::unique_ptr<BaseFont> FontFactory::createFromMemory(const void* data, size_t dataSize, float fontSize)
{
    auto font = std::make_unique<MemoryFont>();
    if (font->loadFromMemory(data, dataSize, fontSize)) {
        return font;
    }
    LOG_ERROR("FontFactory::createFromMemory 失败");
    return nullptr;
}

// ==================== 类型安全的工厂方法 ====================

std::unique_ptr<TrueTypeFont> FontFactory::createTrueTypeFont(const std::string& filePath, float fontSize)
{
    auto font = std::make_unique<TrueTypeFont>();
    if (font->loadFromFile(filePath, fontSize)) {
        return font;
    }
    LOG_ERROR("FontFactory::createTrueTypeFont 失败: %s", filePath.c_str());
    return nullptr;
}

std::unique_ptr<MemoryFont> FontFactory::createMemoryFont(const void* data, size_t dataSize, float fontSize)
{
    auto font = std::make_unique<MemoryFont>();
    if (font->loadFromMemory(data, dataSize, fontSize)) {
        return font;
    }
    LOG_ERROR("FontFactory::createMemoryFont 失败");
    return nullptr;
}
