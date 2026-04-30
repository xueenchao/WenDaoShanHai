/**
 * CombatAI.h - 战斗AI
 *
 * 敌人的回合决策：找到最近玩家 → 移动靠近 → 在范围内释放技能 → 结束回合。
 */

#ifndef COMBATAI_H
#define COMBATAI_H

#include <cstdint>
#include <vector>

class GridMap;
class CombatUnit;
class TurnManager;

class CombatAI {
public:
    /**
     * 执行AI回合
     * @return true如果AI执行了动作（需要短暂延迟展示），false如果在等待动画
     */
    bool executeTurn(GridMap& grid, TurnManager& turnMgr,
                     std::vector<CombatUnit*>& allUnits, float deltaTime);

private:
    float mDelayTimer = 0.0f;   // 动作间延迟
    int mAIStep = 0;           // AI步骤（0=开始, 1=移动完成, 2=攻击完成）
    int mTargetGX = 0, mTargetGY = 0;  // 目标移动位置
};

#endif // COMBATAI_H
