/**
 * JsonLoader.h - JSON文件加载工具
 *
 * 封装cJSON的文件读取和解析，提供统一的错误处理。
 */

#ifndef JSONLOADER_H
#define JSONLOADER_H

#include <string>

struct cJSON;

class JsonLoader {
public:
    /**
     * 初始化数据根目录（基于exe位置自动探测）
     * 必须在 loadAll() 之前调用
     */
    static void initBasePath();

    /**
     * 从文件加载并解析JSON
     * @param path 相对于数据根目录的文件路径
     * @return cJSON根节点，失败返回nullptr（调用者负责cJSON_Delete）
     */
    static cJSON* loadJsonFile(const std::string& path);

    /**
     * 释放cJSON对象
     */
    static void freeJson(cJSON* json);
};

#endif // JSONLOADER_H
