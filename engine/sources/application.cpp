#include <application.h>

namespace engine
{
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
}