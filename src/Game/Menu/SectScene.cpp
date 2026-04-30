/**
 * SectScene.cpp - 门派场景实现
 */

#include "SectScene.h"
#include "../Core/PlayerData.h"
#include "../Data/DataManager.h"
#include "Core/EventHandler.h"
#include "Core/Renderer.h"
#include "Core/Font.h"
#include "Core/UI/UIPanel.h"
#include "Core/UI/UILabel.h"
#include "Core/UI/UIButton.h"
#include "Core/Log.h"
#include <cstdio>

SectScene::SectScene(EventHandler& eh, Font* font, float vpW, float vpH,
                     PlayerData* playerData, SceneManager* sceneMgr)
    : Scene("门派")
    , mEH(eh)
    , mFont(font)
    , mPlayerData(playerData)
    , mSceneManager(sceneMgr)
    , mVpW(vpW)
    , mVpH(vpH)
    , mUIManager(font)
{
}

void SectScene::onEnter()
{
    LOG_INFO("进入门派场景");

    if (mPlayerData && !mPlayerData->sectId.empty()) {
        mSectDef = DataManager::getInstance().getSect(mPlayerData->sectId);
    }

    buildUI();
}

void SectScene::onExit()
{
    mUIManager.clear();
    mNPCButtons.clear();
    mSectNameLabel = nullptr;
    mSectDescLabel = nullptr;
    mNPCNameLabel = nullptr;
    mNPCTitleLabel = nullptr;
    mNPCGreetingLabel = nullptr;
    mSelectedNPCIndex = -1;

    LOG_INFO("离开门派场景");
}

void SectScene::buildUI()
{
    auto backdrop = std::make_unique<UIPanel>();
    backdrop->mX = 0;
    backdrop->mY = 0;
    backdrop->mWidth = mVpW;
    backdrop->mHeight = mVpH;
    backdrop->mBgR = 20;
    backdrop->mBgG = 15;
    backdrop->mBgB = 30;
    backdrop->mBgA = 255;

    // 门派名称
    auto nameLabel = std::make_unique<UILabel>();
    nameLabel->mX = 30;
    nameLabel->mY = 20;
    nameLabel->mText = mSectDef ? mSectDef->name : "散修";
    nameLabel->mTextR = 255;
    nameLabel->mTextG = 220;
    nameLabel->mTextB = 100;
    mSectNameLabel = nameLabel.get();
    backdrop->addChild(std::move(nameLabel));

    // 门派描述
    auto descLabel = std::make_unique<UILabel>();
    descLabel->mX = 30;
    descLabel->mY = 55;
    descLabel->mWidth = static_cast<float>(static_cast<int>(mVpW) - 60);
    descLabel->mText = mSectDef ? mSectDef->desc : "云游四方，无门无派。";
    descLabel->mTextR = 180;
    descLabel->mTextG = 180;
    descLabel->mTextB = 200;
    mSectDescLabel = descLabel.get();
    backdrop->addChild(std::move(descLabel));

    // NPC列表面板
    auto npcListBg = std::make_unique<UIPanel>();
    npcListBg->mX = 20;
    npcListBg->mY = 100;
    npcListBg->mWidth = 240;
    npcListBg->mHeight = mVpH - 200;
    npcListBg->mBgR = 30;
    npcListBg->mBgG = 25;
    npcListBg->mBgB = 40;
    npcListBg->mBgA = 200;
    auto* npcListBgPtr = npcListBg.get();
    backdrop->addChild(std::move(npcListBg));

    // NPC列表标题
    auto npcListTitle = std::make_unique<UILabel>();
    npcListTitle->mX = 10;
    npcListTitle->mY = 8;
    npcListTitle->mText = "-- 门中人物 --";
    npcListTitle->mTextR = 200;
    npcListTitle->mTextG = 200;
    npcListTitle->mTextB = 200;
    npcListBgPtr->addChild(std::move(npcListTitle));

    // NPC按钮
    mNPCButtons.clear();
    if (mSectDef) {
        float btnY = 35;
        for (int i = 0; i < static_cast<int>(mSectDef->npcs.size()); ++i) {
            auto& npc = mSectDef->npcs[i];

            auto btn = std::make_unique<UIButton>();
            btn->mX = 10;
            btn->mY = btnY;
            btn->mWidth = 220;
            btn->mHeight = 30;
            btn->mText = npc.name;
            btn->mNormalR = 200; btn->mNormalG = 200; btn->mNormalB = 200;
            btn->mHoverR = 255;  btn->mHoverG = 220;  btn->mHoverB = 100;
            btn->mPressR = 255;  btn->mPressG = 180;  btn->mPressB = 50;
            btn->mNormalBgR = 45; btn->mNormalBgG = 40; btn->mNormalBgB = 55;
            btn->mHoverBgR = 60;  btn->mHoverBgG = 50;  btn->mHoverBgB = 70;
            btn->mPressBgR = 35;  btn->mPressBgG = 30;  btn->mPressBgB = 45;

            int npcIdx = i;
            btn->mOnClick = [this, npcIdx]() {
                selectNPC(npcIdx);
            };

            mNPCButtons.push_back(btn.get());
            npcListBgPtr->addChild(std::move(btn));
            btnY += 40;
        }
    }

    // 右侧NPC详情面板
    auto detailPanel = std::make_unique<UIPanel>();
    detailPanel->mX = 280;
    detailPanel->mY = 100;
    detailPanel->mWidth = static_cast<float>(static_cast<int>(mVpW) - 300);
    detailPanel->mHeight = mVpH - 260;
    detailPanel->mBgR = 25;
    detailPanel->mBgG = 25;
    detailPanel->mBgB = 35;
    detailPanel->mBgA = 220;
    auto* detailPanelPtr = detailPanel.get();
    backdrop->addChild(std::move(detailPanel));

    // NPC名称
    auto npcName = std::make_unique<UILabel>();
    npcName->mX = 15;
    npcName->mY = 15;
    npcName->mText = "";
    npcName->mTextR = 255;
    npcName->mTextG = 220;
    npcName->mTextB = 100;
    mNPCNameLabel = npcName.get();
    detailPanelPtr->addChild(std::move(npcName));

    // NPC称号
    auto npcTitle = std::make_unique<UILabel>();
    npcTitle->mX = 15;
    npcTitle->mY = 45;
    npcTitle->mText = "";
    npcTitle->mTextR = 150;
    npcTitle->mTextG = 180;
    npcTitle->mTextB = 150;
    mNPCTitleLabel = npcTitle.get();
    detailPanelPtr->addChild(std::move(npcTitle));

    // NPC问候语
    auto npcGreeting = std::make_unique<UILabel>();
    npcGreeting->mX = 15;
    npcGreeting->mY = 85;
    npcGreeting->mWidth = detailPanelPtr->mWidth - 30;
    npcGreeting->mText = "点击左侧人物查看详情";
    npcGreeting->mTextR = 200;
    npcGreeting->mTextG = 200;
    npcGreeting->mTextB = 200;
    mNPCGreetingLabel = npcGreeting.get();
    detailPanelPtr->addChild(std::move(npcGreeting));

    // 下山按钮
    auto leaveBtn = std::make_unique<UIButton>();
    leaveBtn->mX = mVpW / 2 - 80;
    leaveBtn->mY = mVpH - 80;
    leaveBtn->mWidth = 160;
    leaveBtn->mHeight = 40;
    leaveBtn->mText = "下山历练";
    leaveBtn->mNormalR = 220; leaveBtn->mNormalG = 220; leaveBtn->mNormalB = 220;
    leaveBtn->mHoverR = 255;  leaveBtn->mHoverG = 255;  leaveBtn->mHoverB = 200;
    leaveBtn->mPressR = 255;  leaveBtn->mPressG = 200;  leaveBtn->mPressB = 100;
    leaveBtn->mNormalBgR = 50; leaveBtn->mNormalBgG = 40; leaveBtn->mNormalBgB = 60;
    leaveBtn->mHoverBgR = 70;  leaveBtn->mHoverBgG = 55;  leaveBtn->mHoverBgB = 80;
    leaveBtn->mPressBgR = 40;  leaveBtn->mPressBgG = 30;  leaveBtn->mPressBgB = 50;
    leaveBtn->mOnClick = [this]() {
        LOG_INFO("下山历练");
        if (mSceneManager) {
            mSceneManager->requestPopScene();
        }
    };
    backdrop->addChild(std::move(leaveBtn));

    mUIManager.addElement(std::move(backdrop));

    // 默认选中第一个NPC
    if (mSectDef && !mSectDef->npcs.empty()) {
        selectNPC(0);
    }
}

void SectScene::selectNPC(int index)
{
    mSelectedNPCIndex = index;
    updateNPCDisplay();
}

void SectScene::updateNPCDisplay()
{
    if (!mSectDef || mSelectedNPCIndex < 0
        || mSelectedNPCIndex >= static_cast<int>(mSectDef->npcs.size())) {
        return;
    }

    auto& npc = mSectDef->npcs[mSelectedNPCIndex];

    if (mNPCNameLabel) {
        mNPCNameLabel->mText = npc.name;
    }
    if (mNPCTitleLabel) {
        mNPCTitleLabel->mText = npc.title;
    }
    if (mNPCGreetingLabel) {
        mNPCGreetingLabel->mText = npc.greeting;
    }
}

void SectScene::onUpdate(float deltaTime)
{
    (void)deltaTime;

    float mx, my;
    mEH.getMousePosition(mx, my);

    mUIManager.updateHover(mx, my);

    if (mEH.isMouseButtonJustPressed(SDL_BUTTON_LEFT)) {
        mUIManager.handleMouseDown(mx, my);
    }
    if (mEH.isMouseButtonJustReleased(SDL_BUTTON_LEFT)) {
        mUIManager.handleMouseUp(mx, my);
    }

    if (mEH.isKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        if (mSceneManager) {
            mSceneManager->requestPopScene();
        }
    }
}

void SectScene::onRender(Renderer& renderer)
{
    renderer.setDrawColor(20, 15, 30, 255);
    renderer.fillRect(0, 0, mVpW, mVpH);

    renderer.setDrawColor(80, 60, 30, 150);
    renderer.drawRect(10, 10, mVpW - 20, mVpH - 20);

    if (mSectDef) {
        char buf[128];
        snprintf(buf, sizeof(buf), "门派加成: 攻击+%d  防御+%d  HP+%d  速度+%d  灵力+%d",
            mSectDef->bonusAtk, mSectDef->bonusDef, mSectDef->bonusHP,
            mSectDef->bonusSpeed, mSectDef->bonusSP);
        SDL_Texture* tex = mFont->renderText(renderer, buf, 140, 140, 160);
        if (tex) {
            int tw = 0, th = 0;
            mFont->measureText(buf, tw, th);
            renderer.drawTexture(tex, mVpW / 2 - static_cast<float>(tw) / 2, mVpH - 40);
            SDL_DestroyTexture(tex);
        }
    }

    mUIManager.setRenderer(&renderer);
    mUIManager.render();
}
