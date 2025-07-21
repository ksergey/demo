include(FetchContent)

FetchContent_Declare(ftxui
  URL https://github.com/ArthurSonzogni/FTXUI/archive/refs/tags/v6.1.9.tar.gz
  GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)

set(TargetName demo)

file(GLOB_RECURSE TargetSources "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(${TargetName})

target_compile_features(${TargetName}
  PRIVATE
    cxx_std_23
)

target_compile_options(${TargetName}
  PRIVATE
    -Wall -Wextra -Wnrvo -Wattributes -Wpedantic -Wstrict-aliasing -Wcast-align -g
)

set_target_properties(${TargetName}
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_link_libraries(${TargetName}
  PRIVATE
    ftxui::component ftxui::dom ftxui::screen
)

target_sources(${TargetName} PRIVATE ${TargetSources})

