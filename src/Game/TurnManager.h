/**
 * TurnManager.h - 回合管理器
 *
 * 管理战斗中的回合顺序、阶段切换和单位资源恢复。
 * 按先攻值（mSpeed）降序排列所有单位，轮流行动。
 */

#ifndef TURNMANAGER_H
#define TURNMANAGER_H

#include <cstdint>
#include <vector>

class CombatUnit;

enum class CombatPhase {
    PlayerTurn,   // 等待玩家输入
    EnemyTurn,    // AI自动执行
    Animating,    // 播放动画中
    Victory,      // 战斗胜利
    Defeat        // 战斗失败
};

class TurnManager {
public:
    TurnManager();

    /**
     * 开始战斗：收集所有存活单位，按速度降序排列
     */
    void startBattle(std::vector<CombatUnit*>& units);

    /**
     * 获取当前行动的单位
     */
    CombatUnit* getActiveUnit();

    /**
     * 结束当前单位的回合，切换到下一个单位
     * 触发资源恢复、冷却减少
     */
    void endTurn();

    /**
     * 跳到下一个玩家的回合（跳过敌方单位直到遇到玩家）
     */
    void advanceToNextPlayerUnit();

    /**
     * 检查战斗是否结束
     */
    CombatPhase checkBattleEnd(std::vector<CombatUnit*>& units);

    // ==================== 状态 ====================

    CombatPhase mPhase = CombatPhase::PlayerTurn;
    int mRoundNumber = 1;
    int mCurrentIndex = 0;
    int mRemainingMoves = 0;         // 当前单位剩余可移动格数
    bool mHasMoved = false;
    bool mHasActed = false;

private:
    void sortBySpeed();
    CombatUnit* getNextAliveUnit(int& index);

    std::vector<CombatUnit*> mTurnOrder;
};

#endif // TURNMANAGER_H
