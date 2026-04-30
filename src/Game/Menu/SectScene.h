/**
 * SectScene.h - 门派场景
 *
 * 显示门派NPC列表和详情，提供与NPC交互的入口（功能留空）。
 * 可从门派下山回到大世界。
 */

#ifndef SECTSCENE_H
#define SECTSCENE_H

#include "Core/Scene.h"
#include "Core/UI/UIManager.h"
#include <vector>
#include <SDL3/SDL.h>

class EventHandler;
class Renderer;
class Font;
class SceneManager;
struct PlayerData;
struct SectDef;

class SectScene : public Scene {
public:
    SectScene(EventHandler& eh, Font* font, float vpW, float vpH,
              PlayerData* playerData, SceneManager* sceneMgr);

    void onEnter() override;
    void onExit() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

private:
    void buildUI();
    void selectNPC(int index);
    void updateNPCDisplay();

    EventHandler& mEH;
    Font* mFont;
    PlayerData* mPlayerData;
    SceneManager* mSceneManager;

    float mVpW, mVpH;
    UIManager mUIManager;

    const SectDef* mSectDef = nullptr;
    int mSelectedNPCIndex = -1;

    class UILabel* mSectNameLabel = nullptr;
    class UILabel* mSectDescLabel = nullptr;
    class UILabel* mNPCNameLabel = nullptr;
    class UILabel* mNPCTitleLabel = nullptr;
    class UILabel* mNPCGreetingLabel = nullptr;
    std::vector<class UIButton*> mNPCButtons;
};

#endif // SECTSCENE_H
