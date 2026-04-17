file(GLOB_RECURSE CORE_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/../../embedded-core/*.c"
)

idf_component_register(
    SRCS "home-hub.c" ${CORE_SOURCES}
    INCLUDE_DIRS "." "../../embedded-core"
)