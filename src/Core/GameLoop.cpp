/**
 * GameLoop.cpp - 游戏主循环类的实现
 *
 * 实现游戏的核心循环逻辑，包含帧率控制和时间管理。
 */

#include "GameLoop.h"
#include "EventHandler.h"

// ==================== 构造与析构 ====================

GameLoop::GameLoop(EventHandler& eventHandler, int targetFPS)
    : mEventHandler(eventHandler)
    , mOnInit(nullptr)
    , mOnUpdate(nullptr)
    , mOnRender(nullptr)
    , mOnCleanup(nullptr)
    , mTargetFPS(targetFPS)
    , mRunning(false)
    , mCurrentFPS(0.0f)
    , mLastDeltaTime(0.0f)
{
    // 计算每帧的目标时间间隔（秒）
    // 例如 60 FPS → 每帧 1/60 ≈ 0.0167 秒
    if (mTargetFPS > 0) {
        mFrameDelay = 1.0f / static_cast<float>(mTargetFPS);
    } else {
        mFrameDelay = 0.0f;  // 不限制帧率
    }
}

// ==================== 回调设置 ====================

void GameLoop::setOnInit(OnInitCallback callback)
{
    mOnInit = callback;
}

void GameLoop::setOnUpdate(OnUpdateCallback callback)
{
    mOnUpdate = callback;
}

void GameLoop::setOnRender(OnRenderCallback callback)
{
    mOnRender = callback;
}

void GameLoop::setOnCleanup(OnCleanupCallback callback)
{
    mOnCleanup = callback;
}

// ==================== 循环控制 ====================

bool GameLoop::run()
{
    mRunning = true;

    // ========== 阶段一：初始化 ==========
    if (mOnInit) {
        if (!mOnInit()) {
            // 初始化失败，终止循环
            mRunning = false;
            return false;
        }
    }

    // 用于计算帧率和 deltaTime 的时间变量
    Uint64 previousTime = SDL_GetTicks();  // 上一帧的时间戳（毫秒）
    float fpsUpdateTimer = 0.0f;           // FPS 更新计时器
    int frameCount = 0;                    // 帧计数器

    // ========== 阶段二：主循环 ==========
    while (mRunning) {
        // 计算当前帧的时间戳
        Uint64 currentTime = SDL_GetTicks();

        // deltaTime = 当前帧与上一帧的时间差（转换为秒）
        mLastDeltaTime = static_cast<float>(currentTime - previousTime) / 1000.0f;

        // 防止 deltaTime 过大（例如窗口拖拽导致长时间暂停）
        // 如果 deltaTime 超过 0.1 秒，强制限制为 0.1 秒
        // 这样可以避免游戏对象在一帧内移动过大距离
        if (mLastDeltaTime > 0.1f) {
            mLastDeltaTime = 0.1f;
        }

        previousTime = currentTime;

        // ----- 步骤1：处理输入事件 -----
        // pollEvents 返回 false 表示收到退出事件
        if (!mEventHandler.pollEvents()) {
            mRunning = false;
            break;
        }

        // ----- 步骤2：更新游戏逻辑 -----
        if (mOnUpdate) {
            mOnUpdate(mLastDeltaTime);
        }

        // ----- 步骤3：渲染画面 -----
        if (mOnRender) {
            mOnRender();
        }

        // ----- 步骤4：更新事件状态 -----
        // 必须在帧的末尾调用，确保 isKeyJustPressed 等方法下一帧能正确工作
        mEventHandler.update();

        // ----- 步骤5：帧率控制 -----
        if (mTargetFPS > 0 && mFrameDelay > 0.0f) {
            // 计算本帧已用时间
            Uint64 frameEndTime = SDL_GetTicks();
            float frameElapsed = static_cast<float>(frameEndTime - currentTime) / 1000.0f;

            // 如果本帧完成得比目标时间快，则等待剩余时间
            float waitTime = mFrameDelay - frameElapsed;
            if (waitTime > 0.0f) {
                // SDL_Delay 让出 CPU 时间片，避免空转浪费电量和发热
                // 注意：SDL_Delay 的精度有限（通常 1-10ms），
                //       对于精确的帧率控制可能不够，但对大多数游戏足够
                SDL_Delay(static_cast<Uint32>(waitTime * 1000.0f));
            }
        }

        // ----- 步骤6：更新帧率统计 -----
        frameCount++;
        fpsUpdateTimer += mLastDeltaTime;

        // 每 0.5 秒更新一次 FPS 显示值
        if (fpsUpdateTimer >= 0.5f) {
            mCurrentFPS = static_cast<float>(frameCount) / fpsUpdateTimer;
            frameCount = 0;
            fpsUpdateTimer = 0.0f;
        }
    }

    // ========== 阶段三：清理 ==========
    if (mOnCleanup) {
        mOnCleanup();
    }

    mRunning = false;
    return true;
}

void GameLoop::stop()
{
    mRunning = false;
}

bool GameLoop::isRunning() const
{
    return mRunning;
}

// ==================== 帧率信息 ====================

void GameLoop::setTargetFPS(int fps)
{
    mTargetFPS = fps;
    if (mTargetFPS > 0) {
        mFrameDelay = 1.0f / static_cast<float>(mTargetFPS);
    } else {
        mFrameDelay = 0.0f;
    }
}

int GameLoop::getTargetFPS() const
{
    return mTargetFPS;
}

float GameLoop::getCurrentFPS() const
{
    return mCurrentFPS;
}

float GameLoop::getLastDeltaTime() const
{
    return mLastDeltaTime;
}
