set(DESIGN_SOURCE "./Noncopyable.h" "./Singleton.h")
set(DESIGN_HEADER "./Noncopyable.h" "./Singleton.h")

add_library(design ${DESIGN_SOURCE})
set_target_properties(design PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS design DESTINATION lib/xchange)
install(FILES ${DESIGN_HEADER} DESTINATION include/xchange/design)
