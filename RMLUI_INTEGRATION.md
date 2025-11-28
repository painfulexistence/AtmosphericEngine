# RmlUi Integration Summary

## 概述

RmlUi 已成功集成到 AtmosphericEngine，用于创建游戏内 UI（HUD、对话框、背包等）。

## 集成内容

### 1. 核心文件

#### 渲染层
- **rmlui_renderer.hpp/cpp**: RmlUi 渲染接口实现
  - 实现了 `Rml::RenderInterface`
  - 使用 OpenGL 直接渲染（独立于 Canvas 系统）
  - 支持纹理、裁剪区域、变换矩阵

#### 系统接口
- **rmlui_system.hpp/cpp**: RmlUi 系统接口实现
  - 实现了 `Rml::SystemInterface`
  - 提供时间、日志、剪贴板等系统功能
  - Windows 平台剪贴板支持

#### 管理器
- **rmlui_manager.hpp/cpp**: RmlUi 管理器单例
  - 初始化和关闭 RmlUi
  - 文档加载和管理
  - 输入事件处理
  - 渲染循环管理

### 2. 引擎集成

#### GameLayer 修改
- `GameLayer::OnAttach()`: 初始化 RmlUi
- `GameLayer::OnUpdate()`: 更新 RmlUi 上下文
- `GameLayer::OnRender()`: 在 3D 场景之后渲染 UI
- `GameLayer::OnDetach()`: 关闭 RmlUi

#### renderer.cpp 修改
- `UIPass::Execute()`: 为 RmlUi 预留的渲染 pass（当前为空，RmlUi 使用自己的渲染接口）

### 3. CMake 配置

- 添加了 RmlUi 6.0 作为 FetchContent 依赖
- 链接 `RmlCore` 和 `RmlDebugger` 库
- 添加了三个新的源文件到构建系统

### 4. 示例 UI 资源

位于 `assets/ui/` 目录：

- **hud.rml**: 游戏 HUD（生命值条、指南针、目标文字）
- **dialog.rml**: 对话界面（NPC 对话、选项）
- **inventory.rml**: 背包系统（6x4 网格布局）
- **style.rcss**: 全局 UI 样式
- **README.md**: 使用指南

## 使用方法

### 基本用法

```cpp
// 在游戏 OnLoad 中加载 UI
auto hud = RmlUiManager::Get()->LoadDocument("assets/ui/hud.rml");
if (hud) {
    hud->Show();
}
```

### 动态更新 UI

```cpp
auto context = RmlUiManager::Get()->GetContext();
auto document = context->GetDocument("hud");
if (document) {
    auto element = document->GetElementById("health-fill");
    if (element) {
        float healthPercent = playerHealth / maxHealth;
        element->SetProperty("width",
            Rml::String(std::to_string(healthPercent * 100) + "%"));
    }
}
```

### 显示/隐藏文档

```cpp
RmlUiManager::Get()->ShowDocument("inventory");
RmlUiManager::Get()->HideDocument("dialog");
```

## 架构说明

### 渲染流程

1. **GameLayer::OnRender()** 调用
2. **GraphicsServer::Render()** 渲染 3D 场景
3. **RmlUiManager::Render()** 渲染 UI 层
   - 设置正交投影
   - 启用混合模式
   - RmlUi 通过 `RmlUiRenderer` 发送渲染命令
   - 直接使用 OpenGL 绘制

### 输入处理

RmlUiManager 提供了输入方法，可以从游戏的输入系统调用：

```cpp
// 鼠标事件
RmlUiManager::Get()->ProcessMouseMove(x, y, modifiers);
RmlUiManager::Get()->ProcessMouseButtonDown(button, modifiers);
RmlUiManager::Get()->ProcessMouseButtonUp(button, modifiers);
RmlUiManager::Get()->ProcessMouseWheel(delta, modifiers);

// 键盘事件
RmlUiManager::Get()->ProcessKeyDown(key, modifiers);
RmlUiManager::Get()->ProcessKeyUp(key, modifiers);
RmlUiManager::Get()->ProcessTextInput(character);
```

## 功能特性

### 已实现
- ✅ RmlUi 核心集成
- ✅ OpenGL 渲染器
- ✅ 系统接口（时间、日志、剪贴板）
- ✅ 文档加载和管理
- ✅ 输入事件接口
- ✅ 调试器支持（RmlDebugger）
- ✅ 示例 UI 模板（HUD、对话、背包）
- ✅ 样式系统
- ✅ 混合和透明度支持
- ✅ 裁剪区域支持

### 待完善
- ⚠️ 图片纹理加载（LoadTexture 方法需要连接到 AssetManager）
- ⚠️ 输入事件自动转发（需要在 Input 系统中添加）
- ⚠️ 字体文件（需要在 assets/fonts/ 放置字体文件）
- ⚠️ 窗口调整大小处理（需要在 Window 系统中添加回调）

## 依赖

- RmlUi 6.0（通过 FetchContent 自动下载）
- OpenGL 3.3+
- GLM（现有依赖）

## 文件清单

### 新增头文件
- `AtmosphericEngine/include/Atmospheric/rmlui_renderer.hpp`
- `AtmosphericEngine/include/Atmospheric/rmlui_system.hpp`
- `AtmosphericEngine/include/Atmospheric/rmlui_manager.hpp`

### 新增源文件
- `AtmosphericEngine/src/rmlui_renderer.cpp`
- `AtmosphericEngine/src/rmlui_system.cpp`
- `AtmosphericEngine/src/rmlui_manager.cpp`

### 修改的文件
- `AtmosphericEngine/CMakeLists.txt`（添加依赖和源文件）
- `AtmosphericEngine/src/renderer.cpp`（实现 UIPass::Execute）
- `AtmosphericEngine/include/Atmospheric/game_layer.hpp`（添加 OnAttach/OnDetach）
- `AtmosphericEngine/src/game_layer.cpp`（集成 RmlUi 初始化和渲染）

### UI 资源
- `assets/ui/hud.rml`
- `assets/ui/dialog.rml`
- `assets/ui/inventory.rml`
- `assets/ui/style.rcss`
- `assets/ui/README.md`

## 注意事项

1. **字体**: RmlUiManager 尝试加载 `assets/fonts/LatoLatin-Regular.ttf`，如果文件不存在会显示警告但不会崩溃
2. **纹理加载**: 当前图片纹理加载未实现，需要连接到 AssetManager
3. **性能**: RmlUi 使用即时模式渲染，每帧重新提交顶点数据
4. **调试**: 使用 F8 键切换 RmlUi 调试器（在 debug 构建中）

## 下一步

1. 实现图片纹理加载
2. 添加字体文件到 assets/fonts/
3. 集成输入事件自动转发
4. 添加窗口大小调整处理
5. 创建更多游戏 UI 模板
6. 添加 UI 事件处理器（按钮点击、表单提交等）

## 参考资料

- RmlUi 官方文档: https://mikke89.github.io/RmlUiDoc/
- RmlUi GitHub: https://github.com/mikke89/RmlUi
- 示例代码: `assets/ui/README.md`
