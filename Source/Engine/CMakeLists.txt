macro (define_engine_source_files)
	cmake_parse_arguments(DEFINE_SRC_FILES "NO_INSTALL" "" "" ${ARGN} )

	foreach (path ${DEFINE_SRC_FILES_UNPARSED_ARGUMENTS})
		# Get header files
		file (GLOB _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.hpp ${path}/*.h ${path}/*.inl)
		list (APPEND HEADER_FILES ${_files})

		# Install them
		file (GLOB INSTALL_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.hpp ${path}/*.h ${path}/*.inl)

		if (NOT DEFINE_SRC_FILES_NO_INSTALL)
			install (
				DIRECTORY ${path} 
				DESTINATION include
				FILES_MATCHING
				PATTERN "*.h"
				PATTERN "*.hpp"
				PATTERN "Private" EXCLUDE
			)
		endif ()

		# Get source files
		file (GLOB _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.c ${path}/*.cpp)
		list (APPEND SOURCE_FILES ${_files})

		# Get natvis files for MSVC
		if (MSVC)
			file (GLOB NATVIS_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.natvis)
			list (APPEND SOURCE_FILES ${NATVIS_FILES})
		endif ()
	endforeach ()
endmacro()

# Groups sources into subfolders.
macro(group_sources)
	file (GLOB_RECURSE children LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/**)
	foreach (child ${children})
		if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
		  string(REPLACE "/" "\\" groupname "${child}")
		  file (GLOB files LIST_DIRECTORIES false ${CMAKE_CURRENT_SOURCE_DIR}/${child}/*)
		  source_group(${groupname} FILES ${files})
		endif ()
	endforeach ()
endmacro()

# Define target name
set (TARGET_NAME Alimer)

# Add root-level files
file (GLOB HEADER_FILES *.h)
file (GLOB SOURCE_FILES *.c *.cpp)
source_group("" FILES ${HEADER_FILES})
source_group("" FILES ${SOURCE_FILES})

if(MSVC)
  set(MISC_FILES  alimer.natvis)
else()
  set(MISC_FILES "")
endif()

define_engine_source_files(
  Core
  Math
  IO
  RHI
)

define_engine_source_files(Private NO_INSTALL)

# GLFW
if(NOT (EMSCRIPTEN OR ANDROID))
  define_engine_source_files(Platform/GLFW NO_INSTALL)
endif ()

group_sources()

if (ALIMER_BUILD_SHARED)
  add_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${MISC_FILES})
  #set_property(TARGET ${TARGET_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

  target_compile_definitions(${TARGET_NAME}
    PRIVATE ALIMER_EXPORTS=1
  	PUBLIC ALIMER_IMPORTS=1
  )
else()
  add_library(${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${MISC_FILES})

  target_compile_definitions (${TARGET_NAME} PUBLIC ALIMER_STATIC=1)
endif ()

# Include directories
target_include_directories(${TARGET_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  #$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/ImGui
  $<INSTALL_INTERFACE:${INSTALL_BASE_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:${INSTALL_THIRDPARTY_DIR}>
)

# Compile defines
target_compile_definitions(${TARGET_NAME} PUBLIC
  $<IF:$<BOOL:${ALIMER_PROFILING}>,ALIMER_PROFILING=1,ALIMER_PROFILING=0>
  $<IF:$<BOOL:${ALIMER_NETWORK}>,ALIMER_NETWORK=1,ALIMER_NETWORK=0>
  $<IF:$<BOOL:${ALIMER_AUDIO}>,ALIMER_AUDIO=1,ALIMER_AUDIO=0>
  $<IF:$<BOOL:${ALIMER_PHYSICS}>,ALIMER_PHYSICS=1,ALIMER_PHYSICS=0>
)

target_link_libraries(${TARGET_NAME} PUBLIC
  #stb
  spdlog
)

# RHI defines
if (ALIMER_RHI_D3D11)
  target_compile_definitions(${TARGET_NAME} PUBLIC ALIMER_RHI_D3D11)
  target_link_libraries(${TARGET_NAME} PRIVATE d3d11 dxgi dxguid)
endif()

if (ALIMER_RHI_D3D12)
  target_compile_definitions(${TARGET_NAME} PUBLIC ALIMER_RHI_D3D12)
endif()

if (ALIMER_RHI_GL)
  target_compile_definitions(${TARGET_NAME} PUBLIC ALIMER_RHI_GL)
endif()

if (ALIMER_RHI_VULKAN)
  target_compile_definitions(${TARGET_NAME} PUBLIC ALIMER_RHI_VULKAN)
endif()

if (ALIMER_RHI_METAL)
  target_compile_definitions(${TARGET_NAME} PUBLIC ALIMER_RHI_METAL)
endif()


# Compiler-specific setup
if(MSVC)
  # Warnings as Errors Enabled
  target_compile_options(${TARGET_NAME} PRIVATE /W3 /WX)

  # Multiprocessor compiler
  #target_compile_options(${TARGET_NAME} PRIVATE /MP)

  target_compile_options(${TARGET_NAME} PRIVATE /fp:fast)
  target_compile_options(${TARGET_NAME} PRIVATE /permissive- /JMC- /Zc:__cplusplus)

  if(ALIMER_BUILD_ENABLE_CODE_ANALYSIS)
    target_compile_options(${TARGET_NAME} PRIVATE /analyze)
  endif()

  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.26)
    target_compile_options(${TARGET_NAME} PRIVATE /Zc:preprocessor /wd5104 /wd5105)
  endif()
else()
  target_compile_options(${TARGET_NAME} PUBLIC
    -fno-exceptions
	$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
  )

  # Warnings as Errors Enabled
  target_compile_options(${TARGET_NAME} PRIVATE -Werror)
endif()

# GLFW
if(NOT (EMSCRIPTEN OR ANDROID))
  target_compile_definitions(${TARGET_NAME} PRIVATE ALIMER_PLATFORM_GLFW)
  target_link_libraries(${TARGET_NAME} PRIVATE glfw)
endif ()

if (ALIMER_RHI_VULKAN)
  target_link_libraries(${TARGET_NAME} PRIVATE volk vma spirv-reflect)
endif()

# SDK installation
install(TARGETS ${TARGET_NAME}
    EXPORT ${TARGET_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
