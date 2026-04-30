/**
 * WorldMap.cpp - 大世界地图实现
 */

#include "WorldMap.h"

WorldMap::WorldMap(int cols, int rows, float tileSize)
    : mCols(cols)
    , mRows(rows)
    , mTileSize(tileSize)
    , mTiles(cols * rows)
{
    generateDemoMap();
}

void WorldMap::worldToGrid(float wx, float wy, int& gx, int& gy) const
{
    gx = static_cast<int>(wx / mTileSize);
    gy = static_cast<int>(wy / mTileSize);
}

void WorldMap::gridToWorld(int gx, int gy, float& wx, float& wy) const
{
    wx = static_cast<float>(gx) * mTileSize;
    wy = static_cast<float>(gy) * mTileSize;
}

void WorldMap::gridToWorldCenter(int gx, int gy, float& wx, float& wy) const
{
    wx = (static_cast<float>(gx) + 0.5f) * mTileSize;
    wy = (static_cast<float>(gy) + 0.5f) * mTileSize;
}

bool WorldMap::isInBounds(int gx, int gy) const
{
    return gx >= 0 && gx < mCols && gy >= 0 && gy < mRows;
}

bool WorldMap::isWalkable(int gx, int gy) const
{
    if (!isInBounds(gx, gy)) return false;
    return mTiles[index(gx, gy)].walkable;
}

Terrain WorldMap::getTerrain(int gx, int gy) const
{
    if (!isInBounds(gx, gy)) return Terrain::Grass;
    return mTiles[index(gx, gy)].type;
}

void WorldMap::setTile(int gx, int gy, Terrain t, bool walkable)
{
    if (!isInBounds(gx, gy)) return;
    int i = index(gx, gy);
    mTiles[i].type = t;
    mTiles[i].walkable = walkable;
}

void WorldMap::generateDemoMap()
{
    int cols = mCols;
    int rows = mRows;

    // 全部初始化为草地
    for (int gy = 0; gy < rows; ++gy) {
        for (int gx = 0; gx < cols; ++gx) {
            setTile(gx, gy, Terrain::Grass, true);
        }
    }

    // 河流横穿地图（从西到东，中下部）
    for (int gx = 0; gx < cols; ++gx) {
        int gy = rows * 2 / 3 + (gx % 3) - 1;
        if (gy >= 0 && gy < rows) {
            setTile(gx, gy, Terrain::Water, false);
        }
    }

    // 山区在四个角落
    for (int gy = 0; gy < 4; ++gy) {
        for (int gx = 0; gx < 4; ++gx) {
            setTile(gx, gy, Terrain::Mountain, false);
            setTile(cols - 1 - gx, gy, Terrain::Mountain, false);
            setTile(gx, rows - 1 - gy, Terrain::Mountain, false);
            setTile(cols - 1 - gx, rows - 1 - gy, Terrain::Mountain, false);
        }
    }

    // 森林区域（左侧中部）
    for (int gy = 5; gy < rows / 2 + 2; ++gy) {
        for (int gx = 2; gx < 8; ++gx) {
            if (getTerrain(gx, gy) == Terrain::Grass) {
                setTile(gx, gy, Terrain::Forest, true);
            }
        }
    }

    // 森林区域（右侧中部）
    for (int gy = rows / 2 + 5; gy < rows - 2; ++gy) {
        for (int gx = cols - 10; gx < cols - 3; ++gx) {
            if (getTerrain(gx, gy) == Terrain::Grass) {
                setTile(gx, gy, Terrain::Forest, true);
            }
        }
    }

    // 中央空地的小路（南北贯穿）
    for (int gy = 0; gy < rows; ++gy) {
        int gx = cols / 2 + (gy % 3) - 1;
        if (gx >= 0 && gx < cols) {
            if (getTerrain(gx, gy) == Terrain::Grass) {
                setTile(gx, gy, Terrain::Path, true);
            }
        }
    }

    // 沙漠小区域（右下）
    for (int gy = rows - 6; gy < rows - 2; ++gy) {
        for (int gx = cols - 8; gx < cols - 4; ++gx) {
            if (getTerrain(gx, gy) == Terrain::Grass) {
                setTile(gx, gy, Terrain::Sand, true);
            }
        }
    }
}
