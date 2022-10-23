set(EXC_NAME mmr)

set(ROOT_FILES 
    "src/main.cpp"
)

set(UTIL 
    "include/util.h"
    "include/settings.h"
)

set(MMR_VIEWER 
    "include/mmr_viewer.h"
    "src/mmr_viewer.cpp"
)

set(DATABASE
    "include/database.h"
    "src/database.cpp"
    "include/database_gui.h"
    "src/database_gui.cpp"
)

set(NORMALIZATION
    "include/normalization.h"
    "src/normalization.cpp"
)

set(DESCRIPTORS
    "include/descriptors.h"
    "src/descriptors.cpp"
)

set(FEATURE_VECTOR
    "include/feature_vector.h"
    "src/feature_vector.cpp"
    "include/emd.h"
    "src/emd.c"
)

add_executable(${EXC_NAME} 
    ${ROOT_FILES}
    ${UTIL}
    ${MMR_VIEWER}
    ${DATABASE}
    ${NORMALIZATION}
    ${DESCRIPTORS}
    ${FEATURE_VECTOR}
)

source_group(// FILES ${ROOT_FILES})
source_group(util FILES ${UTIL})
source_group(pipeline FILES ${NORMALIZATION} ${DESCRIPTORS} ${FEATURE_VECTOR})
source_group(viewer FILES ${MMR_VIEWER})
source_group(database FILES ${DATABASE})

target_include_directories(${EXC_NAME} PUBLIC
    "include"
)

target_link_libraries(${EXC_NAME}
    pmp_vis
)

target_compile_definitions(${EXC_NAME} PUBLIC 
    MMR_DATA_DIR=${MMR_DATA_DIR}
    MMR_EXPORT_DIR=${MMR_EXPORT_DIR}
)
