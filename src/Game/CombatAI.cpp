#include "CombatAI.h"
#include "GridMap.h"
#include "CombatUnit.h"
#include "TurnManager.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include "Core/Log.h"
#include <cstdio>

bool CombatAI::executeTurn(GridMap& grid, TurnManager& turnMgr,
                           std::vector<CombatUnit*>& allUnits, float deltaTime)
{
    CombatUnit* self = turnMgr.getActiveUnit();
    if (self == nullptr) return false;

    // 延迟计时器（让玩家能看到AI动作）
    mDelayTimer -= deltaTime;
    if (mDelayTimer > 0.0f) return false;

    // ===== 步骤0：启动 =====
    if (mAIStep == 0) {
        // 找到最近的存活玩家单位
        CombatUnit* nearestEnemy = nullptr;
        int nearestDist = 9999;

        for (auto* u : allUnits) {
            if (!u->isAlive() || u->mTeam == self->mTeam) continue;
            int dist = grid.getDistance(self->mGridX, self->mGridY, u->mGridX, u->mGridY);
            if (dist < nearestDist) {
                nearestDist = dist;
                nearestEnemy = u;
            }
        }

        if (nearestEnemy == nullptr) {
            turnMgr.endTurn();
            return false;
        }

        // 检查是否在技能范围内（使用伤害最高的可用技能）
        Skill* bestSkill = nullptr;
        for (auto& s : self->mSkills) {
            if (!self->canAffordSkill(s)) continue;
            if (bestSkill == nullptr || s.mBaseDamage > bestSkill->mBaseDamage) {
                bestSkill = &s;
            }
        }

        bool inRange = false;
        if (bestSkill != nullptr) {
            int d = grid.getDistance(self->mGridX, self->mGridY,
                nearestEnemy->mGridX, nearestEnemy->mGridY);
            if (d <= bestSkill->mRange) {
                inRange = true;
            }
        }

        if (inRange && bestSkill != nullptr && turnMgr.mRemainingMoves >= 0) {
            // 在范围内，直接攻击
            self->paySkillCost(*bestSkill);
            bestSkill->mCurrentCooldown = bestSkill->mCooldown;

            int damage = self->calculateDamage(*bestSkill, *nearestEnemy);
            nearestEnemy->takeDamage(damage);

            char buf[256];
            snprintf(buf, sizeof(buf), "[战斗] %s 对 %s 使用 %s，造成 %d 点伤害",
                self->mName.c_str(), nearestEnemy->mName.c_str(),
                bestSkill->mName.c_str(), damage);
            LOG_INFO("%s", buf);

            turnMgr.mHasActed = true;
            mDelayTimer = 0.8f;
            mAIStep = 0;  // 重置

            turnMgr.checkBattleEnd(allUnits);
            if (turnMgr.mPhase == CombatPhase::Victory || turnMgr.mPhase == CombatPhase::Defeat) {
                return false;
            }

            turnMgr.endTurn();
            return false;
        }

        // 需要移动——BFS寻路靠近
        auto path = grid.getPath(self->mGridX, self->mGridY,
            nearestEnemy->mGridX, nearestEnemy->mGridY);

        if (path.empty()) {
            // 无法到达，结束回合
            turnMgr.endTurn();
            mAIStep = 0;
            return false;
        }

        // 沿着路径移动到身法允许的最远位置（不踏上敌人所在格）
        int maxSteps = static_cast<int>(path.size()) - 1;
        int steps = std::min(turnMgr.mRemainingMoves, maxSteps);
        if (steps > 0) {
            mTargetGX = path[steps - 1].first;
            mTargetGY = path[steps - 1].second;
            grid.moveUnit(self->mId, self->mGridX, self->mGridY, mTargetGX, mTargetGY);
            self->mGridX = mTargetGX;
            self->mGridY = mTargetGY;
            self->payMoveCost(steps);
            turnMgr.mRemainingMoves -= steps;
            turnMgr.mHasMoved = true;
        }

        mDelayTimer = 0.6f;
        mAIStep = 1;
        return false;
    }

    // ===== 步骤1：移动后尝试攻击 =====
    if (mAIStep == 1) {
        CombatUnit* nearestEnemy = nullptr;
        int nearestDist = 9999;

        for (auto* u : allUnits) {
            if (!u->isAlive() || u->mTeam == self->mTeam) continue;
            int dist = grid.getDistance(self->mGridX, self->mGridY, u->mGridX, u->mGridY);
            if (dist < nearestDist) {
                nearestDist = dist;
                nearestEnemy = u;
            }
        }

        if (nearestEnemy != nullptr) {
            for (auto& s : self->mSkills) {
                if (!self->canAffordSkill(s)) continue;
                int d = grid.getDistance(self->mGridX, self->mGridY,
                    nearestEnemy->mGridX, nearestEnemy->mGridY);
                if (d <= s.mRange) {
                    self->paySkillCost(s);
                    s.mCurrentCooldown = s.mCooldown;

                    int damage = self->calculateDamage(s, *nearestEnemy);
                    nearestEnemy->takeDamage(damage);

                    char buf[256];
                    snprintf(buf, sizeof(buf), "[战斗] %s 对 %s 使用 %s，造成 %d 点伤害",
                        self->mName.c_str(), nearestEnemy->mName.c_str(),
                        s.mName.c_str(), damage);
                    LOG_INFO("%s", buf);

                    turnMgr.mHasActed = true;
                    mDelayTimer = 0.8f;
                    break;
                }
            }
        }

        turnMgr.checkBattleEnd(allUnits);
        mAIStep = 0;
        turnMgr.endTurn();
        return false;
    }

    return false;
}
