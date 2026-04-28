/**
 * UIElement.h - UI元素基类
 *
 * 所有UI控件的基类，提供位置、大小、父子层级、可见性等基础功能。
 * UI坐标使用屏幕空间（像素），与Camera2D世界坐标解耦。
 */

#ifndef UIELEMENT_H
#define UIELEMENT_H

#include <SDL3/SDL.h>
#include <functional>
#include <memory>
#include <vector>

class Renderer;
class Font;

class UIElement {
public:
    UIElement();
    virtual ~UIElement() = default;

    // 禁用拷贝
    UIElement(const UIElement&) = delete;
    UIElement& operator=(const UIElement&) = delete;

    // ==================== 位置与大小 ====================

    float mX, mY;
    float mWidth, mHeight;

    /**
     * 获取元素在屏幕空间中的绝对位置
     */
    void getWorldPosition(float& wx, float& wy) const;

    /**
     * 判断屏幕坐标是否在元素内
     */
    virtual bool containsPoint(float px, float py) const;

    // ==================== 层级 ====================

    UIElement* mParent;
    std::vector<std::unique_ptr<UIElement>> mChildren;

    void addChild(std::unique_ptr<UIElement> child);

    /**
     * 查找点中最深的子元素（用于输入命中测试）
     * @return 命中的元素指针，没有命中返回 nullptr
     */
    UIElement* hitTest(float px, float py);

    // ==================== 状态 ====================

    bool mVisible;
    bool mEnabled;

    // ==================== 虚方法 ====================

    virtual void onRender(Renderer& renderer, Font* font) {}
    virtual void onMouseEnter() {}
    virtual void onMouseLeave() {}
    virtual void onMouseDown(float px, float py) { (void)px; (void)py; }
    virtual void onMouseUp(float px, float py) { (void)px; (void)py; }
};

#endif // UIELEMENT_H
