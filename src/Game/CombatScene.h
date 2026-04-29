/**
 * CombatScene.h - 战斗场景
 *
 * 整合网格地图、回合管理、单位、AI和UI，提供完整的战斗体验。
 * 接受PlayerData引用以支持世界↔战斗的玩家数据同步。
 */

#ifndef COMBATSCENE_H
#define COMBATSCENE_H

#include "Core/Scene.h"
#include "Core/UI/UIManager.h"
#include "GridMap.h"
#include "CombatUnit.h"
#include "TurnManager.h"
#include "CombatAI.h"
#include "Core/Camera2D.h"
#include <memory>
#include <vector>

class Renderer;
class EventHandler;
class Font;
class SceneManager;
struct PlayerData;

class CombatScene : public Scene {
public:
    CombatScene(EventHandler& eventHandler, Font* font,
                float viewportW, float viewportH,
                PlayerData* playerData, SceneManager* sceneMgr);
    ~CombatScene() = default;

    void onEnter() override;
    void onExit() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

private:
    void setupBattlefield();
    void setupPlayerTeam();
    void setupEnemyTeam();
    void initBattle();

    void handlePlayerInput(float deltaTime);
    void handleGridClick(int gx, int gy);
    void handleSkillSelect(int skillIndex);
    void executeAttack(CombatUnit& attacker, CombatUnit& target, Skill& skill);

    void renderGrid(Renderer& renderer);
    void renderUnits(Renderer& renderer);
    void renderMoveRange(Renderer& renderer);
    void renderAttackTargets(Renderer& renderer);
    void renderHUD(Renderer& renderer);

    void buildHUD();
    void updateHUD();

    // 胜利奖励
    void grantVictoryRewards();

    CombatUnit* getUnitAt(int gx, int gy);
    CombatUnit* getUnitById(uint32_t id);

    EventHandler& mEventHandler;
    Font* mFont;
    PlayerData* mPlayerData;
    SceneManager* mSceneManager;
    Camera2D mCamera;
    UIManager mUIManager;
    GridMap mGrid;
    TurnManager mTurnManager;
    CombatAI mAI;

    std::vector<std::unique_ptr<CombatUnit>> mUnits;
    std::vector<CombatUnit*> mUnitPtrs;

    bool mUnitSelected = false;
    int mSelectedSkillIndex = -1;
    std::vector<std::pair<int, int>> mMoveRange;
    std::vector<std::pair<int, int>> mAttackTargets;

    UIElement* mTopPanel = nullptr;
    UIElement* mSkillPanel = nullptr;
    UIElement* mBottomBar = nullptr;

    class UILabel* mTurnLabel = nullptr;
    class UILabel* mUnitNameLabel = nullptr;
    class UIProgressBar* mHPBar = nullptr;
    class UIProgressBar* mSPBar = nullptr;

    std::vector<class UIButton*> mSkillButtons;

    float mViewportW, mViewportH;

    // 战斗结束逻辑
    float mBattleEndTimer = 0.0f;
    bool mBattleEnded = false;
    bool mRewardsGiven = false;

    // 技能按钮重建追踪（避免每帧重建导致悬空指针）
    uint32_t mLastActiveUnitId = 0;
    CombatPhase mLastPhase = CombatPhase::PlayerTurn;
    int mLastSkillCount = -1;
    int mLastSelectedSkill = -1;
};

#endif // COMBATSCENE_H
