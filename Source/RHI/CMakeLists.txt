# Define target name
set (TARGET_NAME RHI)

set(RHI_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(RHI_FILES
  "${RHI_DIR}/RHI.cpp"
  "${RHI_DIR}/RHI.h"
  "${RHI_DIR}/RHI_D3D12.cpp"
  "${RHI_DIR}/RHI_D3D12.h"
  "${RHI_DIR}/RHI_Vulkan.cpp"
  "${RHI_DIR}/RHI_Vulkan.h"
)

add_library(${TARGET_NAME} STATIC ${RHI_FILES})
target_include_directories(${TARGET_NAME} PUBLIC ${RHI_DIR})

# RHI defines
if (ALIMER_RHI_D3D12)
  target_compile_definitions(${TARGET_NAME} PRIVATE ALIMER_RHI_D3D12)
  target_link_libraries(${TARGET_NAME} PRIVATE D3D12MemoryAllocator)
endif()

if (ALIMER_RHI_VULKAN)
  target_compile_definitions(${TARGET_NAME} PRIVATE ALIMER_RHI_VULKAN)
  target_link_libraries(${TARGET_NAME} PRIVATE volk vma spirv-reflect)
endif()

if (ALIMER_RHI_METAL)
  target_compile_definitions(${TARGET_NAME} PRIVATE ALIMER_RHI_METAL)
endif()
