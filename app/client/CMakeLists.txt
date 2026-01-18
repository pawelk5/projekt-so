# local
set(EXEC_NAME "park-client")
set(LIBRARY_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/include")

add_executable(${EXEC_NAME}
    "src/client.cpp"
    "src/ClientProc/ClientProc.cpp"
    "src/ClientProc/ClientPark.cpp"
    "src/ClientProc/ClientAttraction.cpp"
    "src/ClientProc/ClientRestaurant.cpp"
)

target_compile_features(${EXEC_NAME} PRIVATE cxx_std_20)

target_include_directories(${EXEC_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/src" 
    ${LIBRARY_INCLUDE_PATH}
)

target_link_libraries(${EXEC_NAME} PRIVATE
    ${PROJECT_NAME}-lib
)