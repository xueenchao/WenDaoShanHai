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
     * 从文件加载并解析JSON
     * @param path 相对于可执行目录的文件路径
     * @return cJSON根节点，失败返回nullptr（调用者负责cJSON_Delete）
     */
    static cJSON* loadJsonFile(const std::string& path);

    /**
     * 释放cJSON对象
     */
    static void freeJson(cJSON* json);
};

#endif // JSONLOADER_H
