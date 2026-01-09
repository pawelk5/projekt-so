# local
set(EXEC_NAME "park-main")
set(LIBRARY_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/include")

add_executable(${EXEC_NAME}
    "src/main.cpp"
    "src/MainProc/MainProc.cpp"
    "src/LoggerService/LoggerService.cpp"
)

target_compile_features(${EXEC_NAME} PRIVATE cxx_std_20)

target_include_directories(${EXEC_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/src" 
    ${LIBRARY_INCLUDE_PATH}
)

target_link_libraries(${EXEC_NAME} PRIVATE
    ${PROJECT_NAME}-lib
)