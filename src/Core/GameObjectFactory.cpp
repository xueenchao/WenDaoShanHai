/**
 * GameObjectFactory.cpp - 游戏对象工厂实现
 */

#include "GameObjectFactory.h"
#include "Log.h"
#include "JsonLoader.h"
#include <cjson/cJSON.h>
#include <algorithm>

// ==================== 单例模式 ====================

GameObjectFactory& GameObjectFactory::instance()
{
    static GameObjectFactory instance;
    return instance;
}

// ==================== 注册功能 ====================

void GameObjectFactory::registerType(const std::string& type, GameObjectCreator creator)
{
    if (mCreators.find(type) != mCreators.end()) {
        LOG_WARNING("游戏对象类型 '%s' 已存在，将覆盖", type.c_str());
    }

    mCreators[type] = std::move(creator);
    LOG_DEBUG("已注册游戏对象类型: %s", type.c_str());
}

bool GameObjectFactory::isRegistered(const std::string& type) const
{
    return mCreators.find(type) != mCreators.end();
}

// ==================== 创建功能 ====================

std::unique_ptr<GameObject> GameObjectFactory::create(const std::string& type, cJSON* config)
{
    auto it = mCreators.find(type);
    if (it == mCreators.end()) {
        LOG_ERROR("创建失败: 未注册的游戏对象类型 '%s'", type.c_str());
        return nullptr;
    }

    auto obj = it->second();
    if (!obj) {
        LOG_ERROR("创建失败: 类型 '%s' 的创建函数返回空", type.c_str());
        return nullptr;
    }

    if (config && !obj->init(config)) {
        LOG_ERROR("创建失败: 类型 '%s' 的初始化失败", type.c_str());
        return nullptr;
    }

    LOG_DEBUG("成功创建游戏对象: %s", type.c_str());
    return obj;
}

std::unique_ptr<GameObject> GameObjectFactory::createFromFile(const std::string& filePath)
{
    cJSON* root = JsonLoader::loadJsonFile(filePath);
    if (root == nullptr) {
        LOG_ERROR("创建失败: 无法加载文件 '%s'", filePath.c_str());
        return nullptr;
    }

    // 检查是否有 "type" 字段
    cJSON* typeJson = cJSON_GetObjectItem(root, "type");
    if (typeJson == nullptr || !cJSON_IsString(typeJson)) {
        LOG_ERROR("创建失败: 文件 '%s' 缺少 'type' 字段", filePath.c_str());
        cJSON_Delete(root);
        return nullptr;
    }

    std::string type = typeJson->valuestring;
    auto obj = create(type, root);
    cJSON_Delete(root);
    return obj;
}

std::unique_ptr<GameObject> GameObjectFactory::createFromString(const std::string& jsonString)
{
    cJSON* root = cJSON_Parse(jsonString.c_str());
    if (root == nullptr) {
        const char* errorPtr = cJSON_GetErrorPtr();
        LOG_ERROR("创建失败: JSON 解析错误 '%s'", errorPtr ? errorPtr : "unknown");
        return nullptr;
    }

    cJSON* typeJson = cJSON_GetObjectItem(root, "type");
    if (typeJson == nullptr || !cJSON_IsString(typeJson)) {
        LOG_ERROR("创建失败: JSON 缺少 'type' 字段");
        cJSON_Delete(root);
        return nullptr;
    }

    std::string type = typeJson->valuestring;
    auto obj = create(type, root);
    cJSON_Delete(root);
    return obj;
}

// ==================== 批量创建 ====================

std::vector<std::unique_ptr<GameObject>> GameObjectFactory::createFromArray(cJSON* array)
{
    std::vector<std::unique_ptr<GameObject>> result;

    if (array == nullptr || !cJSON_IsArray(array)) {
        LOG_ERROR("批量创建失败: 参数不是 JSON 数组");
        return result;
    }

    int size = cJSON_GetArraySize(array);
    for (int i = 0; i < size; ++i) {
        cJSON* item = cJSON_GetArrayItem(array, i);
        if (item == nullptr) {
            continue;
        }

        cJSON* typeJson = cJSON_GetObjectItem(item, "type");
        if (typeJson == nullptr || !cJSON_IsString(typeJson)) {
            LOG_WARNING("跳过数组项 %d: 缺少 'type' 字段", i);
            continue;
        }

        std::string type = typeJson->valuestring;
        auto obj = create(type, item);
        if (obj) {
            result.push_back(std::move(obj));
        }
    }

    LOG_DEBUG("从数组创建了 %zu 个游戏对象", result.size());
    return result;
}

std::vector<std::unique_ptr<GameObject>> GameObjectFactory::createFromArrayFile(const std::string& filePath)
{
    cJSON* root = JsonLoader::loadJsonFile(filePath);
    if (root == nullptr) {
        LOG_ERROR("批量创建失败: 无法加载文件 '%s'", filePath.c_str());
        return {};
    }

    if (!cJSON_IsArray(root)) {
        LOG_ERROR("批量创建失败: 文件 '%s' 不是 JSON 数组", filePath.c_str());
        cJSON_Delete(root);
        return {};
    }

    auto result = createFromArray(root);
    cJSON_Delete(root);
    return result;
}

// ==================== 类型管理 ====================

std::vector<std::string> GameObjectFactory::getRegisteredTypes() const
{
    std::vector<std::string> types;
    types.reserve(mCreators.size());

    for (const auto& pair : mCreators) {
        types.push_back(pair.first);
    }

    return types;
}

void GameObjectFactory::clear()
{
    mCreators.clear();
    LOG_INFO("已清除所有注册的游戏对象类型");
}
