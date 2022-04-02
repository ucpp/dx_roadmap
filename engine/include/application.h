#pragma once

#include <common/pch.h>

namespace engine
{
  class Application
  {
  public:

  private:
    void update();

    static LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

  private:
    bool tearing_supported;
    bool vsync;
  };
}