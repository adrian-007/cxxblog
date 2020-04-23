include(FetchContent)

FetchContent_Declare(
    cmark-gfm
    GIT_REPOSITORY    https://github.com/github/cmark-gfm.git
    GIT_TAG           0.29.0.gfm.0
)

FetchContent_GetProperties(cmark-gfm)

if (NOT cmark_gfm_POPULATED)
    # cmark configuration is, well, crappy. It always builds executable and there's no way to disable it,
    # when used with add_subdirectory it installs all dependencies just as if it was a standalone build.
    # Given that it creates lots of problems, it's easier to create minimal build configuration for
    # existing sources and go from there.
    # As a bonus we can create actual cmark extension since installed headers are missing some crucial
    # symbols needed to register an extension.

    FetchContent_Populate(cmark-gfm)

    include(GenerateExportHeader)
    include(CheckIncludeFile)
    include(CheckCSourceCompiles)
    include(CheckCSourceRuns)
    include(CheckSymbolExists)

    set(PROJECT_VERSION_MAJOR 0)
    set(PROJECT_VERSION_MINOR 29)
    set(PROJECT_VERSION_PATCH 0)
    set(PROJECT_VERSION_GFM 0)
    set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.gfm.${PROJECT_VERSION_GFM})

    set(LIBCMARK_SRC
        ${cmark-gfm_SOURCE_DIR}/src/cmark.c
        ${cmark-gfm_SOURCE_DIR}/src/node.c
        ${cmark-gfm_SOURCE_DIR}/src/iterator.c
        ${cmark-gfm_SOURCE_DIR}/src/blocks.c
        ${cmark-gfm_SOURCE_DIR}/src/inlines.c
        ${cmark-gfm_SOURCE_DIR}/src/scanners.c
        ${cmark-gfm_SOURCE_DIR}/src/scanners.re
        ${cmark-gfm_SOURCE_DIR}/src/utf8.c
        ${cmark-gfm_SOURCE_DIR}/src/buffer.c
        ${cmark-gfm_SOURCE_DIR}/src/references.c
        ${cmark-gfm_SOURCE_DIR}/src/footnotes.c
        ${cmark-gfm_SOURCE_DIR}/src/map.c
        ${cmark-gfm_SOURCE_DIR}/src/render.c
        ${cmark-gfm_SOURCE_DIR}/src/man.c
        ${cmark-gfm_SOURCE_DIR}/src/xml.c
        ${cmark-gfm_SOURCE_DIR}/src/html.c
        ${cmark-gfm_SOURCE_DIR}/src/commonmark.c
        ${cmark-gfm_SOURCE_DIR}/src/plaintext.c
        ${cmark-gfm_SOURCE_DIR}/src/latex.c
        ${cmark-gfm_SOURCE_DIR}/src/houdini_href_e.c
        ${cmark-gfm_SOURCE_DIR}/src/houdini_html_e.c
        ${cmark-gfm_SOURCE_DIR}/src/houdini_html_u.c
        ${cmark-gfm_SOURCE_DIR}/src/cmark_ctype.c
        ${cmark-gfm_SOURCE_DIR}/src/arena.c
        ${cmark-gfm_SOURCE_DIR}/src/linked_list.c
        ${cmark-gfm_SOURCE_DIR}/src/syntax_extension.c
        ${cmark-gfm_SOURCE_DIR}/src/registry.c
        ${cmark-gfm_SOURCE_DIR}/src/plugin.c
    )

    check_include_file(stdbool.h HAVE_STDBOOL_H)
    check_c_source_compiles("int main() { __builtin_expect(0,0); return 0; }" HAVE___BUILTIN_EXPECT)
    check_c_source_compiles("int f(void) __attribute__ (()); int main() { return 0; }" HAVE___ATTRIBUTE__)

    configure_file(${cmark-gfm_SOURCE_DIR}/src/cmark-gfm_version.h.in ${cmark-gfm_SOURCE_DIR}/src/cmark-gfm_version.h)
    configure_file(${cmark-gfm_SOURCE_DIR}/src/config.h.in ${cmark-gfm_SOURCE_DIR}/src/config.h)

    add_library(libcmark SHARED ${LIBCMARK_SRC})
    target_include_directories(libcmark PUBLIC ${cmark-gfm_SOURCE_DIR}/src)
    generate_export_header(libcmark BASE_NAME cmark-gfm EXPORT_FILE_NAME ${cmark-gfm_SOURCE_DIR}/src/cmark-gfm_export.h)

    set(LIBCMARK_GFM_SRC
        ${cmark-gfm_SOURCE_DIR}/extensions/core-extensions.c
        ${cmark-gfm_SOURCE_DIR}/extensions/table.c
        ${cmark-gfm_SOURCE_DIR}/extensions/strikethrough.c
        ${cmark-gfm_SOURCE_DIR}/extensions/autolink.c
        ${cmark-gfm_SOURCE_DIR}/extensions/tagfilter.c
        ${cmark-gfm_SOURCE_DIR}/extensions/ext_scanners.c
        ${cmark-gfm_SOURCE_DIR}/extensions/ext_scanners.re
        ${cmark-gfm_SOURCE_DIR}/extensions/ext_scanners.h
        ${cmark-gfm_SOURCE_DIR}/extensions/tasklist.c
    )

    add_library(libcmark-gfm SHARED ${LIBCMARK_GFM_SRC})
    set_property(TARGET libcmark-gfm PROPERTY C_STANDARD 99)
    target_include_directories(libcmark-gfm PUBLIC ${cmark-gfm_SOURCE_DIR}/extensions)
    generate_export_header(libcmark-gfm BASE_NAME cmark-gfm-extensions EXPORT_FILE_NAME ${cmark-gfm_SOURCE_DIR}/src/cmark-gfm-extensions_export.h)

    target_link_libraries(libcmark-gfm libcmark)

endif()
