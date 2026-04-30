#include "TurnManager.h"
#include "CombatUnit.h"
#include "Core/Log.h"
#include <algorithm>
#include <SDL3/SDL.h>

TurnManager::TurnManager()
{
}

void TurnManager::startBattle(std::vector<CombatUnit*>& units)
{
    mTurnOrder.clear();
    for (auto* u : units) {
        if (u->isAlive()) {
            mTurnOrder.push_back(u);
        }
    }

    sortBySpeed();

    mCurrentIndex = 0;
    mRoundNumber = 1;
    mPhase = CombatPhase::PlayerTurn;
    mHasMoved = false;
    mHasActed = false;

    // 第一个单位回合开始
    CombatUnit* active = getActiveUnit();
    if (active != nullptr) {
        active->onTurnStart();
        mRemainingMoves = active->mMovementPoints;
        if (active->mTeam != 0) {
            mPhase = CombatPhase::EnemyTurn;
        }
    }
}

void TurnManager::sortBySpeed()
{
    std::sort(mTurnOrder.begin(), mTurnOrder.end(),
        [](const CombatUnit* a, const CombatUnit* b) {
            return a->mSpeed > b->mSpeed;
        });
}

CombatUnit* TurnManager::getActiveUnit()
{
    if (mTurnOrder.empty()) return nullptr;

    while (mCurrentIndex < static_cast<int>(mTurnOrder.size())) {
        if (mTurnOrder[mCurrentIndex]->isAlive()) {
            return mTurnOrder[mCurrentIndex];
        }
        mCurrentIndex++;
    }

    // 所有单位都行动完毕，开始新回合
    return nullptr;
}

void TurnManager::endTurn()
{
    CombatUnit* unit = getActiveUnit();
    if (unit != nullptr) {
        unit->reduceCooldowns();
    }

    mCurrentIndex++;
    mHasMoved = false;
    mHasActed = false;

    CombatUnit* next = getNextAliveUnit(mCurrentIndex);

    if (next == nullptr) {
        // 新回合
        mRoundNumber++;
        mCurrentIndex = 0;

        // 重新排列（考虑速度变化）
        sortBySpeed();

        // 移除已死亡单位
        mTurnOrder.erase(
            std::remove_if(mTurnOrder.begin(), mTurnOrder.end(),
                [](CombatUnit* u) { return !u->isAlive(); }),
            mTurnOrder.end());

        if (mTurnOrder.empty()) {
            mPhase = CombatPhase::Defeat;
            return;
        }

        next = mTurnOrder[0];

        LOG_INFO("===== 第 %d 回合 =====", mRoundNumber);
    }

    if (next != nullptr) {
        next->onTurnStart();
        mRemainingMoves = next->mMovementPoints;

        if (next->mTeam == 0) {
            mPhase = CombatPhase::PlayerTurn;
        } else {
            mPhase = CombatPhase::EnemyTurn;
        }
    }
}

void TurnManager::advanceToNextPlayerUnit()
{
    // 跳过所有非玩家单位
    while (true) {
        CombatUnit* unit = getNextAliveUnit(mCurrentIndex);
        if (unit == nullptr) {
            // 新回合
            endTurn();
            return;
        }
        if (unit->mTeam == 0) {
            mCurrentIndex = std::find(mTurnOrder.begin(), mTurnOrder.end(), unit)
                - mTurnOrder.begin();
            unit->onTurnStart();
            mRemainingMoves = unit->mMovementPoints;
            mPhase = CombatPhase::PlayerTurn;
            mHasMoved = false;
            mHasActed = false;
            return;
        }
        mCurrentIndex++;
    }
}

CombatPhase TurnManager::checkBattleEnd(std::vector<CombatUnit*>& units)
{
    bool hasPlayer = false;
    bool hasEnemy = false;

    for (auto* u : units) {
        if (!u->isAlive()) continue;
        if (u->mTeam == 0) hasPlayer = true;
        else hasEnemy = true;
    }

    if (!hasPlayer) {
        mPhase = CombatPhase::Defeat;
    } else if (!hasEnemy) {
        mPhase = CombatPhase::Victory;
    }

    return mPhase;
}

CombatUnit* TurnManager::getNextAliveUnit(int& index)
{
    while (index < static_cast<int>(mTurnOrder.size())) {
        if (mTurnOrder[index]->isAlive()) {
            return mTurnOrder[index];
        }
        index++;
    }
    return nullptr;
}
