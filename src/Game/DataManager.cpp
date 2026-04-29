/**
 * DataManager.cpp - 游戏数据管理器实现
 */

#include "DataManager.h"
#include "Skill.h"
#include "Item.h"
#include "WorldMap.h"
#include "Core/JsonLoader.h"
#include "Core/Log.h"
#include <cjson/cJSON.h>

// ==================== 枚举解析 ====================

Element DataManager::parseElement(const char* str)
{
    if (str == nullptr) return Element::Jin;
    switch (str[0]) {
        case 'j': return Element::Jin;    // jin
        case 'm': return Element::Mu;     // mu
        case 's': return Element::Shui;   // shui
        case 'h': return Element::Huo;    // huo
        case 't': return Element::Tu;     // tu
        default:  return Element::Jin;
    }
}

SkillAreaType DataManager::parseAreaType(const char* str)
{
    if (str == nullptr) return SkillAreaType::Single;
    switch (str[0]) {
        case 'c': return SkillAreaType::Cross;   // cross
        case 's': return (str[1] == 'i') ? SkillAreaType::Single : SkillAreaType::Square;
        default:  return SkillAreaType::Single;
    }
}

ItemType DataManager::parseItemType(const char* str)
{
    if (str == nullptr) return ItemType::Material;
    switch (str[0]) {
        case 'c': return ItemType::Consumable;
        case 'e': return ItemType::Equipment;
        case 'm': return ItemType::Material;
        case 'k': return ItemType::Key;
        default:  return ItemType::Material;
    }
}

EquipSlot DataManager::parseEquipSlot(const char* str)
{
    if (str == nullptr) return EquipSlot::Weapon;
    switch (str[0]) {
        case 'w': return EquipSlot::Weapon;
        case 'a': return (str[1] == 'r') ? EquipSlot::Armor : EquipSlot::Accessory;
        default:  return EquipSlot::Weapon;
    }
}

Terrain DataManager::parseTerrainEnum(const char* str)
{
    if (str == nullptr) return Terrain::Grass;
    switch (str[0]) {
        case 'g': return Terrain::Grass;
        case 'w': return Terrain::Water;
        case 'm': return Terrain::Mountain;
        case 'f': return Terrain::Forest;
        case 'p': return Terrain::Path;
        case 's': return Terrain::Sand;
        default:  return Terrain::Grass;
    }
}

// ==================== 单例 ====================

DataManager& DataManager::getInstance()
{
    static DataManager instance;
    return instance;
}

// ==================== 主加载 ====================

bool DataManager::loadAll()
{
    LOG_INFO("===== 加载游戏数据 =====");

    if (!loadSkills("data/skills.json"))   return false;
    if (!loadItems("data/items.json"))     return false;
    if (!loadEnemies("data/enemies.json")) return false;
    if (!loadConfig("data/config.json"))   return false;
    if (!loadTerrains("data/terrains.json")) return false;

    LOG_INFO("===== 数据加载完成: %zu技能 %zu物品 %zu敌人 %zu地形 =====",
        mSkillMap.size(), mItemMap.size(), mEnemyMap.size(), mTerrainDefs.size());
    return true;
}

// ==================== 技能加载 ====================

bool DataManager::loadSkills(const char* path)
{
    cJSON* root = JsonLoader::loadJsonFile(path);
    if (root == nullptr) return false;

    int count = cJSON_GetArraySize(root);
    for (int i = 0; i < count; ++i) {
        cJSON* obj = cJSON_GetArrayItem(root, i);
        if (obj == nullptr) continue;

        Skill s;

        cJSON* idNode = cJSON_GetObjectItem(obj, "id");
        std::string id = idNode ? cJSON_GetStringValue(idNode) : "";

        cJSON* nameNode = cJSON_GetObjectItem(obj, "name");
        s.mName = nameNode ? cJSON_GetStringValue(nameNode) : "";

        cJSON* descNode = cJSON_GetObjectItem(obj, "desc");
        s.mDescription = descNode ? cJSON_GetStringValue(descNode) : "";

        cJSON* costNode = cJSON_GetObjectItem(obj, "spiritCost");
        s.mSpiritCost = costNode ? static_cast<int>(cJSON_GetNumberValue(costNode)) : 2;

        cJSON* rangeNode = cJSON_GetObjectItem(obj, "range");
        s.mRange = rangeNode ? static_cast<int>(cJSON_GetNumberValue(rangeNode)) : 3;

        cJSON* dmgNode = cJSON_GetObjectItem(obj, "baseDamage");
        s.mBaseDamage = dmgNode ? static_cast<int>(cJSON_GetNumberValue(dmgNode)) : 25;

        cJSON* cdNode = cJSON_GetObjectItem(obj, "cooldown");
        s.mCooldown = cdNode ? static_cast<int>(cJSON_GetNumberValue(cdNode)) : 1;

        cJSON* areaNode = cJSON_GetObjectItem(obj, "areaType");
        s.mAreaType = parseAreaType(areaNode ? cJSON_GetStringValue(areaNode) : "single");

        cJSON* elemNode = cJSON_GetObjectItem(obj, "element");
        s.mElementId = static_cast<int>(parseElement(elemNode ? cJSON_GetStringValue(elemNode) : "jin"));

        s.mCurrentCooldown = 0;

        if (!id.empty()) {
            mSkillMap[id] = s;
        }
    }

    JsonLoader::freeJson(root);
    return true;
}

// ==================== 物品加载 ====================

bool DataManager::loadItems(const char* path)
{
    cJSON* root = JsonLoader::loadJsonFile(path);
    if (root == nullptr) return false;

    // 加载物品列表
    cJSON* itemsArr = cJSON_GetObjectItem(root, "items");
    if (itemsArr != nullptr) {
        int count = cJSON_GetArraySize(itemsArr);
        for (int i = 0; i < count; ++i) {
            cJSON* obj = cJSON_GetArrayItem(itemsArr, i);
            if (obj == nullptr) continue;

            Item item;

            cJSON* idNode = cJSON_GetObjectItem(obj, "id");
            item.id = idNode ? cJSON_GetStringValue(idNode) : "";

            cJSON* nameNode = cJSON_GetObjectItem(obj, "name");
            item.name = nameNode ? cJSON_GetStringValue(nameNode) : "";

            cJSON* descNode = cJSON_GetObjectItem(obj, "desc");
            item.desc = descNode ? cJSON_GetStringValue(descNode) : "";

            cJSON* typeNode = cJSON_GetObjectItem(obj, "type");
            item.type = parseItemType(typeNode ? cJSON_GetStringValue(typeNode) : "material");

            cJSON* maxStackNode = cJSON_GetObjectItem(obj, "maxStack");
            item.maxStack = maxStackNode ? static_cast<int>(cJSON_GetNumberValue(maxStackNode)) : 99;

            item.quantity = 1;

            if (item.type == ItemType::Equipment) {
                cJSON* slotNode = cJSON_GetObjectItem(obj, "equipSlot");
                item.equipSlot = parseEquipSlot(slotNode ? cJSON_GetStringValue(slotNode) : "weapon");

                cJSON* atkNode = cJSON_GetObjectItem(obj, "atkBonus");
                item.atkBonus = atkNode ? static_cast<int>(cJSON_GetNumberValue(atkNode)) : 0;

                cJSON* defNode = cJSON_GetObjectItem(obj, "defBonus");
                item.defBonus = defNode ? static_cast<int>(cJSON_GetNumberValue(defNode)) : 0;

                cJSON* hpNode = cJSON_GetObjectItem(obj, "hpBonus");
                item.hpBonus = hpNode ? static_cast<int>(cJSON_GetNumberValue(hpNode)) : 0;
            } else if (item.type == ItemType::Consumable) {
                cJSON* healHPNode = cJSON_GetObjectItem(obj, "healHP");
                item.healHP = healHPNode ? static_cast<int>(cJSON_GetNumberValue(healHPNode)) : 0;

                cJSON* healSPNode = cJSON_GetObjectItem(obj, "healSP");
                item.healSP = healSPNode ? static_cast<int>(cJSON_GetNumberValue(healSPNode)) : 0;
            }

            if (!item.id.empty()) {
                mItemMap[item.id] = item;
            }
        }
    }

    // 加载起始背包
    cJSON* startInv = cJSON_GetObjectItem(root, "startingInventory");
    if (startInv != nullptr) {
        int count = cJSON_GetArraySize(startInv);
        for (int i = 0; i < count; ++i) {
            cJSON* obj = cJSON_GetArrayItem(startInv, i);
            if (obj == nullptr) continue;

            cJSON* idNode = cJSON_GetObjectItem(obj, "id");
            std::string id = idNode ? cJSON_GetStringValue(idNode) : "";

            cJSON* qtyNode = cJSON_GetObjectItem(obj, "quantity");
            int qty = qtyNode ? static_cast<int>(cJSON_GetNumberValue(qtyNode)) : 1;

            if (!id.empty() && qty > 0) {
                mStartingInventory.push_back({id, qty});
            }
        }
    }

    // 加载起始技能
    cJSON* startSkills = cJSON_GetObjectItem(root, "startingSkills");
    if (startSkills != nullptr) {
        int count = cJSON_GetArraySize(startSkills);
        for (int i = 0; i < count; ++i) {
            cJSON* item = cJSON_GetArrayItem(startSkills, i);
            if (item != nullptr && cJSON_IsString(item)) {
                mStartingSkills.push_back(cJSON_GetStringValue(item));
            }
        }
    }

    JsonLoader::freeJson(root);
    return true;
}

// ==================== 敌人加载 ====================

bool DataManager::loadEnemies(const char* path)
{
    cJSON* root = JsonLoader::loadJsonFile(path);
    if (root == nullptr) return false;

    int count = cJSON_GetArraySize(root);
    for (int i = 0; i < count; ++i) {
        cJSON* obj = cJSON_GetArrayItem(root, i);
        if (obj == nullptr) continue;

        EnemyTemplateDef et;

        cJSON* idNode = cJSON_GetObjectItem(obj, "id");
        std::string id = idNode ? cJSON_GetStringValue(idNode) : "";

        cJSON* nameNode = cJSON_GetObjectItem(obj, "name");
        et.name = nameNode ? cJSON_GetStringValue(nameNode) : "";

        cJSON* elemNode = cJSON_GetObjectItem(obj, "element");
        et.element = parseElement(elemNode ? cJSON_GetStringValue(elemNode) : "jin");

        cJSON* hpNode = cJSON_GetObjectItem(obj, "hp");
        et.hp = hpNode ? static_cast<int>(cJSON_GetNumberValue(hpNode)) : 80;

        cJSON* atkNode = cJSON_GetObjectItem(obj, "attack");
        et.attack = atkNode ? static_cast<int>(cJSON_GetNumberValue(atkNode)) : 10;

        cJSON* defNode = cJSON_GetObjectItem(obj, "defense");
        et.defense = defNode ? static_cast<int>(cJSON_GetNumberValue(defNode)) : 4;

        cJSON* spdNode = cJSON_GetObjectItem(obj, "speed");
        et.speed = spdNode ? static_cast<int>(cJSON_GetNumberValue(spdNode)) : 8;

        cJSON* spNode = cJSON_GetObjectItem(obj, "spirit");
        et.spirit = spNode ? static_cast<int>(cJSON_GetNumberValue(spNode)) : 4;

        cJSON* moveNode = cJSON_GetObjectItem(obj, "moves");
        et.moves = moveNode ? static_cast<int>(cJSON_GetNumberValue(moveNode)) : 3;

        cJSON* skillNode = cJSON_GetObjectItem(obj, "skill");
        et.skillId = skillNode ? cJSON_GetStringValue(skillNode) : "fireball";

        if (!id.empty()) {
            mEnemyMap[id] = et;
            mEnemyIds.push_back(id);
        }
    }

    JsonLoader::freeJson(root);
    return true;
}

// ==================== 配置加载 ====================

bool DataManager::loadConfig(const char* path)
{
    cJSON* root = JsonLoader::loadJsonFile(path);
    if (root == nullptr) return false;

    ConfigData& cfg = mConfig;

    cJSON* win = cJSON_GetObjectItem(root, "window");
    if (win != nullptr) {
        cJSON* titleNode = cJSON_GetObjectItem(win, "title");
        // windowTitle is const char*, we don't store dynamic strings here
        cfg.windowW = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(win, "width")));
        cfg.windowH = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(win, "height")));
    }

    cJSON* vp = cJSON_GetObjectItem(root, "viewport");
    if (vp != nullptr) {
        cfg.viewportW = static_cast<float>(cJSON_GetNumberValue(cJSON_GetObjectItem(vp, "width")));
        cfg.viewportH = static_cast<float>(cJSON_GetNumberValue(cJSON_GetObjectItem(vp, "height")));
    }

    cJSON* fpsNode = cJSON_GetObjectItem(root, "fps");
    cfg.fps = fpsNode ? static_cast<int>(cJSON_GetNumberValue(fpsNode)) : 60;

    cJSON* font = cJSON_GetObjectItem(root, "font");
    if (font != nullptr) {
        cJSON* primNode = cJSON_GetObjectItem(font, "primary");
        cfg.fontPrimary = primNode ? cJSON_GetStringValue(primNode) : "";
        cJSON* fallNode = cJSON_GetObjectItem(font, "fallback");
        cfg.fontFallback = fallNode ? cJSON_GetStringValue(fallNode) : "";
        cfg.fontSize = static_cast<float>(cJSON_GetNumberValue(cJSON_GetObjectItem(font, "size")));
    }

    cJSON* world = cJSON_GetObjectItem(root, "world");
    if (world != nullptr) {
        cfg.worldCols = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(world, "cols")));
        cfg.worldRows = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(world, "rows")));
        cfg.worldTileSize = static_cast<float>(cJSON_GetNumberValue(cJSON_GetObjectItem(world, "tileSize")));
    }

    cJSON* combat = cJSON_GetObjectItem(root, "combat");
    if (combat != nullptr) {
        cfg.combatCols = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(combat, "cols")));
        cfg.combatRows = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(combat, "rows")));
        cfg.combatTileSize = static_cast<float>(cJSON_GetNumberValue(cJSON_GetObjectItem(combat, "tileSize")));
        cfg.combatCameraZoom = static_cast<float>(cJSON_GetNumberValue(cJSON_GetObjectItem(combat, "cameraZoom")));
    }

    cJSON* player = cJSON_GetObjectItem(root, "player");
    if (player != nullptr) {
        cfg.playerSpeed = static_cast<float>(cJSON_GetNumberValue(cJSON_GetObjectItem(player, "speed")));
        cfg.playerSize = static_cast<float>(cJSON_GetNumberValue(cJSON_GetObjectItem(player, "size")));

        cJSON* nameNode = cJSON_GetObjectItem(player, "defaultName");
        cfg.defaultName = nameNode ? cJSON_GetStringValue(nameNode) : "青云修士";

        cfg.defaultLevel = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(player, "defaultLevel")));
        cfg.defaultHP = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(player, "defaultHP")));
        cfg.defaultAttack = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(player, "defaultAttack")));
        cfg.defaultDefense = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(player, "defaultDefense")));
        cfg.defaultSpeed = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(player, "defaultSpeed")));
        cfg.defaultSpiritPower = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(player, "defaultSpiritPower")));
        cfg.defaultMovementPoints = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(player, "defaultMovementPoints")));

        cJSON* elemNode = cJSON_GetObjectItem(player, "element");
        cfg.defaultElement = parseElement(elemNode ? cJSON_GetStringValue(elemNode) : "jin");

        cfg.xpPerLevel = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(player, "xpPerLevel")));
    }

    cJSON* enc = cJSON_GetObjectItem(root, "encounter");
    if (enc != nullptr) {
        cfg.encounterMaxMarkers = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(enc, "maxMarkers")));
        cfg.encounterSpawnMargin = static_cast<int>(cJSON_GetNumberValue(cJSON_GetObjectItem(enc, "spawnMargin")));
        cfg.encounterMinDistPlayer = static_cast<float>(cJSON_GetNumberValue(cJSON_GetObjectItem(enc, "minDistanceFromPlayer")));
        cfg.encounterMinSpacing = static_cast<float>(cJSON_GetNumberValue(cJSON_GetObjectItem(enc, "minSpacing")));
        cfg.encounterTriggerDist = static_cast<float>(cJSON_GetNumberValue(cJSON_GetObjectItem(enc, "triggerDistance")));
        cfg.encounterRespawnDelay = static_cast<float>(cJSON_GetNumberValue(cJSON_GetObjectItem(enc, "respawnDelay")));
    }

    cJSON* ai = cJSON_GetObjectItem(root, "ai");
    if (ai != nullptr) {
        cfg.aiMoveDelay = static_cast<float>(cJSON_GetNumberValue(cJSON_GetObjectItem(ai, "moveDelay")));
        cfg.aiAttackDelay = static_cast<float>(cJSON_GetNumberValue(cJSON_GetObjectItem(ai, "attackDelay")));
    }

    JsonLoader::freeJson(root);
    return true;
}

// ==================== 地形加载 ====================

bool DataManager::loadTerrains(const char* path)
{
    cJSON* root = JsonLoader::loadJsonFile(path);
    if (root == nullptr) return false;

    int count = cJSON_GetArraySize(root);
    for (int i = 0; i < count; ++i) {
        cJSON* obj = cJSON_GetArrayItem(root, i);
        if (obj == nullptr) continue;

        TerrainDef td;

        cJSON* idNode = cJSON_GetObjectItem(obj, "id");
        td.id = idNode ? cJSON_GetStringValue(idNode) : "";

        cJSON* nameNode = cJSON_GetObjectItem(obj, "name");
        td.name = nameNode ? cJSON_GetStringValue(nameNode) : "";

        cJSON* walkNode = cJSON_GetObjectItem(obj, "walkable");
        td.walkable = walkNode ? (cJSON_IsTrue(walkNode) ? true : cJSON_IsFalse(walkNode) ? false : true) : true;

        cJSON* colorArr = cJSON_GetObjectItem(obj, "color");
        if (colorArr != nullptr && cJSON_IsArray(colorArr) && cJSON_GetArraySize(colorArr) >= 3) {
            td.colorR = static_cast<int>(cJSON_GetNumberValue(cJSON_GetArrayItem(colorArr, 0)));
            td.colorG = static_cast<int>(cJSON_GetNumberValue(cJSON_GetArrayItem(colorArr, 1)));
            td.colorB = static_cast<int>(cJSON_GetNumberValue(cJSON_GetArrayItem(colorArr, 2)));
        }

        mTerrainDefs.push_back(td);
    }

    JsonLoader::freeJson(root);
    return true;
}

// ==================== 查询接口 ====================

const Skill* DataManager::getSkill(const std::string& id) const
{
    auto it = mSkillMap.find(id);
    return (it != mSkillMap.end()) ? &it->second : nullptr;
}

const Item* DataManager::getItem(const std::string& id) const
{
    auto it = mItemMap.find(id);
    return (it != mItemMap.end()) ? &it->second : nullptr;
}

Item DataManager::createItem(const std::string& id) const
{
    auto it = mItemMap.find(id);
    if (it != mItemMap.end()) {
        Item item = it->second;
        item.quantity = 1;
        return item;
    }
    Item empty;
    empty.id = id;
    empty.name = "未知物品";
    empty.desc = "未找到此物品的数据";
    return empty;
}

const EnemyTemplateDef* DataManager::getEnemyTemplate(const std::string& id) const
{
    auto it = mEnemyMap.find(id);
    return (it != mEnemyMap.end()) ? &it->second : nullptr;
}

const TerrainDef* DataManager::getTerrainDef(const std::string& id) const
{
    for (auto& td : mTerrainDefs) {
        if (td.id == id) return &td;
    }
    return nullptr;
}

std::vector<Item> DataManager::createStartingInventory() const
{
    std::vector<Item> result;
    for (auto& pair : mStartingInventory) {
        auto it = mItemMap.find(pair.first);
        if (it != mItemMap.end()) {
            Item item = it->second;
            item.quantity = pair.second;
            result.push_back(item);
        }
    }
    return result;
}
