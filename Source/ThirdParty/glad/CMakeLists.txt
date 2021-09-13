add_library(glad STATIC
  "include/KHR/khrplatform.h"
  "include/glad/glad.h"
  "src/glad.c"
)

target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
