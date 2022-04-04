#pragma once

#include <common/pch.h>
#include <device_resources.h>

namespace engine
{
  class Application
  {
  public:
    Application(const std::string& config_path, HINSTANCE instance, int cmd_show);
    ~Application();
    
    void run();

  private:
    void initialize();
    void update();
    void render();
    void destroy();

    void resizeWindow(uint32 width, uint32 height);

    static LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

  private:
    class Window* window { nullptr };
    DeviceResources device_resources;
    HINSTANCE instance;

    bool tearing_supported;
    bool vsync;
    int cmd_show;
  };
}