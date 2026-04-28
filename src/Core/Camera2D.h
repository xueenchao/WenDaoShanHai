/**
 * Camera2D.h - 2D 摄像机系统
 *
 * 提供 2D 场景的视口管理，支持位置移动、缩放、旋转、平滑跟随
 * 和边界限制。摄像机定义世界空间中的一个可见区域，通过
 * worldToScreen / screenToWorld 完成坐标变换。
 *
 * 使用方式：
 *   1. 创建摄像机构造时指定视口大小
 *   2. 调用 setPosition 定位摄像机
 *   3. 调用 follow 让摄像机平滑跟随目标
 *   4. 渲染时用 worldToScreen 将实体世界坐标转为屏幕坐标
 *   5. 鼠标交互时用 screenToWorld 将屏幕坐标转为世界坐标
 */

#ifndef CAMERA2D_H
#define CAMERA2D_H

#include <glm/glm.hpp>
#include "CollisionSystem.h"  // for AABB

class Renderer;

class Camera2D {
public:
    /**
     * 构造函数
     * @param viewportWidth  视口宽度（像素，通常是逻辑分辨率宽度）
     * @param viewportHeight 视口高度（像素）
     */
    Camera2D(float viewportWidth, float viewportHeight);

    // ==================== 位置 ====================

    /**
     * 设置摄像机中心在世界空间中的位置
     */
    void setPosition(float x, float y);
    void setPosition(const glm::vec2& pos);

    /**
     * 获取摄像机中心在世界空间中的位置
     */
    glm::vec2 getPosition() const;

    /**
     * 移动摄像机
     */
    void move(float dx, float dy);

    // ==================== 缩放 ====================

    /**
     * 设置缩放级别
     * @param zoom 缩放系数（1.0=原始大小，2.0=放大两倍，0.5=缩小一半）
     */
    void setZoom(float zoom);

    /**
     * 获取当前缩放级别
     */
    float getZoom() const;

    /**
     * 调整缩放（增量）
     * @param delta 缩放变化量
     */
    void zoomIn(float delta);

    // ==================== 旋转 ====================

    /**
     * 设置旋转角度
     * @param degrees 旋转角度（度）
     */
    void setRotation(float degrees);

    /**
     * 获取当前旋转角度
     */
    float getRotation() const;

    // ==================== 边界限制 ====================

    /**
     * 设置摄像机的世界空间边界
     * 摄像机中心将被限制在此范围内
     * @param minX 左边界
     * @param minY 上边界
     * @param maxX 右边界
     * @param maxY 下边界
     */
    void setBounds(float minX, float minY, float maxX, float maxY);

    /**
     * 清除边界限制
     */
    void clearBounds();

    // ==================== 跟随目标 ====================

    /**
     * 设置摄像机平滑跟随目标
     * @param target      目标位置指针（世界坐标），设为 nullptr 停止跟随
     * @param smoothSpeed 平滑速度（值越大跟随越快，5.0 较平滑，20.0 几乎即时）
     */
    void follow(const glm::vec2* target, float smoothSpeed = 5.0f);

    /**
     * 立即跳转到目标位置（无平滑）
     */
    void snapToTarget();

    /**
     * 停止跟随
     */
    void stopFollow();

    /**
     * 是否正在跟随目标
     */
    bool isFollowing() const;

    // ==================== 更新 ====================

    /**
     * 每帧更新（处理平滑跟随和边界夹紧）
     * @param deltaTime 距上一帧的时间（秒）
     */
    void update(float deltaTime);

    // ==================== 坐标变换 ====================

    /**
     * 将世界坐标转换为屏幕坐标
     * @param wx 世界 X 坐标
     * @param wy 世界 Y 坐标
     * @return 屏幕坐标
     */
    glm::vec2 worldToScreen(float wx, float wy) const;

    /**
     * 将屏幕坐标转换为世界坐标
     * @param sx 屏幕 X 坐标
     * @param sy 屏幕 Y 坐标
     * @return 世界坐标
     */
    glm::vec2 screenToWorld(float sx, float sy) const;

    // ==================== 视口信息 ====================

    /**
     * 获取摄像机在世界空间中的可见区域（AABB）
     */
    AABB getViewBounds() const;

    /**
     * 获取视口大小
     */
    glm::vec2 getViewportSize() const { return mViewportSize; }

private:
    /**
     * 将摄像机位置夹紧到边界内
     */
    void clampToBounds();

    glm::vec2 mPosition;         // 摄像机中心在世界空间中的位置
    float mZoom;                 // 缩放系数
    float mRotation;             // 旋转角度（度）
    glm::vec2 mViewportSize;     // 视口大小（像素）

    // 平滑跟随
    const glm::vec2* mFollowTarget;   // 跟随目标的位置指针
    float mSmoothSpeed;               // 平滑速度

    // 边界限制
    bool mHasBounds;
    glm::vec2 mBoundsMin;
    glm::vec2 mBoundsMax;
};

#endif // CAMERA2D_H
