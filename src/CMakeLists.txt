add_executable(${PROJECT_NAME}Standalone driver.cpp)
add_library(${PROJECT_NAME} SHARED plugin.cpp)

foreach(target ${PROJECT_NAME} ${PROJECT_NAME}Standalone)
  target_link_libraries(${target}
    PRIVATE
    clangAST
    clangBasic
    clangFrontend
    clangSerialization
    clangTooling
    clangIndex
    clangRewrite
  )
endforeach()

install(
  TARGETS ${PROJECT_NAME}
  DESTINATION lib
  EXPORT ScopeChecker
)

install(
  TARGETS ${PROJECT_NAME}Standalone
  DESTINATION bin
  EXPORT ScopeChecker
)

install(
  EXPORT ScopeChecker
  FILE ScopeChecker.cmake
  NAMESPACE ScopeChecker::
  DESTINATION lib/cmake/ScopeChecker
)

