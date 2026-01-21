# local
set(STATIC_LIBRARY "${PROJECT_NAME}-lib")
set(LIBRARY_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/include")

# export
set(STATIC_LIBRARY ${STATIC_LIBRARY} PARENT_SCOPE)
set(LIBRARY_INCLUDE_PATH "${LIBRARY_INCLUDE_PATH}" PARENT_SCOPE)


add_library(${STATIC_LIBRARY} STATIC
    "src/Utils.cpp"
    "src/IPC/SemaphoreArray.cpp"
    "src/IPC/Semaphore.cpp"
    "src/IPC/Signal.cpp"
    "src/Process.cpp"
    "src/File.cpp"
    "src/PredefinedMQ.cpp"
    "src/AttractionHandler.cpp"
    "src/SimulationData.cpp"
)

target_compile_features(${STATIC_LIBRARY} PRIVATE cxx_std_20)
target_compile_definitions(${STATIC_LIBRARY} PRIVATE _BUILD)
target_include_directories(${STATIC_LIBRARY} PUBLIC 
    ${LIBRARY_INCLUDE_PATH}
)

# set up tests
add_subdirectory("test")