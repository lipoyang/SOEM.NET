
set(SOURCES soemlib.c soemlib.def)
add_library(soemlib SHARED ${SOURCES})
target_link_libraries(soemlib soem)
install(TARGETS soemlib DESTINATION lib)
