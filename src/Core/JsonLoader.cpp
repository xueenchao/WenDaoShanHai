/**
 * JsonLoader.cpp - JSON文件加载工具实现
 */

#include "JsonLoader.h"
#include "Log.h"
#include <cjson/cJSON.h>
#include <cstdio>
#include <cstdlib>
#include <windows.h>

static std::string gBasePath;

static bool readFileToString(const std::string& path, std::string& out)
{
    FILE* f = fopen(path.c_str(), "rb");
    if (f == nullptr) return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) { fclose(f); return false; }

    out.resize(static_cast<size_t>(size));
    size_t read = fread(&out[0], 1, static_cast<size_t>(size), f);
    fclose(f);

    if (read != static_cast<size_t>(size)) return false;
    return true;
}

void JsonLoader::initBasePath()
{
    if (!gBasePath.empty()) return;

    char exePath[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return;

    for (int i = static_cast<int>(len) - 1; i >= 0; --i) {
        if (exePath[i] == '\\' || exePath[i] == '/') {
            exePath[i] = '\0';
            break;
        }
    }
    std::string exeDir(exePath);

    // 检查 data/ 在哪个层级
    auto tryPath = [](const std::string& dir) -> bool {
        FILE* f = fopen((dir + "/data/skills.json").c_str(), "rb");
        if (f) { fclose(f); return true; }
        return false;
    };

    if (tryPath(exeDir)) { gBasePath = exeDir; return; }

    // 开发模式: exe在 build/ 下，data/ 在父目录
    for (int i = static_cast<int>(exeDir.size()) - 1; i >= 0; --i) {
        if (exeDir[i] == '\\' || exeDir[i] == '/') {
            std::string parent = exeDir.substr(0, i);
            if (tryPath(parent)) { gBasePath = parent; return; }
            break;
        }
    }
}

cJSON* JsonLoader::loadJsonFile(const std::string& path)
{
    std::string content;

    if (!gBasePath.empty()) {
        std::string absPath = gBasePath + "/" + path;
        if (readFileToString(absPath, content)) goto parse;
    }

    if (readFileToString(path, content)) goto parse;

    LOG_ERROR("无法打开文件: %s", path.c_str());
    return nullptr;

parse:
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
