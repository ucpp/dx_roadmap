#include <application.h>
#include <window.h>
#include <config.h>

namespace engine
{
  Application::Application(const std::string& config_path)
    : tearing_supported(false)
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

  void Application::resizeWindow(uint32 width, uint32 height)
  {
    if (window->getWidth() != width || window->getHeight() != height)
    {
      // Don't allow 0 size swap chain back buffers.
      window->setSize(std::max(1u, width), std::max(1u, height));
      device_resources.resize(width, height);
    }
  }
}