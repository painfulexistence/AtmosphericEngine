#include "script.hpp"
#include "material.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "scene.hpp"
#include <string>

Script* Script::_instance = nullptr;

Script::Script()
{
    if (_instance != nullptr)
        throw std::runtime_error("Script is already initialized!");

    _env = sol::state();

    _instance = this;
}

Script::~Script()
{

}

void Script::Init(Application* app)
{
    Server::Init(app);

    this->_env.open_libraries();
    Source("./assets/config.lua");
    Source("./assets/manifest.lua");
    Source("./assets/main.lua");

    Run("init()");
    //Bind("get_cursor_uv", &Input::GetCursorUV, Input::Get());
    //Bind("get_key_down", &Input::GetKeyDown, Input::Get());
    //Bind("check_errors", &GraphicsServer::CheckErrors, GraphicsServer::Get());
}

void Script::Process(float dt)
{
    Run(fmt::format("update({})", dt));
}

// TODO: an extra argument is needed here
void Script::Bind(const std::string& func)
{
    //this->_env.set_function(func);
}

void Script::Source(const std::string& file)
{
    this->_env.script_file(file);
}

void Script::Run(const std::string& script)
{
    this->_env.script(script);
}

void Script::Print(const std::string& msg)
{
    Run(fmt::format("print('[Script] {}')", msg));
}

sol::table Script::GetData(const std::string& key)
{
    return this->_env.globals()[key];
}

SceneDef Script::GetScene(const sol::table& sceneData)
{
    SceneDef scene;

    const sol::table texturesTable = sceneData["textures"];
    for (const auto& kv : texturesTable) {
        sol::table textureData = kv.second;
        scene.textures.push_back((std::string)textureData["path"]);
    }

    const sol::table shadersTable = sceneData["shaders"];
    scene.shaders["color"] = { shadersTable["color"]["vert"], shadersTable["color"]["frag"] };
    scene.shaders["debug_line"] = { shadersTable["debug_line"]["vert"], shadersTable["debug_line"]["frag"] };
    scene.shaders["depth"] = { shadersTable["depth"]["vert"], shadersTable["depth"]["frag"] };
    scene.shaders["depth_cubemap"] = { shadersTable["depth_cubemap"]["vert"], shadersTable["depth_cubemap"]["frag"] };
    scene.shaders["hdr"] = { shadersTable["hdr"]["vert"], shadersTable["hdr"]["frag"] };
    scene.shaders["terrain"] = { shadersTable["terrain"]["vert"], shadersTable["terrain"]["frag"], shadersTable["terrain"]["tesc"], shadersTable["terrain"]["tese"] };

    const sol::table materialsTable = sceneData["materials"];
    for (const auto& kv : materialsTable) {
        sol::table materialData = kv.second;
        scene.materials.push_back({
            .baseMap = (int)materialData.get_or("baseMapId", -1),
            .normalMap = (int)materialData.get_or("normalMapId", -1),
            .aoMap = (int)materialData.get_or("aoMapId", -1),
            .roughnessMap = (int)materialData.get_or("roughtnessMapId", -1),
            .metallicMap = (int)materialData.get_or("metallicMapId", -1),
            .heightMap = (int)materialData.get_or("heightMapId", -1),
            .diffuse = glm::vec3(materialData["diffuse"][1], materialData["diffuse"][2], materialData["diffuse"][3]),
            .specular = glm::vec3(materialData["specular"][1], materialData["specular"][2], materialData["specular"][3]),
            .ambient = glm::vec3(materialData["ambient"][1], materialData["ambient"][2], materialData["ambient"][3]),
            .shininess = (float)materialData.get_or("shininess", 0.25)
        });
    }

    const sol::table entitiesTable = sceneData["entities"];
    for (const auto& [eKey, eVal] : entitiesTable) {
        sol::table entityData = eVal;
        GameObjectProps def = {
            .name = eKey.as<std::string>(),
            .camera = std::nullopt,
            .light = std::nullopt
        };
        auto pos = entityData.get_or("position", sol::table());
        if (pos.valid()) {
            def.position = glm::vec3(pos[1], pos[2], pos[3]);
        }
        auto rot = entityData.get_or("rotation", sol::table());
        if (rot.valid()) {
            def.rotation = glm::vec3(rot[1], rot[2], rot[3]);
        }
        auto scale = entityData.get_or("scale", sol::table());
        if (scale.valid()) {
            def.scale = glm::vec3(scale[1], scale[2], scale[3]);
        }
        sol::table components = entityData["components"];
        for (const auto& [cKey, cVal] : components) {
            std::string componentType = cKey.as<std::string>();
            sol::table componentData = cVal;
            if (componentType == "camera") {
                def.camera = {
                    .isOrthographic = false,
                    .perspective = {
                        .fieldOfView = (float)componentData.get_or("field_of_view", glm::radians(60.f)),
                        .aspectRatio = (float)componentData.get_or("aspect_ratio", 4.f / 3.f),
                        .nearClip = (float)componentData.get_or("near_clip_plane", 0.1f),
                        .farClip = (float)componentData.get_or("far_clip_plane", 1000.0f),
                    },
                    .verticalAngle = (float)componentData.get_or("vertical_angle", 0),
                    .horizontalAngle = (float)componentData.get_or("horizontal_angle", 0),
                    .eyeOffset = glm::vec3(
                        (float)componentData.get_or("eye_offset.x", 0),
                        (float)componentData.get_or("eye_offset.y", 0),
                        (float)componentData.get_or("eye_offset.z", 0)
                    )
                };
            } else if (componentType == "light") {
                def.light = {
                    .type = static_cast<LightType>(componentData.get_or("type", 1)),
                    .position = glm::vec3(
                        componentData["position"][1],
                        componentData["position"][2],
                        componentData["position"][3]
                    ),
                    .direction = glm::vec3(
                        componentData["direction"][1],
                        componentData["direction"][2],
                        componentData["direction"][3]
                    ),
                    .ambient = glm::vec3(
                        componentData["ambient"][1],
                        componentData["ambient"][2],
                        componentData["ambient"][3]
                    ),
                    .diffuse = glm::vec3(
                        componentData["diffuse"][1],
                        componentData["diffuse"][2],
                        componentData["diffuse"][3]
                    ),
                    .specular = glm::vec3(
                        componentData["specular"][1],
                        componentData["specular"][2],
                        componentData["specular"][3]
                    ),
                    .attenuation = glm::vec3(
                        componentData["attenuation"][1],
                        componentData["attenuation"][2],
                        componentData["attenuation"][3]
                    ),
                    .intensity = (float)componentData.get_or("intensity", 1.0),
                    .castShadow = (bool)componentData.get_or("castShadow", 0)
                };
            }
        }
        scene.gameObjects.push_back(def);
    }
    return scene;
}

std::vector<SceneDef> Script::GetScenes()
{
    std::vector<SceneDef> scenes;

    sol::table data = GetData(std::string("scenes"));
    for (const auto& kv : data) {
        sol::table sceneData = kv.second;
        scenes.push_back(GetScene(sceneData));
    }

    return scenes;
}

void Script::GetData(const std::string& key, sol::table& data)
{
    data = this->_env.globals()[key];
}
