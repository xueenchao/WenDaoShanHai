/**
 * EventHandler.cpp - 事件处理器类的实现
 *
 * 封装 SDL3 事件系统，提供按键/鼠标的状态查询。
 * 核心思路：用两个状态表（当前帧和上一帧）来实现
 * "刚按下"和"刚释放"的检测。
 */

#include "EventHandler.h"

// ==================== 构造与析构 ====================

EventHandler::EventHandler()
    : mMouseX(0.0f)
    , mMouseY(0.0f)
    , mMouseWheelX(0.0f)
    , mMouseWheelY(0.0f)
    , mQuitRequested(false)
    , mWindowResized(false)
{
}

// ==================== 事件轮询 ====================

bool EventHandler::pollEvents()
{
    // 每帧开始时重置一次性事件标志
    mQuitRequested = false;
    mWindowResized = false;
    mMouseWheelX = 0.0f;
    mMouseWheelY = 0.0f;

    SDL_Event event;

    // SDL_PollEvent 从事件队列中取出一个事件
    // 返回 true 表示有事件，false 表示队列为空
    while (SDL_PollEvent(&event)) {
        handleEvent(event);
    }

    // 使用 SDL_GetKeyboardState 获取实时键盘状态，同步到 mCurrentKeys
    // 这比逐个事件记录更准确，因为事件可能在两帧之间快速切换
    int keyCount = 0;
    const bool* keyStates = SDL_GetKeyboardState(&keyCount);

    for (int i = 0; i < keyCount; ++i) {
        mCurrentKeys[static_cast<SDL_Scancode>(i)] = keyStates[i];
    }

    // 使用 SDL_GetMouseState 获取实时鼠标状态
    Uint32 mouseState = SDL_GetMouseState(&mMouseX, &mMouseY);

    // 同步鼠标按钮状态到 mCurrentMouseButtons
    mCurrentMouseButtons[SDL_BUTTON_LEFT]   = (mouseState & SDL_BUTTON_LMASK) != 0;
    mCurrentMouseButtons[SDL_BUTTON_MIDDLE] = (mouseState & SDL_BUTTON_MMASK) != 0;
    mCurrentMouseButtons[SDL_BUTTON_RIGHT]  = (mouseState & SDL_BUTTON_RMASK) != 0;
    mCurrentMouseButtons[SDL_BUTTON_X1]     = (mouseState & SDL_BUTTON_X1MASK) != 0;
    mCurrentMouseButtons[SDL_BUTTON_X2]     = (mouseState & SDL_BUTTON_X2MASK) != 0;

    // 如果收到退出事件，返回 false 通知游戏循环退出
    return !mQuitRequested;
}

// ==================== 键盘输入 ====================

bool EventHandler::isKeyPressed(SDL_Scancode scancode) const
{
    // 查找当前帧状态，默认为 false（未按下）
    auto it = mCurrentKeys.find(scancode);
    if (it != mCurrentKeys.end()) {
        return it->second;
    }
    return false;
}

bool EventHandler::isKeyJustPressed(SDL_Scancode scancode) const
{
    // "刚按下" = 当前帧按下 AND 上一帧未按下
    bool current = false;
    bool previous = false;

    auto itCurr = mCurrentKeys.find(scancode);
    if (itCurr != mCurrentKeys.end()) {
        current = itCurr->second;
    }

    auto itPrev = mPreviousKeys.find(scancode);
    if (itPrev != mPreviousKeys.end()) {
        previous = itPrev->second;
    }

    return current && !previous;
}

bool EventHandler::isKeyJustReleased(SDL_Scancode scancode) const
{
    // "刚释放" = 当前帧未按下 AND 上一帧按下
    bool current = false;
    bool previous = false;

    auto itCurr = mCurrentKeys.find(scancode);
    if (itCurr != mCurrentKeys.end()) {
        current = itCurr->second;
    }

    auto itPrev = mPreviousKeys.find(scancode);
    if (itPrev != mPreviousKeys.end()) {
        previous = itPrev->second;
    }

    return !current && previous;
}

// ==================== 鼠标输入 ====================

void EventHandler::getMousePosition(float& x, float& y) const
{
    x = mMouseX;
    y = mMouseY;
}

bool EventHandler::isMouseButtonPressed(Uint8 button) const
{
    auto it = mCurrentMouseButtons.find(button);
    if (it != mCurrentMouseButtons.end()) {
        return it->second;
    }
    return false;
}

bool EventHandler::isMouseButtonJustPressed(Uint8 button) const
{
    bool current = false;
    bool previous = false;

    auto itCurr = mCurrentMouseButtons.find(button);
    if (itCurr != mCurrentMouseButtons.end()) {
        current = itCurr->second;
    }

    auto itPrev = mPreviousMouseButtons.find(button);
    if (itPrev != mPreviousMouseButtons.end()) {
        previous = itPrev->second;
    }

    return current && !previous;
}

bool EventHandler::isMouseButtonJustReleased(Uint8 button) const
{
    bool current = false;
    bool previous = false;

    auto itCurr = mCurrentMouseButtons.find(button);
    if (itCurr != mCurrentMouseButtons.end()) {
        current = itCurr->second;
    }

    auto itPrev = mPreviousMouseButtons.find(button);
    if (itPrev != mPreviousMouseButtons.end()) {
        previous = itPrev->second;
    }

    return !current && previous;
}

void EventHandler::getMouseWheel(float& x, float& y) const
{
    x = mMouseWheelX;
    y = mMouseWheelY;
}

// ==================== 窗口事件 ====================

bool EventHandler::isQuitRequested() const
{
    return mQuitRequested;
}

bool EventHandler::isWindowResized() const
{
    return mWindowResized;
}

// ==================== 状态更新 ====================

void EventHandler::update()
{
    // 将当前帧状态复制到"上一帧"状态表
    // 这必须在每帧结束时调用，否则 isKeyJustPressed 等方法无法正确工作
    mPreviousKeys = mCurrentKeys;
    mPreviousMouseButtons = mCurrentMouseButtons;
}

// ==================== 内部方法 ====================

void EventHandler::handleEvent(const SDL_Event& event)
{
    switch (event.type) {
    // ---------- 窗口事件 ----------
    case SDL_EVENT_QUIT:
        // 用户点击窗口关闭按钮，或系统发送退出信号
        mQuitRequested = true;
        break;

    case SDL_EVENT_WINDOW_RESIZED:
        // 窗口大小改变（用户拖拽边框、最大化等）
        mWindowResized = true;
        break;

    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        // 窗口关闭请求（macOS 的 Cmd+Q 等）
        mQuitRequested = true;
        break;

    // ---------- 鼠标滚轮事件 ----------
    case SDL_EVENT_MOUSE_WHEEL:
        // 鼠标滚轮滚动
        mMouseWheelX = event.wheel.x;
        mMouseWheelY = event.wheel.y;
        break;

    // ---------- 键盘事件 ----------
    // 注意：键盘的实时状态通过 SDL_GetKeyboardState 获取，
    // 这里只记录事件日志（如需要可扩展）

    default:
        // 其他事件暂不处理
        break;
    }
}
