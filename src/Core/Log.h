/**
 * Log.h - 日志系统
 *
 * 轻量级日志系统，支持：
 *   - 5 级日志级别（Debug/Info/Warning/Error/Fatal）
 *   - 自动捕获文件名、行号、函数名
 *   - 控制台 + 文件双输出
 *   - 线程安全
 *
 * 使用方式：
 *   LOG_INFO("玩家 %s 进入场景", name.c_str());
 *   LOG_ERROR("加载纹理失败: %s", SDL_GetError());
 */

#ifndef LOG_H
#define LOG_H

#include <string>
#include <cstdio>
#include <mutex>

// 日志级别
enum class LogLevel {
    Debug,    // 开发调试信息（默认仅在 Debug 构建输出）
    Info,     // 一般流程信息
    Warning,  // 非致命警告
    Error,    // 可恢复的错误
    Fatal     // 致命错误，输出后终止程序
};

class Log {
public:
    // 获取单例
    static Log& getInstance();

    // ---- 配置 ----

    /// 设置最低输出级别，低于此级别的日志将被忽略
    void setMinLevel(LogLevel level) { mMinLevel = level; }
    LogLevel getMinLevel() const { return mMinLevel; }

    /// 设置是否输出到控制台（stderr）
    void setLogToConsole(bool enable) { mLogToConsole = enable; }

    /// 设置是否输出到文件，并指定文件路径
    void setLogToFile(bool enable, const std::string& path = "game.log");

    // ---- 日志输出 ----

    /**
     * 输出一条日志（通常通过宏调用，不建议直接使用）
     * @param level   日志级别
     * @param file    源文件名（__FILE__）
     * @param line    行号（__LINE__）
     * @param func    函数名（__FUNCTION__）
     * @param format  printf 风格格式化字符串
     * @param ...     可变参数
     */
    void log(LogLevel level, const char* file, int line,
             const char* func, const char* format, ...);

private:
    Log() = default;
    ~Log();
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    /// 将级别转为可读字符串
    static const char* levelToString(LogLevel level);

    /// 从完整路径中提取文件名
    static const char* extractFilename(const char* filePath);

    LogLevel mMinLevel = LogLevel::Debug;
    bool mLogToConsole = true;
    FILE* mFile = nullptr;
    std::string mFilePath;
    std::mutex mMutex;
};

// ============================================================
// 便捷宏（自动捕获 __FILE__, __LINE__, __FUNCTION__）
// ============================================================

#define LOG_DEBUG(fmt, ...)   Log::getInstance().log(LogLevel::Debug,   __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)    Log::getInstance().log(LogLevel::Info,    __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) Log::getInstance().log(LogLevel::Warning, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)   Log::getInstance().log(LogLevel::Error,   __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...)   Log::getInstance().log(LogLevel::Fatal,   __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#endif // LOG_H
