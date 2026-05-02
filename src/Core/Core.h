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

// 工厂模式 - 基础类首先
#include "BaseTexture.h"
#include "BaseFont.h"
#include "BaseAudio.h"

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

// 工厂模式 - 纹理相关
#include "FileTexture.h"
#include "MemoryTexture.h"
#include "BlankTexture.h"
#include "SurfaceTexture.h"
#include "TextureFactory.h"

// 工厂模式 - 字体相关
#include "TrueTypeFont.h"
#include "MemoryFont.h"
#include "FontFactory.h"

// 工厂模式 - 音频相关
#include "FileAudio.h"
#include "MemoryAudio.h"
#include "AudioFactory.h"

// 工厂模式 - 游戏对象相关
#include "GameObjectFactory.h"

// UI系统
#include "UI/UIElement.h"
#include "UI/UIPanel.h"
#include "UI/UILabel.h"
#include "UI/UIButton.h"
#include "UI/UIProgressBar.h"
#include "UI/UIManager.h"

#endif // CORE_H
