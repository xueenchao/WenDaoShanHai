#include "CombatUnit.h"
#include <algorithm>

// ==================== 五行克制表 ====================
// 克：火→金→木→土→水→火
// 五行顺序: Jin=0, Mu=1, Shui=2, Huo=3, Tu=4

static const int COUNTER_TABLE[] = {
    1,  // 金(0) 被火(3)克 → 克制木(1)
    3,  // 木(1) 被金(0)克 → 克制土(3)? 等等让我重新理解
};

// 正确的五行相克:
// 金克木, 木克土, 土克水, 水克火, 火克金
// 即: 克制方对目标的伤害×1.3, 被克方伤害×0.7
// counter[attacker] = 被attacker克制的元素
static const int ELEMENT_COUNTER[] = {
    1,  // 金(0) 克 木(1)
    4,  // 木(1) 克 土(4)
    3,  // 水(2) 克 火(3)
    0,  // 火(3) 克 金(0)
    2   // 土(4) 克 水(2)
};

// counterOf[element] = 克制该element的元素
static const int ELEMENT_COUNTER_OF[] = {
    3,  // 金(0) 被 火(3) 克
    0,  // 木(1) 被 金(0) 克
    4,  // 水(2) 被 土(4) 克
    2,  // 火(3) 被 水(2) 克
    1   // 土(4) 被 木(1) 克
};

CombatUnit::CombatUnit()
{
}

int CombatUnit::calculateDamage(const Skill& skill, const CombatUnit& target) const
{
    float base = static_cast<float>(skill.mBaseDamage + mAttack);

    // 五行克制系数
    int attackerElem = skill.mElementId;
    int targetElem = static_cast<int>(target.mElement);

    float elemMultiplier = 1.0f;
    if (ELEMENT_COUNTER[attackerElem] == targetElem) {
        elemMultiplier = 1.3f;  // 克制
    } else if (ELEMENT_COUNTER_OF[attackerElem] == targetElem) {
        elemMultiplier = 0.7f;  // 被克
    }

    // 防御减伤（简单公式）
    float reduction = static_cast<float>(target.mDefense) * 0.5f;

    float damage = base * elemMultiplier - reduction;
    return std::max(1, static_cast<int>(damage));
}

void CombatUnit::takeDamage(int damage)
{
    mHP = std::max(0, mHP - damage);
}

void CombatUnit::heal(int amount)
{
    mHP = std::min(mMaxHP, mHP + amount);
}

void CombatUnit::onTurnStart()
{
    mSpiritPower = std::min(mSpiritPower + 2, mMaxSpiritPower);  // 每回合恢复2灵力
    mMovementPoints = mMaxMovementPoints;  // 身法回满
}

bool CombatUnit::canAffordSkill(const Skill& skill) const
{
    return mSpiritPower >= skill.mSpiritCost && skill.isReady();
}

void CombatUnit::paySkillCost(const Skill& skill)
{
    mSpiritPower -= skill.mSpiritCost;
    // 冷却在技能使用后由TurnManager设置
}

bool CombatUnit::canMove(int distance) const
{
    return mMovementPoints >= distance;
}

void CombatUnit::payMoveCost(int distance)
{
    mMovementPoints = std::max(0, mMovementPoints - distance);
}

void CombatUnit::reduceCooldowns()
{
    for (auto& skill : mSkills) {
        if (skill.mCurrentCooldown > 0) {
            skill.mCurrentCooldown--;
        }
    }
}
