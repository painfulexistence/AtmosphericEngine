# Mesh System 架構設計

## 概述

本文件描述 Atmospheric Engine 的 Mesh 系統重構設計，目標是建立一個能良好支援 **procedural geometry** 與多種幾何類型（voxel chunk、spline mesh 等）的現代化架構。

## 設計動機

### 現有架構的問題

現有的 `Mesh` class 混合了多種職責：

```cpp
class Mesh {
    // GPU 資源管理
    GLuint vao, vbo, ebo, ibo;

    // 材質系統
    Material* material;

    // 物理系統
    btCollisionShape* shape;

    // 幾何資料
    std::array<glm::vec3, 8> bounds;
    MeshType type;
};
```

這種設計在處理 procedural geometry 時會遇到以下問題：

1. **無法有效 cache 靜態幾何** — Voxel chunk 大多時候是靜態的，不需要每幀重建
2. **資源共享困難** — 多個物件無法共用同一份 GPU 資源
3. **動態更新不直觀** — 更新 mesh 資料需要重建整個物件
4. **擴展性受限** — 新增幾何類型（如 spline mesh）需要修改核心 class

### 設計目標

1. **分離關注點** — GPU 資源管理與邏輯 mesh 概念分離
2. **支援動態更新** — Procedural geometry 可高效更新而不重建物件
3. **資源共享** — 多個 mesh instance 可共用同一份 GPU 資源
4. **格式彈性** — 統一管理不同頂點格式（standard, voxel, spline 等）
5. **封裝完整** — Game layer 不需要知道底層資源管理細節

## 架構設計

### 層級總覽

```
┌─────────────────────────────────────────────────────────────┐
│                      Game Layer                              │
│  VoxelChunk, SplineMesh, TerrainPatch, ...                  │
│  - 使用 MeshBuilder 建構幾何                                 │
│  - 呼叫 Mesh::Update() 更新資料                             │
└─────────────────────────────────────────────────────────────┘
                          │
                          │ mesh.Update(data)
                          ▼
┌─────────────────────────────────────────────────────────────┐
│                        Mesh                                  │
│  (Logical mesh - 邏輯層 API)                                 │
│  - 對外提供簡潔 API                                          │
│  - 封裝 GPU 資源管理細節                                     │
│  - 持有 RenderMeshHandle                                     │
└─────────────────────────────────────────────────────────────┘
                          │
                          │ pool.Update(handle, data)
                          ▼
┌─────────────────────────────────────────────────────────────┐
│                    RenderMeshPool                            │
│  (Resource manager - 引擎內部)                               │
│  - 統一管理所有 RenderMesh 生命週期                          │
│  - 處理 GPU 資源分配與回收                                   │
│  - 可實作 pooling, defragmentation                          │
└─────────────────────────────────────────────────────────────┘
                          │
                          │ renderMesh->Upload(data)
                          ▼
┌─────────────────────────────────────────────────────────────┐
│                      RenderMesh                              │
│  (GPU resource wrapper - 最底層)                             │
│  - 直接管理 VAO, VBO, EBO                                    │
│  - 處理頂點格式設定                                          │
│  - 執行實際的 GPU upload 與 draw call                        │
└─────────────────────────────────────────────────────────────┘
```

### 為什麼需要這樣分層？

#### Mesh vs RenderMesh 分離

傳統設計中，一個 `Mesh` 物件直接擁有 GPU 資源。這在以下情境會造成問題：

**情境一：Instancing**
```
// 傳統設計：100 棵樹 = 100 份 GPU 資源（浪費）
for (int i = 0; i < 100; i++) {
    trees[i] = new Mesh();  // 每個都有自己的 VAO/VBO
}

// 新設計：100 棵樹共用 1 份 GPU 資源
RenderMeshHandle treeGeometry = pool.Allocate(...);
for (int i = 0; i < 100; i++) {
    trees[i].renderMesh = treeGeometry;  // 共用 handle
}
```

**情境二：Procedural Geometry 更新**
```
// 傳統設計：更新 = 重建整個 Mesh 物件
delete chunk.mesh;
chunk.mesh = new Mesh(newData);

// 新設計：原地更新，物件不變
chunk.mesh.Update(newData);  // 內部只更新 GPU buffer
```

#### RenderMeshPool 的必要性

將 `RenderMesh` 的生命週期交給 `RenderMeshPool` 統一管理，而非讓 `Mesh` 直接持有，有以下好處：

1. **資源追蹤** — 引擎可追蹤所有 GPU mesh 資源使用量
2. **Pool 分配** — 可預先分配 buffer pool，減少 allocation 次數
3. **Memory budgeting** — 可設定 mesh 記憶體上限，實作 LOD 卸載策略
4. **統一銷毀** — 場景切換時可批次釋放所有資源

#### Game Layer 的封裝

Game layer（如 `VoxelChunk`）只需要：
```cpp
mesh.Update(builder.Build());
```

完全不需要知道：
- RenderMeshPool 的存在
- Handle 機制如何運作
- GPU 資源何時上傳

這種封裝讓 game code 保持簡潔，也讓引擎內部實作可以自由演進。

## 支援 Procedural Geometry

### 核心優勢

新架構對 procedural geometry 特別友善：

| 特性 | 說明 |
|-----|------|
| **動態更新** | `Mesh::Update()` 可在任意時刻更新幾何資料 |
| **格式彈性** | 不同 procedural 類型可用最適合的頂點格式 |
| **Cache 友善** | 只有 dirty 的 mesh 需要重新上傳 |
| **多線程友善** | MeshBuilder 可在 worker thread 執行，與 GPU upload 分離 |

### 頂點格式支援

不同 procedural geometry 類型有不同的頂點需求：

```cpp
// 標準格式 - 一般 3D 模型
struct StandardVertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};  // 56 bytes

// Voxel 壓縮格式 - 最小化記憶體與頻寬
struct VoxelVertex {
    uint8_t x, y, z;    // 局部座標 (0-255)
    uint8_t voxel_id;   // 方塊類型
    uint8_t face_id;    // 面朝向 (0-5)
};  // 5 bytes

// Spline 格式 - 路徑相關資料
struct SplineVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    float t;            // 曲線參數 [0, 1]
};  // 36 bytes
```

新架構允許 `RenderMesh` 使用任意頂點格式，只需在建立時指定 `VertexFormat`。

### 使用範例

#### Voxel Chunk Mesh

```cpp
class VoxelChunk {
    Mesh mesh;
    bool dirty = true;

    void Rebuild() {
        MeshBuilder builder;

        for (auto& voxel : voxels) {
            for (FaceDir face : GetVisibleFaces(voxel)) {
                builder.PushVoxelFace(voxel.pos, face, voxel.id);
            }
        }

        mesh.Update(builder.Build());
        dirty = false;
    }

    void SetVoxel(glm::ivec3 pos, uint8_t id) {
        voxels[pos] = id;
        dirty = true;  // 標記需要重建
    }
};
```

#### Spline Mesh

```cpp
class SplineMeshComponent {
    Mesh mesh;
    std::vector<SplinePoint> controlPoints;
    int resolution = 32;

    void Rebuild() {
        MeshBuilder builder;

        for (int i = 0; i <= resolution; i++) {
            float t = float(i) / resolution;
            auto [pos, tangent] = EvaluateSpline(controlPoints, t);
            builder.PushSplineSegment(pos, tangent, t);
        }

        mesh.Update(builder.Build());
    }

    void UpdateControlPoint(int index, SplinePoint point) {
        controlPoints[index] = point;
        Rebuild();  // Spline 任一點變動都需要重建
    }
};
```

#### 動態地形 Patch

```cpp
class TerrainPatch {
    Mesh mesh;
    int lod = 0;

    void SetLOD(int newLod) {
        if (newLod == lod) return;
        lod = newLod;

        MeshBuilder builder;
        int resolution = GetResolutionForLOD(lod);

        for (int z = 0; z < resolution; z++) {
            for (int x = 0; x < resolution; x++) {
                float height = SampleHeightmap(x, z, lod);
                builder.PushTerrainVertex(x, height, z);
            }
        }

        mesh.Update(builder.Build());
    }
};
```

## 擴展性

### 新增幾何類型的步驟

新增一種 procedural geometry 類型（例如 cloth mesh）只需要：

1. **定義頂點格式**（如果現有格式不適用）
```cpp
struct ClothVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};
```

2. **擴展 MeshBuilder**（可選）
```cpp
class MeshBuilder {
    // 新增專用 push 方法
    void PushClothQuad(glm::vec3 corners[4], glm::vec2 uvs[4]);
};
```

3. **建立 Game layer class**
```cpp
class ClothSimulation {
    Mesh mesh;
    std::vector<ClothParticle> particles;

    void Simulate(float dt) { /* 物理模擬 */ }
    void UpdateMesh() { mesh.Update(BuildFromParticles()); }
};
```

**不需要修改：**
- `RenderMesh` class
- `RenderMeshPool` class
- `Mesh` class
- 任何核心引擎程式碼

### 對比傳統設計

| 傳統設計 | 新架構 |
|---------|-------|
| 新幾何類型需繼承 Mesh | 新類型只需組合 Mesh |
| 頂點格式寫死在 Mesh 中 | 頂點格式參數化 |
| 特殊類型需修改核心程式碼 | 純擴展，不修改核心 |
| Voxel/Spline 需要各自的 Mesh 子類 | 統一使用 Mesh + MeshBuilder |

## API 簡述

### Mesh

```cpp
class Mesh {
public:
    Mesh(const VertexFormat& format, UpdateFrequency freq = UpdateFrequency::Static);

    void Update(const MeshData& data);
    void Draw();

    Material* GetMaterial() const;
    void SetMaterial(Material* material);
    AABB GetBounds() const;
};
```

### MeshBuilder

```cpp
class MeshBuilder {
public:
    // 通用
    void PushQuad(glm::vec3 pos, glm::vec2 size, glm::vec3 normal);
    void PushCube(glm::vec3 pos, glm::vec3 size);

    // Voxel 專用
    void PushVoxelFace(glm::ivec3 pos, FaceDir dir, uint8_t voxelId);

    // Spline 專用
    void PushSplineSegment(glm::vec3 pos, glm::vec3 tangent, float t);

    MeshData Build();
    void Clear();
};
```

### RenderMesh（引擎內部）

```cpp
class RenderMesh {
public:
    void Initialize(const VertexFormat& format, size_t maxVertices);
    void Upload(const void* data, size_t size);
    void Draw(GLenum primitiveType = GL_TRIANGLES);
};
```

### RenderMeshPool（引擎內部）

```cpp
class RenderMeshPool {
public:
    RenderMeshHandle Allocate(const VertexFormat& format, UpdateFrequency freq);
    void Update(RenderMeshHandle handle, const void* data, size_t size);
    void Free(RenderMeshHandle handle);
    RenderMesh* Get(RenderMeshHandle handle);
};
```

## 總結

本架構透過分離 logical mesh 與 GPU 資源管理，達成以下目標：

- **Procedural geometry 友善** — 動態更新高效且直觀
- **擴展性佳** — 新增幾何類型無需修改核心程式碼
- **資源共享** — 支援 instancing 等最佳化技術
- **封裝完整** — Game layer 使用簡潔的 API
- **Future-proof** — 架構可適應 Vulkan/DX12 等現代 API

這個設計讓引擎能同時良好支援傳統靜態 mesh、voxel chunk、spline mesh、動態地形等多種幾何類型，為 procedural content 的開發提供堅實的基礎。
