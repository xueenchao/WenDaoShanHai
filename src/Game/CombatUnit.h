/**
 * CombatUnit.h - 战斗单位
 *
 * 表示战斗中的所有角色（玩家队伍 + 敌方队伍），包含属性、五行、
 * 战斗资源（灵力/身法）、技能列表和网格位置。
 */

#ifndef COMBATUNIT_H
#define COMBATUNIT_H

#include <cstdint>
#include <string>
#include <vector>
#include "Skill.h"

class CombatUnit {
public:
    CombatUnit();

    uint32_t mId = 0;

    // ==================== 基础属性 ====================

    std::string mName;
    int mHP = 100;
    int mMaxHP = 100;
    int mAttack = 15;
    int mDefense = 5;
    int mSpeed = 10;          // 决定先攻顺序

    Element mElement = Element::Jin;

    // ==================== 战斗资源 ====================

    int mSpiritPower = 6;     // 灵力（当前值，每回合恢复）
    int mMaxSpiritPower = 6;
    int mMovementPoints = 4;  // 身法（当前值，每回合恢复）
    int mMaxMovementPoints = 4;

    // ==================== 位置 ====================

    int mGridX = 0;
    int mGridY = 0;

    // ==================== 技能 ====================

    std::vector<Skill> mSkills;

    // ==================== 队伍 ====================

    int mTeam = 0;            // 0=玩家方, 1=敌方

    // ==================== 方法 ====================

    bool isAlive() const { return mHP > 0; }

    /**
     * 计算对目标造成的伤害（含五行克制）
     */
    int calculateDamage(const Skill& skill, const CombatUnit& target) const;

    void takeDamage(int damage);
    void heal(int amount);

    /**
     * 回合开始时恢复资源
     */
    void onTurnStart();

    /**
     * 消耗灵力
     */
    bool canAffordSkill(const Skill& skill) const;
    void paySkillCost(const Skill& skill);

    /**
     * 消耗身法移动
     */
    bool canMove(int distance) const;
    void payMoveCost(int distance);

    /**
     * 减少所有技能的冷却
     */
    void reduceCooldowns();
};

#endif // COMBATUNIT_H
