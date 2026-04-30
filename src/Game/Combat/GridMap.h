/**
 * GridMap.h - 方形网格战斗地图
 *
 * 提供网格坐标与世界坐标转换、可行走性检测、BFS移动范围和寻路。
 * 每个格子可被一个单位占据（通过 mOccupiers 记录占据该格的单位ID）。
 */

#ifndef GRIDMAP_H
#define GRIDMAP_H

#include <cstdint>
#include <unordered_set>
#include <vector>

class GridMap {
public:
    /**
     * @param cols      网格列数
     * @param rows      网格行数
     * @param tileSize  每格像素大小（默认64）
     */
    GridMap(int cols, int rows, float tileSize = 64.0f);

    // ==================== 坐标转换 ====================

    /**
     * 将世界坐标转为网格坐标
     */
    void worldToGrid(float wx, float wy, int& gx, int& gy) const;

    /**
     * 将网格坐标转为世界坐标（格子左上角）
     */
    void gridToWorld(int gx, int gy, float& wx, float& wy) const;

    /**
     * 将网格坐标转为世界坐标（格子中心）
     */
    void gridToWorldCenter(int gx, int gy, float& wx, float& wy) const;

    // ==================== 占据管理 ====================

    /**
     * 设置格子的占据者
     * @param gx, gy  格子坐标
     * @param unitId  占据者ID（0表示清空）
     */
    void setOccupier(int gx, int gy, uint32_t unitId);

    /**
     * 获取格子的占据者ID
     */
    uint32_t getOccupier(int gx, int gy) const;

    /**
     * 移动单位：从旧位置清除，在新位置设置
     */
    void moveUnit(uint32_t unitId, int fromGX, int fromGY, int toGX, int toGY);

    // ==================== 格子查询 ====================

    bool isInBounds(int gx, int gy) const;
    bool isWalkable(int gx, int gy) const;
    int getDistance(int x1, int y1, int x2, int y2) const;

    /**
     * BFS计算移动范围
     * @param gx, gy    起始格子
     * @param movePoints 可用的移动力（每格消耗1点）
     * @return 可达的格子坐标列表
     */
    std::vector<std::pair<int, int>> getMoveRange(int gx, int gy, int movePoints) const;

    /**
     * BFS寻路，返回从起点到终点的最短路径
     * @return 路径的格子坐标列表（不含起点），无路径时返回空
     */
    std::vector<std::pair<int, int>> getPath(int fromX, int fromY, int toX, int toY) const;

    /**
     * 获取指定格子周围曼哈顿距离为range的所有格子
     */
    std::vector<std::pair<int, int>> getTilesInRange(int gx, int gy, int range) const;

    // ==================== 属性 ====================

    int getCols() const { return mCols; }
    int getRows() const { return mRows; }
    float getTileSize() const { return mTileSize; }
    float getWorldWidth() const { return mCols * mTileSize; }
    float getWorldHeight() const { return mRows * mTileSize; }

private:
    int mCols;
    int mRows;
    float mTileSize;

    // 占据数据：mOccupiers[row * mCols + col] = unitId (0 = empty)
    std::vector<uint32_t> mOccupiers;
};

#endif // GRIDMAP_H
