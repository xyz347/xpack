
find_path(rapidxml_INCLUDE rapidxml/rapidxml.hpp HINTS "${PROJECT_SOURCE_DIR}/../thirdparty/")

if (rapidxml_INCLUDE)
    set(RAPIDXML_FOUND TRUE)
    message(STATUS "${Green}Found RAPIDXML include at: ${rapidxml_INCLUDE}${Reset}")
else()
    message(FATAL_ERROR "${Red}Failed to locate RAPIDXML module.${Reset}" )
endif()
