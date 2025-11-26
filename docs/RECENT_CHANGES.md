# 近期改動摘要

本文檔總結了專案中針對 Job System 和 Bullet Physics 整合所進行的關鍵改動，並明確指出被還原或註解的部分。

## 1. 工作系統 (JobSystem) 優化與重構

### 目標
解決原有 `JobSystem` 中單一中央任務隊列所導致的嚴重鎖競爭問題，提升並行效能，並改進 `Wait()` 機制的效率。

### 主要改動
*   **重構為工作竊取 (Work-Stealing) 模型**：
    *   移除了原有的單一全局任務隊列及其互斥鎖。
    *   為每個工作線程引入了獨立的任務隊列 (`std::deque<Job>`) 和對應的互斥鎖 (`std::unique_ptr<std::mutex>`)。
    *   工作線程現在優先從自己的本地隊列獲取任務（無需競爭），當本地隊列為空時，才嘗試從其他線程的隊列**尾部**「竊取」任務（此時會進行鎖定）。
    *   任務提交（`Execute` 函式）現在以輪詢方式將任務分發到各線程隊列，並使用目標隊列的互斥鎖保護。
*   **`Wait()` 機制優化**：改進了 `Wait()` 函式和任務完成的通知機制，避免了傳統中央隊列模型中常見的「驚群效應」(Thundering Herd Problem)，只在所有任務真正完成時才發出通知，提高了效率。
*   **線程安全改進**：將 `currentLabel` (追蹤總任務數) 改為 `std::atomic<uint64_t>` 以確保其線程安全的增量操作。
*   **簡化設計**：移除了不再使用的 `JobGroup` 相關的代碼以及 `IJob` 及其衍生類別 (`SampleJob`, `TextureLoadJob`)，轉而直接使用 `std::function` 定義的 `Job` lambda 函數，使代碼更現代、簡潔。

## 2. Bullet Physics 整合 `JobSystem`

### 目標
讓 Bullet Physics 函式庫能夠利用引擎的 `JobSystem` 進行並行計算，實現統一的任務管理。

### 主要改動
*   **`BulletTaskScheduler` 實現**：
    *   創建了一個新的類別 `BulletTaskScheduler`，它繼承自 Bullet 的抽象介面 `btITaskScheduler`。
    *   這個排程器作為一個包裝器，將 Bullet 內部發出的並行任務請求 (特別是 `parallelFor`) 委派給引擎的 `JobSystem` 來執行。
*   **`PhysicsServer` 整合**：
    *   修改了 `PhysicsServer` 的初始化過程，使其創建並註冊 `BulletTaskScheduler` 實例到 Bullet Physics 引擎中。
    *   **已還原部分**：由於專案所使用的 Bullet 版本中未包含 `btMultiThreadedConstraintSolver.h` 頭檔，原計畫將 `btSequentialImpulseConstraintSolver` 替換為 `btMultiThreadedConstraintSolver` 的改動已**還原**。`PhysicsServer` 仍使用循序解算器，但 `JobSystem` 仍會被 Bullet 內部其他可並行的子系統使用。
    *   **已註解部分**：`BulletTaskScheduler::parallelSum` 函式體因 Bullet 介面不符而導致編譯錯誤，其內部實作已被**註解掉**，並暫時返回 `0`。

## 3. 資源管理器 (AssetManager)

### 結論
經分析確認，`AssetManager` 的 `LoadTextures` 方法已有效利用 `JobSystem` 進行多線程圖片載入，無需進一步重構。它已良好地將線程安全的文件 I/O 和解碼操作與必須在主線程執行的 GPU 上傳操作分開。
