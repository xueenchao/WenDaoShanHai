/**
 * Scene.cpp - 场景管理系统的实现
 */

#include "Scene.h"
#include "EventHandler.h"
#include "Log.h"
#include <SDL3/SDL.h>

// ==================== Scene 基类 ====================

Scene::Scene(const std::string& name)
    : mName(name)
{
}

// ==================== SceneManager ====================

SceneManager::SceneManager(EventHandler& eventHandler)
    : mEventHandler(eventHandler)
    , mQuitRequested(false)
{
}

void SceneManager::pushScene(std::unique_ptr<Scene> scene)
{
    // 如果有当前场景，先调用其 onExit()
    if (!mSceneStack.empty()) {
        mSceneStack.back()->onExit();
    }

    mSceneStack.push_back(std::move(scene));

    // 调用新场景的 onEnter()
    if (!mSceneStack.empty()) {
        mSceneStack.back()->onEnter();
    }

    LOG_INFO("压入场景: %s (栈深度: %zu)",
            mSceneStack.back()->mName.c_str(), mSceneStack.size());
}

void SceneManager::popScene()
{
    if (mSceneStack.empty()) {
        LOG_INFO("场景栈为空，无法弹出");
        return;
    }

    std::string poppedName = mSceneStack.back()->mName;

    // 退出当前场景
    mSceneStack.back()->onExit();
    mSceneStack.pop_back();

    LOG_INFO("弹出场景: %s (栈深度: %zu)",
            poppedName.c_str(), mSceneStack.size());

    // 恢复上一个场景
    if (!mSceneStack.empty()) {
        mSceneStack.back()->onEnter();
        LOG_INFO("恢复场景: %s", mSceneStack.back()->mName.c_str());
    } else {
        // 栈空，请求退出
        LOG_INFO("场景栈已空，请求退出程序");
        mQuitRequested = true;
    }
}

void SceneManager::switchScene(std::unique_ptr<Scene> scene)
{
    if (mSceneStack.empty()) {
        // 栈为空，相当于 pushScene
        pushScene(std::move(scene));
        return;
    }

    std::string oldName = mSceneStack.back()->mName;

    // 退出旧场景
    mSceneStack.back()->onExit();
    mSceneStack.pop_back();

    // 进入新场景
    mSceneStack.push_back(std::move(scene));
    mSceneStack.back()->onEnter();

    LOG_INFO("切换场景: %s -> %s",
            oldName.c_str(), mSceneStack.back()->mName.c_str());
}

Scene* SceneManager::getActiveScene() const
{
    if (mSceneStack.empty()) {
        return nullptr;
    }
    return mSceneStack.back().get();
}

bool SceneManager::isEmpty() const
{
    return mSceneStack.empty();
}

void SceneManager::onUpdate(float deltaTime)
{
    if (mSceneStack.empty()) {
        return;
    }

    // 将事件分发给活跃场景
    // （EventHandler 已在 GameLoop::run() 中 pollEvents，这里处理场景级事件）
    Scene* active = mSceneStack.back().get();

    // 检查退出事件
    if (mEventHandler.isQuitRequested()) {
        mQuitRequested = true;
        return;
    }

    active->onUpdate(deltaTime);
}

void SceneManager::onRender(Renderer& renderer)
{
    if (mSceneStack.empty()) {
        return;
    }

    mSceneStack.back()->onRender(renderer);
}

void SceneManager::onCleanup()
{
    // 从栈顶到栈底依次退出
    while (!mSceneStack.empty()) {
        mSceneStack.back()->onExit();
        mSceneStack.pop_back();
    }
    LOG_INFO("所有场景已清理");
}
