/**
 * BigWorldScene.cpp - 大世界探索场景实现
 */

#include "BigWorldScene.h"
#include "../Core/PlayerData.h"
#include "../Combat/CombatScene.h"
#include "../Menu/SectScene.h"
#include "../Data/DataManager.h"
#include "Core/EventHandler.h"
#include "Core/Renderer.h"
#include "Core/Font.h"
#include "Core/Scene.h"
#include "Core/UI/UIPanel.h"
#include "Core/UI/UILabel.h"
#include "Core/UI/UIButton.h"
#include "Core/UI/UIProgressBar.h"
#include "Core/Log.h"
#include <cstdio>
#include <cstdlib>

static void getTerrainColor(Terrain t, Uint8& r, Uint8& g, Uint8& b)
{
    const char* id = "grass";
    switch (t) {
    case Terrain::Grass:    id = "grass";    break;
    case Terrain::Water:    id = "water";    break;
    case Terrain::Mountain: id = "mountain"; break;
    case Terrain::Forest:   id = "forest";   break;
    case Terrain::Path:     id = "path";     break;
    case Terrain::Sand:     id = "sand";     break;
    }
    const TerrainDef* def = DataManager::getInstance().getTerrainDef(id);
    if (def) { r = static_cast<Uint8>(def->colorR); g = static_cast<Uint8>(def->colorG); b = static_cast<Uint8>(def->colorB); }
    else     { r = 34; g = 139; b = 34; }
}

BigWorldScene::BigWorldScene(EventHandler& eh, Font* font, float vpW, float vpH,
                             PlayerData* playerData, SceneManager* sceneMgr)
    : Scene("大世界")
    , mEH(eh)
    , mFont(font)
    , mPlayerData(playerData)
    , mSceneManager(sceneMgr)
    , mWorldMap(40, 30, 64.0f)
    , mCamera(vpW, vpH)
    , mUIManager(font)
    , mVpW(vpW)
    , mVpH(vpH)
{
}

void BigWorldScene::onEnter()
{
    LOG_INFO("进入大世界");

    // 玩家出生点：地图中央
    mPlayerX = mWorldMap.getWorldWidth() * 0.5f;
    mPlayerY = mWorldMap.getWorldHeight() * 0.5f;

    // 摄像机设置
    mCamera.setPosition(mPlayerX, mPlayerY);
    mCamera.setZoom(1.0f);
    mCamera.setBounds(0, 0,
        mWorldMap.getWorldWidth(),
        mWorldMap.getWorldHeight());

    // 生成遭遇点
    spawnEncounterMarkers();

    // 构建UI
    buildInventoryUI();
    buildCharPanelUI();

    mInventoryOpen = false;
    mCharPanelOpen = false;
    mSelectedItemIndex = -1;
    mTransitionToCombat = false;
}

void BigWorldScene::onExit()
{
    LOG_INFO("离开大世界");
    mUIManager.clear();
    mInvSlotButtons.clear();
}

// ==================== 更新 ====================

void BigWorldScene::onUpdate(float deltaTime)
{
    // 叠加界面打开时不移动
    if (mInventoryOpen || mCharPanelOpen) {
        handleOverlayInput();
        return;
    }

    handleWorldInput(deltaTime);

    mCamera.setPosition(mPlayerX, mPlayerY);
    mCamera.update(deltaTime);

    checkEncounter();

    if (mTransitionToCombat && mSceneManager) {
        // 恢复玩家状态
        if (mPlayerData) {
            mPlayerData->heal(9999); // 满血进入战斗
            mPlayerData->restoreSP(9999);
        }
        mSceneManager->pushScene(std::make_unique<CombatScene>(
            mEH, mFont, mVpW, mVpH, mPlayerData, mSceneManager));
        mTransitionToCombat = false;
    }

    if (mTransitionToSect && mSceneManager) {
        mSceneManager->pushScene(std::make_unique<SectScene>(
            mEH, mFont, mVpW, mVpH, mPlayerData, mSceneManager));
        mTransitionToSect = false;
    }

    // 遭遇点补充
    int activeCount = 0;
    for (auto& e : mEncounters) {
        if (e.active) ++activeCount;
    }
    if (activeCount == 0) {
        mEncounterSpawnTimer += deltaTime;
        if (mEncounterSpawnTimer > 3.0f) {
            spawnEncounterMarkers();
            mEncounterSpawnTimer = 0.0f;
        }
    }
}

// ==================== 世界输入 ====================

void BigWorldScene::handleWorldInput(float deltaTime)
{
    float dx = 0.0f, dy = 0.0f;

    if (mEH.isKeyPressed(SDL_SCANCODE_W) || mEH.isKeyPressed(SDL_SCANCODE_UP))    dy -= 1.0f;
    if (mEH.isKeyPressed(SDL_SCANCODE_S) || mEH.isKeyPressed(SDL_SCANCODE_DOWN))  dy += 1.0f;
    if (mEH.isKeyPressed(SDL_SCANCODE_A) || mEH.isKeyPressed(SDL_SCANCODE_LEFT))  dx -= 1.0f;
    if (mEH.isKeyPressed(SDL_SCANCODE_D) || mEH.isKeyPressed(SDL_SCANCODE_RIGHT)) dx += 1.0f;

    // 归一化斜向移动
    if (dx != 0.0f && dy != 0.0f) {
        float inv = 1.0f / 1.4142f;
        dx *= inv;
        dy *= inv;
    }

    float newX = mPlayerX + dx * PLAYER_SPEED * deltaTime;
    float newY = mPlayerY + dy * PLAYER_SPEED * deltaTime;

    float half = PLAYER_SIZE * 0.5f;

    // X轴移动检测（仅X变化，Y保持不变）
    {
        float tryX = newX;
        float tryY = mPlayerY;
        int gxTL, gyTL, gxTR, gyTR, gxBL, gyBL, gxBR, gyBR;
        mWorldMap.worldToGrid(tryX - half, tryY - half, gxTL, gyTL);
        mWorldMap.worldToGrid(tryX + half, tryY - half, gxTR, gyTR);
        mWorldMap.worldToGrid(tryX - half, tryY + half, gxBL, gyBL);
        mWorldMap.worldToGrid(tryX + half, tryY + half, gxBR, gyBR);
        if (mWorldMap.isWalkable(gxTL, gyTL) && mWorldMap.isWalkable(gxBL, gyBL)
         && mWorldMap.isWalkable(gxTR, gyTR) && mWorldMap.isWalkable(gxBR, gyBR)) {
            mPlayerX = newX;
        }
    }

    // Y轴移动检测（仅Y变化，X用更新后的值）
    {
        float tryX = mPlayerX;
        float tryY = newY;
        int gxTL, gyTL, gxTR, gyTR, gxBL, gyBL, gxBR, gyBR;
        mWorldMap.worldToGrid(tryX - half, tryY - half, gxTL, gyTL);
        mWorldMap.worldToGrid(tryX + half, tryY - half, gxTR, gyTR);
        mWorldMap.worldToGrid(tryX - half, tryY + half, gxBL, gyBL);
        mWorldMap.worldToGrid(tryX + half, tryY + half, gxBR, gyBR);
        if (mWorldMap.isWalkable(gxTL, gyTL) && mWorldMap.isWalkable(gxBL, gyBL)
         && mWorldMap.isWalkable(gxTR, gyTR) && mWorldMap.isWalkable(gxBR, gyBR)) {
            mPlayerY = newY;
        }
    }

    // 世界边界
    float worldW = mWorldMap.getWorldWidth();
    float worldH = mWorldMap.getWorldHeight();
    if (mPlayerX - half < 0) mPlayerX = half;
    if (mPlayerY - half < 0) mPlayerY = half;
    if (mPlayerX + half > worldW) mPlayerX = worldW - half;
    if (mPlayerY + half > worldH) mPlayerY = worldH - half;

    // 键盘切换
    if (mEH.isKeyJustPressed(SDL_SCANCODE_B)) {
        mInventoryOpen = !mInventoryOpen;
        if (mInventoryOpen) {
            mCharPanelOpen = false;
            updateInventoryDisplay();
        }
    }
    if (mEH.isKeyJustPressed(SDL_SCANCODE_C)) {
        mCharPanelOpen = !mCharPanelOpen;
        if (mCharPanelOpen) {
            mInventoryOpen = false;
            updateCharPanelDisplay();
        }
    }
    if (mEH.isKeyJustPressed(SDL_SCANCODE_T)) {
        if (mPlayerData && !mPlayerData->sectId.empty()) {
            mTransitionToSect = true;
        } else {
            LOG_INFO("尚未加入门派，无法回城");
        }
    }
}

// ==================== 叠加界面输入 ====================

void BigWorldScene::handleOverlayInput()
{
    float mx, my;
    mEH.getMousePosition(mx, my);

    mUIManager.updateHover(mx, my);

    if (mEH.isMouseButtonJustPressed(SDL_BUTTON_LEFT)) {
        mUIManager.handleMouseDown(mx, my);
    }
    if (mEH.isMouseButtonJustReleased(SDL_BUTTON_LEFT)) {
        mUIManager.handleMouseUp(mx, my);
    }

    // B/C关闭
    if (mEH.isKeyJustPressed(SDL_SCANCODE_B) && mInventoryOpen) {
        mInventoryOpen = false;
        mSelectedItemIndex = -1;
        updateInventoryDisplay();
        return;
    }
    if (mEH.isKeyJustPressed(SDL_SCANCODE_C) && mCharPanelOpen) {
        mCharPanelOpen = false;
        updateCharPanelDisplay();
        return;
    }
    if (mEH.isKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        mInventoryOpen = false;
        mCharPanelOpen = false;
        mSelectedItemIndex = -1;
        updateInventoryDisplay();
        updateCharPanelDisplay();
    }
}

// ==================== 遭遇检测 ====================

void BigWorldScene::checkEncounter()
{
    for (auto& e : mEncounters) {
        if (!e.active) continue;
        float dx = mPlayerX - e.wx;
        float dy = mPlayerY - e.wy;
        float dist = dx * dx + dy * dy;
        float triggerDist = PLAYER_SIZE + 20.0f;
        if (dist < triggerDist * triggerDist) {
            e.active = false;
            mTransitionToCombat = true;
            LOG_INFO("遭遇敌人！");
            break;
        }
    }
}

void BigWorldScene::spawnEncounterMarkers()
{
    mEncounters.clear();

    // 在世界可行走区域放置遭遇点
    int cols = mWorldMap.getCols();
    int rows = mWorldMap.getRows();
    float tileSize = mWorldMap.getTileSize();

    int attempts = 0;
    while (static_cast<int>(mEncounters.size()) < 5 && attempts < 100) {
        ++attempts;
        int gx = 3 + rand() % (cols - 6);
        int gy = 3 + rand() % (rows - 6);
        if (!mWorldMap.isWalkable(gx, gy)) continue;

        float wx, wy;
        mWorldMap.gridToWorldCenter(gx, gy, wx, wy);

        // 不要离玩家太近
        float dx = wx - mPlayerX;
        float dy = wy - mPlayerY;
        if (dx * dx + dy * dy < 200.0f * 200.0f) continue;

        // 不要和其他遭遇点重叠
        bool tooClose = false;
        for (auto& e : mEncounters) {
            float ex = wx - e.wx;
            float ey = wy - e.wy;
            if (ex * ex + ey * ey < 100.0f * 100.0f) {
                tooClose = true;
                break;
            }
        }
        if (tooClose) continue;

        EncounterMarker m;
        m.wx = wx;
        m.wy = wy;
        m.active = true;
        mEncounters.push_back(m);
    }

    LOG_INFO("生成 %zu 个遭遇点", mEncounters.size());
}

// ==================== 渲染 ====================

void BigWorldScene::renderWorld(Renderer& renderer)
{
    int cols = mWorldMap.getCols();
    int rows = mWorldMap.getRows();
    float tileSize = mWorldMap.getTileSize();

    // 计算可见范围
    auto topLeft = mCamera.screenToWorld(0, 0);
    auto botRight = mCamera.screenToWorld(mVpW, mVpH);
    int gxMin, gyMin, gxMax, gyMax;
    mWorldMap.worldToGrid(topLeft.x, topLeft.y, gxMin, gyMin);
    mWorldMap.worldToGrid(botRight.x, botRight.y, gxMax, gyMax);
    gxMin = (gxMin < 0) ? 0 : gxMin;
    gyMin = (gyMin < 0) ? 0 : gyMin;
    gxMax = (gxMax >= cols) ? cols - 1 : gxMax;
    gyMax = (gyMax >= rows) ? rows - 1 : gyMax;

    for (int gy = gyMin; gy <= gyMax; ++gy) {
        for (int gx = gxMin; gx <= gxMax; ++gx) {
            float wx, wy;
            mWorldMap.gridToWorld(gx, gy, wx, wy);
            auto screen = mCamera.worldToScreen(wx, wy);
            float sw = tileSize * mCamera.getZoom() + 1.0f;
            float sh = tileSize * mCamera.getZoom() + 1.0f;

            Uint8 r, g, b;
            getTerrainColor(mWorldMap.getTerrain(gx, gy), r, g, b);
            // 不可行走区域加深
            if (!mWorldMap.isWalkable(gx, gy)) {
                r = static_cast<Uint8>(r * 0.7f);
                g = static_cast<Uint8>(g * 0.7f);
                b = static_cast<Uint8>(b * 0.7f);
            }
            renderer.setDrawColor(r, g, b, 255);
            renderer.fillRect(screen.x, screen.y, sw, sh);
        }
    }
}

void BigWorldScene::renderPlayer(Renderer& renderer)
{
    auto screen = mCamera.worldToScreen(mPlayerX, mPlayerY);
    float size = PLAYER_SIZE * mCamera.getZoom();

    // 玩家身体
    renderer.setDrawColor(60, 180, 220, 255);
    renderer.fillRect(screen.x - size * 0.5f, screen.y - size * 0.5f, size, size);

    // 边框
    renderer.setDrawColor(100, 220, 255, 255);
    renderer.drawRect(screen.x - size * 0.5f, screen.y - size * 0.5f, size, size);

    // 名字
    if (mFont && mPlayerData) {
        SDL_Texture* tex = mFont->renderText(renderer,
            mPlayerData->name.c_str(),
            255, 255, 255, Font::RenderQuality::Solid);
        if (tex) {
            float tw, th;
            SDL_GetTextureSize(tex, &tw, &th);
            renderer.drawTexture(tex,
                screen.x - tw * 0.5f,
                screen.y - size * 0.5f - th - 4.0f);
            SDL_DestroyTexture(tex);
        }
    }
}

void BigWorldScene::renderEncounterMarkers(Renderer& renderer)
{
    for (auto& e : mEncounters) {
        if (!e.active) continue;

        auto screen = mCamera.worldToScreen(e.wx, e.wy);

        // 脉冲效果
        Uint32 tick = SDL_GetTicks();
        float pulse = 0.7f + 0.3f * sinf(static_cast<float>(tick) * 0.005f);
        Uint8 alpha = static_cast<Uint8>(200 * pulse);

        float size = 16.0f * mCamera.getZoom();
        renderer.setDrawColor(220, 30, 30, alpha);
        renderer.fillRect(screen.x - size, screen.y - size, size * 2, size * 2);

        // 感叹号
        if (mFont) {
            SDL_Texture* tex = mFont->renderText(renderer, "!",
                255, 50, 50, Font::RenderQuality::Solid);
            if (tex) {
                float tw, th;
                SDL_GetTextureSize(tex, &tw, &th);
                renderer.drawTexture(tex,
                    screen.x - tw * 0.5f,
                    screen.y - th * 0.5f);
                SDL_DestroyTexture(tex);
            }
        }
    }
}

void BigWorldScene::renderMinimap(Renderer& renderer)
{
    int cols = mWorldMap.getCols();
    int rows = mWorldMap.getRows();
    float mmX = mVpW - 170.0f;
    float mmY = 10.0f;
    float mmW = 160.0f;
    float mmH = 120.0f;
    float cellW = mmW / cols;
    float cellH = mmH / rows;

    // 背景
    renderer.setDrawColor(0, 0, 0, 180);
    renderer.fillRect(mmX, mmY, mmW, mmH);

    // 地形
    for (int gy = 0; gy < rows; ++gy) {
        for (int gx = 0; gx < cols; ++gx) {
            Uint8 r, g, b;
            getTerrainColor(mWorldMap.getTerrain(gx, gy), r, g, b);
            renderer.setDrawColor(
                static_cast<Uint8>(r * 0.6f),
                static_cast<Uint8>(g * 0.6f),
                static_cast<Uint8>(b * 0.6f), 200);
            renderer.fillRect(mmX + gx * cellW, mmY + gy * cellH, cellW + 1, cellH + 1);
        }
    }

    // 遭遇点
    for (auto& e : mEncounters) {
        if (!e.active) continue;
        int gx, gy;
        mWorldMap.worldToGrid(e.wx, e.wy, gx, gy);
        renderer.setDrawColor(255, 40, 40, 255);
        renderer.fillRect(mmX + gx * cellW, mmY + gy * cellH, 3, 3);
    }

    // 玩家位置
    int pgx, pgy;
    mWorldMap.worldToGrid(mPlayerX, mPlayerY, pgx, pgy);
    renderer.setDrawColor(60, 200, 255, 255);
    renderer.fillRect(mmX + pgx * cellW - 1, mmY + pgy * cellH - 1, 4, 4);

    // 边框
    renderer.setDrawColor(80, 80, 80, 255);
    renderer.drawRect(mmX, mmY, mmW, mmH);
}

void BigWorldScene::onRender(Renderer& renderer)
{
    // 背景
    renderer.setDrawColor(10, 15, 10, 255);
    renderer.clear();

    renderWorld(renderer);
    renderEncounterMarkers(renderer);
    renderPlayer(renderer);
    renderMinimap(renderer);

    // 叠加UI
    mUIManager.setRenderer(&renderer);
    mUIManager.render();
}

// ==================== 库存UI ====================

void BigWorldScene::buildInventoryUI()
{
    // 全屏半透明背景
    {
        auto backdrop = std::make_unique<UIPanel>();
        backdrop->mX = 0;
        backdrop->mY = 0;
        backdrop->mWidth = mVpW;
        backdrop->mHeight = mVpH;
        backdrop->mBgR = 0; backdrop->mBgG = 0; backdrop->mBgB = 0; backdrop->mBgA = 150;
        backdrop->mVisible = false;
        mInvBackdrop = backdrop.get();

        // 中心面板
        auto panel = std::make_unique<UIPanel>();
        panel->mX = mVpW * 0.5f - 370.0f;
        panel->mY = mVpH * 0.5f - 240.0f;
        panel->mWidth = 740.0f;
        panel->mHeight = 480.0f;
        panel->mBgR = 20; panel->mBgG = 20; panel->mBgB = 35; panel->mBgA = 240;

        // 标题
        auto title = std::make_unique<UILabel>();
        title->mX = 20.0f;
        title->mY = 10.0f;
        title->mText = "背 包";
        title->mTextR = 255; title->mTextG = 215; title->mTextB = 0;
        panel->addChild(std::move(title));

        // 物品格子 (5列 x 4行 = 20格)
        const float slotSize = 70.0f;
        const float startX = 20.0f;
        const float startY = 45.0f;
        const float gap = 4.0f;
        const int cols = 5;

        for (int i = 0; i < PlayerData::MAX_INVENTORY; ++i) {
            int row = i / cols;
            int col = i % cols;

            auto slot = std::make_unique<UIButton>();
            slot->mX = startX + col * (slotSize + gap);
            slot->mY = startY + row * (slotSize + gap);
            slot->mWidth = slotSize;
            slot->mHeight = slotSize;
            slot->mText = "";
            slot->mNormalBgR = 35; slot->mNormalBgG = 35; slot->mNormalBgB = 50;
            slot->mHoverBgR = 50; slot->mHoverBgG = 50; slot->mHoverBgB = 70;
            slot->mPressBgR = 30; slot->mPressBgG = 30; slot->mPressBgB = 45;

            int idx = i;
            slot->mOnClick = [this, idx]() {
                mSelectedItemIndex = idx;
                updateInventoryDisplay();
            };

            mInvSlotButtons.push_back(slot.get());
            panel->addChild(std::move(slot));
        }

        // 右侧物品信息
        float infoX = startX + cols * (slotSize + gap) + 20.0f;
        float infoW = panel->mWidth - infoX - 10.0f;

        auto itemName = std::make_unique<UILabel>();
        itemName->mX = infoX;
        itemName->mY = startY;
        itemName->mText = "";
        itemName->mTextR = 255; itemName->mTextG = 215; itemName->mTextB = 0;
        mInvItemNameLabel = itemName.get();
        panel->addChild(std::move(itemName));

        auto itemDesc = std::make_unique<UILabel>();
        itemDesc->mX = infoX;
        itemDesc->mY = startY + 30.0f;
        itemDesc->mWidth = infoW;
        itemDesc->mText = "点击左侧物品查看详情";
        itemDesc->mTextR = 180; itemDesc->mTextG = 180; itemDesc->mTextB = 200;
        mInvItemDescLabel = itemDesc.get();
        panel->addChild(std::move(itemDesc));

        // 使用按钮
        auto useBtn = std::make_unique<UIButton>();
        useBtn->mX = infoX;
        useBtn->mY = panel->mHeight - 110.0f;
        useBtn->mWidth = 80.0f;
        useBtn->mHeight = 30.0f;
        useBtn->mText = "使用";
        useBtn->mNormalBgR = 30; useBtn->mNormalBgG = 70; useBtn->mNormalBgB = 30;
        useBtn->mHoverBgR = 50; useBtn->mHoverBgG = 110; useBtn->mHoverBgB = 40;
        useBtn->mOnClick = [this]() {
            if (!mPlayerData) return;
            if (mSelectedItemIndex < 0
                || mSelectedItemIndex >= static_cast<int>(mPlayerData->inventory.size()))
                return;
            Item& item = mPlayerData->inventory[mSelectedItemIndex];
            if (item.type == ItemType::Consumable) {
                if (item.healHP > 0) mPlayerData->heal(item.healHP);
                if (item.healSP > 0) mPlayerData->restoreSP(item.healSP);
                item.quantity--;
                if (item.quantity <= 0) {
                    mPlayerData->removeItem(mSelectedItemIndex);
                    mSelectedItemIndex = -1;
                }
                updateInventoryDisplay();
            } else if (item.type == ItemType::Equipment) {
                mPlayerData->equipItem(mSelectedItemIndex);
                mSelectedItemIndex = -1;
                updateInventoryDisplay();
                updateCharPanelDisplay();
            }
        };
        panel->addChild(std::move(useBtn));

        // 丢弃按钮
        auto dropBtn = std::make_unique<UIButton>();
        dropBtn->mX = infoX + 90.0f;
        dropBtn->mY = panel->mHeight - 110.0f;
        dropBtn->mWidth = 80.0f;
        dropBtn->mHeight = 30.0f;
        dropBtn->mText = "丢弃";
        dropBtn->mNormalBgR = 70; dropBtn->mNormalBgG = 30; dropBtn->mNormalBgB = 30;
        dropBtn->mHoverBgR = 110; dropBtn->mHoverBgG = 50; dropBtn->mHoverBgB = 40;
        dropBtn->mOnClick = [this]() {
            if (!mPlayerData) return;
            if (mSelectedItemIndex >= 0
                && mSelectedItemIndex < static_cast<int>(mPlayerData->inventory.size())) {
                mPlayerData->removeItem(mSelectedItemIndex);
                mSelectedItemIndex = -1;
                updateInventoryDisplay();
            }
        };
        panel->addChild(std::move(dropBtn));

        // 关闭按钮
        auto closeBtn = std::make_unique<UIButton>();
        closeBtn->mX = panel->mWidth - 90.0f;
        closeBtn->mY = panel->mHeight - 40.0f;
        closeBtn->mWidth = 70.0f;
        closeBtn->mHeight = 28.0f;
        closeBtn->mText = "关闭(B)";
        closeBtn->mNormalBgR = 50; closeBtn->mNormalBgG = 50; closeBtn->mNormalBgB = 60;
        closeBtn->mHoverBgR = 80; closeBtn->mHoverBgG = 80; closeBtn->mHoverBgB = 90;
        closeBtn->mOnClick = [this]() {
            mInventoryOpen = false;
            mSelectedItemIndex = -1;
            updateInventoryDisplay();
        };
        panel->addChild(std::move(closeBtn));

        backdrop->addChild(std::move(panel));
        mUIManager.addElement(std::move(backdrop));
        mInvOverlay = mInvBackdrop;
    }
}

void BigWorldScene::updateInventoryDisplay()
{
    if (!mPlayerData || !mInvBackdrop) return;

    mInvBackdrop->mVisible = mInventoryOpen;

    // 更新格子
    for (int i = 0; i < static_cast<int>(mInvSlotButtons.size()); ++i) {
        auto* btn = mInvSlotButtons[i];

        if (i < static_cast<int>(mPlayerData->inventory.size())) {
            const Item& item = mPlayerData->inventory[i];

            // 不同类型的物品用不同背景色
            switch (item.type) {
            case ItemType::Consumable:
                btn->mNormalBgR = 30; btn->mNormalBgG = 60; btn->mNormalBgB = 30;
                break;
            case ItemType::Equipment:
                btn->mNormalBgR = 50; btn->mNormalBgG = 40; btn->mNormalBgB = 20;
                break;
            case ItemType::Material:
                btn->mNormalBgR = 40; btn->mNormalBgG = 40; btn->mNormalBgB = 60;
                break;
            default:
                btn->mNormalBgR = 35; btn->mNormalBgG = 35; btn->mNormalBgB = 50;
                break;
            }

            // 选中高亮
            if (i == mSelectedItemIndex) {
                btn->mNormalBgR = 100;
                btn->mNormalBgG = 80;
                btn->mNormalBgB = 20;
            }

            btn->mHoverBgR = static_cast<Uint8>(btn->mNormalBgR * 1.3f);
            btn->mHoverBgG = static_cast<Uint8>(btn->mNormalBgG * 1.3f);
            btn->mHoverBgB = static_cast<Uint8>(btn->mNormalBgB * 1.3f);

            // 物品名（截短）
            std::string shortName = item.name;
            if (shortName.length() > 4) shortName = shortName.substr(0, 4);
            if (item.quantity > 1) {
                shortName += "\nx" + std::to_string(item.quantity);
            }
            btn->mText = shortName;
        } else {
            // 空格
            btn->mNormalBgR = 35; btn->mNormalBgG = 35; btn->mNormalBgB = 50;
            btn->mHoverBgR = 50; btn->mHoverBgG = 50; btn->mHoverBgB = 70;
            btn->mText = "";
            if (i == mSelectedItemIndex && mSelectedItemIndex >= static_cast<int>(mPlayerData->inventory.size())) {
                mSelectedItemIndex = -1;
            }
        }
    }

    // 更新详情
    if (mInvItemNameLabel && mInvItemDescLabel) {
        if (mSelectedItemIndex >= 0
            && mSelectedItemIndex < static_cast<int>(mPlayerData->inventory.size())) {
            const Item& item = mPlayerData->inventory[mSelectedItemIndex];
            mInvItemNameLabel->mText = item.name
                + (item.quantity > 1 ? " x" + std::to_string(item.quantity) : "");
            mInvItemDescLabel->mText = item.desc;
        } else {
            mInvItemNameLabel->mText = "";
            mInvItemDescLabel->mText = "点击左侧物品查看详情";
        }
    }
}

// ==================== 角色面板UI ====================

void BigWorldScene::buildCharPanelUI()
{
    auto backdrop = std::make_unique<UIPanel>();
    backdrop->mX = 0;
    backdrop->mY = 0;
    backdrop->mWidth = mVpW;
    backdrop->mHeight = mVpH;
    backdrop->mBgR = 0; backdrop->mBgG = 0; backdrop->mBgB = 0; backdrop->mBgA = 150;
    backdrop->mVisible = false;
    mCharBackdrop = backdrop.get();

    auto panel = std::make_unique<UIPanel>();
    panel->mX = mVpW * 0.5f - 260.0f;
    panel->mY = mVpH * 0.5f - 250.0f;
    panel->mWidth = 520.0f;
    panel->mHeight = 500.0f;
    panel->mBgR = 20; panel->mBgG = 20; panel->mBgB = 35; panel->mBgA = 240;

    // 标题
    auto title = std::make_unique<UILabel>();
    title->mX = 20.0f;
    title->mY = 10.0f;
    title->mText = "角色属性";
    title->mTextR = 255; title->mTextG = 215; title->mTextB = 0;
    panel->addChild(std::move(title));

    // 名字+等级
    auto nameLabel = std::make_unique<UILabel>();
    nameLabel->mX = 20.0f;
    nameLabel->mY = 40.0f;
    nameLabel->mText = "青云修士 Lv.1";
    nameLabel->mTextR = 255; nameLabel->mTextG = 255; nameLabel->mTextB = 255;
    mCharNameLabel = nameLabel.get();
    panel->addChild(std::move(nameLabel));

    // HP条
    auto hpBar = std::make_unique<UIProgressBar>();
    hpBar->mX = 20.0f;
    hpBar->mY = 70.0f;
    hpBar->mWidth = 220.0f;
    hpBar->mHeight = 18.0f;
    hpBar->mFgR = 200; hpBar->mFgG = 40; hpBar->mFgB = 40;
    hpBar->mBgR = 30; hpBar->mBgG = 30; hpBar->mBgB = 30;
    hpBar->mLabelR = 255; hpBar->mLabelG = 255; hpBar->mLabelB = 255;
    panel->addChild(std::move(hpBar));

    // SP条
    auto spBar = std::make_unique<UIProgressBar>();
    spBar->mX = 20.0f;
    spBar->mY = 94.0f;
    spBar->mWidth = 220.0f;
    spBar->mHeight = 14.0f;
    spBar->mFgR = 80; spBar->mFgG = 120; spBar->mFgB = 220;
    spBar->mBgR = 30; spBar->mBgG = 30; spBar->mBgB = 30;
    spBar->mLabelR = 200; spBar->mLabelG = 200; spBar->mLabelB = 255;
    panel->addChild(std::move(spBar));

    // XP条
    auto xpBar = std::make_unique<UIProgressBar>();
    xpBar->mX = 20.0f;
    xpBar->mY = 114.0f;
    xpBar->mWidth = 220.0f;
    xpBar->mHeight = 10.0f;
    xpBar->mFgR = 180; xpBar->mFgG = 160; xpBar->mFgB = 40;
    xpBar->mBgR = 30; xpBar->mBgG = 30; xpBar->mBgB = 30;
    xpBar->mLabelR = 220; xpBar->mLabelG = 200; xpBar->mLabelB = 100;
    panel->addChild(std::move(xpBar));

    // 属性
    auto statsLabel = std::make_unique<UILabel>();
    statsLabel->mX = 20.0f;
    statsLabel->mY = 140.0f;
    statsLabel->mText = "攻击:\n防御:\n速度:\n五行:";
    statsLabel->mTextR = 180; statsLabel->mTextG = 180; statsLabel->mTextB = 200;
    mCharStatsLabel = statsLabel.get();
    panel->addChild(std::move(statsLabel));

    // 装备
    auto equipTitle = std::make_unique<UILabel>();
    equipTitle->mX = 280.0f;
    equipTitle->mY = 40.0f;
    equipTitle->mText = "-- 装备 --";
    equipTitle->mTextR = 200; equipTitle->mTextG = 150; equipTitle->mTextB = 60;
    panel->addChild(std::move(equipTitle));

    auto equipLabel = std::make_unique<UILabel>();
    equipLabel->mX = 280.0f;
    equipLabel->mY = 70.0f;
    equipLabel->mText = "武器: 无\n护甲: 无\n饰品: 无";
    equipLabel->mTextR = 200; equipLabel->mTextG = 200; equipLabel->mTextB = 200;
    mCharEquipLabel = equipLabel.get();
    panel->addChild(std::move(equipLabel));

    // 技能
    auto skillsTitle = std::make_unique<UILabel>();
    skillsTitle->mX = 280.0f;
    skillsTitle->mY = 220.0f;
    skillsTitle->mText = "-- 技能 --";
    skillsTitle->mTextR = 200; skillsTitle->mTextG = 150; skillsTitle->mTextB = 60;
    panel->addChild(std::move(skillsTitle));

    auto skillsLabel = std::make_unique<UILabel>();
    skillsLabel->mX = 280.0f;
    skillsLabel->mY = 250.0f;
    skillsLabel->mText = "";
    skillsLabel->mTextR = 180; skillsLabel->mTextG = 180; skillsLabel->mTextB = 200;
    mCharSkillsLabel = skillsLabel.get();
    panel->addChild(std::move(skillsLabel));

    // 关闭按钮
    auto closeBtn = std::make_unique<UIButton>();
    closeBtn->mX = panel->mWidth - 90.0f;
    closeBtn->mY = panel->mHeight - 40.0f;
    closeBtn->mWidth = 70.0f;
    closeBtn->mHeight = 28.0f;
    closeBtn->mText = "关闭(C)";
    closeBtn->mNormalBgR = 50; closeBtn->mNormalBgG = 50; closeBtn->mNormalBgB = 60;
    closeBtn->mHoverBgR = 80; closeBtn->mHoverBgG = 80; closeBtn->mHoverBgB = 90;
    closeBtn->mOnClick = [this]() {
        mCharPanelOpen = false;
        updateCharPanelDisplay();
    };
    panel->addChild(std::move(closeBtn));

    backdrop->addChild(std::move(panel));
    mUIManager.addElement(std::move(backdrop));
    mCharOverlay = mCharBackdrop;
}

void BigWorldScene::updateCharPanelDisplay()
{
    if (!mPlayerData || !mCharBackdrop) return;

    mCharBackdrop->mVisible = mCharPanelOpen;

    if (!mCharPanelOpen) return;

    // 名字+等级
    if (mCharNameLabel) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s  Lv.%d  (XP: %d/100)",
            mPlayerData->name.c_str(), mPlayerData->level, mPlayerData->xp);
        mCharNameLabel->mText = buf;
    }

    // 属性
    if (mCharStatsLabel) {
        const char* elemNames[] = {"金","木","水","火","土"};
        const char* elem = elemNames[static_cast<int>(mPlayerData->element)];
        char buf[256];
        snprintf(buf, sizeof(buf),
            "攻击: %d\n防御: %d\n速度: %d\n五行: %s",
            mPlayerData->getEffectiveAttack(),
            mPlayerData->getEffectiveDefense(),
            mPlayerData->speed,
            elem);
        mCharStatsLabel->mText = buf;
    }

    // 装备
    if (mCharEquipLabel) {
        char buf[256];
        snprintf(buf, sizeof(buf),
            "武器: %s\n护甲: %s\n饰品: %s",
            mPlayerData->weapon ? mPlayerData->weapon->name.c_str() : "无",
            mPlayerData->armor ? mPlayerData->armor->name.c_str() : "无",
            mPlayerData->accessory ? mPlayerData->accessory->name.c_str() : "无");
        mCharEquipLabel->mText = buf;
    }

    // 技能
    if (mCharSkillsLabel) {
        std::string skillText;
        for (auto& s : mPlayerData->skills) {
            if (!skillText.empty()) skillText += "\n";
            skillText += s.mName + " 灵力:" + std::to_string(s.mSpiritCost)
                      + " 伤害:" + std::to_string(s.mBaseDamage);
        }
        mCharSkillsLabel->mText = skillText;
    }
}
