/**
 * UIManager.h - UI管理器
 *
 * 管理UI元素树的根节点，负责输入事件路由和渲染调度。
 * 使用聚合输入模型（配合EventHandler的状态查询），不直接处理SDL_Event。
 */

#ifndef UIMANAGER_H
#define UIMANAGER_H

#include "UIElement.h"
#include <memory>

class Renderer;
class Font;

class UIManager {
public:
    UIManager(Font* font);
    ~UIManager() = default;

    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;

    void setRenderer(Renderer* renderer) { mRenderer = renderer; }

    /**
     * 渲染整棵UI树
     */
    void render();

    /**
     * 命中测试：返回指定坐标处最深层的可见且启用的元素
     * @return 命中的元素指针，未命中返回 nullptr
     */
    UIElement* getElementAt(float mx, float my);

    /**
     * 更新鼠标悬停状态
     */
    void updateHover(float mx, float my);

    /**
     * 处理鼠标按下（只对当前悬停元素生效）
     */
    void handleMouseDown(float mx, float my);

    /**
     * 处理鼠标释放（触发按钮回调等）
     */
    void handleMouseUp(float mx, float my);

    /**
     * 添加顶层元素
     */
    void addElement(std::unique_ptr<UIElement> element);

    /**
     * 获取根元素
     */
    UIElement* getRoot() { return mRoot.get(); }

    /**
     * 清空所有UI元素
     */
    void clear();

private:
    Renderer* mRenderer;
    Font* mFont;

    std::unique_ptr<UIElement> mRoot;
    UIElement* mHoveredElement;
    UIElement* mPressedElement;
};

#endif // UIMANAGER_H
