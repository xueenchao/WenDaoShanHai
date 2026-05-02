/**
 * CombatScene.cpp - 战斗场景实现
 *
 * BG3风格的回合制网格战棋战斗。
 * 支持从大世界进入，战斗结束后返回大世界。
 */

#include "CombatScene.h"
#include "../Core/PlayerData.h"
#include "../Data/DataManager.h"
#include "Core/Scene.h"       // SceneManager
#include "Core/Renderer.h"
#include "Core/EventHandler.h"
#include "Core/Font.h"
#include "Core/UI/UIPanel.h"
#include "Core/UI/UILabel.h"
#include "Core/UI/UIButton.h"
#include "Core/UI/UIProgressBar.h"
#include "Core/Log.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <cstdlib>

// ==================== 构造 ====================

CombatScene::CombatScene(EventHandler& eventHandler, Font* font,
                         float viewportW, float viewportH,
                         PlayerData* playerData, SceneManager* sceneMgr)
    : Scene("战斗场景")
    , mEventHandler(eventHandler)
    , mFont(font)
    , mPlayerData(playerData)
    , mSceneManager(sceneMgr)
    , mCamera(viewportW, viewportH)
    , mUIManager(font)
    , mGrid(8, 6, 64.0f)
    , mViewportW(viewportW)
    , mViewportH(viewportH)
{
}

// ==================== 生命周期 ====================

void CombatScene::onEnter()
{
    LOG_INFO("进入战斗场景");

    mBattleEnded = false;
    mBattleEndTimer = 0.0f;
    mRewardsGiven = false;
    mSelectedSkillIndex = -1;
    mMoveRange.clear();
    mAttackTargets.clear();

    mLastActiveUnitId = 0;
    mLastPhase = CombatPhase::PlayerTurn;
    mLastSkillCount = -1;
    mLastSelectedSkill = -1;

    setupBattlefield();
    setupPlayerTeam();
    setupEnemyTeam();
    initBattle();

    float gridW = mGrid.getWorldWidth();
    float gridH = mGrid.getWorldHeight();
    mCamera.setPosition(gridW * 0.5f, gridH * 0.5f);
    mCamera.setZoom(1.5f);

    buildHUD();
}

void CombatScene::onExit()
{
    LOG_INFO("离开战斗场景");
    mUIManager.clear();
    mUnits.clear();
    mUnitPtrs.clear();
    mSkillButtons.clear();
    mTalismanButtons.clear();
}

// ==================== 更新 ====================

void CombatScene::onUpdate(float deltaTime)
{
    mCamera.update(deltaTime);

    float mx, my;
    mEventHandler.getMousePosition(mx, my);
    mUIManager.updateHover(mx, my);

    // 战斗结束状态
    if (mBattleEnded) {
        mBattleEndTimer += deltaTime;
        // 2秒后或按空格返回
        if (mBattleEndTimer > 2.0f
            || mEventHandler.isKeyJustPressed(SDL_SCANCODE_SPACE)
            || mEventHandler.isKeyJustPressed(SDL_SCANCODE_ESCAPE)
            || mEventHandler.isKeyJustPressed(SDL_SCANCODE_RETURN)) {
            if (mSceneManager) {
                mSceneManager->requestPopScene();
            }
        }
        updateHUD();
        // 处理鼠标释放以触发"返回世界"按钮
        if (mEventHandler.isMouseButtonJustReleased(SDL_BUTTON_LEFT)) {
            mUIManager.handleMouseUp(mx, my);
        }
        return;
    }

    // 检查战斗结束
    CombatPhase phase = mTurnManager.checkBattleEnd(mUnitPtrs);
    if (phase == CombatPhase::Victory || phase == CombatPhase::Defeat) {
        LOG_INFO("战斗结束! 阶段=%s 玩家单位数=%zu 敌方单位数=%zu",
            phase == CombatPhase::Victory ? "胜利" : "失败",
            mUnitPtrs.size(), mUnits.size());
        for (auto& u : mUnits) {
            LOG_INFO("  单位: %s HP=%d/%d team=%d alive=%d",
                u->mName.c_str(), u->mHP, u->mMaxHP, u->mTeam, u->isAlive());
        }
        mBattleEnded = true;
        mBattleEndTimer = 0.0f;
        if (phase == CombatPhase::Victory && !mRewardsGiven) {
            grantVictoryRewards();
            mRewardsGiven = true;
        }
        updateHUD();
        return;
    }

    if (mTurnManager.mPhase == CombatPhase::EnemyTurn) {
        mAI.executeTurn(mGrid, mTurnManager, mUnitPtrs, deltaTime);
        updateHUD();
        return;
    }

    if (mTurnManager.mPhase == CombatPhase::PlayerTurn) {
        handlePlayerInput(deltaTime);
    }

    updateHUD();
}

// ==================== 渲染 ====================

void CombatScene::onRender(Renderer& renderer)
{
    renderer.setDrawColor(15, 30, 20, 255);
    renderer.clear();

    renderGrid(renderer);
    renderMoveRange(renderer);
    renderAttackTargets(renderer);
    renderUnits(renderer);
    renderHUD(renderer);
}

// ==================== 初始化 ====================

void CombatScene::setupBattlefield()
{
    LOG_INFO("战场: %d x %d 网格, 每格 %.0fpx",
        mGrid.getCols(), mGrid.getRows(), mGrid.getTileSize());
}

void CombatScene::setupPlayerTeam()
{
    static uint32_t nextId = 100;

    // 主角色来自PlayerData
    {
        auto unit = std::make_unique<CombatUnit>();
        unit->mId = nextId++;
        unit->mName = mPlayerData ? mPlayerData->name : "修士";
        unit->mHP = mPlayerData ? mPlayerData->hp : 120;
        unit->mMaxHP = mPlayerData ? mPlayerData->maxHP : 120;
        unit->mAttack = mPlayerData ? mPlayerData->attack : 20;
        unit->mDefense = mPlayerData ? mPlayerData->defense : 8;
        unit->mSpeed = mPlayerData ? mPlayerData->speed : 12;
        unit->mElement = mPlayerData ? mPlayerData->element : Element::Jin;
        unit->mSpiritPower = mPlayerData ? mPlayerData->spiritPower : 6;
        unit->mMaxSpiritPower = mPlayerData ? mPlayerData->maxSpiritPower : 6;
        unit->mMovementPoints = mPlayerData ? mPlayerData->movementPoints : 4;
        unit->mMaxMovementPoints = mPlayerData ? mPlayerData->maxMovementPoints : 4;
        unit->mGridX = 1;
        unit->mGridY = 2;
        unit->mTeam = 0;

        if (mPlayerData && !mPlayerData->skills.empty()) {
            unit->mSkills = mPlayerData->skills;
        } else {
            // 从DataManager加载默认技能
            for (auto& skillId : DataManager::getInstance().getStartingSkills()) {
                const Skill* sk = DataManager::getInstance().getSkill(skillId);
                if (sk) unit->mSkills.push_back(*sk);
            }
        }

        mGrid.setOccupier(unit->mGridX, unit->mGridY, unit->mId);
        mUnitPtrs.push_back(unit.get());
        mUnits.push_back(std::move(unit));
    }

    // 灵狐（宠物）
    {
        auto unit = std::make_unique<CombatUnit>();
        unit->mId = nextId++;
        unit->mName = "灵狐";
        unit->mHP = 80;
        unit->mMaxHP = 80;
        unit->mAttack = 12;
        unit->mDefense = 4;
        unit->mSpeed = 10;
        unit->mElement = Element::Mu;
        unit->mSpiritPower = 4;
        unit->mMaxSpiritPower = 4;
        unit->mMovementPoints = 5;
        unit->mMaxMovementPoints = 5;
        unit->mGridX = 1;
        unit->mGridY = 4;
        unit->mTeam = 0;

        const Skill* vineSkill = DataManager::getInstance().getSkill("vine_whip");
        if (vineSkill) unit->mSkills.push_back(*vineSkill);

        mGrid.setOccupier(unit->mGridX, unit->mGridY, unit->mId);
        mUnitPtrs.push_back(unit.get());
        mUnits.push_back(std::move(unit));
    }
}

void CombatScene::setupEnemyTeam()
{
    static uint32_t nextId = 200;

    auto& dm = DataManager::getInstance();
    const auto& enemyIds = dm.getEnemyIds();
    int poolSize = static_cast<int>(enemyIds.size());
    if (poolSize == 0) return;

    int level = mPlayerData ? mPlayerData->level : 1;

    // 随机选3个不同的敌人
    int idx1 = rand() % poolSize;
    int idx2 = (idx1 + 1 + rand() % (poolSize - 1)) % poolSize;
    int idx3 = (idx1 + 2 + rand() % (poolSize - 2)) % poolSize;

    const char* picks[3] = {enemyIds[idx1].c_str(), enemyIds[idx2].c_str(), enemyIds[idx3].c_str()};
    int positions[3][2] = {{6,1}, {6,3}, {5,5}};

    for (int i = 0; i < 3; ++i) {
        const EnemyTemplateDef* et = dm.getEnemyTemplate(picks[i]);
        if (!et) continue;

        auto unit = std::make_unique<CombatUnit>();
        unit->mId = nextId++;
        unit->mName = et->name;
        unit->mHP = et->hp + (level - 1) * 5;
        unit->mMaxHP = unit->mHP;
        unit->mAttack = et->attack + (level - 1) * 1;
        unit->mDefense = et->defense;
        unit->mSpeed = et->speed;
        unit->mElement = et->element;
        unit->mSpiritPower = et->spirit;
        unit->mMaxSpiritPower = et->spirit;
        unit->mMovementPoints = et->moves;
        unit->mMaxMovementPoints = et->moves;
        unit->mGridX = positions[i][0];
        unit->mGridY = positions[i][1];
        unit->mTeam = 1;

        const Skill* sk = dm.getSkill(et->skillId);
        if (sk) unit->mSkills.push_back(*sk);

        mGrid.setOccupier(unit->mGridX, unit->mGridY, unit->mId);
        mUnitPtrs.push_back(unit.get());
        mUnits.push_back(std::move(unit));
    }
}

void CombatScene::initBattle()
{
    mTurnManager.startBattle(mUnitPtrs);

    CombatUnit* active = mTurnManager.getActiveUnit();
    if (active != nullptr && active->mTeam == 0) {
        mMoveRange = mGrid.getMoveRange(active->mGridX, active->mGridY,
            mTurnManager.mRemainingMoves);
    }
}

// ==================== 胜利奖励 ====================

void CombatScene::grantVictoryRewards()
{
    if (!mPlayerData) return;

    // XP奖励
    int xpGain = 20 + rand() % 21; // 20-40
    mPlayerData->xp += xpGain;
    LOG_INFO("战斗胜利！获得 %d 经验值 (总计: %d)", xpGain, mPlayerData->xp);

    // 继承玩家单位剩余的HP/SP
    for (auto& unit : mUnits) {
        if (unit->mTeam == 0 && unit->isAlive()
            && unit->mName == mPlayerData->name) {
            mPlayerData->hp = unit->mHP;
            mPlayerData->spiritPower = unit->mSpiritPower;
            break;
        }
    }

    // 60%几率掉落物品（从DataManager获取物品数据）
    if (rand() % 100 < 60) {
        const char* lootIds[] = {"heal_potion", "spirit_pill", "iron_sword", "leather_armor", "jade_ring"};
        int roll = rand() % 5;
        Item loot = DataManager::getInstance().createItem(lootIds[roll]);
        loot.quantity = 1;
        bool added = mPlayerData->addItem(loot);
        if (added) {
            LOG_INFO("获得战利品: %s", loot.name.c_str());
        } else {
            LOG_INFO("背包已满，无法获得: %s", loot.name.c_str());
        }
    }
}

// ==================== 玩家输入处理 ====================

void CombatScene::handlePlayerInput(float /*deltaTime*/)
{
    float mx, my;
    mEventHandler.getMousePosition(mx, my);

    mUIManager.updateHover(mx, my);

    if (mEventHandler.isMouseButtonJustPressed(SDL_BUTTON_LEFT)) {
        UIElement* uiHit = mUIManager.getElementAt(mx, my);
        if (uiHit != nullptr) {
            mUIManager.handleMouseDown(mx, my);
            return;
        }

        auto world = mCamera.screenToWorld(mx, my);
        int gx, gy;
        mGrid.worldToGrid(world.x, world.y, gx, gy);

        if (mGrid.isInBounds(gx, gy)) {
            handleGridClick(gx, gy);
        } else {
            mSelectedSkillIndex = -1;
            mAttackTargets.clear();
        }
    }

    if (mEventHandler.isMouseButtonJustReleased(SDL_BUTTON_LEFT)) {
        mUIManager.handleMouseUp(mx, my);
    }

    if (mEventHandler.isMouseButtonJustPressed(SDL_BUTTON_RIGHT)) {
        mSelectedSkillIndex = -1;
        mAttackTargets.clear();
    }
}

void CombatScene::handleGridClick(int gx, int gy)
{
    CombatUnit* active = mTurnManager.getActiveUnit();
    if (active == nullptr || active->mTeam != 0) return;

    if (mSelectedSkillIndex >= 0
        && mSelectedSkillIndex < static_cast<int>(active->mSkills.size())) {

        Skill& skill = active->mSkills[mSelectedSkillIndex];

        for (auto& t : mAttackTargets) {
            if (t.first == gx && t.second == gy) {
                CombatUnit* target = getUnitAt(gx, gy);
                if (target != nullptr && active->canAffordSkill(skill)) {
                    executeAttack(*active, *target, skill);
                    mTurnManager.mHasActed = true;
                    mSelectedSkillIndex = -1;
                    mAttackTargets.clear();

                    mTurnManager.checkBattleEnd(mUnitPtrs);
                    updateHUD();
                    return;
                }
            }
        }

        mSelectedSkillIndex = -1;
        mAttackTargets.clear();
        return;
    }

    if (!mTurnManager.mHasMoved) {
        for (auto& cell : mMoveRange) {
            if (cell.first == gx && cell.second == gy) {
                int dist = mGrid.getDistance(active->mGridX, active->mGridY, gx, gy);
                if (active->canMove(dist)) {
                    mGrid.moveUnit(active->mId,
                        active->mGridX, active->mGridY, gx, gy);
                    active->mGridX = gx;
                    active->mGridY = gy;
                    active->payMoveCost(dist);
                    mTurnManager.mRemainingMoves -= dist;
                    mTurnManager.mHasMoved = true;
                    mMoveRange.clear();

                    LOG_INFO("%s 移动到 (%d, %d)",
                        active->mName.c_str(), gx, gy);
                }
                return;
            }
        }
    }
}

void CombatScene::handleSkillSelect(int skillIndex)
{
    CombatUnit* active = mTurnManager.getActiveUnit();
    if (active == nullptr || active->mTeam != 0) return;

    if (skillIndex < 0 || skillIndex >= static_cast<int>(active->mSkills.size())) {
        mSelectedSkillIndex = -1;
        mAttackTargets.clear();
        return;
    }

    Skill& skill = active->mSkills[skillIndex];

    if (!active->canAffordSkill(skill)) {
        LOG_WARNING("无法使用技能 %s：灵力不足或冷却中", skill.mName.c_str());
        return;
    }

    mSelectedSkillIndex = skillIndex;

    mAttackTargets.clear();
    auto tilesInRange = mGrid.getTilesInRange(active->mGridX, active->mGridY, skill.mRange);

    for (auto& t : tilesInRange) {
        CombatUnit* target = getUnitAt(t.first, t.second);
        if (target != nullptr && target->mTeam != active->mTeam) {
            mAttackTargets.push_back(t);
        }
    }
}

void CombatScene::executeAttack(CombatUnit& attacker, CombatUnit& target, Skill& skill)
{
    attacker.paySkillCost(skill);
    skill.mCurrentCooldown = skill.mCooldown;

    int damage = attacker.calculateDamage(skill, target);
    target.takeDamage(damage);

    char buf[256];
    snprintf(buf, sizeof(buf), "[战斗] %s 对 %s 使用 %s，造成 %d 伤害！",
        attacker.mName.c_str(), target.mName.c_str(),
        skill.mName.c_str(), damage);
    LOG_INFO("%s", buf);

    if (!target.isAlive()) {
        LOG_INFO("%s 被击败！", target.mName.c_str());
        mGrid.setOccupier(target.mGridX, target.mGridY, 0);
    }
}

// ==================== 渲染 ====================

void CombatScene::renderGrid(Renderer& renderer)
{
    float tileSize = mGrid.getTileSize();
    int cols = mGrid.getCols();
    int rows = mGrid.getRows();

    for (int gy = 0; gy < rows; ++gy) {
        for (int gx = 0; gx < cols; ++gx) {
            float wx, wy;
            mGrid.gridToWorld(gx, gy, wx, wy);
            auto screen = mCamera.worldToScreen(wx, wy);
            float sw = tileSize * mCamera.getZoom();
            float sh = tileSize * mCamera.getZoom();

            bool dark = (gx + gy) % 2 == 0;
            renderer.setDrawColor(dark ? 25 : 35, dark ? 45 : 55, dark ? 20 : 28, 255);
            renderer.fillRect(screen.x, screen.y, sw, sh);
        }
    }

    renderer.setDrawColor(60, 80, 55, 120);

    for (int gx = 0; gx <= cols; ++gx) {
        float wx = static_cast<float>(gx) * tileSize;
        auto top = mCamera.worldToScreen(wx, 0);
        auto bot = mCamera.worldToScreen(wx, rows * tileSize);
        renderer.drawLine(top.x, top.y, bot.x, bot.y);
    }

    for (int gy = 0; gy <= rows; ++gy) {
        float wy = static_cast<float>(gy) * tileSize;
        auto left = mCamera.worldToScreen(0, wy);
        auto right = mCamera.worldToScreen(cols * tileSize, wy);
        renderer.drawLine(left.x, left.y, right.x, right.y);
    }
}

void CombatScene::renderMoveRange(Renderer& renderer)
{
    if (mTurnManager.mPhase != CombatPhase::PlayerTurn) return;

    renderer.setDrawColor(50, 120, 220, 100);
    float tileSize = mGrid.getTileSize();
    float zoom = mCamera.getZoom();
    float sw = tileSize * zoom;
    float sh = tileSize * zoom;

    for (auto& cell : mMoveRange) {
        float wx, wy;
        mGrid.gridToWorld(cell.first, cell.second, wx, wy);
        auto screen = mCamera.worldToScreen(wx, wy);
        renderer.fillRect(screen.x, screen.y, sw, sh);
    }
}

void CombatScene::renderAttackTargets(Renderer& renderer)
{
    renderer.setDrawColor(220, 40, 40, 140);
    float tileSize = mGrid.getTileSize();
    float zoom = mCamera.getZoom();
    float sw = tileSize * zoom;
    float sh = tileSize * zoom;

    for (auto& cell : mAttackTargets) {
        float wx, wy;
        mGrid.gridToWorld(cell.first, cell.second, wx, wy);
        auto screen = mCamera.worldToScreen(wx, wy);
        renderer.fillRect(screen.x, screen.y, sw, sh);
    }
}

void CombatScene::renderUnits(Renderer& renderer)
{
    float tileSize = mGrid.getTileSize();
    float zoom = mCamera.getZoom();
    float unitSize = tileSize * zoom * 0.8f;
    float offset = (tileSize * zoom - unitSize) * 0.5f;

    for (auto& unit : mUnits) {
        if (!unit->isAlive()) continue;

        float wx, wy;
        mGrid.gridToWorld(unit->mGridX, unit->mGridY, wx, wy);
        auto screen = mCamera.worldToScreen(wx, wy);

        if (unit->mTeam == 0) {
            renderer.setDrawColor(50, 200, 80, 255);
        } else {
            renderer.setDrawColor(200, 50, 50, 255);
        }

        renderer.fillRect(screen.x + offset, screen.y + offset, unitSize, unitSize);

        CombatUnit* active = mTurnManager.getActiveUnit();
        if (active != nullptr && active->mId == unit->mId) {
            renderer.setDrawColor(255, 215, 0, 255);
        } else if (unit->mTeam == 0) {
            renderer.setDrawColor(100, 220, 120, 255);
        } else {
            renderer.setDrawColor(255, 100, 100, 255);
        }
        renderer.drawRect(screen.x + offset, screen.y + offset, unitSize, unitSize);

        if (mFont != nullptr) {
            const char* name = unit->mName.c_str();
            SDL_Texture* tex = mFont->renderText(renderer, name, 240, 240, 240,
                Font::RenderQuality::Solid);
            if (tex != nullptr) {
                float tw, th;
                SDL_GetTextureSize(tex, &tw, &th);
                renderer.drawTexture(tex,
                    screen.x + offset + (unitSize - tw) * 0.5f,
                    screen.y + offset + unitSize + 2.0f);
                SDL_DestroyTexture(tex);
            }
        }

        float hpRatio = static_cast<float>(unit->mHP) / static_cast<float>(unit->mMaxHP);
        float barW = unitSize;
        float barH = 4.0f;
        float barY = screen.y + offset + unitSize + 16.0f;

        renderer.setDrawColor(40, 40, 40, 200);
        renderer.fillRect(screen.x + offset, barY, barW, barH);
        renderer.setDrawColor(
            static_cast<Uint8>(200 * (1.0f - hpRatio)),
            static_cast<Uint8>(200 * hpRatio),
            30, 255);
        renderer.fillRect(screen.x + offset, barY, barW * hpRatio, barH);
    }
}

void CombatScene::renderHUD(Renderer& renderer)
{
    mUIManager.setRenderer(&renderer);
    mUIManager.render();
}

// ==================== HUD构建 ====================

void CombatScene::buildHUD()
{
    // 顶部面板
    {
        auto panel = std::make_unique<UIPanel>();
        panel->mX = 0.0f;
        panel->mY = 0.0f;
        panel->mWidth = mViewportW;
        panel->mHeight = 80.0f;
        panel->mBgR = 0; panel->mBgG = 0; panel->mBgB = 0; panel->mBgA = 200;

        auto nameLabel = std::make_unique<UILabel>();
        nameLabel->mX = 15.0f;
        nameLabel->mY = 8.0f;
        nameLabel->mText = "等待中...";
        nameLabel->mTextR = 255; nameLabel->mTextG = 215; nameLabel->mTextB = 0;
        mUnitNameLabel = nameLabel.get();
        panel->addChild(std::move(nameLabel));

        auto hpBar = std::make_unique<UIProgressBar>();
        hpBar->mX = 15.0f;
        hpBar->mY = 30.0f;
        hpBar->mWidth = 220.0f;
        hpBar->mHeight = 18.0f;
        hpBar->mFgR = 200; hpBar->mFgG = 40; hpBar->mFgB = 40;
        hpBar->mBgR = 30; hpBar->mBgG = 30; hpBar->mBgB = 30;
        hpBar->mLabelR = 255; hpBar->mLabelG = 255; hpBar->mLabelB = 255;
        mHPBar = hpBar.get();
        panel->addChild(std::move(hpBar));

        auto spBar = std::make_unique<UIProgressBar>();
        spBar->mX = 15.0f;
        spBar->mY = 52.0f;
        spBar->mWidth = 220.0f;
        spBar->mHeight = 14.0f;
        spBar->mFgR = 80; spBar->mFgG = 120; spBar->mFgB = 220;
        spBar->mBgR = 30; spBar->mBgG = 30; spBar->mBgB = 30;
        spBar->mLabelR = 200; spBar->mLabelG = 200; spBar->mLabelB = 255;
        mSPBar = spBar.get();
        panel->addChild(std::move(spBar));

        auto turnLabel = std::make_unique<UILabel>();
        turnLabel->mX = mViewportW - 250.0f;
        turnLabel->mY = 10.0f;
        turnLabel->mText = "第 1 回合";
        turnLabel->mTextR = 200; turnLabel->mTextG = 200; turnLabel->mTextB = 200;
        mTurnLabel = turnLabel.get();
        panel->addChild(std::move(turnLabel));

        mTopPanel = panel.get();
        mUIManager.addElement(std::move(panel));
    }

    // 技能面板
    {
        auto panel = std::make_unique<UIPanel>();
        panel->mX = mViewportW - 220.0f;
        panel->mY = 100.0f;
        panel->mWidth = 200.0f;
        panel->mHeight = 400.0f;
        panel->mBgR = 20; panel->mBgG = 20; panel->mBgB = 30; panel->mBgA = 210;

        auto title = std::make_unique<UILabel>();
        title->mX = 10.0f;
        title->mY = 10.0f;
        title->mText = "-- 技能 --";
        title->mTextR = 255; title->mTextG = 215; title->mTextB = 0;
        panel->addChild(std::move(title));

        mSkillPanel = panel.get();
        mUIManager.addElement(std::move(panel));
    }

    // 符箓面板
    {
        auto panel = std::make_unique<UIPanel>();
        panel->mX = mViewportW - 220.0f;
        panel->mY = 510.0f;
        panel->mWidth = 200.0f;
        panel->mHeight = 170.0f;
        panel->mBgR = 20; panel->mBgG = 20; panel->mBgB = 30; panel->mBgA = 210;

        auto title = std::make_unique<UILabel>();
        title->mX = 10.0f;
        title->mY = 8.0f;
        title->mText = "-- 符箓 (5/战) --";
        title->mTextR = 200; title->mTextG = 180; title->mTextB = 60;
        mTalismanLabel = title.get();
        panel->addChild(std::move(title));

        mUIManager.addElement(std::move(panel));
    }

    // 底部操作栏
    {
        auto bar = std::make_unique<UIPanel>();
        bar->mX = 0.0f;
        bar->mY = mViewportH - 45.0f;
        bar->mWidth = mViewportW;
        bar->mHeight = 45.0f;
        bar->mBgR = 0; bar->mBgG = 0; bar->mBgB = 0; bar->mBgA = 200;

        // 结束回合按钮
        auto endBtn = std::make_unique<UIButton>();
        endBtn->mX = mViewportW - 150.0f;
        endBtn->mY = 5.0f;
        endBtn->mWidth = 130.0f;
        endBtn->mHeight = 32.0f;
        endBtn->mText = "结束回合";
        endBtn->mNormalBgR = 60; endBtn->mNormalBgG = 30; endBtn->mNormalBgB = 30;
        endBtn->mHoverBgR = 120; endBtn->mHoverBgG = 50; endBtn->mHoverBgB = 40;
        endBtn->mPressBgR = 40; endBtn->mPressBgG = 20; endBtn->mPressBgB = 20;
        endBtn->mOnClick = [this]() {
            if (mBattleEnded) return;
            mSelectedSkillIndex = -1;
            mMoveRange.clear();
            mAttackTargets.clear();
            mTurnManager.endTurn();

            CombatUnit* next = mTurnManager.getActiveUnit();
            if (next != nullptr && next->mTeam == 0) {
                mMoveRange = mGrid.getMoveRange(next->mGridX, next->mGridY,
                    mTurnManager.mRemainingMoves);
            }
        };
        bar->addChild(std::move(endBtn));

        // 返回世界按钮（仅在战斗结束时显示）
        auto returnBtn = std::make_unique<UIButton>();
        returnBtn->mX = mViewportW - 310.0f;
        returnBtn->mY = 5.0f;
        returnBtn->mWidth = 140.0f;
        returnBtn->mHeight = 32.0f;
        returnBtn->mText = "返回世界";
        returnBtn->mNormalBgR = 30; returnBtn->mNormalBgG = 60; returnBtn->mNormalBgB = 30;
        returnBtn->mHoverBgR = 50; returnBtn->mHoverBgG = 120; returnBtn->mHoverBgB = 40;
        returnBtn->mPressBgR = 20; returnBtn->mPressBgG = 40; returnBtn->mPressBgB = 20;
        returnBtn->mOnClick = [this]() {
            if (mSceneManager) {
                mSceneManager->requestPopScene();
            }
        };
        bar->addChild(std::move(returnBtn));

        mBottomBar = bar.get();
        mUIManager.addElement(std::move(bar));
    }
}

void CombatScene::updateHUD()
{
    CombatUnit* active = mTurnManager.getActiveUnit();

    if (mUnitNameLabel != nullptr) {
        if (active != nullptr) {
            mUnitNameLabel->mText = active->mName
                + (active->mTeam == 0 ? " [我方]" : " [敌方]");
        } else {
            mUnitNameLabel->mText = "无行动单位";
        }
    }

    if (mHPBar != nullptr && active != nullptr) {
        mHPBar->mValue = static_cast<float>(active->mHP);
        mHPBar->mMaxValue = static_cast<float>(active->mMaxHP);
        char buf[64];
        snprintf(buf, sizeof(buf), "HP %d/%d", active->mHP, active->mMaxHP);
        mHPBar->mLabel = buf;
    }

    if (mSPBar != nullptr && active != nullptr) {
        mSPBar->mValue = static_cast<float>(active->mSpiritPower);
        mSPBar->mMaxValue = static_cast<float>(active->mMaxSpiritPower);
        char buf[64];
        snprintf(buf, sizeof(buf), "灵力 %d/%d  身法 %d/%d",
            active->mSpiritPower, active->mMaxSpiritPower,
            mTurnManager.mRemainingMoves, active->mMaxMovementPoints);
        mSPBar->mLabel = buf;
    }

    if (mTurnLabel != nullptr) {
        const char* phaseStr = "玩家回合";
        if (mTurnManager.mPhase == CombatPhase::EnemyTurn) phaseStr = "敌方回合";
        else if (mTurnManager.mPhase == CombatPhase::Victory) phaseStr = "战斗胜利！";
        else if (mTurnManager.mPhase == CombatPhase::Defeat) phaseStr = "战斗失败...";

        char buf[128];
        snprintf(buf, sizeof(buf), "第 %d 回合 | %s",
            mTurnManager.mRoundNumber, phaseStr);
        mTurnLabel->mText = buf;

        if (mTurnManager.mPhase == CombatPhase::Victory) {
            mTurnLabel->mTextR = 100; mTurnLabel->mTextG = 255; mTurnLabel->mTextB = 100;
        } else if (mTurnManager.mPhase == CombatPhase::Defeat) {
            mTurnLabel->mTextR = 255; mTurnLabel->mTextG = 80; mTurnLabel->mTextB = 80;
        }
    }

    // 技能按钮：仅在必要时重建，避免每帧销毁导致UIManager悬空指针
    if (mSkillPanel != nullptr) {
        uint32_t curUnitId = (active != nullptr) ? active->mId : 0;
        int curSkillCount = (active != nullptr) ? static_cast<int>(active->mSkills.size()) : 0;

        bool needRebuild =
            (curUnitId != mLastActiveUnitId) ||
            (mTurnManager.mPhase != mLastPhase) ||
            (curSkillCount != mLastSkillCount);

        if (needRebuild) {
            // 安全清除：先重置UIManager交互状态避免悬空指针
            mUIManager.resetInteractionState();

            while (mSkillPanel->mChildren.size() > 1) {
                mSkillPanel->mChildren.pop_back();
            }
            mSkillButtons.clear();
    mTalismanButtons.clear();

            mLastActiveUnitId = curUnitId;
            mLastPhase = mTurnManager.mPhase;
            mLastSkillCount = curSkillCount;
            mLastSelectedSkill = -1;  // 强制刷新选中状态

            if (active != nullptr && active->mTeam == 0
                && mTurnManager.mPhase == CombatPhase::PlayerTurn
                && !mBattleEnded) {

                float btnY = 40.0f;
                for (int i = 0; i < curSkillCount; ++i) {
                    Skill& skill = active->mSkills[i];

                    auto btn = std::make_unique<UIButton>();
                    btn->mX = 10.0f;
                    btn->mY = btnY;
                    btn->mWidth = 180.0f;
                    btn->mHeight = 48.0f;

                    char buf[128];
                    const char* cdStr = skill.isReady() ? "" : " [CD]";
                    snprintf(buf, sizeof(buf), "%s%s\n灵力:%d 伤害:%d 范围:%d",
                        skill.mName.c_str(), cdStr,
                        skill.mSpiritCost, skill.mBaseDamage, skill.mRange);
                    btn->mText = buf;

                    bool canUse = active->canAffordSkill(skill);
                    btn->mNormalBgR = canUse ? 40 : 30;
                    btn->mNormalBgG = canUse ? 50 : 30;
                    btn->mNormalBgB = canUse ? 70 : 30;
                    btn->mHoverBgR = canUse ? 60 : 35;
                    btn->mHoverBgG = canUse ? 70 : 35;
                    btn->mHoverBgB = canUse ? 100 : 35;
                    btn->mEnabled = canUse;

                    if (i == mSelectedSkillIndex) {
                        btn->mNormalBgR = 100;
                        btn->mNormalBgG = 80;
                        btn->mNormalBgB = 30;
                    }

                    int skillIdx = i;
                    btn->mOnClick = [this, skillIdx]() {
                        if (mSelectedSkillIndex == skillIdx) {
                            mSelectedSkillIndex = -1;
                            mAttackTargets.clear();
                        } else {
                            handleSkillSelect(skillIdx);
                        }
                    };

                    mSkillButtons.push_back(btn.get());
                    mSkillPanel->addChild(std::move(btn));
                    btnY += 55.0f;
                }
            }
        } else if (mSelectedSkillIndex != mLastSelectedSkill) {
            // 仅更新选中高亮，不重建按钮
            for (int i = 0; i < static_cast<int>(mSkillButtons.size()); ++i) {
                auto* btn = mSkillButtons[i];
                bool canUse = (active != nullptr) ? active->canAffordSkill(active->mSkills[i]) : false;
                if (i == mSelectedSkillIndex) {
                    btn->mNormalBgR = 100;
                    btn->mNormalBgG = 80;
                    btn->mNormalBgB = 30;
                } else {
                    btn->mNormalBgR = canUse ? 40 : 30;
                    btn->mNormalBgG = canUse ? 50 : 30;
                    btn->mNormalBgB = canUse ? 70 : 30;
                }
            }
            mLastSelectedSkill = mSelectedSkillIndex;
        }
    }

    // 符箓按钮更新 - 仅在玩家回合且变化时重建
    if (mTalismanLabel && mPlayerData) {
        auto* talismanPanel = mTalismanLabel->mParent;
        if (talismanPanel && mTurnManager.mPhase == CombatPhase::PlayerTurn && !mBattleEnded) {
            int curTalismanCount = static_cast<int>(mPlayerData->equippedTalismans.size());
            static int lastTalismanCount = -1;
            if (curTalismanCount != lastTalismanCount) {
                lastTalismanCount = curTalismanCount;
                mUIManager.resetInteractionState();
                // Remove old talisman buttons (children after the title)
                while (talismanPanel->mChildren.size() > 1) {
                    talismanPanel->mChildren.pop_back();
                }
                mTalismanButtons.clear();

                float btnY = 35.0f;
                for (int i = 0; i < curTalismanCount; ++i) {
                    auto& talisman = mPlayerData->equippedTalismans[i];
                    auto btn = std::make_unique<UIButton>();
                    btn->mX = 10.0f;
                    btn->mY = btnY;
                    btn->mWidth = 180.0f;
                    btn->mHeight = 22.0f;
                    btn->mText = talisman.name + " " + std::string(talisman.typeName());
                    btn->mNormalBgR = 50; btn->mNormalBgG = 40; btn->mNormalBgB = 30;
                    btn->mHoverBgR = 80; btn->mHoverBgG = 60; btn->mHoverBgB = 40;
                    int talismanIdx = i;
                    btn->mOnClick = [this, talismanIdx]() {
                        if (!mPlayerData || talismanIdx >= static_cast<int>(mPlayerData->equippedTalismans.size())) return;
                        auto& t = mPlayerData->equippedTalismans[talismanIdx];
                        CombatUnit* active = mTurnManager.getActiveUnit();
                        if (!active || active->mTeam != 0) return;
                        // 使用符箓
                        if (t.type == TalismanType::Attack) {
                            // 对最近的敌人造成真实伤害
                            for (auto& u : mUnits) {
                                if (u->mTeam != 0 && u->isAlive()) {
                                    u->takeDamage(t.value);
                                    LOG_INFO("使用 %s 对 %s 造成 %d 伤害！", t.name.c_str(), u->mName.c_str(), t.value);
                                    if (!u->isAlive()) mGrid.setOccupier(u->mGridX, u->mGridY, 0);
                                    break;
                                }
                            }
                        } else if (t.type == TalismanType::Support && t.value > 10) {
                            active->mHP += t.value;
                            if (active->mHP > active->mMaxHP) active->mHP = active->mMaxHP;
                            LOG_INFO("使用 %s 恢复 %d HP", t.name.c_str(), t.value);
                        } else if (t.type == TalismanType::Support && t.value <= 5) {
                            mTurnManager.mRemainingMoves += t.value;
                            LOG_INFO("使用 %s 获得 %d 额外身法", t.name.c_str(), t.value);
                        }
                        mPlayerData->equippedTalismans.erase(mPlayerData->equippedTalismans.begin() + talismanIdx);
                        updateHUD();
                    };
                    mTalismanButtons.push_back(btn.get());
                    talismanPanel->addChild(std::move(btn));
                    btnY += 25.0f;
                }
            }
        }
    }
}

// ==================== 辅助 ====================

CombatUnit* CombatScene::getUnitAt(int gx, int gy)
{
    uint32_t id = mGrid.getOccupier(gx, gy);
    if (id == 0) return nullptr;
    return getUnitById(id);
}

CombatUnit* CombatScene::getUnitById(uint32_t id)
{
    for (auto& u : mUnits) {
        if (u->mId == id && u->isAlive()) return u.get();
    }
    return nullptr;
}
