FetchContent_Declare(gtest
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG     v1.17.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(gtest)