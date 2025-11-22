#pragma once
#include <memory>
#include <string>

class Mesh;

class Image {
public:
    Image(int width, int height, int numChannels, unsigned char* data) :
        width(width), height(height), channelCount(numChannels),
        byteArray(std::vector<unsigned char>(data, data + width * height * numChannels)) {}

    int width;
    int height;
    int channelCount;
    std::vector<unsigned char> byteArray;
};

class AssetManager {
public:
    static std::shared_ptr<Image> loadImage(const std::string& filename);
    static std::shared_ptr<Mesh> loadOBJ(const std::string& filename);
    // static std::shared_ptr<Mesh> loadGLTF(const std::string& filename);
private:

};