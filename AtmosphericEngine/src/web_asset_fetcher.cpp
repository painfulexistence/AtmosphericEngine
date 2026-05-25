#ifdef __EMSCRIPTEN__
#include "web_asset_fetcher.hpp"
#include "asset_manager.hpp"
#include "console.hpp"

#include <emscripten/fetch.h>
#include <atomic>
#include <memory>

// ─────────────────────────────────────────────────────────────────────────────
// Internal helpers
// ─────────────────────────────────────────────────────────────────────────────
namespace {

// Shared state for a batch Preload() call.
struct BatchState {
    std::atomic<int>     remaining{0};
    std::function<void()> onComplete;
};

// Per-fetch context allocated on the heap; freed in the callbacks.
struct FetchCtx {
    std::function<void(const uint8_t*, size_t)> onSuccess;
    std::function<void(const std::string&)>     onError;
    std::shared_ptr<BatchState>                  batch; // null for one-shot fetches
};

void OnSuccess(emscripten_fetch_t* f) {
    auto* ctx = static_cast<FetchCtx*>(f->userData);

    if (ctx->onSuccess)
        ctx->onSuccess(reinterpret_cast<const uint8_t*>(f->data),
                       static_cast<size_t>(f->numBytes));

    // Notify the batch counter after onSuccess so the completion callback fires
    // only after all user callbacks have finished.
    if (ctx->batch && --ctx->batch->remaining == 0)
        if (ctx->batch->onComplete)
            ctx->batch->onComplete();

    // Free the fetch buffer and our context.
    emscripten_fetch_close(f);
    delete ctx;
}

void OnError(emscripten_fetch_t* f) {
    auto* ctx = static_cast<FetchCtx*>(f->userData);

    ENGINE_LOG("[WebAssetFetcher] HTTP {} — failed to fetch '{}'", f->status, f->url);

    if (ctx->onError)
        ctx->onError(std::string(f->url));

    // Errors are non-fatal for batch loads so the game can still start.
    if (ctx->batch && --ctx->batch->remaining == 0)
        if (ctx->batch->onComplete)
            ctx->batch->onComplete();

    emscripten_fetch_close(f);
    delete ctx;
}

void LaunchFetch(const std::string& url, FetchCtx* ctx) {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");

    // LOAD_TO_MEMORY  → make fetch->data / fetch->numBytes available in callbacks
    // PERSIST_FILE    → write the response to IndexedDB (keyed by URL);
    //                   subsequent requests load from IndexedDB without touching
    //                   the network.
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE;
    attr.onsuccess  = OnSuccess;
    attr.onerror    = OnError;
    attr.userData   = ctx;

    emscripten_fetch(&attr, url.c_str());
}

} // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void WebAssetFetcher::Fetch(
    const std::string& url,
    std::function<void(const uint8_t*, size_t)> onSuccess,
    std::function<void(const std::string&)>     onError)
{
    auto* ctx = new FetchCtx{std::move(onSuccess), std::move(onError), nullptr};
    LaunchFetch(url, ctx);
}

void WebAssetFetcher::Preload(
    const std::vector<std::string>& urls,
    std::function<void()> onAllComplete)
{
    if (urls.empty()) {
        if (onAllComplete) onAllComplete();
        return;
    }

    auto batch            = std::make_shared<BatchState>();
    batch->remaining      = static_cast<int>(urls.size());
    batch->onComplete     = std::move(onAllComplete);

    for (const auto& url : urls) {
        // Capture url by value; batch by shared_ptr so it stays alive until
        // the last callback fires.
        auto* ctx = new FetchCtx{
            // onSuccess: copy bytes into AssetManager's web cache.
            // After this lambda returns the fetch buffer is closed → freed.
            [url](const uint8_t* data, size_t size) {
                AssetManager::Get().StorePreloadedAsset(
                    url, std::vector<uint8_t>(data, data + size));
                ENGINE_LOG("[WebAssetFetcher] cached '{}' ({} KiB)", url, size / 1024);
            },
            /*onError=*/nullptr,
            batch
        };
        LaunchFetch(url, ctx);
    }
}

#endif // __EMSCRIPTEN__
