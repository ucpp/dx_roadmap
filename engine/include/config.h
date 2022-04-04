#pragma once

#include <common/pch.h>
#include <json.hpp>

namespace engine
{
  struct ApplicationSettingsData
  {
    std::string name;
    uint32 window_width {1280};
    uint32 window_height {768};
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ApplicationSettingsData, name, window_width, window_height);

  struct Data
  {
    ApplicationSettingsData application_settings;
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Data, application_settings);

  class Config
  {
  public:
    bool Load(const std::string& path);

  public:
    Data data;
  };
}