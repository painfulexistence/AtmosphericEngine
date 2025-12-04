# Procedural Mesh Builder 延伸應用：CSG Geometry for Level Blockouts

## 概述

本文件描述如何擴展現有的 Procedural Mesh Builder 系統，實現 CSG (Constructive Solid Geometry) 功能，用於快速搭建遊戲關卡的 blockout。

### 目標使用者

- 獨立開發者（單人或小團隊）
- 需要快速迭代關卡設計
- 熟悉程式碼，偏好 code-driven 工作流

### 設計原則

1. **簡單優先** — 只實作 level blockout 真正需要的功能
2. **Code-driven** — 主要工作流程透過程式碼完成
3. **Tooling 輔助** — ImGui 提供視覺化輔助，而非完整編輯器
4. **與現有系統整合** — 輸出到 MeshBuilder → Mesh 管線

---

## 第一章：CSG 基礎概念

### 1.1 什麼是 CSG？

CSG (Constructive Solid Geometry) 是一種透過布林運算組合簡單形狀來建構複雜幾何的技術。這個技術最早用於 CAD 軟體，後來被 Quake、Half-Life 等遊戲引擎採用作為關卡編輯的基礎。

### 1.2 核心布林運算

CSG 的核心是三種布林運算：

```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│   Union (聯集 ∪)        Subtract (差集 −)      Intersect (交集 ∩)
│                                                             │
│   ┌───┐                 ┌───┐                 ┌───┐        │
│   │ A ├──┐              │ A │                 │ A ├──┐     │
│   └───┴──┤              └─┬─┘                 └───┴──┤     │
│      ┌───┤                │ ┌───┐                ┌───┤     │
│      │ B │              ──┴─┤ B │                │ B │     │
│      └───┘                  └───┘                └───┘     │
│                                                             │
│   結果：A + B           結果：A - B           結果：A ∩ B   │
│   合併兩個形狀          從 A 挖掉 B           只保留重疊部分 │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 1.3 各運算在關卡設計的用途

| 運算 | 符號 | 用途 | 實際範例 |
|-----|------|------|---------|
| **Union** | A ∪ B | 合併多個形狀成複雜結構 | 組合牆壁和地板、連接多個房間 |
| **Subtract** | A − B | 從實體中挖掉部分 | 開門、開窗、挖走廊、製作凹槽 |
| **Intersect** | A ∩ B | 取兩形狀的交集 | 較少用於 blockout，可用於特殊造型 |

### 1.4 基礎形狀 (Primitives)

對於 level blockout，只需要少數幾種基礎形狀：

```
┌─────────────────────────────────────────────────────────────┐
│                      基礎形狀                                │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   Box (方塊)           Cylinder (圓柱)        Wedge (楔形)   │
│   ┌─────────┐          ╭─────╮              ┌─────────┐    │
│   │         │          │     │              │        ╱     │
│   │         │          │     │              │      ╱       │
│   │         │          │     │              │    ╱         │
│   └─────────┘          ╰─────╯              └──╱           │
│                                                             │
│   用途：                用途：               用途：          │
│   - 牆壁、地板          - 柱子              - 斜坡          │
│   - 平台、方塊          - 管道              - 樓梯          │
│   - 90% 的 blockout    - 圓形結構           - 屋頂          │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

**重要觀察**：在 level blockout 階段，90% 以上的幾何都可以用 Box 完成。因此我們的實作可以先專注在 Box CSG，之後再擴充其他形狀。

---

## 第二章：架構設計

### 2.1 整體架構

```
┌─────────────────────────────────────────────────────────────┐
│                      Game Layer                              │
│                                                             │
│   LevelBlockout.cpp                                         │
│   - 定義房間、走廊、門窗                                      │
│   - 使用 CSG API 組合形狀                                    │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ CSG::Box(), CSG::Subtract()
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      CSG Layer                               │
│                                                             │
│   CSGPrimitive (形狀定義)                                    │
│   CSGNode (運算樹)                                           │
│   CSGCompiler (執行布林運算)                                  │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ 編譯成頂點資料
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                   MeshBuilder Layer                          │
│                                                             │
│   接收 CSG 編譯結果                                          │
│   產生 Mesh 資料                                             │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ mesh.Update(data)
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      Mesh Layer                              │
│                                                             │
│   管理 RenderMesh                                            │
│   處理 GPU 資源                                              │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 方案比較

#### 方案 A：Mesh-based CSG（推薦）

直接對 mesh 進行布林運算，最終輸出普通的三角形 mesh。

```cpp
// 概念
MeshData boxA = CreateBoxMesh(size);
MeshData boxB = CreateBoxMesh(size);
MeshData result = BooleanSubtract(boxA, boxB);  // Mesh 布林運算
```

**優點：**
- 最終輸出是普通 Mesh，與現有渲染系統完全相容
- 可以 bake 成靜態 mesh 用於 runtime（零運算成本）
- 編輯時可即時預覽
- 實作相對直觀

**缺點：**
- 複雜形狀的布林運算可能較慢
- 需要處理 mesh 布林運算的數值精度問題
- 可能產生退化三角形，需要清理

#### 方案 B：BSP-based CSG

使用 BSP (Binary Space Partitioning) tree 表示實體空間，這是 Quake/Source 引擎的傳統做法。

```cpp
// 概念
BSPTree treeA = BuildBSP(brushA);
BSPTree treeB = BuildBSP(brushB);
BSPTree result = BSPSubtract(treeA, treeB);
MeshData mesh = BSPToMesh(result);
```

**優點：**
- 數學上精確，無數值精度問題
- 自然地產生 convex 區域，適合碰撞檢測
- 可同時編譯成 visibility 資料（PVS）

**缺點：**
- 實作複雜度高
- 對於簡單 blockout 是過度工程
- BSP 樹的建構和維護成本較高

#### 方案 C：簡化的 Box-Only CSG（最適合 Blockout）

只支援 axis-aligned box 的 CSG 運算，大幅簡化實作。

```cpp
// 概念
Box roomA = Box({0,0,0}, {20,5,20});
Box door = Box({10,0,2}, {3,4,1});
std::vector<Box> result = SubtractBox(roomA, door);  // 回傳多個 box
```

**優點：**
- 實作極度簡單（不需要複雜的 mesh 布林演算法）
- 運算速度快
- 結果永遠是合法的 box 集合
- 涵蓋 90% 的 blockout 需求

**缺點：**
- 只能處理 axis-aligned 形狀
- 無法製作斜面、曲面
- Subtract 可能產生多個 box（視覺上有接縫）

### 2.3 建議方案

對於獨立開發者的 level blockout 需求，建議採用 **方案 C（Box-Only CSG）** 作為起點：

1. **實作簡單** — 幾百行程式碼就能完成
2. **效能優異** — 即時運算無壓力
3. **涵蓋多數需求** — Blockout 階段不需要精細幾何
4. **可漸進擴充** — 之後可加入 Wedge、Cylinder

---

## 第三章：資料結構設計

### 3.1 CSG 原語 (Primitive)

```cpp
// 基礎形狀類型
enum class CSGPrimitiveType {
    Box,        // 方塊（最常用）
    Cylinder,   // 圓柱
    Wedge,      // 楔形/斜坡
    Sphere      // 球體（較少用於 blockout）
};

// CSG 原語定義
struct CSGPrimitive {
    CSGPrimitiveType type = CSGPrimitiveType::Box;

    glm::vec3 position = {0, 0, 0};  // 中心位置
    glm::vec3 size = {1, 1, 1};      // 尺寸 (寬, 高, 深)
    glm::quat rotation = glm::identity<glm::quat>();  // 旋轉（可選）

    // Box 專用：是否 axis-aligned（簡化運算）
    bool axisAligned = true;

    // 材質/外觀提示（用於視覺化）
    uint32_t materialHint = 0;
};
```

### 3.2 CSG 運算節點

使用樹狀結構表示 CSG 運算：

```cpp
// CSG 運算類型
enum class CSGOperation {
    Primitive,   // 葉節點：單一原語
    Union,       // 聯集：合併兩個形狀
    Subtract,    // 差集：從左邊減去右邊
    Intersect    // 交集：取重疊部分
};

// CSG 節點（樹狀結構）
struct CSGNode {
    CSGOperation operation = CSGOperation::Primitive;

    // Primitive 節點使用
    CSGPrimitive primitive;

    // 運算節點使用（左、右子節點）
    std::shared_ptr<CSGNode> left = nullptr;
    std::shared_ptr<CSGNode> right = nullptr;

    // 除錯用名稱
    std::string name;
};
```

### 3.3 視覺化範例

以下是一個簡單房間（有一扇門）的 CSG 樹：

```
                    Subtract
                   ╱        ╲
                  ╱          ╲
            room_box        door_box
           (20×5×20)        (3×4×1)
          at (0,0,0)      at (10,0,2)
```

對應的程式碼：
```cpp
auto room = CSG::Box({0,0,0}, {20,5,20});
auto door = CSG::Box({10,0,2}, {3,4,1});
auto result = CSG::Subtract(room, door);
```

### 3.4 Level Brush（編輯單位）

將 CSG 結構封裝成可管理的編輯單位：

```cpp
// Level Brush - 關卡編輯的基本單位
struct LevelBrush {
    uint32_t id;                    // 唯一識別碼
    std::string name;               // 名稱（如 "room1", "corridor_a"）

    CSGNode root;                   // CSG 樹的根節點

    bool isDirty = true;            // 是否需要重新編譯
    Mesh* bakedMesh = nullptr;      // 編譯後的 mesh（快取）

    // 變換
    glm::vec3 worldPosition = {0,0,0};
    glm::quat worldRotation = glm::identity<glm::quat>();

    // 編輯器狀態
    bool isSelected = false;
    bool isVisible = true;
};

// Level - 關卡資料
struct Level {
    std::string name;
    std::vector<LevelBrush> brushes;

    // 編譯所有 brush 並輸出到 mesh
    void CompileAll();

    // 只編譯髒的 brush
    void CompileDirty();
};
```

---

## 第四章：簡化 CSG 演算法

### 4.1 Box-Only Subtract 演算法

對於 axis-aligned box，subtract 運算可以大幅簡化。核心概念是：從一個 box 減去另一個 box，結果最多是 6 個 box（沿著切割面分割）。

```
原始 Box A（大）減去 Box B（小）：

俯視圖：
┌─────────────────────────┐
│                         │
│    ┌───────────┐        │
│    │     B     │        │
│    │  (挖掉)   │        │
│    └───────────┘        │
│           A             │
└─────────────────────────┘

結果：最多 6 個 box
┌────┬───────────┬────────┐
│ L  │     T     │   R    │  T = Top (上方)
├────┼───────────┼────────┤  B = Bottom (下方，被擋住)
│    │  (空洞)   │        │  L = Left (左側)
├────┼───────────┼────────┤  R = Right (右側)
│    │     B     │        │  F = Front (前方，未顯示)
└────┴───────────┴────────┘  K = Back (後方，未顯示)
```

### 4.2 實作程式碼

```cpp
// AABB (Axis-Aligned Bounding Box) 定義
struct AABB {
    glm::vec3 min;  // 最小角
    glm::vec3 max;  // 最大角

    bool IsValid() const {
        return min.x < max.x && min.y < max.y && min.z < max.z;
    }

    // 計算兩個 AABB 的交集
    static AABB Intersect(const AABB& a, const AABB& b) {
        return {
            glm::max(a.min, b.min),
            glm::min(a.max, b.max)
        };
    }

    // 檢查是否有交集
    bool Intersects(const AABB& other) const {
        return Intersect(*this, other).IsValid();
    }
};

// Box Subtract 實作
std::vector<AABB> SubtractBox(const AABB& a, const AABB& b) {
    std::vector<AABB> result;

    // 計算交集
    AABB intersection = AABB::Intersect(a, b);

    // 如果不相交，回傳原始 box
    if (!intersection.IsValid()) {
        result.push_back(a);
        return result;
    }

    // 沿著 6 個方向切割

    // +X 方向（右側剩餘）
    if (intersection.max.x < a.max.x) {
        result.push_back({
            {intersection.max.x, a.min.y, a.min.z},
            {a.max.x, a.max.y, a.max.z}
        });
    }

    // -X 方向（左側剩餘）
    if (intersection.min.x > a.min.x) {
        result.push_back({
            {a.min.x, a.min.y, a.min.z},
            {intersection.min.x, a.max.y, a.max.z}
        });
    }

    // +Y 方向（上方剩餘）
    if (intersection.max.y < a.max.y) {
        result.push_back({
            {intersection.min.x, intersection.max.y, a.min.z},
            {intersection.max.x, a.max.y, a.max.z}
        });
    }

    // -Y 方向（下方剩餘）
    if (intersection.min.y > a.min.y) {
        result.push_back({
            {intersection.min.x, a.min.y, a.min.z},
            {intersection.max.x, intersection.min.y, a.max.z}
        });
    }

    // +Z 方向（前方剩餘）
    if (intersection.max.z < a.max.z) {
        result.push_back({
            {intersection.min.x, intersection.min.y, intersection.max.z},
            {intersection.max.x, intersection.max.y, a.max.z}
        });
    }

    // -Z 方向（後方剩餘）
    if (intersection.min.z > a.min.z) {
        result.push_back({
            {intersection.min.x, intersection.min.y, a.min.z},
            {intersection.max.x, intersection.max.y, intersection.min.z}
        });
    }

    return result;
}
```

### 4.3 編譯成 Mesh

```cpp
class CSGCompiler {
public:
    // 編譯 CSG 樹成 box 列表
    std::vector<AABB> Compile(const CSGNode& node) {
        switch (node.operation) {
            case CSGOperation::Primitive:
                return CompilePrimitive(node.primitive);

            case CSGOperation::Union:
                return CompileUnion(node);

            case CSGOperation::Subtract:
                return CompileSubtract(node);

            default:
                return {};
        }
    }

private:
    std::vector<AABB> CompilePrimitive(const CSGPrimitive& prim) {
        if (prim.type != CSGPrimitiveType::Box) {
            // 目前只支援 Box
            return {};
        }

        glm::vec3 halfSize = prim.size * 0.5f;
        return {{
            prim.position - halfSize,
            prim.position + halfSize
        }};
    }

    std::vector<AABB> CompileUnion(const CSGNode& node) {
        auto leftBoxes = Compile(*node.left);
        auto rightBoxes = Compile(*node.right);

        // Union 就是合併兩個列表
        leftBoxes.insert(leftBoxes.end(),
                         rightBoxes.begin(), rightBoxes.end());
        return leftBoxes;
    }

    std::vector<AABB> CompileSubtract(const CSGNode& node) {
        auto leftBoxes = Compile(*node.left);
        auto rightBoxes = Compile(*node.right);

        // 對每個左側 box，減去所有右側 box
        std::vector<AABB> result;

        for (const auto& leftBox : leftBoxes) {
            std::vector<AABB> current = {leftBox};

            for (const auto& rightBox : rightBoxes) {
                std::vector<AABB> next;
                for (const auto& box : current) {
                    auto subtracted = SubtractBox(box, rightBox);
                    next.insert(next.end(),
                               subtracted.begin(), subtracted.end());
                }
                current = std::move(next);
            }

            result.insert(result.end(), current.begin(), current.end());
        }

        return result;
    }
};

// 將 AABB 列表轉換成 Mesh
void CSGToMesh(const std::vector<AABB>& boxes, Mesh& outMesh) {
    MeshBuilder builder;

    for (const auto& box : boxes) {
        glm::vec3 center = (box.min + box.max) * 0.5f;
        glm::vec3 size = box.max - box.min;
        builder.PushCube(center, size);
    }

    outMesh.Update(builder.Build());
}
```

---

## 第五章：Code-Driven 工作流程

### 5.1 設計理念

對於程式設計背景的開發者，**code-driven** 工作流程通常比視覺編輯器更有效率：

| 視覺編輯器 | Code-Driven |
|-----------|-------------|
| 拖曳、點擊、手動輸入數值 | 直接寫數字 |
| 難以精確對齊 | 數學計算精確 |
| 重複操作繁瑣 | 迴圈、函數重複使用 |
| 版本控制困難 | Git 友善 |
| 需要學習編輯器操作 | 用熟悉的 IDE |

### 5.2 核心 API 設計

簡潔的 fluent API，讓 level blockout 程式碼易讀易寫：

```cpp
namespace CSG {
    // 建立基礎形狀
    CSGNode Box(glm::vec3 position, glm::vec3 size);
    CSGNode Cylinder(glm::vec3 position, float radius, float height);
    CSGNode Wedge(glm::vec3 position, glm::vec3 size, Direction slopeDir);

    // 布林運算
    CSGNode Union(CSGNode a, CSGNode b);
    CSGNode Subtract(CSGNode a, CSGNode b);
    CSGNode Intersect(CSGNode a, CSGNode b);

    // 編譯成 mesh 資料
    std::vector<AABB> Compile(const CSGNode& node);
}
```

### 5.3 完整範例：簡單關卡

```cpp
// level_blockout.cpp
#include "csg.hpp"

class LevelBlockout {
public:
    Mesh levelMesh;

    void Build() {
        // ═══════════════════════════════════════════
        // 第一區：主廳
        // ═══════════════════════════════════════════

        // 地板
        auto floor = CSG::Box({0, -0.5f, 0}, {50, 1, 50});

        // 主廳房間
        auto mainHall = CSG::Box({0, 2.5f, 0}, {20, 5, 30});

        // 主廳入口（從南側）
        auto mainEntrance = CSG::Box({0, 1.5f, 15}, {4, 3, 2});
        mainHall = CSG::Subtract(mainHall, mainEntrance);

        // 主廳側門（往東西兩側）
        auto sideDoorEast = CSG::Box({10, 1.5f, 0}, {2, 3, 3});
        auto sideDoorWest = CSG::Box({-10, 1.5f, 0}, {2, 3, 3});
        mainHall = CSG::Subtract(mainHall, sideDoorEast);
        mainHall = CSG::Subtract(mainHall, sideDoorWest);

        // ═══════════════════════════════════════════
        // 第二區：東側走廊
        // ═══════════════════════════════════════════

        auto corridorEast = CSG::Box({18, 2, 0}, {6, 4, 3});

        // 東側小房間
        auto roomEast = CSG::Box({28, 2, 0}, {10, 4, 8});
        auto roomEastDoor = CSG::Box({23, 1.5f, 0}, {2, 3, 2});
        roomEast = CSG::Subtract(roomEast, roomEastDoor);

        // ═══════════════════════════════════════════
        // 第三區：西側走廊（帶窗戶）
        // ═══════════════════════════════════════════

        auto corridorWest = CSG::Box({-18, 2, 0}, {6, 4, 15});

        // 沿走廊開窗
        for (int i = -2; i <= 2; i++) {
            auto window = CSG::Box({-21, 2.5f, i * 3.0f}, {1, 1.5f, 1.5f});
            corridorWest = CSG::Subtract(corridorWest, window);
        }

        // ═══════════════════════════════════════════
        // 組合所有區域
        // ═══════════════════════════════════════════

        auto level = CSG::Union(floor, mainHall);
        level = CSG::Union(level, corridorEast);
        level = CSG::Union(level, roomEast);
        level = CSG::Union(level, corridorWest);

        // ═══════════════════════════════════════════
        // 編譯成 Mesh
        // ═══════════════════════════════════════════

        auto boxes = CSG::Compile(level);
        CSGToMesh(boxes, levelMesh);
    }
};
```

### 5.4 使用輔助函數提高可讀性

```cpp
// level_helpers.hpp

// 建立標準房間（帶單一門）
CSGNode MakeRoom(glm::vec3 pos, glm::vec3 size,
                 Direction doorSide, glm::vec3 doorOffset = {0,0,0}) {
    auto room = CSG::Box(pos, size);

    glm::vec3 doorPos = pos + doorOffset;
    glm::vec3 doorSize = {1.0f, 2.5f, 0.2f};

    // 根據門的方向調整位置
    switch (doorSide) {
        case Direction::North:
            doorPos.z += size.z / 2;
            break;
        case Direction::South:
            doorPos.z -= size.z / 2;
            break;
        // ... 其他方向
    }

    auto door = CSG::Box(doorPos, doorSize);
    return CSG::Subtract(room, door);
}

// 建立走廊
CSGNode MakeCorridor(glm::vec3 start, glm::vec3 end,
                     float width = 3.0f, float height = 3.0f) {
    glm::vec3 center = (start + end) * 0.5f;
    glm::vec3 size = glm::abs(end - start);
    size.x = std::max(size.x, width);
    size.y = height;
    size.z = std::max(size.z, width);

    return CSG::Box(center, size);
}

// 建立樓梯（使用多個 wedge）
CSGNode MakeStairs(glm::vec3 pos, int stepCount,
                   float stepWidth, float stepHeight, float stepDepth) {
    CSGNode stairs;
    bool first = true;

    for (int i = 0; i < stepCount; i++) {
        glm::vec3 stepPos = pos + glm::vec3{0, i * stepHeight, i * stepDepth};
        auto step = CSG::Box(stepPos, {stepWidth, stepHeight, stepDepth});

        if (first) {
            stairs = step;
            first = false;
        } else {
            stairs = CSG::Union(stairs, step);
        }
    }

    return stairs;
}

// 使用範例
void BuildLevel() {
    auto lobby = MakeRoom({0, 0, 0}, {15, 5, 15}, Direction::North);
    auto corridor = MakeCorridor({0, 0, 7.5f}, {0, 0, 20});
    auto office = MakeRoom({0, 0, 27.5f}, {10, 4, 10}, Direction::South);

    auto level = CSG::Union(lobby, corridor);
    level = CSG::Union(level, office);
}
```

### 5.5 Hot Reload 支援

讓修改程式碼後立即看到結果：

```cpp
// hot_reload.hpp

class HotReloadManager {
public:
    using ReloadCallback = std::function<void()>;

    void WatchFile(const std::string& filepath, ReloadCallback callback) {
        _watchedFiles[filepath] = {
            GetLastModifiedTime(filepath),
            callback
        };
    }

    void Update() {
        for (auto& [path, info] : _watchedFiles) {
            auto currentTime = GetLastModifiedTime(path);
            if (currentTime > info.lastModified) {
                info.lastModified = currentTime;

                // 重新編譯
                if (RecompileFile(path)) {
                    // 呼叫 callback 重建關卡
                    info.callback();
                }
            }
        }
    }

private:
    struct WatchInfo {
        std::filesystem::file_time_type lastModified;
        ReloadCallback callback;
    };

    std::unordered_map<std::string, WatchInfo> _watchedFiles;
};

// 使用範例
void SetupHotReload(LevelBlockout& level) {
    HotReloadManager::Get().WatchFile(
        "src/level_blockout.cpp",
        [&level]() {
            level.Build();  // 重新建構關卡
            fmt::print("Level reloaded!\n");
        }
    );
}
```

---

## 第六章：ImGui Tooling（輔助工具）

### 6.1 設計原則

ImGui tooling 的定位是 **輔助**，而非取代 code-driven 工作流程：

- **不做**：完整的視覺編輯器、拖曳操作、複雜的 UI
- **要做**：顯示座標、尺寸標註、數值微調、快速操作

### 6.2 Panel 設計

#### Panel 1: Brush Inspector（形狀檢視器）

顯示選中形狀的詳細資訊，並允許微調：

```
┌─ Brush Inspector ─────────────────────────────────────────┐
│                                                           │
│ Selected: mainHall                                        │
│ Type: Box (Subtract)                                      │
│                                                           │
│ ┌─ Transform ───────────────────────────────────────────┐ │
│ │ Position:  X [  0.00] Y [  2.50] Z [  0.00]          │ │
│ │ Size:      W [ 20.00] H [  5.00] D [ 30.00]          │ │
│ │ Rotation:  [ 0.0°]                                    │ │
│ └───────────────────────────────────────────────────────┘ │
│                                                           │
│ ┌─ Operations ──────────────────────────────────────────┐ │
│ │ ● Union  ○ Subtract  ○ Intersect                     │ │
│ └───────────────────────────────────────────────────────┘ │
│                                                           │
│ ┌─ Quick Actions ───────────────────────────────────────┐ │
│ │ [Duplicate] [Delete] [Focus Camera]                   │ │
│ └───────────────────────────────────────────────────────┘ │
│                                                           │
│ ┌─ Code ────────────────────────────────────────────────┐ │
│ │ auto mainHall = CSG::Box(                             │ │
│ │     {0.00, 2.50, 0.00},                               │ │
│ │     {20.00, 5.00, 30.00}                              │ │
│ │ );                                                    │ │
│ │                                                       │ │
│ │ [Copy to Clipboard]                                   │ │
│ └───────────────────────────────────────────────────────┘ │
│                                                           │
└───────────────────────────────────────────────────────────┘
```

#### Panel 2: Viewport Overlay（視窗覆蓋層）

在 3D 視窗上顯示輔助資訊：

```
┌─ 3D Viewport ─────────────────────────────────────────────┐
│                                                           │
│     Grid: 1.0m                                            │
│                                                           │
│              ← 20.0 →                                     │
│            ┌─────────────────┐                            │
│            │                 │ ↑                          │
│     ← 10.0 │    mainHall     │ 5.0                        │
│            │                 │ ↓                          │
│            │    ┌───┐        │                            │
│            │    │   │ door   │                            │
│            └────┴───┴────────┘                            │
│                                                           │
│     Origin: (0.0, 2.5, 0.0)                               │
│                                                           │
│  ┌────────────────────────────────────────────────────┐   │
│  │ [Grid: 1.0▾] [Snap: ON] [Show Dimensions: ON]      │   │
│  └────────────────────────────────────────────────────┘   │
│                                                           │
└───────────────────────────────────────────────────────────┘
```

顯示內容：
- 選中物件的尺寸標註（箭頭 + 數字）
- 世界座標原點
- Grid 參考線
- 物件名稱標籤

#### Panel 3: Level Hierarchy（階層檢視）

顯示關卡中所有 brush 的樹狀結構：

```
┌─ Level Hierarchy ─────────────────────────────────────────┐
│                                                           │
│ ▼ Level: demo_level                                       │
│   ├─ ▼ floor (Box)                                        │
│   ├─ ▼ mainHall (Subtract)                                │
│   │   ├─ room (Box)                                       │
│   │   ├─ mainEntrance (Box)                               │
│   │   ├─ sideDoorEast (Box)                               │
│   │   └─ sideDoorWest (Box)                               │
│   ├─ ▶ corridorEast (Box)                                 │
│   ├─ ▼ roomEast (Subtract)                                │
│   │   ├─ room (Box)                                       │
│   │   └─ door (Box)                                       │
│   └─ ▼ corridorWest (Subtract)                            │
│       ├─ corridor (Box)                                   │
│       ├─ window_0 (Box)                                   │
│       ├─ window_1 (Box)                                   │
│       ├─ window_2 (Box)                                   │
│       ├─ window_3 (Box)                                   │
│       └─ window_4 (Box)                                   │
│                                                           │
│ Brushes: 15 | Triangles: 2,340                            │
│                                                           │
└───────────────────────────────────────────────────────────┘
```

#### Panel 4: Quick Actions（快速操作）

常用操作的快捷按鈕：

```
┌─ Quick Actions ───────────────────────────────────────────┐
│                                                           │
│ ┌─ Create Primitive ──────────────────────────────────┐   │
│ │ [Box] [Cylinder] [Wedge] [Sphere]                   │   │
│ └─────────────────────────────────────────────────────┘   │
│                                                           │
│ ┌─ Operations ────────────────────────────────────────┐   │
│ │ [Union Selected] [Subtract Selected]                │   │
│ │ [Duplicate] [Delete Selected]                       │   │
│ └─────────────────────────────────────────────────────┘   │
│                                                           │
│ ┌─ Snap Settings ─────────────────────────────────────┐   │
│ │ Grid Size: [0.5 ▾]   Angle Snap: [15° ▾]            │   │
│ │ [x] Snap to Grid    [x] Snap Rotation               │   │
│ └─────────────────────────────────────────────────────┘   │
│                                                           │
│ ┌─ Build ─────────────────────────────────────────────┐   │
│ │ [Rebuild All] [Rebuild Selected]                    │   │
│ │ [Export to Code] [Save Level]                       │   │
│ └─────────────────────────────────────────────────────┘   │
│                                                           │
└───────────────────────────────────────────────────────────┘
```

#### Panel 5: Code Preview（程式碼預覽）

即時顯示目前關卡對應的程式碼：

```
┌─ Generated Code ──────────────────────────────────────────┐
│                                                           │
│ // Auto-generated level blockout                          │
│ // Generated at: 2024-01-15 14:32:05                      │
│                                                           │
│ void BuildLevel() {                                       │
│     // Floor                                              │
│     auto floor = CSG::Box(                                │
│         {0.00, -0.50, 0.00},                              │
│         {50.00, 1.00, 50.00}                              │
│     );                                                    │
│                                                           │
│     // Main Hall                                          │
│     auto mainHall = CSG::Box(                             │
│         {0.00, 2.50, 0.00},                               │
│         {20.00, 5.00, 30.00}                              │
│     );                                                    │
│     auto mainEntrance = CSG::Box(                         │
│         {0.00, 1.50, 15.00},                              │
│         {4.00, 3.00, 2.00}                                │
│     );                                                    │
│     mainHall = CSG::Subtract(mainHall, mainEntrance);     │
│                                                           │
│     // ... more code ...                                  │
│ }                                                         │
│                                                           │
│ [Copy All] [Save to File] [Apply Changes]                 │
│                                                           │
└───────────────────────────────────────────────────────────┘
```

### 6.3 Viewport 互動

即使是 code-driven 工作流程，基本的視窗互動還是有幫助的：

#### 選取
- **左鍵點擊**：選取 brush
- **Ctrl + 左鍵**：多選
- **框選**：不實作（保持簡單）

#### 相機控制
- **右鍵拖曳**：旋轉視角
- **中鍵拖曳**：平移
- **滾輪**：縮放
- **F 鍵**：聚焦選中物件

#### 數值輸入
- 在 Inspector 中直接輸入數值
- 支援數學運算式（如輸入 "10/2" 得到 5）
- 拖曳數值欄位微調

### 6.4 實作程式碼

```cpp
// csg_editor.hpp

class CSGEditor {
public:
    void DrawImGui() {
        DrawBrushInspector();
        DrawLevelHierarchy();
        DrawQuickActions();
        DrawCodePreview();
        DrawViewportOverlay();
    }

private:
    LevelBrush* _selectedBrush = nullptr;
    Level* _currentLevel = nullptr;

    void DrawBrushInspector() {
        ImGui::Begin("Brush Inspector");

        if (_selectedBrush == nullptr) {
            ImGui::Text("No brush selected");
            ImGui::End();
            return;
        }

        ImGui::Text("Selected: %s", _selectedBrush->name.c_str());
        ImGui::Separator();

        // Transform
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool changed = false;

            changed |= ImGui::DragFloat3("Position",
                &_selectedBrush->root.primitive.position.x, 0.1f);
            changed |= ImGui::DragFloat3("Size",
                &_selectedBrush->root.primitive.size.x, 0.1f, 0.1f);

            if (changed) {
                _selectedBrush->isDirty = true;
            }
        }

        // Code preview
        if (ImGui::CollapsingHeader("Code")) {
            std::string code = GenerateCode(*_selectedBrush);
            ImGui::TextWrapped("%s", code.c_str());

            if (ImGui::Button("Copy to Clipboard")) {
                ImGui::SetClipboardText(code.c_str());
            }
        }

        ImGui::End();
    }

    void DrawViewportOverlay() {
        if (_selectedBrush == nullptr) return;

        // 取得 brush 的世界座標
        auto& prim = _selectedBrush->root.primitive;
        glm::vec3 worldPos = prim.position;
        glm::vec3 size = prim.size;

        // 計算螢幕座標
        glm::vec2 screenPos = WorldToScreen(worldPos);

        // 繪製標籤
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        // 名稱標籤
        drawList->AddText(
            ImVec2(screenPos.x, screenPos.y - 20),
            IM_COL32(255, 255, 255, 255),
            _selectedBrush->name.c_str()
        );

        // 座標標籤
        std::string posLabel = fmt::format("({:.1f}, {:.1f}, {:.1f})",
            worldPos.x, worldPos.y, worldPos.z);
        drawList->AddText(
            ImVec2(screenPos.x, screenPos.y),
            IM_COL32(200, 200, 200, 255),
            posLabel.c_str()
        );

        // 尺寸標註線
        DrawDimensionLine(worldPos, size.x, Axis::X);
        DrawDimensionLine(worldPos, size.y, Axis::Y);
        DrawDimensionLine(worldPos, size.z, Axis::Z);
    }

    std::string GenerateCode(const LevelBrush& brush) {
        std::stringstream ss;

        auto& prim = brush.root.primitive;
        ss << "auto " << brush.name << " = CSG::Box(\n";
        ss << "    {" << prim.position.x << ", "
                      << prim.position.y << ", "
                      << prim.position.z << "},\n";
        ss << "    {" << prim.size.x << ", "
                      << prim.size.y << ", "
                      << prim.size.z << "}\n";
        ss << ");";

        return ss.str();
    }
};
```

---

## 第七章：完整工作流程示範

### 7.1 場景：製作一個小型地下城關卡

我們將製作一個包含以下元素的關卡：
- 入口大廳
- 兩條走廊
- 三個小房間
- 樓梯連接不同高度

### 7.2 步驟 1：規劃（紙筆/心智圖）

```
              ┌─────────────┐
              │   Room C    │
              │  (寶箱室)   │
              └──────┬──────┘
                     │ 走廊
         ┌───────────┴───────────┐
         │                       │
    ┌────┴────┐             ┌────┴────┐
    │  Room A │             │  Room B │
    │  (左室) │             │  (右室) │
    └────┬────┘             └────┬────┘
         │                       │
         └───────────┬───────────┘
                     │ 走廊
              ┌──────┴──────┐
              │   Lobby     │
              │  (入口大廳) │
              └──────┬──────┘
                     │
                  [入口]
```

### 7.3 步驟 2：建立專案結構

```
project/
├── src/
│   ├── levels/
│   │   ├── dungeon_01.hpp
│   │   └── dungeon_01.cpp
│   └── main.cpp
└── assets/
    └── levels/
        └── dungeon_01.json  (可選，用於儲存)
```

### 7.4 步驟 3：撰寫關卡程式碼

```cpp
// dungeon_01.hpp
#pragma once
#include "csg.hpp"
#include "mesh.hpp"

class Dungeon01 {
public:
    Mesh levelMesh;

    void Build();
    void Rebuild() { Build(); }  // 供 hot reload 使用

private:
    // 輔助函數
    CSGNode MakeRoom(const std::string& name, glm::vec3 pos, glm::vec3 size);
    CSGNode MakeDoor(glm::vec3 pos, glm::vec3 size);
    CSGNode MakeCorridor(glm::vec3 start, glm::vec3 end, float width, float height);
};
```

```cpp
// dungeon_01.cpp
#include "dungeon_01.hpp"

void Dungeon01::Build() {
    // ═══════════════════════════════════════════════════════
    // 常數定義
    // ═══════════════════════════════════════════════════════

    const float WALL_THICKNESS = 0.5f;
    const float ROOM_HEIGHT = 4.0f;
    const float DOOR_WIDTH = 2.0f;
    const float DOOR_HEIGHT = 3.0f;
    const float CORRIDOR_WIDTH = 3.0f;

    // ═══════════════════════════════════════════════════════
    // 入口大廳 (Lobby)
    // 位置：原點，作為關卡的起始點
    // ═══════════════════════════════════════════════════════

    auto lobby = CSG::Box(
        {0, ROOM_HEIGHT/2, 0},      // 位置（中心點）
        {12, ROOM_HEIGHT, 12}       // 尺寸
    );

    // 入口（南側）
    auto lobbyEntrance = CSG::Box(
        {0, DOOR_HEIGHT/2, -6},
        {DOOR_WIDTH, DOOR_HEIGHT, WALL_THICKNESS * 2}
    );
    lobby = CSG::Subtract(lobby, lobbyEntrance);

    // 往北的門（通往主走廊）
    auto lobbyNorthDoor = CSG::Box(
        {0, DOOR_HEIGHT/2, 6},
        {DOOR_WIDTH, DOOR_HEIGHT, WALL_THICKNESS * 2}
    );
    lobby = CSG::Subtract(lobby, lobbyNorthDoor);

    // ═══════════════════════════════════════════════════════
    // 主走廊 (Main Corridor)
    // 連接大廳到 T 字路口
    // ═══════════════════════════════════════════════════════

    auto mainCorridor = CSG::Box(
        {0, ROOM_HEIGHT/2, 15},     // 在大廳北邊
        {CORRIDOR_WIDTH, ROOM_HEIGHT, 12}
    );

    // ═══════════════════════════════════════════════════════
    // T 字路口 (Junction)
    // ═══════════════════════════════════════════════════════

    auto junction = CSG::Box(
        {0, ROOM_HEIGHT/2, 24},
        {15, ROOM_HEIGHT, 6}
    );

    // ═══════════════════════════════════════════════════════
    // Room A - 左側房間
    // ═══════════════════════════════════════════════════════

    auto roomA = CSG::Box(
        {-15, ROOM_HEIGHT/2, 24},
        {10, ROOM_HEIGHT, 10}
    );

    // Room A 入口
    auto roomADoor = CSG::Box(
        {-10, DOOR_HEIGHT/2, 24},
        {WALL_THICKNESS * 2, DOOR_HEIGHT, DOOR_WIDTH}
    );
    roomA = CSG::Subtract(roomA, roomADoor);

    // ═══════════════════════════════════════════════════════
    // Room B - 右側房間
    // ═══════════════════════════════════════════════════════

    auto roomB = CSG::Box(
        {15, ROOM_HEIGHT/2, 24},
        {10, ROOM_HEIGHT, 10}
    );

    // Room B 入口
    auto roomBDoor = CSG::Box(
        {10, DOOR_HEIGHT/2, 24},
        {WALL_THICKNESS * 2, DOOR_HEIGHT, DOOR_WIDTH}
    );
    roomB = CSG::Subtract(roomB, roomBDoor);

    // Room B 窗戶
    auto roomBWindow = CSG::Box(
        {20, ROOM_HEIGHT/2 + 0.5f, 24},
        {WALL_THICKNESS * 2, 1.5f, 2.0f}
    );
    roomB = CSG::Subtract(roomB, roomBWindow);

    // ═══════════════════════════════════════════════════════
    // 北側走廊（通往 Room C）
    // ═══════════════════════════════════════════════════════

    auto northCorridor = CSG::Box(
        {0, ROOM_HEIGHT/2, 33},
        {CORRIDOR_WIDTH, ROOM_HEIGHT, 12}
    );

    // ═══════════════════════════════════════════════════════
    // Room C - 寶箱室（最北端）
    // ═══════════════════════════════════════════════════════

    auto roomC = CSG::Box(
        {0, ROOM_HEIGHT/2, 45},
        {14, ROOM_HEIGHT, 10}
    );

    // Room C 入口
    auto roomCDoor = CSG::Box(
        {0, DOOR_HEIGHT/2, 40},
        {DOOR_WIDTH, DOOR_HEIGHT, WALL_THICKNESS * 2}
    );
    roomC = CSG::Subtract(roomC, roomCDoor);

    // ═══════════════════════════════════════════════════════
    // 地板
    // ═══════════════════════════════════════════════════════

    auto floor = CSG::Box(
        {0, -0.25f, 20},
        {50, 0.5f, 60}
    );

    // ═══════════════════════════════════════════════════════
    // 組合所有元素
    // ═══════════════════════════════════════════════════════

    auto level = floor;
    level = CSG::Union(level, lobby);
    level = CSG::Union(level, mainCorridor);
    level = CSG::Union(level, junction);
    level = CSG::Union(level, roomA);
    level = CSG::Union(level, roomB);
    level = CSG::Union(level, northCorridor);
    level = CSG::Union(level, roomC);

    // ═══════════════════════════════════════════════════════
    // 編譯成 Mesh
    // ═══════════════════════════════════════════════════════

    auto boxes = CSG::Compile(level);
    CSGToMesh(boxes, levelMesh);

    fmt::print("Level compiled: {} boxes, {} vertices\n",
               boxes.size(), levelMesh.vertCount);
}
```

### 7.5 步驟 4：在引擎中載入

```cpp
// main.cpp

#include "dungeon_01.hpp"

class Game : public Application {
    Dungeon01 dungeon;

    void OnInit() override {
        // 建構關卡
        dungeon.Build();

        // 設定 hot reload
        HotReloadManager::Get().WatchFile(
            "src/levels/dungeon_01.cpp",
            [this]() { dungeon.Rebuild(); }
        );
    }

    void OnRender() override {
        // 渲染關卡
        renderer.Draw(dungeon.levelMesh);
    }
};
```

### 7.6 步驟 5：使用 ImGui 工具微調

1. **執行遊戲**，關卡會根據程式碼建構

2. **打開 Brush Inspector**
   - 選取一個 brush（例如 roomA）
   - 查看其座標和尺寸

3. **微調數值**
   - 在 Inspector 中拖曳 Position 或 Size
   - 觀察 3D 視窗即時更新
   - 尺寸標註顯示新的數值

4. **複製調整後的程式碼**
   - 點擊 "Copy to Clipboard"
   - 貼回 dungeon_01.cpp
   - 儲存檔案

5. **Hot Reload**
   - 檔案儲存後自動重新載入
   - 無需重啟遊戲

### 7.7 步驟 6：迭代改進

```cpp
// 發現 Room B 太小，調整尺寸
auto roomB = CSG::Box(
    {15, ROOM_HEIGHT/2, 24},
    {14, ROOM_HEIGHT, 12}    // 改大一些
);

// 想在走廊加些裝飾凹槽
for (int i = 0; i < 3; i++) {
    auto niche = CSG::Box(
        {-1.5f, ROOM_HEIGHT/2, 10.0f + i * 3.0f},
        {1.0f, 2.0f, 1.0f}
    );
    mainCorridor = CSG::Subtract(mainCorridor, niche);
}

// 在 Room C 加個平台
auto platform = CSG::Box(
    {0, 0.5f, 47},
    {6, 1, 4}
);
level = CSG::Union(level, platform);
```

### 7.8 工作流程總結

```
┌─────────────────────────────────────────────────────────────┐
│                     開發循環                                 │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   1. 規劃關卡結構（紙筆/心智圖）                              │
│              │                                              │
│              ▼                                              │
│   2. 撰寫 CSG 程式碼                                         │
│      - 使用 CSG::Box, Subtract, Union                       │
│      - 定義常數提高可讀性                                    │
│              │                                              │
│              ▼                                              │
│   3. 執行遊戲，即時預覽                                      │
│              │                                              │
│              ▼                                              │
│   4. 使用 ImGui 工具                                        │
│      - 查看座標/尺寸                                        │
│      - 微調數值                                             │
│      - 複製調整後的程式碼                                    │
│              │                                              │
│              ▼                                              │
│   5. 更新程式碼，Hot Reload 自動重載                         │
│              │                                              │
│              ▼                                              │
│   6. 重複步驟 4-5 直到滿意                                   │
│              │                                              │
│              ▼                                              │
│   7. 完成 blockout，進入細節美術階段                         │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 第八章：實作優先順序

### 8.1 Phase 1：核心功能

| 項目 | 預估工作量 | 說明 |
|-----|-----------|------|
| CSGPrimitive 結構 | 1 小時 | 資料結構定義 |
| CSGNode 結構 | 1 小時 | 樹狀結構 |
| Box CSG 運算 | 2 小時 | SubtractBox 演算法 |
| CSGCompiler | 2 小時 | 編譯 CSG 樹 |
| CSGToMesh | 1 小時 | 輸出到 MeshBuilder |
| **小計** | **7 小時** | |

### 8.2 Phase 2：ImGui 工具

| 項目 | 預估工作量 | 說明 |
|-----|-----------|------|
| Brush Inspector | 2 小時 | 顯示/編輯 transform |
| Level Hierarchy | 1 小時 | 樹狀列表 |
| Viewport Overlay | 3 小時 | 尺寸標註、座標顯示 |
| Quick Actions | 1 小時 | 常用操作按鈕 |
| Code Preview | 1 小時 | 程式碼生成 |
| **小計** | **8 小時** | |

### 8.3 Phase 3：進階功能（可選）

| 項目 | 預估工作量 | 說明 |
|-----|-----------|------|
| Hot Reload | 3 小時 | 檔案監視 + 重載 |
| Cylinder 支援 | 4 小時 | 圓柱 CSG |
| Wedge 支援 | 4 小時 | 楔形 CSG |
| 關卡序列化 | 3 小時 | JSON 儲存/載入 |
| Undo/Redo | 4 小時 | 編輯歷史 |
| **小計** | **18 小時** | |

### 8.4 總時間預估

- **最小可用版本**（Phase 1）：約 1 個工作天
- **含基本工具**（Phase 1 + 2）：約 2 個工作天
- **完整功能**（全部）：約 4 個工作天

---

## 第九章：與現有系統的整合

### 9.1 與 MeshBuilder 的關係

```
CSG Layer                    現有系統
─────────────────────────────────────────────
CSGPrimitive
     │
     ▼
CSGCompiler::Compile()
     │
     ▼
std::vector<AABB>
     │
     ▼
┌─────────────────┐
│  MeshBuilder    │◄──── 現有的 PushCube, PushQuad
│  .PushCube()    │
│  .Build()       │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│     Mesh        │◄──── 現有的 Mesh 類別
│  .Update()      │
└─────────────────┘
```

### 9.2 程式碼整合

```cpp
// 在現有 MeshBuilder 中加入 CSG 支援

class MeshBuilder {
public:
    // 現有方法
    void PushCube(glm::vec3 pos, glm::vec3 size);
    void PushQuad(...);

    // 新增：從 CSG 編譯結果建構
    void PushCSG(const std::vector<AABB>& boxes) {
        for (const auto& box : boxes) {
            glm::vec3 center = (box.min + box.max) * 0.5f;
            glm::vec3 size = box.max - box.min;
            PushCube(center, size);
        }
    }

    const std::vector<Vertex>& Build();
};

// 使用範例
void BuildLevel(Mesh& mesh) {
    auto room = CSG::Box({0,0,0}, {10,5,10});
    auto door = CSG::Box({5,1,0}, {2,3,1});
    auto result = CSG::Subtract(room, door);

    auto boxes = CSG::Compile(result);

    MeshBuilder builder;
    builder.PushCSG(boxes);
    mesh.Update(builder.Build());
}
```

---

## 總結

本設計提供了一個輕量級的 CSG 系統，專注於：

1. **簡單實用** — Box-only CSG 涵蓋 90% 的 blockout 需求
2. **Code-Driven** — 程式碼為主，版本控制友善
3. **ImGui 輔助** — 視覺化輔助，非完整編輯器
4. **漸進擴充** — 可依需求加入更多形狀和功能
5. **與現有系統整合** — 輸出到 MeshBuilder → Mesh 管線

這個方案讓獨立開發者能用最少的工具投資，達到快速 level blockout 的目的。
