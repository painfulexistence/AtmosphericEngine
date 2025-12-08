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

將 `CSBExporter.cs` 複製到 Unity 專案的 `Assets/Editor/` 資料夾。

檔案位置：`AtmosphericEngine/tools/unity/CSBExporter.cs`

---

## 使用方式

1. 在 Unity 中選擇要匯出的根 GameObject
2. 選單：**Tools > CSB Exporter**
3. 設定匯出選項：
   - **Export Path**：輸出目錄（預設 `Assets/ExportedCSB`）
   - **Include Animations**：是否匯出動畫
   - **Include Inactive**：是否包含隱藏物件
   - **Animation Frame Rate**：動畫取樣率（預設 60fps）
4. 點擊 **Export Selected to CSB**

---

## 支援的元件

| Unity 元件 | CSB 類型 | 說明 |
|-----------|----------|------|
| SpriteRenderer | Sprite | 2D 精靈 |
| UI.Image | ImageView | UI 圖片 |
| UI.Text | Text | 文字（基本支援） |
| UI.Button | Button | 按鈕（基本支援） |
| 空 GameObject | Node | 純節點/容器 |

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

| Unity | CSB | 說明 |
|-------|-----|------|
| m_LocalPosition.x/y | PointFrame | 位移動畫 |
| m_LocalScale.x/y | ScaleFrame | 縮放動畫 |
| m_Color.r/g/b/a | ColorFrame | 顏色動畫 |

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
└── ZOrderTest (Node)
    ├── Layer0 (Sprite 50x50, 紅色, zOrder=0)
    ├── Layer1 (Sprite 50x50, 綠色, zOrder=1, 偏移 15,15)
    └── Layer2 (Sprite 50x50, 藍色, zOrder=2, 偏移 30,30)
    └── 用途: 驗證渲染順序（藍色應該在最上面）
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
| 9-slice (scale9) | ❌ | 尚未實作 |
| Plist 精靈圖 | ⚠️ | 會警告並當作普通 texture 載入 |
| BlendFunc | ⚠️ | 引擎使用 BlendMode enum，CSB 使用 GL 常數 |
| 骨骼動畫 | ❌ | 尚未實作 |
| 粒子系統 | ❌ | 尚未實作 |

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

---

## 檔案結構

```
AtmosphericEngine/
├── schemas/
│   └── CSParseBinary.fbs          # FlatBuffers schema
├── tools/unity/
│   └── CSBExporter.cs             # Unity Editor 腳本
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
