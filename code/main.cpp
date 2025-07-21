// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include <chrono>
#include <print>
#include <thread>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

void app_run() {
  auto screen = ftxui::ScreenInteractive::Fullscreen();

  auto component = ftxui::Renderer([&] {
    return ftxui::vbox({
               ftxui::text("Hello"),
               ftxui::text("World"),
               ftxui::separator(),
           }) |
           ftxui::borderStyled(ftxui::Color::RGB(127, 255, 0));
  });

  component |= ftxui::CatchEvent([&](ftxui::Event event) -> bool {
    if (event == ftxui::Event::Character('q')) {
      screen.ExitLoopClosure()();
      return true;
    }
    return false;
  });

  auto loop = ftxui::Loop(&screen, component);
  while (!loop.HasQuitted()) {
    loop.RunOnce();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
  try {
    app_run();
  } catch (std::exception const& e) {
    std::print(stderr, "ERROR: {}\n", e.what());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
