#pragma once

struct LightProps;

struct CameraProps;

struct SceneData {
    std::vector<std::string> texturePaths;
    std::unordered_map<std::string, std::string> shaderPaths;
    std::vector<LightProps> lightDataList;
    std::vector<CameraProps> cameraDataList;
};

class Scene {
public:
    Scene(const SceneData& data);
};