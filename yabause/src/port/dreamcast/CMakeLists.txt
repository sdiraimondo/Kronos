project(kronos-dc)

if(NOT dreamcast)
    return()
endif(NOT dreamcast)

enable_language(ASM-ATT)
include_directories(${PORT_INCLUDE_DIRS})
add_definitions(${PORT_CFLAGS})

set(kronos_dc_SOURCES
    cd.s
    localtime.c
    perdc.c
    viddc.c
    yui.c)

set(kronos_dc_HEADERS
    localtime.h
    perdc.h
    viddc.h)

link_directories(..)
add_executable(kronos-dc ${yabause_dc_SOURCES})
set_target_properties(kronos-dc PROPERTIES OUTPUT_NAME kronos.elf)
target_link_libraries(kronos-dc ${YABAUSE_LIBRARIES})
target_link_libraries(kronos-dc ${PORT_LIBRARIES})
target_link_libraries(kronos-dc kronos)
target_link_libraries(kronos-dc m)
