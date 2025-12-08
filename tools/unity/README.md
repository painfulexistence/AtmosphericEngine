# Unity CSB Exporter

將 Unity 場景匯出為 Cocos Studio Binary (.csb) 格式，可同時用於：
- AtmosphericEngine（本專案）
- Cocos2d-x 專案（原生支援）

---

## 安裝步驟

### 1. 安裝 FlatBuffers 套件

**方法 A：NuGet（推薦）**
```bash
# 在 Unity 專案根目錄
nuget install Google.FlatBuffers -OutputDirectory Packages
```

**方法 B：手動下載**
1. 從 [FlatBuffers Releases](https://github.com/google/flatbuffers/releases) 下載
2. 將 `net/FlatBuffers` 資料夾複製到 `Assets/Plugins/FlatBuffers`

### 2. 產生 C# Binding

```bash
# 下載 flatc 編譯器（如果還沒有）
# Windows: https://github.com/google/flatbuffers/releases 下載 flatc.exe
# macOS: brew install flatbuffers
# Linux: apt install flatbuffers-compiler

# 產生 C# 類別
flatc --csharp CSParseBinary.fbs -o Assets/Scripts/Generated/
```

Schema 檔案位置：`AtmosphericEngine/schemas/CSParseBinary.fbs`

### 3. 安裝 Exporter

將以下檔案複製到 Unity 專案：
- `CSBExporter.cs` → `Assets/Editor/`
- `CSBNodeData.cs` → `Assets/Scripts/`

檔案位置：`AtmosphericEngine/tools/unity/`

---

## 使用方式

1. 在 Unity 中選擇要匯出的根 GameObject
2. 選單：**Tools > CSB Exporter**
3. 設定匯出選項：
   - **Export Path**：輸出目錄（預設 `Assets/ExportedCSB`）
   - **Include Animations**：是否匯出動畫
   - **Include Inactive**：是否包含隱藏物件
   - **Animation Frame Rate**：動畫取樣率（預設 60fps）
   - **Copy Textures**：是否複製貼圖到輸出目錄
4. 點擊 **Export Selected to CSB**

---

## 支援的元件

### 自動偵測

| Unity 元件組合 | CSB 類型 | 說明 |
|---------------|----------|------|
| SpriteRenderer | Sprite | 2D 精靈 |
| UI.Image | ImageView | UI 圖片（支援 9-slice） |
| UI.RawImage | ImageView | Raw 圖片 |
| UI.Text | Text | 文字 |
| UI.Button | Button | 按鈕 |
| UI.Toggle | CheckBox | 核取方塊 |
| UI.InputField | TextField | 輸入框 |
| UI.Slider | Slider | 滑桿 |
| UI.ScrollRect | ScrollView | 捲動視圖 |
| ScrollRect + VerticalLayoutGroup | ListView | 垂直列表 |
| ScrollRect + GridLayoutGroup | ListView | 網格列表 |
| ScrollRect + HorizontalLayoutGroup | PageView | 分頁視圖 |
| RectTransform (無其他元件) | Panel | 面板/容器 |
| 空 GameObject | Node | 純節點 |

### CSBNodeData 強制類型

當自動偵測不夠用時，添加 `CSBNodeData` 元件並設定 `forceNodeType`：

| CSBNodeType | 用途 |
|-------------|------|
| ProjectNode | 嵌套 CSB 檔案引用 |
| LoadingBar | 進度條 |
| ListView | 強制為列表視圖 |
| PageView | 強制為分頁視圖 |

---

## CSBNodeData 元件

`CSBNodeData` 是輔助元件，用於設定 CSB 專屬資料：

### User Data（自訂屬性）

```csharp
// 對應 Cocos Studio 的 "User Data" 功能
// 匯出到 WidgetOptions.customProperty
public string customProperty = "";
```

**使用範例：**
```
customProperty = "type=enemy;hp=100;dropRate=0.5"
```

Cocos2d-x 端可透過 `node->getCustomProperty()` 讀取。

### ProjectNode（嵌套 CSB）

```csharp
// 引用另一個 .csb 檔案作為子節點
public string projectNodePath = "";      // 完整路徑如 "ui/common/Button.csb"
public string projectNodeFileName = "";  // 或只填檔名 "Button.csb"
```

**使用範例：**
1. 建立空 GameObject
2. 添加 `CSBNodeData` 元件
3. 設定 `projectNodePath = "ui/ButtonPrefab.csb"`
4. 匯出 → 該節點會變成 ProjectNode

### 其他設定

| 欄位 | 說明 |
|------|------|
| `forceNodeType` | 強制節點類型（覆蓋自動偵測） |
| `actionTag` | 動畫標籤（-1 = 自動產生） |
| `touchEnabled` | 是否啟用觸控 |
| `callbackName` | 回調函數名稱 |
| `callbackType` | 回調類型（0=None, 1=Touch, 2=Click） |

---

## 屬性映射

### Transform

| Unity | CSB | 說明 |
|-------|-----|------|
| localPosition | position | 相對位置 |
| localScale | scale | 縮放 |
| localEulerAngles.z | rotationSkew.rotationSkewX | Z 軸旋轉 |
| RectTransform.pivot | anchorPoint | 錨點/軸心 |
| RectTransform.sizeDelta | size | 尺寸 |

### 視覺屬性

| Unity | CSB | 說明 |
|-------|-----|------|
| SpriteRenderer.color | color + alpha | 顏色與透明度 |
| SpriteRenderer.flipX | flipX | X 軸翻轉 |
| SpriteRenderer.flipY | flipY | Y 軸翻轉 |
| SiblingIndex | zOrder | 渲染順序 |
| activeSelf | visible | 可見性 |

### 動畫

| Unity 屬性 | CSB Frame 類型 | 說明 |
|-----------|----------------|------|
| m_LocalPosition.x/y | PointFrame | 位移動畫 |
| m_LocalScale.x/y | ScaleFrame | 縮放動畫 |
| localEulerAngles.z | IntFrame (Rotation) | 旋轉動畫 |
| m_Color.r/g/b/a | ColorFrame | 顏色動畫 |
| m_Alpha | IntFrame (Alpha) | 透明度動畫 |
| m_IsActive | BoolFrame (Visible) | 顯示/隱藏動畫 |
| m_Sprite | TextureFrame | 貼圖切換動畫 |

### 動畫緩動 (Easing)

匯出器會分析 Unity AnimationCurve 的 tangent 來推測緩動類型：
- 支援 30 種 Cocos 緩動類型（Linear, EaseIn/Out/InOut 各種曲線）
- 自定義貝茲曲線控制點 **尚未支援**

### 尚未支援的動畫類型

| Frame 類型 | 說明 |
|-----------|------|
| EventFrame | Animation Events → 字串事件 |
| InnerActionFrame | ProjectNode 內部動畫控制 |
| BlendFrame | 混合模式動畫 |

---

## 驗證用測試場景

建立以下場景結構來驗證匯出是否正確：

```
TestScene (Node)
│
├── Origin (Sprite)
│   └── 位置: (0, 0)
│   └── 大小: 20x20
│   └── 顏色: 紅色
│   └── 用途: 驗證座標原點
│
├── PivotTests (Node) @ (200, 300)
│   ├── PivotBL (Sprite 80x60, pivot=0,0, 橘色)
│   ├── PivotCenter (Sprite 80x60, pivot=0.5,0.5, 綠色)
│   └── PivotTR (Sprite 80x60, pivot=1,1, 藍色)
│   └── 用途: 三個 sprite 應該以不同方式對齊 (200,300)
│
├── ScaleTest (Sprite)
│   └── 位置: (400, 300)
│   └── 基礎大小: 50x50
│   └── 縮放: (2, 1.5)
│   └── 顏色: 黃色
│   └── 用途: 最終顯示應為 100x75
│
├── RotationTest (Sprite)
│   └── 位置: (550, 300)
│   └── 大小: 60x40
│   └── 旋轉: 45°
│   └── 顏色: 紫色
│   └── 用途: 驗證旋轉正確
│
├── FlipTests (Node)
│   ├── Normal (Sprite, 用有方向性的圖片如箭頭)
│   ├── FlippedX (同上, flipX=true)
│   └── FlippedY (同上, flipY=true)
│   └── 用途: 驗證翻轉功能（需要不對稱的 texture）
│
├── AspectRatioTest (Sprite)
│   └── 位置: (400, 500)
│   └── 大小: 160x90 (16:9)
│   └── 顏色: 灰色
│   └── 用途: 驗證長寬比保持
│
├── ZOrderTest (Node)
│   ├── Layer0 (Sprite 50x50, 紅色, zOrder=0)
│   ├── Layer1 (Sprite 50x50, 綠色, zOrder=1, 偏移 15,15)
│   └── Layer2 (Sprite 50x50, 藍色, zOrder=2, 偏移 30,30)
│   └── 用途: 驗證渲染順序（藍色應該在最上面）
│
├── ListViewTest (ScrollRect + VerticalLayoutGroup)
│   └── 用途: 驗證 ListView 匯出
│
└── ProjectNodeTest (GameObject + CSBNodeData)
    └── projectNodePath: "prefabs/Item.csb"
    └── 用途: 驗證嵌套 CSB 引用
```

### 驗證步驟

1. 在 Unity 中建立上述場景
2. 匯出為 CSB
3. 在 AtmosphericEngine 的 Example_CSB 中載入：
   - 將 CSB 檔案放到 `Example_CSB/assets/test_scene.csb`
   - 執行程式
4. 按 **1** 開啟座標系網格
5. 按 **2** 開啟節點資訊面板
6. 對照檢查每個 sprite 的位置、大小、旋轉是否正確

---

## 已知限制

| 功能 | 狀態 | 說明 |
|------|------|------|
| rotationSkewY | ⚠️ | 引擎只使用 rotationSkewX，Y 軸 skew 會被忽略 |
| Plist 精靈圖 | ⚠️ | 會警告並當作普通 texture 載入 |
| BlendFunc | ⚠️ | 引擎使用 BlendMode enum，CSB 使用 GL 常數 |
| 自定義緩動曲線 | ⚠️ | 只匯出緩動類型，不匯出貝茲控制點 |
| EventFrame | ❌ | Animation Events 尚未支援 |
| InnerActionFrame | ❌ | ProjectNode 內部動畫控制尚未支援 |
| 骨骼動畫 | ❌ | 尚未實作 |
| 粒子系統 | ❌ | Unity 粒子與 Cocos 不相容 |
| GameMap | ❌ | Tiled 地圖不從 Unity 匯出 |

---

## 故障排除

### CSB 載入失敗
- 確認 FlatBuffers 版本相容
- 檢查檔案路徑是否正確
- 查看 console 的錯誤訊息

### Texture 沒有顯示
- 確認 texture 檔案已複製到 `assets/` 目錄
- 確認路徑大小寫正確（Linux/macOS 區分大小寫）

### 位置不對
- 檢查 Unity 的座標系統（Y 軸方向）
- 確認 pivot/anchor 設定
- 使用 Example_CSB 的座標網格對照

### 動畫沒有播放
- 確認 AtmosphericEngine 已實作 ActionManager（目前尚未完成）
- CSB 動畫資料有正確匯出，但引擎端需要動畫系統來播放

### ListView/PageView 沒有正確識別
- 確認 ScrollRect 元件存在
- ListView 需要 VerticalLayoutGroup 或 GridLayoutGroup
- PageView 需要 HorizontalLayoutGroup 且 ScrollRect 設為 horizontal only

### ProjectNode 沒有匯出
- 確認已添加 CSBNodeData 元件
- 確認 projectNodePath 或 projectNodeFileName 已填寫

---

## 檔案結構

```
AtmosphericEngine/
├── schemas/
│   └── CSParseBinary.fbs          # FlatBuffers schema
├── tools/unity/
│   ├── CSBExporter.cs             # Unity Editor 腳本
│   ├── CSBNodeData.cs             # 輔助 MonoBehaviour
│   └── README.md                  # 本文件
├── AtmosphericEngine/
│   ├── include/Atmospheric/
│   │   └── csb_loader.hpp         # CSBLoader 介面
│   └── src/
│       └── csb_loader.cpp         # CSBLoader 實作
└── Example_CSB/
    ├── main.cpp                   # 測試範例（含除錯功能）
    └── assets/
        └── test_scene.csb         # 放置測試用 CSB
```
