#pragma once

// General
#define INIT_SCREEN_TITLE "Atmospheric"
#define INIT_SCREEN_WIDTH 1120 //unit
#define INIT_SCREEN_HEIGHT 840 //unit
#define INIT_FRAMEBUFFER_WIDTH 1120 //px
#define INIT_FRAMEBUFFER_HEIGHT 840 //px
#define SINGLE_THREAD 1
#define RUNTIME_LOG_ON 1
#define SHOW_PROCESS_COST 0
#define SHOW_RENDER_AND_DRAW_COST 0
#define SHOW_SYNC_COST 0
#define SHOW_VSYNC_COST 0

// Graphics
#define SHADOW_W 1024
#define SHADOW_H 1024
#define SHADOW_CASCADES 3
#define MAX_UNI_LIGHTS 1
#define MAX_OMNI_LIGHTS 1 // NOTE: Currently, at least one omni shadow map is necessary, as there is no way to disable shadows for now
#define UNI_SHADOW_MAP_COUNT MAX_UNI_LIGHTS
#define OMNI_SHADOW_MAP_COUNT MAX_OMNI_LIGHTS
#define SHADOW_MAP_COUNT UNI_SHADOW_MAP_COUNT + OMNI_SHADOW_MAP_COUNT
#define DEFAULT_TEXTURE_BASE_INDEX SHADOW_MAP_COUNT
#define DEFAULT_TEXTURE_COUNT 0
#define SCENE_TEXTURE_BASE_INDEX SHADOW_MAP_COUNT + DEFAULT_TEXTURE_COUNT
#define VSYNC_ON 1
#define FRUSTUM_CULLING_ON 0
#define MSAA_ON 1
#define MSAA_NUM_SAMPLES 4