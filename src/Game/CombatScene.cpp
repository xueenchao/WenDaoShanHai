/**
 * CombatScene.cpp - 战斗场景实现
 *
 * BG3风格的回合制网格战棋战斗。
 */

#include "CombatScene.h"
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

// ==================== 构造 ====================

CombatScene::CombatScene(EventHandler& eventHandler, Font* font,
                         float viewportW, float viewportH)
    : Scene("战斗场景")
    , mEventHandler(eventHandler)
    , mFont(font)
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

    setupBattlefield();
    setupPlayerTeam();
    setupEnemyTeam();
    initBattle();

    // 设置摄像机：居中显示网格
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
}

// ==================== 更新 ====================

void CombatScene::onUpdate(float deltaTime)
{
    // 更新摄像机
    mCamera.update(deltaTime);

    // 更新鼠标悬停
    float mx, my;
    mEventHandler.getMousePosition(mx, my);
    mUIManager.updateHover(mx, my);

    // 检查战斗结束
    CombatPhase phase = mTurnManager.checkBattleEnd(mUnitPtrs);
    if (phase == CombatPhase::Victory || phase == CombatPhase::Defeat) {
        // 按任意键退出（或自动）
        if (mEventHandler.isKeyJustPressed(SDL_SCANCODE_SPACE)
            || mEventHandler.isKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
            // 通过SceneManager弹出
        }
        updateHUD();
        return;
    }

    // 敌方回合：AI自动执行
    if (mTurnManager.mPhase == CombatPhase::EnemyTurn) {
        mAI.executeTurn(mGrid, mTurnManager, mUnitPtrs, deltaTime);
        updateHUD();
        return;
    }

    // 玩家回合
    if (mTurnManager.mPhase == CombatPhase::PlayerTurn) {
        handlePlayerInput(deltaTime);
    }

    updateHUD();
}

// ==================== 渲染 ====================

void CombatScene::onRender(Renderer& renderer)
{
    // 背景
    renderer.setDrawColor(15, 30, 20, 255);
    renderer.clear();

    renderGrid(renderer);
    renderMoveRange(renderer);
    renderAttackTargets(renderer);
    renderUnits(renderer);
    renderHUD(renderer);

    // 鼠标坐标调试（可选）
    if (mTurnManager.mPhase == CombatPhase::PlayerTurn) {
        // 通过Camera转换用于定位点击
        float mx, my;
        mEventHandler.getMousePosition(mx, my);
        auto world = mCamera.screenToWorld(mx, my);
        int gx, gy;
        mGrid.worldToGrid(world.x, world.y, gx, gy);
    }
}

// ==================== 初始化 ====================

void CombatScene::setupBattlefield()
{
    // Grid already created in constructor
    LOG_INFO("战场: %d x %d 网格, 每格 %.0fpx",
        mGrid.getCols(), mGrid.getRows(), mGrid.getTileSize());
}

void CombatScene::setupPlayerTeam()
{
    // 玩家角色
    {
        auto unit = std::make_unique<CombatUnit>();
        unit->mId = 1;
        unit->mName = "青云修士";
        unit->mHP = 120;
        unit->mMaxHP = 120;
        unit->mAttack = 20;
        unit->mDefense = 8;
        unit->mSpeed = 12;
        unit->mElement = Element::Jin;  // 金属性
        unit->mSpiritPower = 6;
        unit->mMaxSpiritPower = 6;
        unit->mMovementPoints = 4;
        unit->mMaxMovementPoints = 4;
        unit->mGridX = 1;
        unit->mGridY = 2;
        unit->mTeam = 0;

        unit->mSkills.push_back(Skill::makeSwordSlash());
        unit->mSkills.push_back(Skill::makeFireball());
        unit->mSkills.push_back(Skill::makeIceShard());

        mGrid.setOccupier(unit->mGridX, unit->mGridY, unit->mId);
        mUnitPtrs.push_back(unit.get());
        mUnits.push_back(std::move(unit));
    }

    // 灵兽（玩家方宠物）
    {
        auto unit = std::make_unique<CombatUnit>();
        unit->mId = 2;
        unit->mName = "灵狐";
        unit->mHP = 80;
        unit->mMaxHP = 80;
        unit->mAttack = 12;
        unit->mDefense = 4;
        unit->mSpeed = 10;
        unit->mElement = Element::Mu;  // 木属性
        unit->mSpiritPower = 4;
        unit->mMaxSpiritPower = 4;
        unit->mMovementPoints = 5;
        unit->mMaxMovementPoints = 5;
        unit->mGridX = 1;
        unit->mGridY = 4;
        unit->mTeam = 0;

        unit->mSkills.push_back(Skill::makeVineWhip());

        mGrid.setOccupier(unit->mGridX, unit->mGridY, unit->mId);
        mUnitPtrs.push_back(unit.get());
        mUnits.push_back(std::move(unit));
    }
}

void CombatScene::setupEnemyTeam()
{
    // 妖兽1 — 火属性
    {
        auto unit = std::make_unique<CombatUnit>();
        unit->mId = 10;
        unit->mName = "炎妖";
        unit->mHP = 80;
        unit->mMaxHP = 80;
        unit->mAttack = 15;
        unit->mDefense = 4;
        unit->mSpeed = 8;
        unit->mElement = Element::Huo;
        unit->mSpiritPower = 4;
        unit->mMaxSpiritPower = 4;
        unit->mMovementPoints = 3;
        unit->mMaxMovementPoints = 3;
        unit->mGridX = 6;
        unit->mGridY = 1;
        unit->mTeam = 1;

        unit->mSkills.push_back(Skill::makeFireball());

        mGrid.setOccupier(unit->mGridX, unit->mGridY, unit->mId);
        mUnitPtrs.push_back(unit.get());
        mUnits.push_back(std::move(unit));
    }

    // 妖兽2 — 土属性
    {
        auto unit = std::make_unique<CombatUnit>();
        unit->mId = 11;
        unit->mName = "岩甲兽";
        unit->mHP = 120;
        unit->mMaxHP = 120;
        unit->mAttack = 10;
        unit->mDefense = 12;
        unit->mSpeed = 5;
        unit->mElement = Element::Tu;
        unit->mSpiritPower = 4;
        unit->mMaxSpiritPower = 4;
        unit->mMovementPoints = 2;
        unit->mMaxMovementPoints = 2;
        unit->mGridX = 6;
        unit->mGridY = 3;
        unit->mTeam = 1;

        unit->mSkills.push_back(Skill::makeRockSmash());

        mGrid.setOccupier(unit->mGridX, unit->mGridY, unit->mId);
        mUnitPtrs.push_back(unit.get());
        mUnits.push_back(std::move(unit));
    }

    // 妖兽3 — 水属性
    {
        auto unit = std::make_unique<CombatUnit>();
        unit->mId = 12;
        unit->mName = "寒蛟";
        unit->mHP = 90;
        unit->mMaxHP = 90;
        unit->mAttack = 13;
        unit->mDefense = 5;
        unit->mSpeed = 9;
        unit->mElement = Element::Shui;
        unit->mSpiritPower = 3;
        unit->mMaxSpiritPower = 3;
        unit->mMovementPoints = 3;
        unit->mMaxMovementPoints = 3;
        unit->mGridX = 5;
        unit->mGridY = 5;
        unit->mTeam = 1;

        unit->mSkills.push_back(Skill::makeIceShard());

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

// ==================== 玩家输入处理 ====================

void CombatScene::handlePlayerInput(float /*deltaTime*/)
{
    float mx, my;
    mEventHandler.getMousePosition(mx, my);

    // 更新悬停
    mUIManager.updateHover(mx, my);

    // 鼠标左键按下
    if (mEventHandler.isMouseButtonJustPressed(SDL_BUTTON_LEFT)) {
        // 先检查是否点击了UI
        UIElement* uiHit = mUIManager.getElementAt(mx, my);
        if (uiHit != nullptr) {
            mUIManager.handleMouseDown(mx, my);
            return;
        }

        // 点击在游戏区域 — 转换到网格坐标
        auto world = mCamera.screenToWorld(mx, my);
        int gx, gy;
        mGrid.worldToGrid(world.x, world.y, gx, gy);

        if (mGrid.isInBounds(gx, gy)) {
            handleGridClick(gx, gy);
        } else {
            // 点击了空地，取消技能选择
            mSelectedSkillIndex = -1;
            mAttackTargets.clear();
        }
    }

    // 鼠标释放
    if (mEventHandler.isMouseButtonJustReleased(SDL_BUTTON_LEFT)) {
        mUIManager.handleMouseUp(mx, my);
    }

    // 右键取消
    if (mEventHandler.isMouseButtonJustPressed(SDL_BUTTON_RIGHT)) {
        mSelectedSkillIndex = -1;
        mAttackTargets.clear();
    }
}

void CombatScene::handleGridClick(int gx, int gy)
{
    CombatUnit* active = mTurnManager.getActiveUnit();
    if (active == nullptr || active->mTeam != 0) return;

    // 如果选了技能，尝试攻击目标格的单位
    if (mSelectedSkillIndex >= 0
        && mSelectedSkillIndex < static_cast<int>(active->mSkills.size())) {

        Skill& skill = active->mSkills[mSelectedSkillIndex];

        // 检查目标格是否在有效目标列表内
        for (auto& t : mAttackTargets) {
            if (t.first == gx && t.second == gy) {
                CombatUnit* target = getUnitAt(gx, gy);
                if (target != nullptr && active->canAffordSkill(skill)) {
                    executeAttack(*active, *target, skill);
                    mTurnManager.mHasActed = true;
                    mSelectedSkillIndex = -1;
                    mAttackTargets.clear();

                    // 检查战斗结束
                    mTurnManager.checkBattleEnd(mUnitPtrs);
                    updateHUD();
                    return;
                }
            }
        }

        // 点击了无效目标，取消技能选择
        mSelectedSkillIndex = -1;
        mAttackTargets.clear();
        return;
    }

    // 没有选中技能 — 尝试移动
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

    // 点击了自己的其他单位 — 暂不切换（未来可扩展）
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

    // 计算可攻击的目标格子（在技能范围内的敌人位置）
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
        // 清空格子
        mGrid.setOccupier(target.mGridX, target.mGridY, 0);
    }
}

// ==================== 渲染 ====================

void CombatScene::renderGrid(Renderer& renderer)
{
    float tileSize = mGrid.getTileSize();
    int cols = mGrid.getCols();
    int rows = mGrid.getRows();

    // 绘制每个格子的背景
    for (int gy = 0; gy < rows; ++gy) {
        for (int gx = 0; gx < cols; ++gx) {
            float wx, wy;
            mGrid.gridToWorld(gx, gy, wx, wy);
            auto screen = mCamera.worldToScreen(wx, wy);
            float sw = tileSize * mCamera.getZoom();
            float sh = tileSize * mCamera.getZoom();

            // 棋盘格颜色
            bool dark = (gx + gy) % 2 == 0;
            renderer.setDrawColor(dark ? 25 : 35, dark ? 45 : 55, dark ? 20 : 28, 255);
            renderer.fillRect(screen.x, screen.y, sw, sh);
        }
    }

    // 网格线
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

        // 单位颜色：玩家方绿色，敌方红色
        if (unit->mTeam == 0) {
            renderer.setDrawColor(50, 200, 80, 255);
        } else {
            renderer.setDrawColor(200, 50, 50, 255);
        }

        renderer.fillRect(screen.x + offset, screen.y + offset, unitSize, unitSize);

        // 当前行动单位金边高亮
        CombatUnit* active = mTurnManager.getActiveUnit();
        if (active != nullptr && active->mId == unit->mId) {
            renderer.setDrawColor(255, 215, 0, 255);
        } else if (unit->mTeam == 0) {
            renderer.setDrawColor(100, 220, 120, 255);
        } else {
            renderer.setDrawColor(255, 100, 100, 255);
        }
        renderer.drawRect(screen.x + offset, screen.y + offset, unitSize, unitSize);

        // 单位名字（缩小显示）
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

        // HP条（在单位下方）
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

        // 单位名称标签
        auto nameLabel = std::make_unique<UILabel>();
        nameLabel->mX = 15.0f;
        nameLabel->mY = 8.0f;
        nameLabel->mText = "等待中...";
        nameLabel->mTextR = 255; nameLabel->mTextG = 215; nameLabel->mTextB = 0;
        mUnitNameLabel = nameLabel.get();
        panel->addChild(std::move(nameLabel));

        // HP进度条
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

        // 灵力进度条
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

        // 回合/阶段标签
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

    // 技能面板（右侧）
    {
        auto panel = std::make_unique<UIPanel>();
        panel->mX = mViewportW - 220.0f;
        panel->mY = 100.0f;
        panel->mWidth = 200.0f;
        panel->mHeight = 400.0f;
        panel->mBgR = 20; panel->mBgG = 20; panel->mBgB = 30; panel->mBgA = 210;

        // 标题
        auto title = std::make_unique<UILabel>();
        title->mX = 10.0f;
        title->mY = 10.0f;
        title->mText = "-- 技能 --";
        title->mTextR = 255; title->mTextG = 215; title->mTextB = 0;
        panel->addChild(std::move(title));

        mSkillPanel = panel.get();
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
            mSelectedSkillIndex = -1;
            mMoveRange.clear();
            mAttackTargets.clear();
            mTurnManager.endTurn();

            // 如果新回合是玩家，重新计算移动范围
            CombatUnit* next = mTurnManager.getActiveUnit();
            if (next != nullptr && next->mTeam == 0) {
                mMoveRange = mGrid.getMoveRange(next->mGridX, next->mGridY,
                    mTurnManager.mRemainingMoves);
            }
        };
        bar->addChild(std::move(endBtn));

        mBottomBar = bar.get();
        mUIManager.addElement(std::move(bar));
    }
}

void CombatScene::updateHUD()
{
    CombatUnit* active = mTurnManager.getActiveUnit();

    // 更新顶部面板
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

    // 重建技能按钮
    if (mSkillPanel != nullptr) {
        // 清空旧按钮（保留标题）
        while (mSkillPanel->mChildren.size() > 1) {
            mSkillPanel->mChildren.pop_back();
        }
        mSkillButtons.clear();

        if (active != nullptr && active->mTeam == 0
            && mTurnManager.mPhase == CombatPhase::PlayerTurn) {

            float btnY = 40.0f;
            for (int i = 0; i < static_cast<int>(active->mSkills.size()); ++i) {
                Skill& skill = active->mSkills[i];

                auto btn = std::make_unique<UIButton>();
                btn->mX = 10.0f;
                btn->mY = btnY;
                btn->mWidth = 180.0f;
                btn->mHeight = 48.0f;

                // 技能信息
                char buf[128];
                const char* cdStr = skill.isReady() ? "" : " [CD]";
                snprintf(buf, sizeof(buf), "%s%s\n灵力:%d 伤害:%d 范围:%d",
                    skill.mName.c_str(), cdStr,
                    skill.mSpiritCost, skill.mBaseDamage, skill.mRange);
                btn->mText = buf;

                // 不可用技能显示为灰色
                bool canUse = active->canAffordSkill(skill);
                btn->mNormalBgR = canUse ? 40 : 30;
                btn->mNormalBgG = canUse ? 50 : 30;
                btn->mNormalBgB = canUse ? 70 : 30;
                btn->mHoverBgR = canUse ? 60 : 35;
                btn->mHoverBgG = canUse ? 70 : 35;
                btn->mHoverBgB = canUse ? 100 : 35;
                btn->mEnabled = canUse;

                // 当前选中的技能高亮
                if (i == mSelectedSkillIndex) {
                    btn->mNormalBgR = 100;
                    btn->mNormalBgG = 80;
                    btn->mNormalBgB = 30;
                }

                int skillIdx = i;
                btn->mOnClick = [this, skillIdx]() {
                    // 切换：如果已选中则取消
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
