get_directory_property(CURRENT_EP_PREFIX EP_PREFIX)
set(CMAKR_INSTALL_PREFIX ${CURRENT_EP_PREFIX}/install/cmark_gfm)

ExternalProject_Add(cmark_gfm
    GIT_REPOSITORY    https://github.com/github/cmark-gfm.git
    GIT_TAG           0.29.0.gfm.0
    CMAKE_CACHE_ARGS
    -DCMARK_TESTS:BOOL=OFF
    -DCMARK_STATIC:BOOL=OFF
    -DCMARK_LIB_FUZZER:BOOL=OFF
    -DCMAKE_BUILD_TYPE:STRING=RelWithDbgInfo
    -DCMAKE_INSTALL_PREFIX:STRING=${CMAKR_INSTALL_PREFIX}
)

add_library(libcmark INTERFACE)
add_dependencies(libcmark cmark_gfm)

target_include_directories(libcmark INTERFACE ${CMAKR_INSTALL_PREFIX}/include)
target_link_directories(libcmark INTERFACE ${CMAKR_INSTALL_PREFIX}/lib)
target_link_libraries(libcmark INTERFACE cmark-gfm cmark-gfm-extensions)
