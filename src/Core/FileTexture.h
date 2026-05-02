/**
 * FileTexture.h - 从文件加载的纹理类
 *
 * 工厂模式的具体产品类，负责从文件系统加载图片纹理。
 * 支持 BMP、PNG、JPG、WEBP 等常见图片格式。
 */

#ifndef FILETEXTURE_H
#define FILETEXTURE_H

#include "BaseTexture.h"
#include <string>

class FileTexture : public BaseTexture {
public:
    // ==================== 构造与析构 ====================
    FileTexture();
    ~FileTexture() override;

    // 禁用拷贝
    FileTexture(const FileTexture&) = delete;
    FileTexture& operator=(const FileTexture&) = delete;

    // 支持移动语义
    FileTexture(FileTexture&& other) noexcept;
    FileTexture& operator=(FileTexture&& other) noexcept;

    // ==================== 文件加载 ====================

    /**
     * 从图片文件加载纹理
     * @param renderer 渲染器对象引用
     * @param filePath 图片文件路径
     * @return 加载成功返回 true
     */
    bool loadFromFile(Renderer& renderer, const std::string& filePath);

    /**
     * 获取加载的文件路径
     * @return 文件路径字符串
     */
    const std::string& getFilePath() const;

    // ==================== 实现基类接口 ====================
    SDL_Texture* getRawTexture() const override;
    int getWidth() const override;
    int getHeight() const override;
    bool setColorMod(Uint8 r, Uint8 g, Uint8 b) override;
    bool setAlphaMod(Uint8 alpha) override;
    Uint8 getAlphaMod() const override;
    bool setBlendMode(SDL_BlendMode blendMode) override;
    bool isValid() const override;
    void destroy() override;

private:
    SDL_Texture* mTexture;
    int mWidth;
    int mHeight;
    std::string mFilePath;
};

#endif // FILETEXTURE_H
