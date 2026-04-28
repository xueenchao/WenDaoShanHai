#include "GridMap.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>

// ==================== 构造 ====================

GridMap::GridMap(int cols, int rows, float tileSize)
    : mCols(cols), mRows(rows), mTileSize(tileSize)
    , mOccupiers(cols * rows, 0)
{
}

// ==================== 坐标转换 ====================

void GridMap::worldToGrid(float wx, float wy, int& gx, int& gy) const
{
    gx = static_cast<int>(std::floor(wx / mTileSize));
    gy = static_cast<int>(std::floor(wy / mTileSize));
}

void GridMap::gridToWorld(int gx, int gy, float& wx, float& wy) const
{
    wx = static_cast<float>(gx) * mTileSize;
    wy = static_cast<float>(gy) * mTileSize;
}

void GridMap::gridToWorldCenter(int gx, int gy, float& wx, float& wy) const
{
    wx = (static_cast<float>(gx) + 0.5f) * mTileSize;
    wy = (static_cast<float>(gy) + 0.5f) * mTileSize;
}

// ==================== 占据管理 ====================

void GridMap::setOccupier(int gx, int gy, uint32_t unitId)
{
    if (!isInBounds(gx, gy)) return;
    mOccupiers[gy * mCols + gx] = unitId;
}

uint32_t GridMap::getOccupier(int gx, int gy) const
{
    if (!isInBounds(gx, gy)) return 0;
    return mOccupiers[gy * mCols + gx];
}

void GridMap::moveUnit(uint32_t unitId, int fromGX, int fromGY, int toGX, int toGY)
{
    setOccupier(fromGX, fromGY, 0);
    setOccupier(toGX, toGY, unitId);
}

// ==================== 格子查询 ====================

bool GridMap::isInBounds(int gx, int gy) const
{
    return gx >= 0 && gx < mCols && gy >= 0 && gy < mRows;
}

bool GridMap::isWalkable(int gx, int gy) const
{
    return isInBounds(gx, gy) && getOccupier(gx, gy) == 0;
}

int GridMap::getDistance(int x1, int y1, int x2, int y2) const
{
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}

// ==================== BFS 移动范围 ====================

std::vector<std::pair<int, int>> GridMap::getMoveRange(int gx, int gy, int movePoints) const
{
    std::vector<std::pair<int, int>> result;
    if (!isInBounds(gx, gy)) return result;

    // BFS: (x, y, remaining)
    std::queue<std::tuple<int, int, int>> q;
    std::unordered_set<int> visited;

    // 用 int 编码坐标作为 visited key
    auto encode = [this](int x, int y) { return y * mCols + x; };

    q.push({gx, gy, movePoints});
    visited.insert(encode(gx, gy));

    const int dx[] = {0, 0, -1, 1};
    const int dy[] = {-1, 1, 0, 0};

    while (!q.empty()) {
        auto [cx, cy, remaining] = q.front();
        q.pop();

        if (remaining <= 0) continue;

        for (int i = 0; i < 4; ++i) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];

            if (!isInBounds(nx, ny)) continue;
            if (!isWalkable(nx, ny)) continue;

            int key = encode(nx, ny);
            if (visited.count(key)) continue;

            visited.insert(key);
            result.push_back({nx, ny});
            q.push({nx, ny, remaining - 1});
        }
    }

    return result;
}

// ==================== BFS 寻路 ====================

std::vector<std::pair<int, int>> GridMap::getPath(int fromX, int fromY, int toX, int toY) const
{
    std::vector<std::pair<int, int>> path;
    if (!isInBounds(fromX, fromY) || !isInBounds(toX, toY)) return path;
    if (fromX == toX && fromY == toY) return path;

    auto encode = [this](int x, int y) { return y * mCols + x; };

    std::queue<std::pair<int, int>> q;
    std::unordered_map<int, std::pair<int, int>> cameFrom;

    int startKey = encode(fromX, fromY);
    int endKey = encode(toX, toY);

    q.push({fromX, fromY});
    cameFrom[startKey] = {-1, -1};

    const int dx[] = {0, 0, -1, 1};
    const int dy[] = {-1, 1, 0, 0};

    bool found = false;
    while (!q.empty()) {
        auto [cx, cy] = q.front();
        q.pop();

        if (cx == toX && cy == toY) {
            found = true;
            break;
        }

        for (int i = 0; i < 4; ++i) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];

            if (!isInBounds(nx, ny)) continue;
            // 目标格允许被占据（我们要走到那个格的旁边）
            if (!isWalkable(nx, ny) && !(nx == toX && ny == toY)) continue;

            int key = encode(nx, ny);
            if (cameFrom.count(key)) continue;

            cameFrom[key] = {cx, cy};
            q.push({nx, ny});
        }
    }

    if (!found) return path;

    // 回溯路径
    int cx = toX, cy = toY;
    while (true) {
        auto it = cameFrom.find(encode(cx, cy));
        if (it == cameFrom.end() || (it->second.first == -1 && it->second.second == -1)) break;

        path.push_back({cx, cy});
        cx = it->second.first;
        cy = it->second.second;
    }

    std::reverse(path.begin(), path.end());
    return path;
}

// ==================== 范围查询 ====================

std::vector<std::pair<int, int>> GridMap::getTilesInRange(int gx, int gy, int range) const
{
    std::vector<std::pair<int, int>> result;

    for (int dx = -range; dx <= range; ++dx) {
        for (int dy = -range; dy <= range; ++dy) {
            if (std::abs(dx) + std::abs(dy) > range) continue;  // 曼哈顿距离
            int nx = gx + dx;
            int ny = gy + dy;
            if (isInBounds(nx, ny) && (dx != 0 || dy != 0)) {
                result.push_back({nx, ny});
            }
        }
    }

    return result;
}
