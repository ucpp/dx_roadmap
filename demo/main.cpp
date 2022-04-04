#include <common/pch.h>
#include <application.h>

int CALLBACK wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE h_prev_instance, _In_ PWSTR lp_cmd_line, _In_ int cmd_show)
{
  try
  {
    const std::string path = "resources/config.json";
    engine::Log::info("Start demo with config: %s\n", path.c_str());

    engine::Application application(path, instance, cmd_show);
    application.run();
  }
  catch (const std::exception& ex)
  {
    engine::Log::error(ex.what());
    return EXIT_FAILURE;
  }

  engine::Log::info("Success quit demo.\n");

  return EXIT_SUCCESS;
}
