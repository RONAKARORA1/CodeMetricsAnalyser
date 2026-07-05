cmake_minimum_required(VERSION 3.16)
project(CodeMetricsAnalyzer VERSION 1.0.0 LANGUAGES CXX)

# ── Project-wide settings ──────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)       # -std=c++17 not -std=gnu++17

# Strict warnings — treat analysis quality as a first-class concern
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# ── Shared include root ────────────────────────────────────────────────────
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# ── Module libraries ───────────────────────────────────────────────────────
# Each module is its own static library.
# Benefits:
#   • Compiler caches unchanged modules across rebuilds
#   • Linkage errors surface at the library boundary, not at link-all time
#   • Tests can link only the module they're testing

add_library(cma_lexer STATIC
    src/lexer/Lexer.cpp
)
target_include_directories(cma_lexer PUBLIC ${INCLUDE_DIR})

add_library(cma_filesystem STATIC
    src/filesystem/FileScanner.cpp
)
target_include_directories(cma_filesystem PUBLIC ${INCLUDE_DIR})

add_library(cma_parser STATIC
    src/parser/Parser.cpp
)
target_include_directories(cma_parser PUBLIC ${INCLUDE_DIR})
target_link_libraries(cma_parser PUBLIC cma_lexer)

add_library(cma_metrics STATIC
    src/metrics/MetricsEngine.cpp
)
target_include_directories(cma_metrics PUBLIC ${INCLUDE_DIR})
target_link_libraries(cma_metrics PUBLIC cma_parser)

add_library(cma_report STATIC
    src/report/ReportGenerator.cpp
)
target_include_directories(cma_report PUBLIC ${INCLUDE_DIR})
target_link_libraries(cma_report PUBLIC cma_metrics)

# ── Main executable ────────────────────────────────────────────────────────
add_executable(cma src/main.cpp)
target_link_libraries(cma PRIVATE
    cma_lexer
    cma_filesystem
    cma_parser
    cma_metrics
    cma_report
)

# std::filesystem needs explicit linking on GCC < 9
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    target_link_libraries(cma PRIVATE stdc++fs)
    target_link_libraries(cma_filesystem PRIVATE stdc++fs)
endif()

# ── Optional: tests ────────────────────────────────────────────────────────
# Uncomment when you add a tests/ directory with Google Test or Catch2:
# enable_testing()
# add_subdirectory(tests)
