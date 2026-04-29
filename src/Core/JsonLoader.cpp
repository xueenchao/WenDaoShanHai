/**
 * JsonLoader.cpp - JSON文件加载工具实现
 */

#include "JsonLoader.h"
#include "Log.h"
#include <cjson/cJSON.h>
#include <cstdio>
#include <cstdlib>

static bool readFileToString(const std::string& path, std::string& out)
{
    FILE* f = fopen(path.c_str(), "rb");
    if (f == nullptr) {
        LOG_ERROR("无法打开文件: %s", path.c_str());
        return false;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        LOG_ERROR("文件为空或读取失败: %s", path.c_str());
        return false;
    }

    out.resize(static_cast<size_t>(size));
    size_t read = fread(&out[0], 1, static_cast<size_t>(size), f);
    fclose(f);

    if (read != static_cast<size_t>(size)) {
        LOG_ERROR("读取文件不完整: %s (期望 %ld, 实际 %zu)", path.c_str(), size, read);
        return false;
    }

    return true;
}

cJSON* JsonLoader::loadJsonFile(const std::string& path)
{
    std::string content;
    if (!readFileToString(path, content)) {
        return nullptr;
    }

    cJSON* root = cJSON_ParseWithLength(content.c_str(), content.size());
    if (root == nullptr) {
        const char* err = cJSON_GetErrorPtr();
        if (err != nullptr) {
            LOG_ERROR("JSON解析失败 (%s): %s", path.c_str(), err);
        } else {
            LOG_ERROR("JSON解析失败 (%s): 未知错误", path.c_str());
        }
        return nullptr;
    }

    LOG_INFO("已加载JSON: %s (%zu字节)", path.c_str(), content.size());
    return root;
}

void JsonLoader::freeJson(cJSON* json)
{
    if (json != nullptr) {
        cJSON_Delete(json);
    }
}
