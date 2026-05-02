/**
 * GameObjectFactory.h - 游戏对象工厂
 *
 * 注册式工厂模式，支持通过 JSON 配置创建游戏对象。
 * 允许在运行时动态创建不同类型的游戏实体。
 */

#ifndef GAMEOBJECTFACTORY_H
#define GAMEOBJECTFACTORY_H

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

struct cJSON;
class Renderer;
class BaseTexture;

// 游戏对象基类（示例）
class GameObject {
public:
    virtual ~GameObject() = default;
    virtual bool init(cJSON* config) { return true; }
    virtual void update(float deltaTime) {}
    virtual void render(Renderer* renderer) {}
    virtual const std::string& getType() const = 0;
};

// 游戏对象创建函数类型
using GameObjectCreator = std::function<std::unique_ptr<GameObject>()>;

class GameObjectFactory {
public:
    // ==================== 单例模式 ====================

    static GameObjectFactory& instance();

    // ==================== 注册功能 ====================

    /**
     * 注册游戏对象类型
     * @param type 类型名称
     * @param creator 创建函数
     */
    void registerType(const std::string& type, GameObjectCreator creator);

    /**
     * 检查类型是否已注册
     * @param type 类型名称
     */
    bool isRegistered(const std::string& type) const;

    // ==================== 创建功能 ====================

    /**
     * 创建游戏对象
     * @param type 类型名称
     * @param config JSON 配置（可为 nullptr）
     */
    std::unique_ptr<GameObject> create(const std::string& type, cJSON* config = nullptr);

    /**
     * 从 JSON 文件创建游戏对象
     * @param filePath JSON 文件路径
     */
    std::unique_ptr<GameObject> createFromFile(const std::string& filePath);

    /**
     * 从 JSON 字符串创建游戏对象
     * @param jsonString JSON 字符串
     */
    std::unique_ptr<GameObject> createFromString(const std::string& jsonString);

    // ==================== 批量创建 ====================

    /**
     * 从 JSON 数组批量创建游戏对象
     * @param array JSON 数组
     */
    std::vector<std::unique_ptr<GameObject>> createFromArray(cJSON* array);

    /**
     * 从 JSON 配置文件批量创建游戏对象
     * @param filePath JSON 文件路径
     */
    std::vector<std::unique_ptr<GameObject>> createFromArrayFile(const std::string& filePath);

    // ==================== 类型管理 ====================

    /**
     * 获取所有已注册的类型
     */
    std::vector<std::string> getRegisteredTypes() const;

    /**
     * 清除所有注册的类型
     */
    void clear();

private:
    GameObjectFactory() = default;
    ~GameObjectFactory() = default;
    GameObjectFactory(const GameObjectFactory&) = delete;
    GameObjectFactory& operator=(const GameObjectFactory&) = delete;

    // 类型注册表
    std::unordered_map<std::string, GameObjectCreator> mCreators;
};

// ==================== 注册宏 ====================

#define REGISTER_GAMEOBJECT(type, class) \
    static bool registered_##class = [](){ \
        GameObjectFactory::instance().registerType(type, [](){ \
            return std::make_unique<class>(); \
        }); \
        return true; \
    }()

#endif // GAMEOBJECTFACTORY_H
