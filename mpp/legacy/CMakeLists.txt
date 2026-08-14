# vim: syntax=cmake
include_directories(.)

# ----------------------------------------------------------------------------
# add mpp implement
# ----------------------------------------------------------------------------
set (MPP_LEGACY_SRC
    vpu.c
    vpu_api.cpp
    vpu_api_legacy.cpp
    vpu_api_mlvec.cpp
    vpu_mem_legacy.c
    rk_list.cpp
    )

set(VPU_VERSION "0")
set(VPU_ABI_VERSION "1")

# NOTE: link_directories should be before add_library or add_executable
link_directories(${CMAKE_BINARY_DIR}/mpp)

add_library(${VPU_STATIC} STATIC ${MPP_LEGACY_SRC})
set_target_properties(${VPU_STATIC} PROPERTIES
    FOLDER "mpp/legacy"
    CLEAN_DIRECT_OUTPUT 1
    OUTPUT_NAME "${VPU_SHARED}"
    POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(${VPU_STATIC} PUBLIC ${LIBM} ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

if(BUILD_SHARED_LIBS)
    add_library(${VPU_SHARED} SHARED ${MPP_LEGACY_SRC})
    add_dependencies(${VPU_SHARED} ${MPP_SHARED})
    set_target_properties(${VPU_SHARED} PROPERTIES FOLDER "mpp/legacy")
    set_target_properties(${VPU_SHARED} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
    set_target_properties(${VPU_SHARED} PROPERTIES C_VISIBILITY_PRESET default)
    set_target_properties(${VPU_SHARED} PROPERTIES CXX_VISIBILITY_PRESET default)
    if(NOT ANDROID)
        set_target_properties(${VPU_SHARED} PROPERTIES VERSION ${VPU_VERSION})
        set_target_properties(${VPU_SHARED} PROPERTIES SOVERSION ${VPU_ABI_VERSION})
    endif()
    target_link_libraries(${VPU_SHARED} ${ASAN_LIB} -Wl,-Bdynamic dl lib${MPP_SHARED}.so)
    install(TARGETS ${VPU_SHARED} LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()
install(TARGETS ${VPU_STATIC} ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
