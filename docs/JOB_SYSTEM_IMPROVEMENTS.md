# 工作系統改進報告 (Job System Improvements)

本文檔旨在記錄近期對引擎工作系統（Job System）的改進，並為未來的強化方向提供建議。

## 變更摘要

本次重構的主要目標是將 Bullet Physics 物理函式庫的多線程能力與我們引擎現有的 `JobSystem` 進行整合。此舉可以避免在應用程式中同時存在兩個獨立的線程池（一個屬於引擎，一個屬於 Bullet），從而實現更集中的任務管理與控制。

為了達成此目標，我們進行了以下變更：

1.  **`BulletTaskScheduler` 的實現**:
    *   我們建立了一個名為 `BulletTaskScheduler` 的新類別（位於 `AtmosphericEngine/src/bullet_task_scheduler.hpp` 和 `.cpp`）。
    *   該類別繼承自 Bullet 的抽象介面 `btITaskScheduler`。
    *   它作為一個包裝器（Wrapper），將 Bullet 的並行任務請求（特別是 `parallelFor`）導向至我們引擎自己的 `JobSystem`。
    *   `parallelFor` 的實作會將一個大的循環（loop）拆分成多個小區塊，並將它們作為獨立的 job 提交，然後等待所有 job 完成後才返回。

2.  **`PhysicsServer` 的整合**:
    *   我們更新了 `PhysicsServer`，讓它使用新的 `BulletTaskScheduler`。
    *   在 `PhysicsServer::Init` 初始化函式中，伺服器現在會創建我們自訂排程器（scheduler）的實例，並通過 `btSetTaskScheduler` 將其註冊到 Bullet 中。
    *   原本的單線程約束解算器 `btSequentialImpulseConstraintSolver` 被替換為 `btMultiThreadedConstraintSolver`，後者是專為配合任務排程器而設計的。
    *   這一改動使得 Bullet 可以在多個線程上運行其物理約束解算階段，而這些線程由我們的 `JobSystem` 統一管理。

3.  **`AssetManager` 的分析**:
    *   我們對 `AssetManager` 進行了分析，並確認它已經在 `LoadTextures` 函式中有效地利用了 `JobSystem` 來進行並行的圖片資源載入。該實現正確地將「檔案 I/O 與解碼」（在並行 job 中完成）和「GPU 上傳」（在主線程中完成）這兩個階段分開。因此，此部分無需任何變更。

## 未來改進建議

當前的 `JobSystem` 對於分派和等待一組獨立任務非常有效。然而，它的核心設計存在性能瓶頸，可以透過更現代的平行運算模式來改進。

### 1. 減少鎖競爭：從中央隊列到工作竊取模型 (Reduce Lock Contention: From Central Queue to Work-Stealing)

*   **問題**: 目前的實作使用一個受單一互斥鎖 (mutex) 保護的中央任務隊列。所有工作線程和任務提交者都會爭奪這個鎖，當線程數和任務數增加時，這會成為一個嚴重的性能瓶頸。此外，`Wait` 機制的通知方式會引發「驚群效應」，效率低下。
*   **解決方案**: 轉向**工作竊取 (Work-Stealing)** 架構。
    *   **概念**: 為每個工作線程分配一個本地的雙端隊列 (deque)。線程主要從自己的隊列獲取任務（無鎖操作）。當本地隊列為空時，它會嘗試從其他線程的隊列中「竊取」一個任務（此時才需要鎖定目標隊列）。
    *   **優點**: 極大地減少了鎖的競爭，因為大多數時候線程都在無鎖的情況下訪問自己的隊列。這是現代高性能任務系統（如 Intel TBB、Windows Concurrency Runtime）的標準實踐。
    *   **實現**: 將單一的 `std::queue` 和 `std::mutex` 替換為 `std::vector<std::deque>` 和對應的鎖。重寫工作線程的循環邏輯以實現工作竊取，並優化 `Wait` 機制以避免驚群效應。

### 2. 任務依賴性與任務圖 (Task Dependencies and Graphing)

*   **概念**: 允許 job 宣告對其他 job 的依賴關係。一個 job 只有在它所依賴的所有 job 都完成後，才會被排程執行。
*   **優點**: 這將使更複雜的並行演算法成為可能，而無需使用粗糙的 `Wait()` 呼叫來同步。例如，在一個渲染管線中，一個剔除（culling）job 可以先運行，其後跟隨多個獨立的陰影貼圖（shadow-map）生成 job，最後一個光照 job 則依賴於前面所有任務的結果。
*   **實現**: 這通常需要創建一個任務圖（task graph），其中 job 是節點，依賴關係是邊。`JobSystem` 將負責遍歷此圖並排程已就緒的 job。

### 3. 基於優先級的排程 (Priority-Based Scheduling)

*   **概念**: 為 job 分配優先級（例如 `高`、`中`、`低`）。
*   **優點**: 這可以確保關鍵的、對時間敏感的任務（如物理模擬、玩家輸入處理）在後台任務（如遠處資源的串流載入、非關鍵的 AI 計算）之前執行。
*   **實現**: 這需要在 job 中增加一個優先級欄位，並在 `JobSystem` 內部使用優先級隊列（priority queue）來管理 job 佇列。

### 4. 基於纖程的任務 (Fiber-Based Tasks / Micro-Coroutines)

*   **概念**: 從使用「線程」執行 job 切換到使用「纖程」（Fiber）。纖程是輕量級的、由用戶空間管理的線程。單個工作線程可以執行許多個纖程。
*   **優點**: 對於需要等待其他任務的場景，這種模型非常高效。纖程不必讓整個線程休眠（這會浪費資源），而是可以簡單地讓出（yield）執行權，工作線程可以立即去運行另一個已就緒的纖程。這大大降低了任務切換的開銷，也是許多現代高性能 job system 的基礎（可參考 Naughty Dog 在 GDC 上的演講）。
*   **實現**: 這是一個重大的架構變更。它需要引入一個纖程函式庫，以及一個能夠在固定的工作線程池上管理和切換纖程的排程器。

### 5. 標籤化與線程親和性 (Tagging and Thread Affinity)

*   **概念**: 允許 job 被「標記」到特定的工作線程上。例如，一個 job 可以被標記為只能在「渲染線程」上運行。
*   **優點**: 這對於整合那些具有線程局部狀態（thread-local state）的第三方函式庫非常有用，或者可以將特定核心專門用於特定子系統，以提高快取性能。
*   **實現**: 這將需要為帶有標籤的線程創建專門的 job 佇列，以及一個能將 job 分派到正確佇列的機制。
