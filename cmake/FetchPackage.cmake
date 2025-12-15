# cmake_policy(SET CMP0135 NEW)

include(FetchContent)

# Use FetchContent to download and populate a package without calling add_subdirectory().
# This is used for downloading prebuilt external binaries.
macro(FetchPackage name)
    cmake_parse_arguments(FETCH "" "URL" "" ${ARGN})
    FetchContent_Declare(
        ${name}
        URL ${FETCH_URL}
        SOURCE_SUBDIR _does_not_exist_ # avoid adding contained CMakeLists.txt
    )
    FetchContent_GetProperties(${name})
    if(NOT ${name}_POPULATED)
        FetchContent_MakeAvailable(${name})
    endif()
endmacro()
