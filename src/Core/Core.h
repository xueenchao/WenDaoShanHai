/**
 * Core.h - 核心层统一头文件
 *
 * 包含所有核心层类的头文件，方便使用时只需 #include "Core/Core.h"
 *
 * 核心层包含以下模块：
 *   - Window:       窗口管理（创建、属性、全屏切换）
 *   - Renderer:     2D 渲染器（绘制图元、纹理渲染、逻辑分辨率）
 *   - Texture:      纹理管理（图片加载、空白纹理、纹理效果）
 *   - Audio:        音频管理（音效/音乐加载播放、音量控制）
 *   - Font:         字体与文字渲染（TTF字体、文字绘制、自动换行）
 *   - EventHandler: 事件处理（键盘/鼠标输入、窗口事件）
 *   - GameLoop:     游戏主循环（初始化-更新-渲染循环、帧率控制）
 */

#ifndef CORE_H
#define CORE_H

#include "Window.h"
#include "Renderer.h"
#include "Texture.h"
#include "Audio.h"
#include "Font.h"
#include "EventHandler.h"
#include "GameLoop.h"
#include "Scene.h"
#include "ResourceManager.h"
#include "CollisionSystem.h"
#include "Camera2D.h"
#include "Log.h"

// UI系统
#include "UI/UIElement.h"
#include "UI/UIPanel.h"
#include "UI/UILabel.h"
#include "UI/UIButton.h"
#include "UI/UIProgressBar.h"
#include "UI/UIManager.h"

#endif // CORE_H
