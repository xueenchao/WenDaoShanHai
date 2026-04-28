/**
 * Camera2D.cpp - 2D 摄像机系统的实现
 *
 * 坐标变换公式：
 *   screen = (world - cameraPosition) * zoom + viewportCenter
 *   world  = (screen - viewportCenter) / zoom + cameraPosition
 */

#include "Camera2D.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <algorithm>

// ==================== 构造 ====================

Camera2D::Camera2D(float viewportWidth, float viewportHeight)
    : mPosition(0.0f, 0.0f)
    , mZoom(1.0f)
    , mRotation(0.0f)
    , mViewportSize(viewportWidth, viewportHeight)
    , mFollowTarget(nullptr)
    , mSmoothSpeed(5.0f)
    , mHasBounds(false)
    , mBoundsMin(0.0f, 0.0f)
    , mBoundsMax(0.0f, 0.0f)
{
}

// ==================== 位置 ====================

void Camera2D::setPosition(float x, float y)
{
    mPosition.x = x;
    mPosition.y = y;
    clampToBounds();
}

void Camera2D::setPosition(const glm::vec2& pos)
{
    setPosition(pos.x, pos.y);
}

glm::vec2 Camera2D::getPosition() const
{
    return mPosition;
}

void Camera2D::move(float dx, float dy)
{
    mPosition.x += dx;
    mPosition.y += dy;
    clampToBounds();
}

// ==================== 缩放 ====================

void Camera2D::setZoom(float zoom)
{
    // 限制缩放范围，避免极端值
    mZoom = std::max(0.1f, std::min(zoom, 10.0f));
    clampToBounds();
}

float Camera2D::getZoom() const
{
    return mZoom;
}

void Camera2D::zoomIn(float delta)
{
    setZoom(mZoom + delta);
}

// ==================== 旋转 ====================

void Camera2D::setRotation(float degrees)
{
    mRotation = degrees;
}

float Camera2D::getRotation() const
{
    return mRotation;
}

// ==================== 边界限制 ====================

void Camera2D::setBounds(float minX, float minY, float maxX, float maxY)
{
    mHasBounds = true;
    mBoundsMin.x = minX;
    mBoundsMin.y = minY;
    mBoundsMax.x = maxX;
    mBoundsMax.y = maxY;
    clampToBounds();
}

void Camera2D::clearBounds()
{
    mHasBounds = false;
}

void Camera2D::clampToBounds()
{
    if (!mHasBounds) {
        return;
    }

    // 摄像机可见区域的半宽半高（世界空间）
    float halfW = mViewportSize.x / (2.0f * mZoom);
    float halfH = mViewportSize.y / (2.0f * mZoom);

    // 摄像机中心不能超出边界 + 半视口
    float minCamX = mBoundsMin.x + halfW;
    float minCamY = mBoundsMin.y + halfH;
    float maxCamX = mBoundsMax.x - halfW;
    float maxCamY = mBoundsMax.y - halfH;

    // 如果边界范围小于视口大小，则居中
    if (maxCamX < minCamX) {
        mPosition.x = (mBoundsMin.x + mBoundsMax.x) * 0.5f;
    } else {
        mPosition.x = std::max(minCamX, std::min(mPosition.x, maxCamX));
    }

    if (maxCamY < minCamY) {
        mPosition.y = (mBoundsMin.y + mBoundsMax.y) * 0.5f;
    } else {
        mPosition.y = std::max(minCamY, std::min(mPosition.y, maxCamY));
    }
}

// ==================== 跟随目标 ====================

void Camera2D::follow(const glm::vec2* target, float smoothSpeed)
{
    mFollowTarget = target;
    mSmoothSpeed = smoothSpeed;
}

void Camera2D::snapToTarget()
{
    if (mFollowTarget != nullptr) {
        mPosition = *mFollowTarget;
        clampToBounds();
    }
}

void Camera2D::stopFollow()
{
    mFollowTarget = nullptr;
}

bool Camera2D::isFollowing() const
{
    return mFollowTarget != nullptr;
}

// ==================== 更新 ====================

void Camera2D::update(float deltaTime)
{
    if (mFollowTarget == nullptr) {
        return;
    }

    // 平滑跟随：使用指数衰减插值
    // lerp factor = 1 - e^(-smoothSpeed * deltaTime)
    // 这保证了与帧率无关的平滑效果
    float t = 1.0f - std::exp(-mSmoothSpeed * deltaTime);
    t = std::min(t, 1.0f);  // 防止 deltaTime 过大导致 overshoot

    mPosition = mPosition + (*mFollowTarget - mPosition) * t;

    clampToBounds();
}

// ==================== 坐标变换 ====================

glm::vec2 Camera2D::worldToScreen(float wx, float wy) const
{
    // 视口中心（屏幕空间）
    glm::vec2 viewportCenter = mViewportSize * 0.5f;

    // 世界坐标相对于摄像机中心的偏移，乘以缩放，加上视口中心
    float sx = (wx - mPosition.x) * mZoom + viewportCenter.x;
    float sy = (wy - mPosition.y) * mZoom + viewportCenter.y;

    return glm::vec2(sx, sy);
}

glm::vec2 Camera2D::screenToWorld(float sx, float sy) const
{
    glm::vec2 viewportCenter = mViewportSize * 0.5f;

    float wx = (sx - viewportCenter.x) / mZoom + mPosition.x;
    float wy = (sy - viewportCenter.y) / mZoom + mPosition.y;

    return glm::vec2(wx, wy);
}

// ==================== 视口信息 ====================

AABB Camera2D::getViewBounds() const
{
    float halfW = mViewportSize.x / (2.0f * mZoom);
    float halfH = mViewportSize.y / (2.0f * mZoom);

    return AABB(
        mPosition.x - halfW,
        mPosition.y - halfH,
        halfW * 2.0f,
        halfH * 2.0f
    );
}
