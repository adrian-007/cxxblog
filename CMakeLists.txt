cmake_minimum_required(VERSION 3.8)
project(cxxblog)

set(CMAKE_CXX_STANDARD 17)

include(ExternalProject)
set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/deps)

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
find_library(LIBWTDBO_POSTGRES   NAMES wtdbopostgresd wtdbopostgresd)
find_library(LIBWTDBO_MYSQL      NAMES wtdbomysqld wtdbomysql)

if (NOT LIBWT OR NOT LIBWTHTTP OR NOT LIBWTDBO OR (NOT LIBWTDBO_SQLITE AND NOT LIBWTDBO_POSTGRES AND LIBWTDBO_MYSQL))
    message(FATAL_ERROR "Cannot find Wt libraries")
endif()

add_executable(cxxblog ${SOURCES})
target_include_directories(cxxblog PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(cxxblog
    ${LIBWT}
    ${LIBWTHTTP}
    ${LIBWTDBO}
    libcmark
)

if (LIBWTDBO_SQLITE)
    target_link_libraries(cxxblog ${LIBWTDBO_SQLITE})
endif()

if (LIBWTDBO_POSTGRES)
    target_link_libraries(cxxblog ${LIBWTDBO_POSTGRES})
endif()

if (LIBWTDBO_MYSQL)
    target_link_libraries(cxxblog ${LIBWTDBO_MYSQL})
endif()

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
    add_custom_target(copy_${RESOURCE_COPY_TARGET_NAME} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE})
    add_dependencies(cxxblog copy_${RESOURCE_COPY_TARGET_NAME})
endforeach()
