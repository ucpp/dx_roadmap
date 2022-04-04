#include <config.h>
#include <fstream>

namespace engine
{
  bool Config::Load(const std::string& path)
  {
    std::ifstream file_stream(path);
    nlohmann::json json_file;
    file_stream >> json_file;
    data = json_file.get<Data>();

    return true;
  }
}