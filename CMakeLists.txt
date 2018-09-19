cmake_minimum_required(VERSION 3.5)

macro(assert_binary_in_path name binary)
    find_program(${name} ${binary})
    if(NOT ${name})
        message(FATAL_ERROR "${binary} not found!")
    endif()
endmacro()

assert_binary_in_path(IDL_PROGRAM orbit-idl-2)
assert_binary_in_path(GIT_PROGRAM git)
assert_binary_in_path(APXS_PROGRAM apxs)
assert_binary_in_path(APR_PROGRAM apr-1-config)
assert_binary_in_path(DOXY_PROGRAM doxygen)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(module_name fred-mod-corba)
project(${module_name} C)

macro(set_default variable_name variable_value)
    if(NOT ${variable_name})
        set(${variable_name} ${variable_value})
    endif()
endmacro()

set_default(CMAKE_BUILD_TYPE "Release")
set(CMAKE_C_FLAGS_RELEASE "-O2")

set_default(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT VERSION)
    if(NOT GIT_PROGRAM)
        message(FATAL_ERROR "git not found; you need to pass -DVERSION=")
    endif()
    execute_process(COMMAND ${GIT_PROGRAM} "describe" "--tags"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE RAWVERSION)
    string(REGEX REPLACE "-.*\n" "" VERSION ${RAWVERSION})
endif()
set(TARBALL_FILE_NAME "fred-mod-corba-${VERSION}")

set_default(DISTRO_PREFIX "")

set_default(CONTEXT_NAME fred)
set_default(SVN_REV ${VERSION})
set_default(CONFIG_FILE_NAME 01-fred-mod-corba-apache.conf)
set_default(DATAROOTDIR ${CMAKE_INSTALL_PREFIX}/usr/share/)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

set_default(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR})
set_default(PROJECT ${module_name})
set_default(DOCDIR doc)
set_default(PERL_PATH "/usr/bin/perl")
set_default(HAVE_DOT "YES")
set_default(DOT_PATH "/usr/bin/dot")
set_default(GENERATE_MAN "NO")
set_default(GENERATE_RTF "NO")
set_default(GENERATE_XML "NO")
set_default(GENERATE_HTMLHELP "NO")
set_default(GENERATE_CHI "NO")
set_default(GENERATE_HTML "YES")
set_default(GENERATE_LATEX "NO")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxy.conf.cmake ${CMAKE_BINARY_DIR}/doxy.conf)

execute_process(COMMAND ${DOXY_PROGRAM} doxy.conf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

macro(store_flag_info key value)
    string(STRIP "${${value}}" cmake_sucks_${key})
    string(REPLACE " " ";" cmake_sucks_${key} "${cmake_sucks_${key}}")
    set(flag_info_${key} "${cmake_sucks_${key}}")
    message(STATUS "flag_info_${key}: ${flag_info_${key}}")
endmacro()

macro(store_include_info key value)
    string(STRIP "${${value}}" cmake_sucks_${key})
    string(REPLACE " " ";" cmake_sucks_${key} "${cmake_sucks_${key}}")
    set(include_info_${key} "${cmake_sucks_${key}}")
    message(STATUS "include_info_${key}: ${include_info_${key}}")
endmacro()

macro(store_linker_info key value)
    string(STRIP "${${value}}" cmake_sucks_${key})
    string(REPLACE " " ";" cmake_sucks_${key} "${cmake_sucks_${key}}")
    set(linker_info_${key} "${cmake_sucks_${key}}")
    message(STATUS "linker_info_${key}: ${linker_info_${key}}")
endmacro()

execute_process(COMMAND ${APXS_PROGRAM} "-q" "CFLAGS" OUTPUT_VARIABLE APXS_CFLAGS)
execute_process(COMMAND ${APXS_PROGRAM} "-q" "CPPFLAGS" OUTPUT_VARIABLE APXS_CPPFLAGS)
execute_process(COMMAND ${APXS_PROGRAM} "-q" "LDFLAGS" OUTPUT_VARIABLE APXS_LDFLAGS)
string(STRIP "${APXS_CFLAGS}" APXS_CFLAGS)
string(STRIP "${APXS_CPPFLAGS}" APXS_CPPFLAGS)
string(STRIP "${APXS_LDFLAGS}" APXS_LDFLAGS)
string(CONCAT APXS_FLAGS ${APXS_CFLAGS} " " ${APXS_CPPFLAGS} " " ${APXS_LDFLAGS})
store_flag_info(apxs APXS_FLAGS)

execute_process(COMMAND ${APXS_PROGRAM} "-q" "INCLUDEDIR" OUTPUT_VARIABLE APXS_INCLUDES)
store_include_info(apxs APXS_INCLUDES)

execute_process(COMMAND ${APXS_PROGRAM} "-q" "LIBS" OUTPUT_VARIABLE APXS_LIBS)
store_linker_info(apxs APXS_LIBS)

execute_process(COMMAND ${APXS_PROGRAM} "-q" "LIBEXECDIR" OUTPUT_VARIABLE APXS_MODULES)
string(STRIP ${APXS_MODULES} APXS_MODULES)
string(REGEX REPLACE "^/" "" APXS_MODULES ${APXS_MODULES})

set_default(MODULE_INSTALLDIR ${CMAKE_INSTALL_PREFIX}/${APXS_MODULES})

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/conf/${CONFIG_FILE_NAME}.in
    ${CMAKE_BINARY_DIR}/conf/${CONFIG_FILE_NAME})

execute_process(COMMAND ${APR_PROGRAM} "--cflags" OUTPUT_VARIABLE APR_CFLAGS)
execute_process(COMMAND ${APR_PROGRAM} "--cppflags" OUTPUT_VARIABLE APR_CPPFLAGS)
execute_process(COMMAND ${APR_PROGRAM} "--ldflags" OUTPUT_VARIABLE APR_LDFLAGS)
string(STRIP "${APR_CFLAGS}" APR_CFLAGS)
string(STRIP "${APR_CPPFLAGS}" APR_CPPFLAGS)
string(STRIP "${APR_LDFLAGS}" APR_LDFLAGS)
string(CONCAT APR_FLAGS ${APR_CFLAGS} " " ${APR_CPPFLAGS} " " ${APR_LDFLAGS})
store_flag_info(apr APR_FLAGS)

execute_process(COMMAND ${APR_PROGRAM} "--includedir" OUTPUT_VARIABLE APR_INCLUDES)
store_include_info(apr APR_INCLUDES)

execute_process(COMMAND ${APR_PROGRAM} "--libs" OUTPUT_VARIABLE APR_LIBS)
store_linker_info(apr APR_LIBS)


pkg_search_module(ORBIT2 REQUIRED ORBit-2.0)
store_flag_info(orbit2 ORBIT2_CFLAGS)
store_include_info(orbit2 ORBIT2_INCLUDE_DIRS)
store_linker_info(orbit2 ORBIT2_LIBRARIES)


function(set_common_properties_on_targets)
foreach(target_name ${ARGN})
    message(STATUS "setting properties on: " ${target_name})
    set_target_properties(${target_name} PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    target_compile_options(${target_name}
        PUBLIC "-ggdb"
        PUBLIC "-grecord-gcc-switches"
        PUBLIC "-Wall"
        PUBLIC "-Wextra"
        PUBLIC "-fdiagnostics-color=auto"
        PUBLIC "-Wno-unused-parameter"
        PUBLIC "-fPIC"
        PUBLIC "-DHAVE_CONFIG_H")
    target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_include_directories(${target_name} PUBLIC ${CMAKE_BINARY_DIR})
endforeach()
endfunction()

function(target_link_external_libraries target_name)
foreach(library_name ${ARGN})
    target_link_libraries(${target_name} ${linker_info_${library_name}})
endforeach()
endfunction()

function(target_include_external_libraries target_name)
foreach(library_name ${ARGN})
    target_include_directories(${target_name} PUBLIC ${include_info_${library_name}})
endforeach()
endfunction()

function(target_add_flags_for_external_libraries target_name)
foreach(library_name ${ARGN})
    foreach(library_name ${flag_info_${library_name}})
        target_compile_options(${target_name} PRIVATE ${library_name})
    endforeach()
endforeach()
endfunction()


add_library(${module_name} SHARED
    mod_corba.c)
target_include_external_libraries(${module_name}
    apr
    apxs
    orbit2)
target_link_external_libraries(${module_name}
    apr
    apxs
    orbit2)
target_add_flags_for_external_libraries(${module_name}
    apr
    apxs
    orbit2)

set_common_properties_on_targets(
    ${module_name})

install(TARGETS ${module_name} LIBRARY DESTINATION ${APXS_MODULES})
install(DIRECTORY ${CMAKE_BINARY_DIR}/conf/ DESTINATION ${DISTRO_PREFIX}share/fred-mod-corba FILES_MATCHING PATTERN "*.conf")
install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/html/ DESTINATION ${DISTRO_PREFIX}share/doc/fred-mod-corba FILES_MATCHING PATTERN "*")

add_custom_target(uninstall_module COMMAND rm ${CMAKE_INSTALL_PREFIX}/${APXS_MODULES}/lib${module_name}.so)
add_custom_target(uninstall_configuration COMMAND rm ${CMAKE_INSTALL_PREFIX}/${DISTRO_PREFIX}share/fred-mod-corba/${CONFIG_FILE_NAME})
add_custom_target(uninstall_doc COMMAND rm -rf ${CMAKE_INSTALL_PREFIX}/${DISTRO_PREFIX}share/doc/fred-mod-corba)
add_custom_target(uninstall DEPENDS uninstall_module uninstall_configuration uninstall_doc)

if(EXISTS ${CMAKE_SOURCE_DIR}/.git AND GIT_PROGRAM)
    if(NOT TARGET dist)
        add_custom_target(dist
            COMMAND ${GIT_PROGRAM} archive --format=tar --prefix=${TARBALL_FILE_NAME}/ HEAD | gzip > ${CMAKE_BINARY_DIR}/${TARBALL_FILE_NAME}.tar.gz
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    endif()

    if(NOT TARGET distcheck)
        set(DISTCHECK_BUILD_DIR "_build")
        set(DISTCHECK_INSTALL_DIR "_inst")
        if(${CMAKE_GENERATOR} STREQUAL "Ninja")
            set(DISTCHECK_BUILD_CMD "ninja")
            set(DISTCHECK_MAKEFLAGS "")
        else()
            set(DISTCHECK_BUILD_CMD "make")
            execute_process(COMMAND "grep" "-c" "model name" "/proc/cpuinfo"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE NOCPUS)
            string(REGEX REPLACE "\n" "" NOCPUS ${NOCPUS})
            set(DISTCHECK_MAKEFLAGS "MAKEFLAGS=-j${NOCPUS}")
        endif()
        add_custom_target(distcheck
            COMMAND
            rm -rf ${DISTCHECK_BUILD_DIR} ${DISTCHECK_INSTALL_DIR} ${TARBALL_FILE_NAME}
            && mkdir ${DISTCHECK_BUILD_DIR} ${DISTCHECK_INSTALL_DIR}
            && (tar xzf ${TARBALL_FILE_NAME}.tar.gz
                || (echo 'Error: failed to unpack the archive' 2>&1\; false))
            && (cd ${DISTCHECK_BUILD_DIR}
                && (cmake -G${CMAKE_GENERATOR} -DCMAKE_INSTALL_PREFIX=../${DISTCHECK_INSTALL_DIR} ../${TARBALL_FILE_NAME}
                    || (echo 'Error: failed to configure the archive' 2>&1\; false))
                && (${DISTCHECK_MAKEFLAGS} ${DISTCHECK_BUILD_CMD}
                    || (echo 'Error: the build target failed ' 2>&1\; false))
                && (${DISTCHECK_BUILD_CMD} install
                    || (echo 'Error: the install target failed' 2>&1\; false))
                && (${DISTCHECK_BUILD_CMD} uninstall
                    || (echo 'Error: the uninstall target failed' 2>&1\; false))
                && (${DISTCHECK_BUILD_CMD} clean
                    || (echo 'Error: the clean target failed' 2>&1\; false)))
            && rm -rf ${DISTCHECK_BUILD_DIR} ${DISTCHECK_INSTALL_DIR} ${TARBALL_FILE_NAME}
            && echo 'PASS: ' ${TARBALL_FILE_NAME}.tar.gz 'is ready for distribution.'
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Checking the distribution archive...")
        add_dependencies(distcheck dist)
    endif()
else()
    message(STATUS "no git -- not including dist and distcheck targets")
endif()