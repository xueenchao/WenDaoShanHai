/**
 * CollisionSystem.h - 碰撞检测系统
 *
 * 提供 AABB（轴对齐包围盒）碰撞检测，支持碰撞层位掩码过滤和
 * 空间哈希网格 broad-phase 优化。
 *
 * 碰撞层设计（位掩码，可组合）：
 *   - Player(1) | Enemy(2)  → 玩家与敌人碰撞
 *   - layer 决定"我是什么"，mask 决定"我能与什么碰撞"
 *
 * 使用方式：
 *   1. 创建 CollisionSystem 实例
 *   2. 为每个实体调用 addCollider 添加包围盒
 *   3. 实体移动时调用 updateCollider 更新位置
 *   4. 移动前调用 checkMove 判断是否会碰撞
 */

#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include <cstdint>
#include <unordered_map>
#include <vector>

// ==================== AABB 包围盒 ====================

struct AABB {
    float x, y;       // 左上角坐标
    float w, h;       // 宽高

    AABB() : x(0), y(0), w(0), h(0) {}
    AABB(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}

    /**
     * 判断两个 AABB 是否重叠
     */
    bool overlaps(const AABB& other) const
    {
        return x < other.x + other.w
            && x + w > other.x
            && y < other.y + other.h
            && y + h > other.y;
    }

    /**
     * 判断一个点是否在 AABB 内
     */
    bool contains(float px, float py) const
    {
        return px >= x && px <= x + w
            && py >= y && py <= y + h;
    }

    /**
     * 获取中心 X 坐标
     */
    float centerX() const { return x + w * 0.5f; }

    /**
     * 获取中心 Y 坐标
     */
    float centerY() const { return y + h * 0.5f; }
};

// ==================== 碰撞层定义 ====================

namespace CollisionLayer {
    constexpr uint32_t None       = 0;
    constexpr uint32_t Player     = 1 << 0;   // 玩家
    constexpr uint32_t Enemy      = 1 << 1;   // 敌人
    constexpr uint32_t Terrain    = 1 << 2;   // 地形/障碍物
    constexpr uint32_t Projectile = 1 << 3;   // 弹道/技能
    constexpr uint32_t Item       = 1 << 4;   // 掉落物品
    constexpr uint32_t Trigger    = 1 << 5;   // 触发器区域
    constexpr uint32_t All        = 0xFFFFFFFF;
}

// ==================== 碰撞体 ====================

struct Collider {
    uint32_t id;           // 唯一 ID
    AABB bounds;           // 包围盒
    uint32_t layer;        // 所属碰撞层
    uint32_t mask;         // 可与哪些层碰撞
    bool active;           // 是否激活
};

// ==================== 碰撞检测系统 ====================

class CollisionSystem {
public:
    /**
     * 构造函数
     * @param cellSize 空间哈希网格单元大小（默认 64 像素）
     */
    CollisionSystem(float cellSize = 64.0f);

    /**
     * 添加碰撞体
     * @param bounds 包围盒
     * @param layer  碰撞层（决定"我是谁"）
     * @param mask   碰撞掩码（决定"我能与谁碰撞"）
     * @return 碰撞体 ID（用于后续更新/删除）
     */
    uint32_t addCollider(const AABB& bounds, uint32_t layer, uint32_t mask);

    /**
     * 移除碰撞体
     * @param id 碰撞体 ID
     */
    void removeCollider(uint32_t id);

    /**
     * 更新碰撞体的包围盒位置
     * @param id     碰撞体 ID
     * @param bounds 新的包围盒
     */
    void updateCollider(uint32_t id, const AABB& bounds);

    /**
     * 设置碰撞体的激活状态
     * @param id     碰撞体 ID
     * @param active 是否激活
     */
    void setActive(uint32_t id, bool active);

    // ==================== 碰撞查询 ====================

    /**
     * 查询某个位置上所有重叠的碰撞体
     * @param x    查询点 X 坐标
     * @param y    查询点 Y 坐标
     * @param mask 碰撞掩码（只返回与此掩码匹配的碰撞体）
     * @return 命中的碰撞体 ID 列表
     */
    std::vector<uint32_t> queryPoint(float x, float y, uint32_t mask = CollisionLayer::All) const;

    /**
     * 查询与指定区域重叠的所有碰撞体
     * @param area 查询区域
     * @param mask 碰撞掩码
     * @return 命中的碰撞体 ID 列表
     */
    std::vector<uint32_t> queryAABB(const AABB& area, uint32_t mask = CollisionLayer::All) const;

    /**
     * 检查碰撞体移动到新位置是否会与其它碰撞体碰撞
     * 只检查 mask 中指定的层，且只返回 layer & extraMask 匹配的碰撞体
     * @param id        移动的碰撞体 ID
     * @param newBounds 移动后的包围盒
     * @param extraMask 额外过滤（0 表示使用碰撞体自身的 mask）
     * @return 碰撞到的碰撞体 ID 列表（不包含自身）
     */
    std::vector<uint32_t> checkMove(uint32_t id, const AABB& newBounds, uint32_t extraMask = 0) const;

    /**
     * 重建空间哈希网格（通常在大量更新后调用）
     */
    void rebuild() const;

    /**
     * 清空所有碰撞体
     */
    void clear();

    /**
     * 获取碰撞体
     * @param id 碰撞体 ID
     * @return 碰撞体指针，不存在返回 nullptr
     */
    const Collider* getCollider(uint32_t id) const;

    /**
     * 获取碰撞体总数
     */
    size_t getColliderCount() const { return mColliders.size(); }

private:
    /**
     * 计算网格单元的哈希键
     */
    uint64_t hashCell(int cx, int cy) const;

    /**
     * 获取 AABB 覆盖的网格单元范围
     */
    void getCoveredCells(const AABB& aabb, int& minCX, int& minCY, int& maxCX, int& maxCY) const;

    std::vector<Collider> mColliders;                             // 碰撞体数组
    std::unordered_map<uint32_t, size_t> mIdToIndex;              // ID → 数组索引

    // 空间哈希网格：gridKey → 碰撞体索引列表
    mutable std::unordered_map<uint64_t, std::vector<size_t>> mGrid;
    float mCellSize;
    uint32_t mNextId;
};

#endif // COLLISIONSYSTEM_H
