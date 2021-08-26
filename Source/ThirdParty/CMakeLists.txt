# GLFW
if(NOT (EMSCRIPTEN OR ANDROID))
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
  set(GLFW_INSTALL OFF CACHE BOOL "")
    
  add_subdirectory(glfw EXCLUDE_FROM_ALL)
  set_property(TARGET glfw PROPERTY FOLDER "ThirdParty")
endif()

# spdlog
add_subdirectory(spdlog EXCLUDE_FROM_ALL)
set_property(TARGET spdlog PROPERTY FOLDER "ThirdParty")