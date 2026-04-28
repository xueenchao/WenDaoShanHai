/**
 * ResourceManager.h - 游戏资源管理器
 *
 * 统一管理游戏中所有资源的加载、缓存和释放。
 * 使用 ResourceCache 模板实现按名称查找的资源缓存。
 *
 * 管理的资源类型：
 *   - 纹理（Texture）：图片素材
 *   - 字体（Font）：TTF/OTF 字体
 *   - 音频（Audio）：音效和音乐（委托给 Audio 模块）
 *
 * 使用方式：
 *   1. 创建 ResourceManager 并传入 Renderer/Audio 引用
 *   2. 通过 loadTexture/loadFont/loadAudio 预加载资源
 *   3. 通过 getTexture/getFont 获取缓存的资源指针
 *   4. 资源加载失败时返回 nullptr，不崩溃
 */

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <memory>
#include <string>
#include <unordered_map>

class Renderer;
class Audio;
class Texture;
class Font;

// ==================== 通用资源缓存模板 ====================

/**
 * ResourceCache - 泛型资源缓存
 * 按名称（字符串键）存储和管理资源对象
 * @tparam T 资源类型（Texture、Font 等）
 */
template<typename T>
class ResourceCache {
public:
    ResourceCache() = default;
    ~ResourceCache() = default;

    // 禁用拷贝
    ResourceCache(const ResourceCache&) = delete;
    ResourceCache& operator=(const ResourceCache&) = delete;

    /**
     * 获取指定名称的资源
     * @param key 资源名称
     * @return 资源指针，不存在返回 nullptr
     */
    T* get(const std::string& key) const
    {
        auto it = mResources.find(key);
        if (it != mResources.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    /**
     * 检查资源是否存在
     * @param key 资源名称
     */
    bool has(const std::string& key) const
    {
        return mResources.find(key) != mResources.end();
    }

    /**
     * 添加资源到缓存
     * @param key      资源名称
     * @param resource 资源对象（unique_ptr，所有权转移）
     */
    void add(const std::string& key, std::unique_ptr<T> resource)
    {
        mResources[key] = std::move(resource);
    }

    /**
     * 移除并销毁指定资源
     * @param key 资源名称
     * @return 移除成功返回 true
     */
    bool remove(const std::string& key)
    {
        return mResources.erase(key) > 0;
    }

    /**
     * 清空所有缓存资源
     */
    void clear()
    {
        mResources.clear();
    }

    /**
     * 获取缓存资源数量
     */
    size_t size() const
    {
        return mResources.size();
    }

private:
    std::unordered_map<std::string, std::unique_ptr<T>> mResources;
};

// ==================== 资源管理器 ====================

class ResourceManager {
public:
    /**
     * 构造函数
     * @param renderer 渲染器引用（纹理和字体加载需要）
     * @param audio    音频系统引用（音频加载需要），可为 nullptr
     */
    ResourceManager(Renderer& renderer, Audio* audio = nullptr);

    ~ResourceManager() = default;

    // 禁用拷贝
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // ==================== 纹理资源 ====================

    /**
     * 加载纹理并缓存
     * @param key      纹理名称
     * @param filePath 图片文件路径
     * @return 纹理指针，失败返回 nullptr
     */
    Texture* loadTexture(const std::string& key, const std::string& filePath);

    /**
     * 获取已缓存的纹理
     * @param key 纹理名称
     * @return 纹理指针，不存在返回 nullptr
     */
    Texture* getTexture(const std::string& key) const;

    /**
     * 卸载纹理
     * @param key 纹理名称
     */
    void unloadTexture(const std::string& key);

    // ==================== 字体资源 ====================

    /**
     * 加载字体并缓存
     * @param key      字体名称
     * @param filePath 字体文件路径
     * @param fontSize 字体大小
     * @return 字体指针，失败返回 nullptr
     */
    Font* loadFont(const std::string& key, const std::string& filePath, float fontSize);

    /**
     * 获取已缓存的字体
     * @param key 字体名称
     * @return 字体指针，不存在返回 nullptr
     */
    Font* getFont(const std::string& key) const;

    /**
     * 卸载字体
     * @param key 字体名称
     */
    void unloadFont(const std::string& key);

    // ==================== 音频资源 ====================

    /**
     * 加载音频（委托给 Audio 模块）
     * @param key      音频名称
     * @param filePath 音频文件路径
     * @return 成功返回 true
     */
    bool loadAudio(const std::string& key, const std::string& filePath);

    /**
     * 卸载音频（委托给 Audio 模块）
     * @param key 音频名称
     */
    void unloadAudio(const std::string& key);

    // ==================== 全局管理 ====================

    /**
     * 清空所有缓存的资源
     */
    void clearAll();

    /**
     * 获取缓存的资源总数
     */
    size_t getTotalResourceCount() const;

private:
    Renderer& mRenderer;
    Audio* mAudio;

    ResourceCache<Texture> mTextureCache;
    ResourceCache<Font> mFontCache;
    // 音频资源由 Audio 模块内部管理，此处只记录 key 用于统一接口
};

#endif // RESOURCEMANAGER_H
