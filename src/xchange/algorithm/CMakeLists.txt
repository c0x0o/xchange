set(ALGORITHM_SOURCE "./LockFreeQueue.h" "./RedBlackTree.h")
set(ALGORITHM_HEADER "./LockFreeQueue.h" "./RedBlackTree.h")

add_library(algorithm ${ALGORITHM_SOURCE})
target_link_libraries(algorithm pthread)
set_target_properties(algorithm PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS algorithm DESTINATION lib/xchange)
install(FILES ${ALGORITHM_HEADER} DESTINATION include/xchange/algorithm)
