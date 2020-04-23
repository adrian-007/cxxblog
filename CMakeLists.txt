cmake_minimum_required(VERSION 3.14)
project(cxxblog)

set(CMAKE_CXX_STANDARD 17)

set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/deps)

include(GNUInstallDirs)
include(ExternalProject)
include(cmake/cmark-gfm.cmake)

set(SOURCES
    src/Application.cpp
    src/AttachmentCache.cpp
    src/AttachmentCache.h
    src/AttachmentIconResource.cpp
    src/AttachmentIconResource.h
    src/AttachmentResource.cpp
    src/AttachmentResource.h
    src/AvatarGenerator.cpp
    src/AvatarGenerator.h
    src/AvatarResource.cpp
    src/AvatarResource.h
    src/ExpressionParser.cpp
    src/ExpressionParser.h
    src/main.cpp
    src/Markdown.cpp
    src/models/Attachment.cpp
    src/models/BasicSession.cpp
    src/models/BasicSession.h
    src/models/ConfigStore.cpp
    src/models/ConfigStore.h
    src/models/EditorContactDetail.cpp
    src/models/EditorContactDetail.h
    src/models/Editor.cpp
    src/models/EditorJobOffer.cpp
    src/models/EditorJobOffer.h
    src/models/EditorResume.cpp
    src/models/Post.cpp
    src/models/PostDraft.cpp
    src/models/Session.cpp
    src/NotificationDialog.cpp
    src/views/EditorView.cpp
    src/views/EditorView.h
    src/views/JobOffersView.cpp
    src/views/JobOffersView.h
    src/views/LoadingWidget.cpp
    src/views/LoadingWidget.h
    src/views/MainView.cpp
    src/views/ManageAttachmentsDialog.cpp
    src/views/ManageAttachmentsDialog.h
    src/views/PostsListView.cpp
    src/views/PostView.cpp
    src/views/settings/EditorChangeCredentialsModels.cpp
    src/views/settings/EditorChangeCredentialsModels.h
    src/views/settings/EditorContactDetailModels.cpp
    src/views/settings/EditorContactDetailModels.h
    src/views/settings/EditorContactDetailsWidget.cpp
    src/views/settings/EditorContactDetailsWidget.h
    src/views/settings/EditorPersonalInformationModels.cpp
    src/views/settings/EditorPersonalInformationModels.h
    src/views/settings/SettingsTabEditor.cpp
    src/views/SettingsView.cpp
)

find_library(LIBWT               NAMES wtd wt)
find_library(LIBWTHTTP           NAMES wthttpd wthttp)
find_library(LIBWTDBO            NAMES wtdbod wtdbo)
find_library(LIBWTDBO_SQLITE     NAMES wtdbosqlite3d wtdbosqlite3)
find_library(LIBWTDBO_POSTGRES   NAMES wtdbopostgresd wtdbopostgres)
find_library(LIBWTDBO_MYSQL      NAMES wtdbomysqld wtdbomysql)

if (NOT LIBWT OR NOT LIBWTHTTP OR NOT LIBWTDBO OR (NOT LIBWTDBO_SQLITE AND NOT LIBWTDBO_POSTGRES AND LIBWTDBO_MYSQL))
    message(FATAL_ERROR "Cannot find Wt libraries")
endif()

find_package(Boost REQUIRED COMPONENTS system)

add_executable(cxxblog ${SOURCES})
target_include_directories(cxxblog PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(cxxblog
    PUBLIC
    ${LIBWT}
    ${LIBWTHTTP}
    ${LIBWTDBO}
    ${Boost_LIBRARIES}
    stdc++fs

    PRIVATE
    cmark
    cmark-gfm
)

if (LIBWTDBO_SQLITE)
    message(" -- Adding SQLite3 Wt::Dbo backend: ${LIBWTDBO_SQLITE}")
    target_link_libraries(cxxblog PUBLIC ${LIBWTDBO_SQLITE})
endif()

if (LIBWTDBO_POSTGRES)
    message(" -- Adding PostgreSQL Wt::Dbo backend: ${LIBWTDBO_POSTGRES}")
    target_link_libraries(cxxblog PUBLIC ${LIBWTDBO_POSTGRES})
endif()

if (LIBWTDBO_MYSQL)
    message(" -- Adding MySQL Wt::Dbo backend: ${LIBWTDBO_MYSQL}")
    target_link_libraries(cxxblog PUBLIC ${LIBWTDBO_MYSQL})
endif()

# TODO:
# 1. Break down resources to approot and docroot target resources.
# 2. Write proper targets that could be used in install process.

set(STATIC_RESOURCES
    wt_config.xml

    xml/strings.xml
    xml/views.xml

    assets/css/cxxblog.css
    assets/css/highlight/default.min.css
    assets/css/highlight/vs.min.css

    assets/js/highlight.min.js
    assets/js/cxxblog.js

    assets/font-awesome
    resources
)

foreach(RESOURCE ${STATIC_RESOURCES})
    message("--  Adding copy command to ${RESOURCE}")

    if (IS_DIRECTORY ${RESOURCE})
        set(__target_command copy_directory)
    else()
        set(__target_command copy)
    endif()

    add_custom_command(
        OUTPUT             ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE}
        COMMAND            ${CMAKE_COMMAND} -E ${__target_command} ${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE} ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE}
        MAIN_DEPENDENCY    ${RESOURCE}
        COMMENT            "Updating ${RESOURCE}"
    )

    string(REPLACE "/" "_" RESOURCE_COPY_TARGET_NAME ${RESOURCE})
    add_custom_target(${__target_command}_${RESOURCE_COPY_TARGET_NAME} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE})
    add_dependencies(cxxblog ${__target_command}_${RESOURCE_COPY_TARGET_NAME})
endforeach()

set(CXXBLOG_INSTALL_APPROOT cxxblog/approot)
set(CXXBLOG_INSTALL_DOCROOT cxxblog/docroot)

# Application files and resources, private.
install(
    TARGETS cxxblog
    DESTINATION cxxblog
)

install(
    DIRECTORY xml
    DESTINATION ${CXXBLOG_INSTALL_APPROOT}
)

install(
    FILES
    wt_config.xml
    DESTINATION ${CXXBLOG_INSTALL_APPROOT}
    RENAME wt_config.xml.sample
)

# Static resources available for everyone.
install(
    DIRECTORY assets
    DESTINATION ${CXXBLOG_INSTALL_DOCROOT}
)

install(
    DIRECTORY resources
    DESTINATION ${CXXBLOG_INSTALL_DOCROOT}
)
