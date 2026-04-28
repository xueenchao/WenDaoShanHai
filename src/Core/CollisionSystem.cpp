/**
 * CollisionSystem.cpp - 碰撞检测系统的实现
 *
 * 使用空间哈希网格进行 broad-phase 优化：
 * 将世界空间划分为固定大小的网格单元，碰撞体注册到覆盖的单元中。
 * 查询时只检查同一单元内的碰撞体，避免 O(n²) 的全量比较。
 */

#include "CollisionSystem.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

// ==================== 构造 ====================

CollisionSystem::CollisionSystem(float cellSize)
    : mCellSize(cellSize > 0.0f ? cellSize : 64.0f)
    , mNextId(1)
{
}

// ==================== 碰撞体管理 ====================

uint32_t CollisionSystem::addCollider(const AABB& bounds, uint32_t layer, uint32_t mask)
{
    uint32_t id = mNextId++;

    Collider collider;
    collider.id = id;
    collider.bounds = bounds;
    collider.layer = layer;
    collider.mask = mask;
    collider.active = true;

    size_t index = mColliders.size();
    mColliders.push_back(collider);
    mIdToIndex[id] = index;

    return id;
}

void CollisionSystem::removeCollider(uint32_t id)
{
    auto it = mIdToIndex.find(id);
    if (it == mIdToIndex.end()) {
        return;
    }

    size_t removedIndex = it->second;
    size_t lastIndex = mColliders.size() - 1;

    // 将最后一个元素移到被删除的位置（swap-and-pop）
    if (removedIndex != lastIndex) {
        mColliders[removedIndex] = mColliders[lastIndex];
        mIdToIndex[mColliders[removedIndex].id] = removedIndex;
    }

    mColliders.pop_back();
    mIdToIndex.erase(it);
}

void CollisionSystem::updateCollider(uint32_t id, const AABB& bounds)
{
    auto it = mIdToIndex.find(id);
    if (it == mIdToIndex.end()) {
        return;
    }

    mColliders[it->second].bounds = bounds;
}

void CollisionSystem::setActive(uint32_t id, bool active)
{
    auto it = mIdToIndex.find(id);
    if (it == mIdToIndex.end()) {
        return;
    }

    mColliders[it->second].active = active;
}

// ==================== 空间哈希 ====================

uint64_t CollisionSystem::hashCell(int cx, int cy) const
{
    // 使用简单的位组合：高 32 位存 X，低 32 位存 Y
    return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32)
         | static_cast<uint64_t>(static_cast<uint32_t>(cy));
}

void CollisionSystem::getCoveredCells(const AABB& aabb,
    int& minCX, int& minCY, int& maxCX, int& maxCY) const
{
    minCX = static_cast<int>(std::floor(aabb.x / mCellSize));
    minCY = static_cast<int>(std::floor(aabb.y / mCellSize));
    maxCX = static_cast<int>(std::floor((aabb.x + aabb.w) / mCellSize));
    maxCY = static_cast<int>(std::floor((aabb.y + aabb.h) / mCellSize));
}

void CollisionSystem::rebuild() const
{
    mGrid.clear();

    for (size_t i = 0; i < mColliders.size(); ++i) {
        if (!mColliders[i].active) {
            continue;
        }

        int minCX, minCY, maxCX, maxCY;
        getCoveredCells(mColliders[i].bounds, minCX, minCY, maxCX, maxCY);

        for (int cy = minCY; cy <= maxCY; ++cy) {
            for (int cx = minCX; cx <= maxCX; ++cx) {
                uint64_t key = hashCell(cx, cy);
                mGrid[key].push_back(i);
            }
        }
    }
}

void CollisionSystem::clear()
{
    mColliders.clear();
    mIdToIndex.clear();
    mGrid.clear();
    mNextId = 1;
}

const Collider* CollisionSystem::getCollider(uint32_t id) const
{
    auto it = mIdToIndex.find(id);
    if (it == mIdToIndex.end()) {
        return nullptr;
    }
    return &mColliders[it->second];
}

// ==================== 碰撞查询 ====================

std::vector<uint32_t> CollisionSystem::queryPoint(float x, float y, uint32_t mask) const
{
    std::vector<uint32_t> results;

    // 重建网格以保证查询准确性
    rebuild();

    int cx = static_cast<int>(std::floor(x / mCellSize));
    int cy = static_cast<int>(std::floor(y / mCellSize));
    uint64_t key = hashCell(cx, cy);

    auto it = mGrid.find(key);
    if (it == mGrid.end()) {
        return results;
    }

    for (size_t index : it->second) {
        const Collider& c = mColliders[index];
        if (!c.active) continue;
        if (!(c.layer & mask)) continue;
        if (c.bounds.contains(x, y)) {
            results.push_back(c.id);
        }
    }

    return results;
}

std::vector<uint32_t> CollisionSystem::queryAABB(const AABB& area, uint32_t mask) const
{
    std::vector<uint32_t> results;

    rebuild();

    int minCX, minCY, maxCX, maxCY;
    getCoveredCells(area, minCX, minCY, maxCX, maxCY);

    // 用于去重（同一碰撞体可能出现在多个网格单元中）
    std::vector<bool> checked(mColliders.size(), false);

    for (int cy = minCY; cy <= maxCY; ++cy) {
        for (int cx = minCX; cx <= maxCX; ++cx) {
            uint64_t key = hashCell(cx, cy);
            auto it = mGrid.find(key);
            if (it == mGrid.end()) {
                continue;
            }

            for (size_t index : it->second) {
                if (checked[index]) continue;
                checked[index] = true;

                const Collider& c = mColliders[index];
                if (!c.active) continue;
                if (!(c.layer & mask)) continue;
                if (c.bounds.overlaps(area)) {
                    results.push_back(c.id);
                }
            }
        }
    }

    return results;
}

std::vector<uint32_t> CollisionSystem::checkMove(uint32_t id, const AABB& newBounds, uint32_t extraMask) const
{
    std::vector<uint32_t> results;

    auto idIt = mIdToIndex.find(id);
    if (idIt == mIdToIndex.end()) {
        return results;
    }

    const Collider& mover = mColliders[idIt->second];
    if (!mover.active) {
        return results;
    }

    // 确定查询掩码：优先使用 extraMask，否则使用碰撞体自身的 mask
    uint32_t queryMask = (extraMask != 0) ? extraMask : mover.mask;

    rebuild();

    int minCX, minCY, maxCX, maxCY;
    getCoveredCells(newBounds, minCX, minCY, maxCX, maxCY);

    std::vector<bool> checked(mColliders.size(), false);
    // 排除自身
    checked[idIt->second] = true;

    for (int cy = minCY; cy <= maxCY; ++cy) {
        for (int cx = minCX; cx <= maxCX; ++cx) {
            uint64_t key = hashCell(cx, cy);
            auto it = mGrid.find(key);
            if (it == mGrid.end()) {
                continue;
            }

            for (size_t index : it->second) {
                if (checked[index]) continue;
                checked[index] = true;

                const Collider& c = mColliders[index];
                if (!c.active) continue;
                if (!(c.layer & queryMask)) continue;
                if (c.bounds.overlaps(newBounds)) {
                    results.push_back(c.id);
                }
            }
        }
    }

    return results;
}
