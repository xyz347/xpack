
find_path(rapidjson_INCLUDE rapidjson/document.h HINTS "${PROJECT_SOURCE_DIR}/../thirdparty/")

if (rapidjson_INCLUDE)
    set(RAPIDJSON_FOUND TRUE)
    message(STATUS "${Green}Found RAPIDJSON include at: ${rapidjson_INCLUDE}${Reset}")
else()
    message(FATAL_ERROR "${Red}Failed to locate RAPIDJSON module.${Reset}" )
endif()
