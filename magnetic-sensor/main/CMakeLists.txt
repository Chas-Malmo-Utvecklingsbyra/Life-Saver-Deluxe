file(GLOB_RECURSE CORE_SOURCES "../../embedded-core/*.c")

idf_component_register(SRCS "magnetic-sensor.c" ${CORE_SOURCES}
                    INCLUDE_DIRS "." "../../embedded-core")
