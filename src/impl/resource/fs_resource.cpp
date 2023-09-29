#include <resources/resource.hpp>
#include <fstream>

#define RES_PATH_PREFIX _resPathPrefix.c_str()

static std::string ResPathHack()
{
    std::string path = __FILE__;
    size_t end = path.rfind("\\src\\");
    if (end == std::string::npos)
        end = path.rfind("/src/");
    path = path.substr(0, end + 1) + "resources/";
    return path;
}
static std::string _resPathPrefix = ResPathHack();

/**
 * @brief Load resources from files within the current directory
 */
class FilesytemResource : public Resource {
public:
    FilesytemResource(char* data, size_t len) : Resource(data, len) {}
    FilesytemResource(FilesytemResource&&) = default;
    ~FilesytemResource() { delete[] Data(); }
};

Resource::Ptr Resource::LoadInternal(const std::string& url) {
    std::string fs_path = _resPathPrefix + url;
    std::fstream file(fs_path, std::ios::in | std::ios::binary);
    if (!file.is_open())
        return nullptr;

    file.seekg(0, std::ios::end);

    size_t len = file.tellg();
    if (file.fail())
        return nullptr;
    
    char* data = new char[len];

    file.seekg(0, std::ios::beg);
    file.read(data, len);

    return std::make_shared<FilesytemResource>(data, len);
}

void Resource::LoadAsyncInternal(const std::string& url, bool notify_failure, LoadCallback callback) {
    auto res = Resource::Load(url);
    if (!res && !notify_failure)
        return;
    callback(res);
}