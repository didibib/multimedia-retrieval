set(EXC_NAME mmr)

set(ROOT_FILES 
    "src/main.cpp"
)

set(UTIL 
    "include/util.h"
)

set(MMR_VIEWER 
    "include/mmr_viewer.h"
    "src/mmr_viewer.cpp"
)

add_executable(${EXC_NAME} 
    ${ROOT_FILES}
    ${UTIL}
    ${MMR_VIEWER}
)

source_group(// FILES ${ROOT_FILES})
source_group(util FILES ${UTIL})
source_group(viewer FILES ${MMR_VIEWER})

target_include_directories(${EXC_NAME} PUBLIC
    "include"
)

target_link_libraries(${EXC_NAME}
    pmp_vis
)

target_compile_definitions(${EXC_NAME} PUBLIC 
    MMR_ASSETS_DIR=${MMR_ASSETS_DIR}
)

if (EMSCRIPTEN)
    set_target_properties(myviewer PROPERTIES LINK_FLAGS "--shell-file ${PROJECT_SOURCE_DIR}/external/pmp-library/src/apps/data/shell.html --preload-file ${PROJECT_SOURCE_DIR}/external/pmp-library/external/pmp-data/off/bunny.off@input.off")
endif()