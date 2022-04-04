#pragma once

#include <common/pch.h>
#include <application.h>

namespace engine
{
  class Window
  {
  public:
    Window(std::wstring name, uint32 width, uint32 height);
    ~Window();

    void setSize(uint32 width, uint32 height);

    uint32 getWidth() const { return width; }
    uint32 getHeight() const { return height; }

  private:
    void registerWindowClass(HINSTANCE hinst, const wchar_t* window_class_name, WNDPROC wnd_proc);
    HWND createWindow(const wchar_t* window_class_name, HINSTANCE hinst, const wchar_t* window_title, uint32 width, uint32 height, Application* application = nullptr);

  private:
    uint32 width;
    uint32 height;

    bool use_wrap;
    bool fullscreen;
    std::wstring title;

    HWND hwnd;
    RECT window_rect;
  };
}