set(DLL_NAME ann)

file(GLOB_RECURSE SOURCES ./*.cpp)
file(GLOB_RECURSE HEADERS ./*.h)

add_library(${DLL_NAME} SHARED ${SOURCES} ${HEADERS})

target_include_directories(${DLL_NAME} PUBLIC "include")

target_compile_definitions(${DLL_NAME} PUBLIC 
    DLL_EXPORTS
)