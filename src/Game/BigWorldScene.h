/**
 * BigWorldScene.h - 大世界探索场景
 *
 * 玩家在大世界地图上自由移动，遭遇敌人后进入战斗。
 * 提供背包(B)和角色面板(C)的叠加界面。
 */

#ifndef BIGWORLDSCENE_H
#define BIGWORLDSCENE_H

#include "Core/Scene.h"
#include "Core/UI/UIManager.h"
#include "Core/Camera2D.h"
#include "WorldMap.h"
#include <vector>
#include <SDL3/SDL.h>

class EventHandler;
class Renderer;
class Font;
class SceneManager;
struct PlayerData;

class BigWorldScene : public Scene {
public:
    BigWorldScene(EventHandler& eh, Font* font, float vpW, float vpH,
                  PlayerData* playerData, SceneManager* sceneMgr);

    void onEnter() override;
    void onExit() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

private:
    // 渲染
    void renderWorld(Renderer& renderer);
    void renderPlayer(Renderer& renderer);
    void renderEncounterMarkers(Renderer& renderer);
    void renderMinimap(Renderer& renderer);

    // 输入
    void handleWorldInput(float deltaTime);
    void handleOverlayInput();

    // 世界交互
    void checkEncounter();
    void spawnEncounterMarkers();

    // UI构建
    void buildInventoryUI();
    void buildCharPanelUI();
    void updateInventoryDisplay();
    void updateCharPanelDisplay();

    EventHandler& mEH;
    Font* mFont;
    PlayerData* mPlayerData;
    SceneManager* mSceneManager;

    WorldMap mWorldMap;
    Camera2D mCamera;
    UIManager mUIManager;

    // 玩家世界位置
    float mPlayerX = 0.0f;
    float mPlayerY = 0.0f;
    static constexpr float PLAYER_SPEED = 200.0f;
    static constexpr float PLAYER_SIZE = 32.0f;

    // 视角
    float mVpW, mVpH;

    // 遭遇点
    struct EncounterMarker {
        float wx, wy;
        bool active = true;
    };
    std::vector<EncounterMarker> mEncounters;
    float mEncounterSpawnTimer = 0.0f;
    bool mTransitionToCombat = false;

    // 叠加界面状态
    bool mInventoryOpen = false;
    bool mCharPanelOpen = false;

    // 库存UI元素引用
    UIElement* mInvOverlay = nullptr;
    UIElement* mInvBackdrop = nullptr;
    class UILabel* mInvItemNameLabel = nullptr;
    class UILabel* mInvItemDescLabel = nullptr;
    int mSelectedItemIndex = -1;
    std::vector<class UIButton*> mInvSlotButtons;

    // 角色面板UI元素引用
    UIElement* mCharOverlay = nullptr;
    UIElement* mCharBackdrop = nullptr;
    class UILabel* mCharNameLabel = nullptr;
    class UILabel* mCharStatsLabel = nullptr;
    class UILabel* mCharEquipLabel = nullptr;
    class UILabel* mCharSkillsLabel = nullptr;
};

#endif // BIGWORLDSCENE_H
