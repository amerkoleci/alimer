# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")

# ShaderMakeBlob
add_subdirectory(ShaderMakeBlob)

# fmt
add_subdirectory(fmt EXCLUDE_FROM_ALL)
set_property(TARGET fmt PROPERTY FOLDER "ThirdParty")
#install (DIRECTORY spdlog/include/spdlog DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/ThirdParty)

# spdlog
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "Use external fmt library instead of bundled" FORCE)
set(SPDLOG_USE_STD_FORMAT OFF CACHE BOOL "Use external fmt library instead of bundled" FORCE)
add_subdirectory(spdlog EXCLUDE_FROM_ALL)
set_property(TARGET spdlog PROPERTY FOLDER "ThirdParty")
install (DIRECTORY spdlog/include/spdlog DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/ThirdParty)

# unordered_dense
add_subdirectory(unordered_dense)
set_property(TARGET unordered_dense PROPERTY FOLDER "ThirdParty")
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/unordered_dense/include/ankerl
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/ThirdParty
)

# lz4
add_subdirectory(lz4)
set_property(TARGET lz4 PROPERTY FOLDER "ThirdParty")
