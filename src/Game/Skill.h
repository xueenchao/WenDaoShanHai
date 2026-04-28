/**
 * Skill.h - 技能数据定义
 *
 * 数据驱动的技能结构体，将来可从JSON加载。
 */

#ifndef SKILL_H
#define SKILL_H

#include <string>

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

    // ==================== 预设技能工厂 ====================

    static Skill makeFireball();
    static Skill makeIceShard();
    static Skill makeSwordSlash();
    static Skill makeRockSmash();
    static Skill makeVineWhip();
};

#endif // SKILL_H
