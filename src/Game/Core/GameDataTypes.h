/**
 * GameDataTypes.h - 游戏核心数据类型
 *
 * 定义技能、物品、本命法宝、符箓等基础数据结构。
 */

#ifndef GAMEDATATYPES_H
#define GAMEDATATYPES_H

#include <string>

// ==================== 枚举 ====================

// 五行属性
enum class Element {
    Jin  = 0,  // 金
    Mu   = 1,  // 木
    Shui = 2,  // 水
    Huo  = 3,  // 火
    Tu   = 4   // 土
};

enum class SkillAreaType {
    Single,  // 单体目标
    Cross,   // 十字范围（中心+上下左右）
    Square   // 3x3方形范围
};

enum class ItemType { Consumable, Equipment, Material, Key };
enum class EquipSlot { Weapon, Armor, Accessory };

enum class TalentPath { ShaFa, ShouHu, XiaoYao }; // 杀伐/守护/逍遥

enum class TalismanType { Attack, Defense, Support, Summon };

// ==================== 数据结构 ====================

struct Skill {
    std::string mName;
    std::string mDescription;

    int mSpiritCost = 2;       // 灵力消耗
    int mRange = 3;            // 射程（格子，曼哈顿距离）
    SkillAreaType mAreaType = SkillAreaType::Single;
    int mBaseDamage = 25;
    int mCooldown = 1;         // 冷却回合数（0=无冷却）
    int mCurrentCooldown = 0;  // 当前剩余冷却
    int mElementId = 0;        // 对应 Element enum 值

    bool isReady() const { return mCurrentCooldown <= 0; }
};

struct Item {
    std::string id;
    std::string name;
    std::string desc;
    ItemType type = ItemType::Consumable;
    int quantity = 1;
    int maxStack = 99;

    EquipSlot equipSlot = EquipSlot::Weapon;
    int atkBonus = 0;
    int defBonus = 0;
    int hpBonus = 0;

    int healHP = 0;
    int healSP = 0;
};

struct LifeboundTreasure {
    std::string id;
    std::string name;
    std::string desc;
    Element element;
    int level = 1;
    int maxLevel = 10;
    int atkBonus = 0;
    int defBonus = 0;
    int hpBonus = 0;
    int speedBonus = 0;
    TalentPath talentPath = TalentPath::ShaFa;

    int getScaledAtk() const { return atkBonus + (level - 1) * 2; }
    int getScaledDef() const { return defBonus + (level - 1) * 1; }
    int getScaledHP() const { return hpBonus + (level - 1) * 5; }
    int getScaledSpeed() const { return speedBonus + (level - 1) * 1; }

    void levelUp() { if (level < maxLevel) ++level; }

    const char* talentPathName() const
    {
        switch (talentPath) {
        case TalentPath::ShaFa:  return "杀伐";
        case TalentPath::ShouHu: return "守护";
        case TalentPath::XiaoYao:return "逍遥";
        }
        return "未知";
    }
};

struct Talisman {
    std::string id;
    std::string name;
    std::string desc;
    TalismanType type = TalismanType::Attack;
    int value = 0;
    Element element;
    int quantity = 1;

    const char* typeName() const
    {
        switch (type) {
        case TalismanType::Attack:  return "攻击";
        case TalismanType::Defense: return "防御";
        case TalismanType::Support: return "辅助";
        case TalismanType::Summon:  return "召唤";
        }
        return "未知";
    }
};

#endif // GAMEDATATYPES_H
