/**
 * DataManager.h - 游戏数据管理器
 *
 * 单例，从JSON文件加载所有游戏数据（技能、物品、敌人、地形、配置）。
 * 提供按ID查找的统一接口。
 */

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <string>
#include <vector>
#include <unordered_map>

class Skill;
class Item;
struct LifeboundTreasure;
struct Talisman;
enum class ItemType;
enum class EquipSlot;
enum class Element;
enum class SkillAreaType;
enum class Terrain;
enum class TalismanType;

struct NPCInfo {
    std::string name;
    std::string title;
    std::string greeting;
};

struct SectDef {
    std::string id;
    std::string name;
    std::string desc;
    int bonusAtk = 0;
    int bonusDef = 0;
    int bonusHP = 0;
    int bonusSpeed = 0;
    int bonusSP = 0;
    std::vector<NPCInfo> npcs;
};

// 地形定义（从JSON加载）
struct TerrainDef {
    std::string id;
    std::string name;
    bool walkable = true;
    int colorR = 0, colorG = 0, colorB = 0;
};

// 敌人模板（从JSON加载）
struct EnemyTemplateDef {
    std::string id;
    std::string name;
    Element element;
    int hp = 0, attack = 0, defense = 0, speed = 0;
    int spirit = 0, moves = 0;
    std::string skillId;  // 引用技能ID
};

// 游戏配置
struct ConfigData {
    // 窗口
    const char* windowTitle = "问道·山海";
    int windowW = 1280, windowH = 720;

    // 视口
    float viewportW = 1280.0f, viewportH = 720.0f;

    // 帧率
    int fps = 60;

    // 字体
    std::string fontPrimary;
    std::string fontFallback;
    float fontSize = 14.0f;

    // 世界
    int worldCols = 40, worldRows = 30;
    float worldTileSize = 64.0f;

    // 战斗
    int combatCols = 8, combatRows = 6;
    float combatTileSize = 64.0f;
    float combatCameraZoom = 1.5f;

    // 玩家
    float playerSpeed = 200.0f;
    float playerSize = 32.0f;
    std::string defaultName = "青云修士";
    int defaultLevel = 1;
    int defaultHP = 120, defaultAttack = 20, defaultDefense = 8, defaultSpeed = 12;
    int defaultSpiritPower = 6, defaultMovementPoints = 4;
    Element defaultElement;
    int xpPerLevel = 100;

    // 遭遇
    int encounterMaxMarkers = 5;
    int encounterSpawnMargin = 3;
    float encounterMinDistPlayer = 200.0f;
    float encounterMinSpacing = 100.0f;
    float encounterTriggerDist = 52.0f;
    float encounterRespawnDelay = 3.0f;

    // AI
    float aiMoveDelay = 0.6f;
    float aiAttackDelay = 0.8f;
};

class DataManager {
public:
    static DataManager& getInstance();

    /**
     * 加载所有JSON数据文件，在游戏启动时调用
     * @return 成功返回true
     */
    bool loadAll();

    // 技能
    const Skill* getSkill(const std::string& id) const;

    // 物品
    const Item* getItem(const std::string& id) const;

    /**
     * 根据ID创建物品副本（quantity=1）
     */
    Item createItem(const std::string& id) const;

    // 敌人
    const EnemyTemplateDef* getEnemyTemplate(const std::string& id) const;
    const std::vector<std::string>& getEnemyIds() const { return mEnemyIds; }

    // 地形
    const TerrainDef* getTerrainDef(const std::string& id) const;
    const std::vector<TerrainDef>& getTerrainDefs() const { return mTerrainDefs; }

    // 配置
    const ConfigData& getConfig() const { return mConfig; }

    // 本命法宝
    const LifeboundTreasure* getLifeboundTreasure(const std::string& id) const;
    LifeboundTreasure createLifeboundTreasure(const std::string& id) const;

    // 符箓
    const Talisman* getTalisman(const std::string& id) const;
    Talisman createTalisman(const std::string& id) const;

    // 门派
    const SectDef* getSect(const std::string& id) const;
    const std::vector<std::string>& getSectIds() const { return mSectIds; }

    // 起始数据
    const std::vector<std::string>& getStartingSkills() const { return mStartingSkills; }
    std::vector<Item> createStartingInventory() const;

    // 工具：字符串→枚举
    static Element parseElement(const char* str);
    static SkillAreaType parseAreaType(const char* str);
    static ItemType parseItemType(const char* str);
    static EquipSlot parseEquipSlot(const char* str);
    static Terrain parseTerrainEnum(const char* str);

private:
    DataManager() = default;
    ~DataManager() = default;
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;

    bool loadSkills(const char* path);
    bool loadItems(const char* path);
    bool loadEnemies(const char* path);
    bool loadConfig(const char* path);
    bool loadTerrains(const char* path);
    bool loadLifeboundTreasures(const char* path);
    bool loadTalismans(const char* path);
    bool loadSects(const char* path);

    std::unordered_map<std::string, Skill> mSkillMap;
    std::unordered_map<std::string, Item> mItemMap;
    std::unordered_map<std::string, EnemyTemplateDef> mEnemyMap;
    std::vector<TerrainDef> mTerrainDefs;
    std::vector<std::string> mEnemyIds;
    std::vector<std::string> mStartingSkills;
    std::vector<std::pair<std::string, int>> mStartingInventory;  // id, quantity
    ConfigData mConfig;

    std::unordered_map<std::string, LifeboundTreasure> mLifeboundMap;
    std::unordered_map<std::string, Talisman> mTalismanMap;
    std::unordered_map<std::string, SectDef> mSectMap;
    std::vector<std::string> mSectIds;
};

#endif // DATAMANAGER_H
