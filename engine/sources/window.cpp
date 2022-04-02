#include <window.h>

namespace engine
{
  void Window::registerWindowClass(HINSTANCE hinst, const wchar_t* window_class_name, WNDPROC wnd_proc)
  {
    WNDCLASSEXW window_class = {};

    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = wnd_proc;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = hinst;
    window_class.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    window_class.lpszMenuName = nullptr;
    window_class.lpszClassName = window_class_name;
#pragma warning(suppress: 6387)
    window_class.hIcon = ::LoadIcon(hinst, nullptr);
    window_class.hIconSm = ::LoadIcon(hinst, nullptr);
#pragma warning(default: 6387)

    static HRESULT hr = ::RegisterClassExW(&window_class);
    assert(SUCCEEDED(hr));
  }

  HWND Window::createWindow(const wchar_t* window_class_name, HINSTANCE hinst, const wchar_t* window_title, uint32 w, uint32 h, Application* application)
  {
    int screen_width = ::GetSystemMetrics(SM_CXSCREEN);
    int screen_height = ::GetSystemMetrics(SM_CYSCREEN);

    window_rect = { 0, 0, static_cast<LONG>(w), static_cast<LONG>(h) };
    ::AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);

    width = window_rect.right - window_rect.left;
    height = window_rect.bottom - window_rect.top;

    int window_x = std::max<int>(0, (screen_width - width) / 2);
    int window_y = std::max<int>(0, (screen_height - height) / 2);

    HWND hwnd = ::CreateWindowExW(NULL, window_class_name, window_title, WS_OVERLAPPEDWINDOW, window_x, window_y, width, height, NULL, NULL, hinst, application);

    assert(hwnd && "Failed to create window");

    return hwnd;
  }
}