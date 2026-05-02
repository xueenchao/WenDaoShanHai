/**
 * FontFactory.h - 字体工厂类
 *
 * 简单工厂模式实现，负责创建各种类型的字体对象。
 * 封装字体的创建过程，提高代码的可扩展性和可维护性。
 *
 * 支持创建的字体类型：
 *   - TrueTypeFont：从文件加载的 TrueType/OpenType 字体
 *   - MemoryFont：从内存加载的字体数据
 */

#ifndef FONTFACTORY_H
#define FONTFACTORY_H

#include <memory>
#include <string>

// 前向声明
class BaseFont;
class TrueTypeFont;
class MemoryFont;

class FontFactory {
public:
    // ==================== 工厂方法 ====================

    /**
     * 从文件加载字体
     * @param filePath 文件路径（.ttf 或 .otf 文件）
     * @param fontSize 字体大小（磅值）
     * @return 字体对象的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<BaseFont> createFromFile(const std::string& filePath, float fontSize);

    /**
     * 从内存加载字体
     * @param data 字体数据的内存指针
     * @param dataSize 数据大小（字节）
     * @param fontSize 字体大小（磅值）
     * @return 字体对象的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<BaseFont> createFromMemory(const void* data, size_t dataSize, float fontSize);

    // ==================== 类型安全的工厂方法 ====================

    /**
     * 创建 TrueTypeFont 对象并从文件加载
     * @param filePath 文件路径
     * @param fontSize 字体大小
     * @return TrueTypeFont 的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<TrueTypeFont> createTrueTypeFont(const std::string& filePath, float fontSize);

    /**
     * 创建 MemoryFont 对象并从内存加载
     * @param data 字体数据的内存指针
     * @param dataSize 数据大小（字节）
     * @param fontSize 字体大小
     * @return MemoryFont 的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<MemoryFont> createMemoryFont(const void* data, size_t dataSize, float fontSize);

private:
    // 私有构造函数，防止创建工厂实例
    FontFactory() = delete;
    ~FontFactory() = delete;
    FontFactory(const FontFactory&) = delete;
    FontFactory& operator=(const FontFactory&) = delete;
};

#endif // FONTFACTORY_H
