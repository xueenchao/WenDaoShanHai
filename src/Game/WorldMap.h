/**
 * WorldMap.h - 大世界地图
 *
 * 基于Tile的2D世界地图，管理地形信息、可行走性。
 */

#ifndef WORLDMAP_H
#define WORLDMAP_H

#include <cstdint>
#include <vector>

enum class Terrain {
    Grass,
    Water,
    Mountain,
    Forest,
    Path,
    Sand
};

struct Tile {
    Terrain type = Terrain::Grass;
    bool walkable = true;
};

class WorldMap {
public:
    WorldMap(int cols, int rows, float tileSize = 64.0f);

    void worldToGrid(float wx, float wy, int& gx, int& gy) const;
    void gridToWorld(int gx, int gy, float& wx, float& wy) const;
    void gridToWorldCenter(int gx, int gy, float& wx, float& wy) const;

    bool isInBounds(int gx, int gy) const;
    bool isWalkable(int gx, int gy) const;
    Terrain getTerrain(int gx, int gy) const;

    int getCols() const { return mCols; }
    int getRows() const { return mRows; }
    float getTileSize() const { return mTileSize; }
    float getWorldWidth() const { return mCols * mTileSize; }
    float getWorldHeight() const { return mRows * mTileSize; }

    void generateDemoMap();

private:
    int mCols;
    int mRows;
    float mTileSize;
    std::vector<Tile> mTiles;

    int index(int gx, int gy) const { return gy * mCols + gx; }
    void setTile(int gx, int gy, Terrain t, bool walkable);
};

#endif // WORLDMAP_H
