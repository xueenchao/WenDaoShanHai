/**
 * BaseAudio.h - 音频抽象基类
 *
 * 定义所有音频类的统一接口，作为工厂模式的产品接口。
 * 封装 SDL3_mixer 的基本操作，提供统一的访问方式。
 */

#ifndef BASEAUDIO_H
#define BASEAUDIO_H

#include <string>

// 前向声明
struct MIX_Audio;

class BaseAudio {
public:
    // ==================== 构造与析构 ====================
    BaseAudio() = default;
    virtual ~BaseAudio() = default;

    // 禁用拷贝
    BaseAudio(const BaseAudio&) = delete;
    BaseAudio& operator=(const BaseAudio&) = delete;

    // 支持移动语义
    BaseAudio(BaseAudio&& other) noexcept = default;
    BaseAudio& operator=(BaseAudio&& other) noexcept = default;

    // ==================== 音频属性 ====================

    /**
     * 获取 SDL3_mixer 原生音频指针
     * @return MIX_Audio 指针
     */
    virtual MIX_Audio* getRawAudio() const = 0;

    /**
     * 获取音频名称
     * @return 音频名称字符串
     */
    virtual const std::string& getName() const = 0;

    /**
     * 判断音频是否有效
     * @return 有效返回 true
     */
    virtual bool isValid() const = 0;

    // ==================== 音频控制 ====================

    /**
     * 设置音频预解码状态
     * @param predecode 是否预解码
     */
    virtual void setPredecode(bool predecode) = 0;

    /**
     * 获取预解码状态
     * @return 预解码返回 true
     */
    virtual bool isPredecode() const = 0;

    // ==================== 音频操作 ====================

    /**
     * 销毁音频资源
     */
    virtual void destroy() = 0;
};

#endif // BASEAUDIO_H
