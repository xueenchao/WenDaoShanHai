#include "Log.h"
#include <cstdarg>
#include <ctime>
#include <cstdlib>

Log& Log::getInstance()
{
    static Log instance;
    return instance;
}

Log::~Log()
{
    if (mFile != nullptr) {
        fclose(mFile);
        mFile = nullptr;
    }
}

void Log::setLogToFile(bool enable, const std::string& path)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (mFile != nullptr) {
        fclose(mFile);
        mFile = nullptr;
    }

    if (enable && !path.empty()) {
        mFilePath = path;
        mFile = fopen(path.c_str(), "a");
        if (mFile == nullptr) {
            // 文件打开失败，回退到仅控制台
            fprintf(stderr, "[Log] 无法打开日志文件: %s\n", path.c_str());
        }
    }
}

void Log::log(LogLevel level, const char* file, int line,
              const char* func, const char* format, ...)
{
    // 级别过滤
    if (level < mMinLevel) return;

    std::lock_guard<std::mutex> lock(mMutex);

    // 格式化时间戳
    char timeBuf[32];
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", tm_info);

    // 级别字符串
    const char* levelStr = levelToString(level);

    // 提取文件名（去掉路径前缀）
    const char* filename = extractFilename(file);

    // 格式化用户消息
    char msgBuf[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(msgBuf, sizeof(msgBuf), format, args);
    va_end(args);

    // 组装完整日志行
    char lineBuf[1280];
    snprintf(lineBuf, sizeof(lineBuf), "[%s] [%s] [%s:%d] %s: %s\n",
             timeBuf, levelStr, filename, line, func, msgBuf);

    // 输出到控制台
    if (mLogToConsole) {
        fprintf(stderr, "%s", lineBuf);
        fflush(stderr);
    }

    // 输出到文件
    if (mFile != nullptr) {
        fputs(lineBuf, mFile);
        fflush(mFile);
    }

    // Fatal 级别终止程序
    if (level == LogLevel::Fatal) {
        abort();
    }
}

const char* Log::levelToString(LogLevel level)
{
    switch (level) {
    case LogLevel::Debug:   return "DEBUG";
    case LogLevel::Info:    return "INFO";
    case LogLevel::Warning: return "WARN";
    case LogLevel::Error:   return "ERROR";
    case LogLevel::Fatal:   return "FATAL";
    default:                return "????";
    }
}

const char* Log::extractFilename(const char* filePath)
{
    if (filePath == nullptr) return "???";

    // 查找最后一个路径分隔符（支持 / 和 \）
    const char* last = filePath;
    for (const char* p = filePath; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\') {
            last = p + 1;
        }
    }
    return last;
}
