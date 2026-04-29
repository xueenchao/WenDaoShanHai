/**
 * Scene.h - 场景管理系统
 *
 * 提供场景基类和场景管理器。场景管理器使用栈结构管理场景的切换与生命周期，
 * 支持场景的压入（暂停当前场景）、弹出（恢复上一场景）和切换（替换当前场景）。
 *
 * 使用方式：
 *   1. 继承 Scene 实现自定义场景
 *   2. 通过 SceneManager 管理场景切换
 *   3. 将 SceneManager 的 onUpdate/onRender 绑定到 GameLoop 回调
 */

#ifndef SCENE_H
#define SCENE_H

#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <vector>

class Renderer;
class EventHandler;

// ==================== 场景基类 ====================

class Scene {
public:
    /**
     * 场景名称，用于调试和日志
     */
    std::string mName;

    Scene(const std::string& name = "未命名场景");
    virtual ~Scene() = default;

    // 禁用拷贝
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    /**
     * 进入场景时调用（激活时）
     */
    virtual void onEnter() {}

    /**
     * 离开场景时调用（暂停或销毁时）
     */
    virtual void onExit() {}

    /**
     * 每帧更新
     * @param deltaTime 距上一帧的时间（秒）
     */
    virtual void onUpdate(float deltaTime) = 0;

    /**
     * 每帧渲染
     * @param renderer 渲染器引用
     */
    virtual void onRender(Renderer& renderer) = 0;

    /**
     * 处理 SDL 事件（可选覆写）
     * @param event SDL 事件
     */
    virtual void onEvent(const SDL_Event& event) {}
};

// ==================== 场景管理器 ====================

class SceneManager {
public:
    /**
     * 构造函数
     * @param eventHandler 事件处理器引用，用于传递事件给活跃场景
     */
    SceneManager(EventHandler& eventHandler);

    ~SceneManager() = default;

    // 禁用拷贝
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    /**
     * 压入新场景，当前场景被暂停（保留在栈中，不销毁）
     * @param scene 要压入的场景
     */
    void pushScene(std::unique_ptr<Scene> scene);

    /**
     * 弹出当前场景，恢复上一个场景
     * 如果栈为空，程序将退出
     */
    void popScene();

    /**
     * 切换场景：弹出当前场景，压入新场景
     * @param scene 新场景
     */
    void switchScene(std::unique_ptr<Scene> scene);

    /**
     * 获取当前活跃场景
     * @return 活跃场景指针，栈为空时返回 nullptr
     */
    Scene* getActiveScene() const;

    /**
     * 场景栈是否为空
     */
    bool isEmpty() const;

    // ==================== 生命周期（供 GameLoop 回调绑定） ====================

    /**
     * 每帧更新：先处理事件，再更新活跃场景
     * @param deltaTime 距上一帧的时间（秒）
     */
    void onUpdate(float deltaTime);

    /**
     * 每帧渲染活跃场景
     * @param renderer 渲染器引用
     */
    void onRender(Renderer& renderer);

    /**
     * 清理所有场景
     */
    void onCleanup();

    /**
     * 判断是否应该退出程序
     */
    bool isQuitRequested() const { return mQuitRequested; }

    /**
     * 请求延迟弹出当前场景（安全，在onUpdate返回后执行）
     */
    void requestPopScene() { mPendingPop = true; }

private:
    EventHandler& mEventHandler;
    std::vector<std::unique_ptr<Scene>> mSceneStack;
    bool mQuitRequested;
    bool mPendingPop = false;
};

#endif // SCENE_H
