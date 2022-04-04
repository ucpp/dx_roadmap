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

    void initialize(Application* application, HINSTANCE instance, WNDPROC wnd_proc, int cmd_show);
    void setSize(uint32 width, uint32 height);
    void setFullscreen(bool fullscreen);
    void show();

    uint32 getWidth() const { return width; }
    uint32 getHeight() const { return height; }
    bool getUseWarp() const { return use_warp; }
    HWND getHwnd() const { return hwnd; }

  private:
    void registerWindowClass(HINSTANCE hinst, const wchar_t* window_class_name, WNDPROC wnd_proc);
    HWND createWindow(const wchar_t* window_class_name, HINSTANCE hinst, const wchar_t* window_title, uint32 width, uint32 height, Application* application = nullptr);

  private:
    uint32 width;
    uint32 height;

    bool use_warp;
    bool fullscreen;
    int cmd_show;
    std::wstring title;

    HWND hwnd;
    RECT window_rect;
  };
}