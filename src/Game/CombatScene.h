/**
 * CombatScene.h - 战斗场景
 *
 * 整合网格地图、回合管理、单位、AI和UI，提供完整的战斗体验。
 * 继承Scene，直接挂载到SceneManager。
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

class CombatScene : public Scene {
public:
    CombatScene(EventHandler& eventHandler, Font* font,
                float viewportW, float viewportH);
    ~CombatScene() = default;

    void onEnter() override;
    void onExit() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

private:
    // ==================== 初始化 ====================

    void setupBattlefield();
    void setupPlayerTeam();
    void setupEnemyTeam();
    void initBattle();

    // ==================== 玩家输入处理 ====================

    void handlePlayerInput(float deltaTime);
    void handleGridClick(int gx, int gy);
    void handleSkillSelect(int skillIndex);
    void executeAttack(CombatUnit& attacker, CombatUnit& target, Skill& skill);

    // ==================== 渲染 ====================

    void renderGrid(Renderer& renderer);
    void renderUnits(Renderer& renderer);
    void renderMoveRange(Renderer& renderer);
    void renderAttackTargets(Renderer& renderer);
    void renderHUD(Renderer& renderer);

    // ==================== UI构建 ====================

    void buildHUD();
    void updateHUD();

    // ==================== 辅助 ====================

    CombatUnit* getUnitAt(int gx, int gy);
    CombatUnit* getUnitById(uint32_t id);

    // ==================== 子系统 ====================

    EventHandler& mEventHandler;
    Font* mFont;
    Camera2D mCamera;
    UIManager mUIManager;
    GridMap mGrid;
    TurnManager mTurnManager;
    CombatAI mAI;

    // ==================== 数据 ====================

    std::vector<std::unique_ptr<CombatUnit>> mUnits;  // 拥有所有单位
    std::vector<CombatUnit*> mUnitPtrs;               // 指针列表（给TurnManager/AI）

    // ==================== 玩家输入状态 ====================

    bool mUnitSelected = false;
    int mSelectedSkillIndex = -1;  // -1 = 未选技能
    std::vector<std::pair<int, int>> mMoveRange;
    std::vector<std::pair<int, int>> mAttackTargets;

    // ==================== UI元素引用 ====================

    UIElement* mTopPanel = nullptr;
    UIElement* mSkillPanel = nullptr;
    UIElement* mBottomBar = nullptr;

    // 动态更新的元素
    class UILabel* mTurnLabel = nullptr;
    class UILabel* mUnitNameLabel = nullptr;
    class UIProgressBar* mHPBar = nullptr;
    class UIProgressBar* mSPBar = nullptr;

    // 技能按钮
    std::vector<class UIButton*> mSkillButtons;

    float mViewportW, mViewportH;
};

#endif // COMBATSCENE_H
