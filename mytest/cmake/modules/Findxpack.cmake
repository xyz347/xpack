
find_path(xpack_INCLUDE xpack/xpack.h HINTS "${PROJECT_SOURCE_DIR}/../..")

if (xpack_INCLUDE)
    set(XPACK_FOUND TRUE)
    message(STATUS "${Green}Found XPACK include at: ${xpack_INCLUDE}${Reset}")
else()
    message(FATAL_ERROR "${Red}Failed to locate XPACK module.${Reset}" )
endif()
