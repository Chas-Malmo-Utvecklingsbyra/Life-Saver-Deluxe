file(GLOB_RECURSE CORE_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/../../embedded-core/*.c"
)

list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/test/.*")

list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/mocks/.*")

list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/.*_test\\.c$")

message(STATUS "CORE_SOURCES: ${CORE_SOURCES}")

idf_component_register(
    SRCS "magnetic-sensor.c" ${CORE_SOURCES}
    INCLUDE_DIRS "." "../../embedded-core"
)