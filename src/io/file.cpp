#include "file.hpp"

File::File(const std::string& filename) : _filename(filename) {}

std::string File::GetContent() const 
{
    std::ifstream ifs(_filename.c_str());
    std::string content(
        (std::istreambuf_iterator<char>(ifs)), //start of stream iterator
        (std::istreambuf_iterator<char>()) //end of stream iterator
    );
    return content;
}
