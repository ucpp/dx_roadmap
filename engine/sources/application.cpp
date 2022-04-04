#include <application.h>
#include <window.h>
#include <config.h>

namespace engine
{
  Application::Application(const std::string& config_path, HINSTANCE instance, int cmd_show)
    : instance(instance)
    , cmd_show(cmd_show)
    , tearing_supported(false)
    , vsync(false)
  {
    Config config;
    config.Load(config_path);
    auto& settings = config.data.application_settings;
    std::wstring app_name(settings.name.begin(), settings.name.end());

    window = new Window(app_name, settings.window_width, settings.window_height);
  }

  Application::~Application()
  {
    delete window;
    window = nullptr;
  }

  void Application::run()
  {
    initialize();

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
      if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
      {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
      }
    }

    destroy();
  }

  void Application::initialize()
  {
    window->initialize(this, instance, &Application::wndProc, cmd_show);
    device_resources.loadPipeline(*window);
    tearing_supported = device_resources.checkTearingSupport();
    window->show();
  }

  void Application::update()
  {
    static uint64 frame_counter = 0;
    static double elapsed_seconds = 0.0;
    static std::chrono::high_resolution_clock clock;
    static auto t0 = clock.now();

    frame_counter++;
    auto t1 = clock.now();
    auto delta_time = t1 - t0;
    t0 = t1;

    elapsed_seconds += delta_time.count() * 1e-9; // nanoseconds to seconds
    if (elapsed_seconds > 1.0)
    {
      auto fps = frame_counter / elapsed_seconds;
      Log::info("FPS: %f\n", fps);

      frame_counter = 0;
      elapsed_seconds = 0.0;
    }
  }

  void Application::render()
  {
    device_resources.render(true, tearing_supported);
  }

  void Application::destroy()
  {
    device_resources.flush();
    CloseHandle(device_resources.getFenceEvent());
  }

  void Application::resizeWindow(uint32 width, uint32 height)
  {
    if (window->getWidth() != width || window->getHeight() != height)
    {
      // Don't allow 0 size swap chain back buffers.
      window->setSize(std::max(1u, width), std::max(1u, height));
      device_resources.resize(width, height);
    }
  }

  LRESULT CALLBACK Application::wndProc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
  {
    Application* application = reinterpret_cast<Application*>(GetWindowLongPtr(wnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
    {
      LPCREATESTRUCT create_struct = reinterpret_cast<LPCREATESTRUCT>(lparam);
      SetWindowLongPtr(wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
    }
    break;

    case WM_PAINT:
    {
      if (application)
      {
        application->update();
        application->render();
      }
    }
    break;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
    }
    break;

    case WM_KEYUP:
    {
    }
    break;

    case WM_SIZE:
    {
      RECT client_rect = {};
      ::GetClientRect(wnd, &client_rect);
      int width = client_rect.right - client_rect.left;
      int height = client_rect.bottom - client_rect.top;

      application->resizeWindow(width, height);
    }
    break;

    case WM_DESTROY:
      ::PostQuitMessage(0);
      break;

    default:
      return ::DefWindowProcW(wnd, message, wparam, lparam);
    }

    return 0;
  }
}