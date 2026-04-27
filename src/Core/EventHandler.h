/**
 * EventHandler.h - 事件处理器类
 *
 * 封装 SDL3 的事件系统，提供统一的输入事件查询接口。
 * SDL3 的事件系统是游戏循环的核心，负责处理：
 *   - 窗口事件（关闭、大小改变、获得/失去焦点）
 *   - 键盘事件（按键按下、释放）
 *   - 鼠标事件（移动、点击、滚轮）
 *   - 游戏手柄事件
 *
 * 使用方式：每帧调用 pollEvents() 获取事件，
 *           然后通过 isKeyPressed() 等方法查询当前输入状态。
 */

#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <SDL3/SDL.h>
#include <unordered_map>

class EventHandler {
public:
    // ==================== 构造与析构 ====================

    EventHandler();

    // ==================== 事件轮询 ====================

    /**
     * 轮询所有待处理的事件
     * 必须在每帧的游戏循环中调用
     * 会自动更新内部状态（按键状态、鼠标位置等）
     * @return 如果收到退出事件（窗口关闭）返回 false，表示应该退出游戏循环；
     *         否则返回 true，表示继续运行
     */
    bool pollEvents();

    // ==================== 键盘输入 ====================

    /**
     * 判断某个按键当前是否被按下
     * @param scancode SDL 扫描码（如 SDL_SCANCODE_A、SDL_SCANCODE_ESCAPE）
     *                 扫描码基于键盘物理位置，不受输入法影响
     * @return 按下返回 true，未按下返回 false
     */
    bool isKeyPressed(SDL_Scancode scancode) const;

    /**
     * 判断某个按键是否刚刚被按下（本帧按下，上一帧未按下）
     * 适合处理单次触发的操作（如跳跃、确认等）
     * @param scancode SDL 扫描码
     * @return 刚按下返回 true
     */
    bool isKeyJustPressed(SDL_Scancode scancode) const;

    /**
     * 判断某个按键是否刚刚被释放（本帧释放，上一帧按下）
     * @param scancode SDL 扫描码
     * @return 刚释放返回 true
     */
    bool isKeyJustReleased(SDL_Scancode scancode) const;

    // ==================== 鼠标输入 ====================

    /**
     * 获取鼠标当前位置
     * @param x 输出：鼠标X坐标（相对于窗口左上角）
     * @param y 输出：鼠标Y坐标
     */
    void getMousePosition(float& x, float& y) const;

    /**
     * 判断鼠标某个按钮当前是否被按下
     * @param button 鼠标按钮：
     *   - SDL_BUTTON_LEFT:   左键
     *   - SDL_BUTTON_MIDDLE: 中键（滚轮按下）
     *   - SDL_BUTTON_RIGHT:  右键
     *   - SDL_BUTTON_X1:     侧键1
     *   - SDL_BUTTON_X2:     侧键2
     * @return 按下返回 true
     */
    bool isMouseButtonPressed(Uint8 button) const;

    /**
     * 判断鼠标某个按钮是否刚刚被按下
     * @param button 鼠标按钮
     * @return 刚按下返回 true
     */
    bool isMouseButtonJustPressed(Uint8 button) const;

    /**
     * 判断鼠标某个按钮是否刚刚被释放
     * @param button 鼠标按钮
     * @return 刚释放返回 true
     */
    bool isMouseButtonJustReleased(Uint8 button) const;

    /**
     * 获取鼠标滚轮的滚动量
     * @param x 输出：水平滚动量（正值向右，负值向左）
     * @param y 输出：垂直滚动量（正值向上，负值向下）
     */
    void getMouseWheel(float& x, float& y) const;

    // ==================== 窗口事件 ====================

    /**
     * 判断是否收到窗口关闭请求
     * @return 收到关闭请求返回 true
     */
    bool isQuitRequested() const;

    /**
     * 判断窗口是否刚刚改变大小
     * @return 大小改变返回 true
     */
    bool isWindowResized() const;

    // ==================== 状态更新 ====================

    /**
     * 在每帧结束时调用，更新"上一帧"的状态
     * 这是 isKeyJustPressed / isMouseButtonJustPressed 正常工作的关键
     * 必须在每帧处理完事件后、渲染前调用
     */
    void update();

private:
    /**
     * 处理单个 SDL 事件
     * @param event SDL 事件结构体
     */
    void handleEvent(const SDL_Event& event);

    // ---------- 键盘状态 ----------
    std::unordered_map<SDL_Scancode, bool> mCurrentKeys;    // 当前帧的按键状态
    std::unordered_map<SDL_Scancode, bool> mPreviousKeys;   // 上一帧的按键状态

    // ---------- 鼠标状态 ----------
    float mMouseX;                                          // 鼠标X坐标
    float mMouseY;                                          // 鼠标Y坐标
    std::unordered_map<Uint8, bool> mCurrentMouseButtons;   // 当前帧鼠标按钮状态
    std::unordered_map<Uint8, bool> mPreviousMouseButtons;  // 上一帧鼠标按钮状态
    float mMouseWheelX;                                     // 鼠标滚轮水平滚动量
    float mMouseWheelY;                                     // 鼠标滚轮垂直滚动量

    // ---------- 窗口事件状态 ----------
    bool mQuitRequested;    // 是否请求退出
    bool mWindowResized;    // 窗口是否改变大小
};

#endif // EVENTHANDLER_H
