/**
 * GameLoop.h - 游戏主循环类
 *
 * 封装游戏的核心循环逻辑：初始化 → 事件处理 → 更新 → 渲染 → 呈现。
 * 提供固定时间步长的更新机制，确保游戏逻辑在不同帧率下行为一致。
 *
 * 游戏循环的基本流程：
 *   1. 处理输入事件（键盘、鼠标、窗口等）
 *   2. 更新游戏逻辑（固定时间步长，如每秒60次）
 *   3. 渲染画面（每帧一次）
 *   4. 呈现画面到屏幕
 *   5. 等待下一帧（控制帧率）
 *
 * 使用方式：
 *   1. 创建 GameLoop 对象
 *   2. 设置 OnInit / OnUpdate / OnRender 回调
 *   3. 调用 run() 启动循环
 */

#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <SDL3/SDL.h>
#include <functional>

class EventHandler;

class GameLoop {
public:
    // ==================== 回调函数类型定义 ====================

    /**
     * 初始化回调：在游戏循环开始前调用一次
     * 用于加载资源、初始化游戏状态等
     * @return 初始化成功返回 true，失败返回 false（将终止循环）
     */
    using OnInitCallback = std::function<bool()>;

    /**
     * 更新回调：每帧调用，用于更新游戏逻辑
     * @param deltaTime 距离上一帧的时间（秒），用于平滑动画
     *                  例如：移动速度 = 基础速度 * deltaTime
     */
    using OnUpdateCallback = std::function<void(float deltaTime)>;

    /**
     * 渲染回调：每帧调用，用于绘制画面
     */
    using OnRenderCallback = std::function<void()>;

    /**
     * 清理回调：在游戏循环结束后调用一次
     * 用于释放资源、保存状态等
     */
    using OnCleanupCallback = std::function<void()>;

    // ==================== 构造与析构 ====================

    /**
     * 构造函数
     * @param eventHandler 事件处理器引用
     * @param targetFPS    目标帧率（每秒帧数），默认 60 FPS
     *                     设置为 0 表示不限制帧率（尽可能快地运行）
     */
    GameLoop(EventHandler& eventHandler, int targetFPS = 60);

    /**
     * 析构函数
     */
    ~GameLoop() = default;

    // ==================== 回调设置 ====================

    /**
     * 设置初始化回调
     * @param callback 回调函数
     */
    void setOnInit(OnInitCallback callback);

    /**
     * 设置更新回调
     * @param callback 回调函数
     */
    void setOnUpdate(OnUpdateCallback callback);

    /**
     * 设置渲染回调
     * @param callback 回调函数
     */
    void setOnRender(OnRenderCallback callback);

    /**
     * 设置清理回调
     * @param callback 回调函数
     */
    void setOnCleanup(OnCleanupCallback callback);

    // ==================== 循环控制 ====================

    /**
     * 启动游戏主循环
     * 会依次执行：OnInit → (pollEvents + OnUpdate + OnRender) 循环 → OnCleanup
     * 此方法会阻塞，直到收到退出事件或调用 stop()
     * @return 正常退出返回 true，初始化失败返回 false
     */
    bool run();

    /**
     * 请求停止游戏循环
     * 可以在回调函数中调用，循环会在当前帧结束后退出
     */
    void stop();

    /**
     * 判断游戏循环是否正在运行
     * @return 正在运行返回 true
     */
    bool isRunning() const;

    // ==================== 帧率信息 ====================

    /**
     * 设置目标帧率
     * @param fps 目标帧率，0 表示不限制
     */
    void setTargetFPS(int fps);

    /**
     * 获取目标帧率
     * @return 目标帧率
     */
    int getTargetFPS() const;

    /**
     * 获取实际的当前帧率（每秒帧数）
     * 通过计算最近一段时间的平均帧间隔得出
     * @return 当前帧率
     */
    float getCurrentFPS() const;

    /**
     * 获取上一帧的耗时
     * @return 上一帧耗时（秒）
     */
    float getLastDeltaTime() const;

private:
    EventHandler& mEventHandler;    // 事件处理器引用

    OnInitCallback mOnInit;         // 初始化回调
    OnUpdateCallback mOnUpdate;     // 更新回调
    OnRenderCallback mOnRender;     // 渲染回调
    OnCleanupCallback mOnCleanup;   // 清理回调

    int mTargetFPS;                 // 目标帧率
    float mFrameDelay;              // 每帧目标时间（秒），由 mTargetFPS 计算得出
    bool mRunning;                  // 循环是否正在运行

    float mCurrentFPS;              // 当前实际帧率
    float mLastDeltaTime;           // 上一帧耗时（秒）
};

#endif // GAMELOOP_H
