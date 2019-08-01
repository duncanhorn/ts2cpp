cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/inc)

# E.g. replace_cxx_flag("/W[0-4]", "/W4")
macro(replace_cxx_flag pattern text)
    foreach (flag
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)

        string(REGEX REPLACE "${pattern}" "${text}" ${flag} "${${flag}}")

    endforeach()
endmacro()

macro(append_cxx_flag text)
    foreach (flag
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)

        string(APPEND ${flag} " ${text}")

    endforeach()
endmacro()

replace_cxx_flag("/W[0-4]" "/W4")
append_cxx_flag("/WX")
append_cxx_flag("/permissive-")

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    append_cxx_flag("-fno-delayed-template-parsing")
    append_cxx_flag("-fno-ms-compatibility")
else()
    # append_cxx_flag("/experimental:preprocessor")

    # CRT headers are not yet /experimental:preprocessor clean, so work around the known issues
    # append_cxx_flag("/Wv:18")
endif()

add_subdirectory(src)
