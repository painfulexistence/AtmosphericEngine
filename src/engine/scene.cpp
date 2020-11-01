#include "scene.hpp"

Scene::Scene()
{
    // Setup materials
    _materials.push_back(
        //Terrain material
        Material(
            "terrainMat",
            (GLuint)4,
            glm::vec3(.25, .20725, .20725),
            glm::vec3(1, .829, .829),
            glm::vec3(.296648, .296648, .296648),
            0.088
        )
    );
    _materials.push_back(
        //Skybox material
        Material(
            "skyboxMat",
            (GLuint)1,
            glm::vec3(.1, .18725, .1745),
            glm::vec3(.396, .74151, .69102),
            glm::vec3(.992157, .941176, .807843),
            0.1
        )
    );       
    _materials.push_back(
        //Cube material
        Material(
            "mazeBlockMat",
            (GLuint)4,
            glm::vec3(0.19225, 0.19225, 0.19225),
            glm::vec3(0.50754, 0.50754, 0.50754),
            glm::vec3(0.508273, 0.508273, 0.508273),
            0.4
        )
    );
    _materials.push_back(
        //Green plastic
        Material(
            "defaultMat",
            (GLuint)3,
            glm::vec3(.0, .0, .0),
            glm::vec3(.5, .0, .0),
            glm::vec3(.7, .6, .6),
            0.25
        )
    );
}

Scene::~Scene() {}

void Scene::Create(const std::shared_ptr<Instantiation>& ins)
{
    _instantiations.push_back(ins);
}

static void UpdateInstances(const std::shared_ptr<Instantiation>& ins, float time)
{
    for (int i = 0; i < ins->instances.size(); i++)
    {
        ins->instances[i]->Update(time);
        //TODO: Update skybox transform to glm::rotate(glm::mat4(1.0f), time * glm::radians(10.0f), glm::vec3(0, 1, 0));
    }
}

void Scene::Update(float time)
{
    for (const auto& ins : _instantiations)
    {
        UpdateInstances(ins, time);
    }
}

static void RenderInstances(const std::shared_ptr<Instantiation>& ins)
{
    int num = ins->instances.size();

    std::vector<glm::mat4> wms(num);
    for (int i = 0; i < num; i++)
    {
        wms[i] = ins->instances[i]->GetTransform();
    }
    ins->prefab->Render(wms);
}

void Scene::Render(Program& program)
{
    for (const auto& ins : _instantiations) // For every instantiations
    {
        auto& mat = _materials[ins->materialIdx];
        program.SetUniform(std::string("surf.ambient"), mat.GetAmbient());
        program.SetUniform(std::string("surf.diffuse"), mat.GetDiffuse());
        program.SetUniform(std::string("surf.specular"), mat.GetSpecular());
        program.SetUniform(std::string("surf.shininess"), mat.GetShininess());
        program.SetUniform(std::string("tex_unit"), (int)mat.GetTexUnit());
        
        RenderInstances(ins);
    }
}
