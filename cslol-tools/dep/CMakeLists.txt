cmake_minimum_required(VERSION 3.20)

include(FetchContent)

cmake_policy(SET CMP0169 OLD)

# xxhash: 本地路径
set(XXHASH_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xxhash")
if(NOT EXISTS "${XXHASH_SOURCE_DIR}/xxhash.c")
    message(FATAL_ERROR "xxhash source not found at ${XXHASH_SOURCE_DIR}")
endif()
add_library(xxhash STATIC ${XXHASH_SOURCE_DIR}/xxhash.c)
target_include_directories(xxhash PUBLIC ${XXHASH_SOURCE_DIR})
target_compile_definitions(xxhash INTERFACE -DXXH_STATIC_LINKING_ONLY)
target_compile_definitions(xxhash PRIVATE -DXXH_CPU_LITTLE_ENDIAN)

# zstd: 本地路径
set(ZSTD_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zstd")
if(NOT EXISTS "${ZSTD_SOURCE_DIR}/lib")
    message(FATAL_ERROR "zstd source not found at ${ZSTD_SOURCE_DIR}")
endif()
file(GLOB zstd_SRCS
    ${ZSTD_SOURCE_DIR}/lib/common/*.c
    ${ZSTD_SOURCE_DIR}/lib/compress/*.c
    ${ZSTD_SOURCE_DIR}/lib/decompress/*.c
)
add_library(zstd STATIC ${zstd_SRCS})
target_include_directories(zstd PUBLIC ${ZSTD_SOURCE_DIR}/lib)
target_include_directories(zstd PRIVATE ${ZSTD_SOURCE_DIR}/lib/common)
target_compile_definitions(zstd INTERFACE -DZSTD_STATIC_LINKING_ONLY)
target_compile_definitions(zstd PRIVATE -DZSTD_DISABLE_ASM)
if(MSVC)
    target_compile_options(zstd PRIVATE /wd4267)
endif()

# miniz: 本地路径
set(MINIZ_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/miniz")
if(NOT EXISTS "${MINIZ_SOURCE_DIR}")
    message(FATAL_ERROR "miniz source not found at ${MINIZ_SOURCE_DIR}")
endif()
add_subdirectory(${MINIZ_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/miniz)
target_compile_definitions(miniz PRIVATE -DMINIZ_DISABLE_ZIP_READER_CRC32_CHECKS=1)

# 以下保持在线拉取（可根据需要改为本地）
FetchContent_Declare(
    libdeflate
    GIT_REPOSITORY https://github.com/ebiggers/libdeflate.git
    GIT_TAG        495fee110ebb48a5eb63b75fd67e42b2955871e2 # v1.18
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)
FetchContent_GetProperties(libdeflate)
if(NOT libdeflate_POPULATED)
    FetchContent_Populate(libdeflate)
    file(GLOB libdeflate_SRCS
        ${libdeflate_SOURCE_DIR}/lib/*.c
        ${libdeflate_SOURCE_DIR}/lib/arm/*.c
        ${libdeflate_SOURCE_DIR}/lib/x86/*.c
    )
    add_library(libdeflate STATIC ${libdeflate_SRCS})
    target_include_directories(libdeflate PUBLIC ${libdeflate_SOURCE_DIR})
    target_include_directories(libdeflate PRIVATE ${libdeflate_SOURCE_DIR}/lib)
endif()

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        a33701196adfad74917046096bf5a2aa0ab0bb50 # 9.1.0
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)
FetchContent_GetProperties(fmt)
if(NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
    add_library(fmt STATIC ${fmt_SOURCE_DIR}/src/format.cc ${fmt_SOURCE_DIR}/src/os.cc)
    target_include_directories(fmt PUBLIC ${fmt_SOURCE_DIR}/include)
    target_compile_definitions(fmt PRIVATE -DFMT_UNICODE=1)
endif()

FetchContent_Declare(
    fmtlog
    GIT_REPOSITORY https://github.com/MengRao/fmtlog.git
    GIT_TAG        5aea0764c820358ec1a842cd34cb6d494f65cfa1 # 2.2.1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)
FetchContent_GetProperties(fmtlog)
if(NOT fmtlog_POPULATED)
    FetchContent_Populate(fmtlog)
    add_library(fmtlog INTERFACE)
    target_include_directories(fmtlog INTERFACE ${fmtlog_SOURCE_DIR})
    target_link_libraries(fmtlog INTERFACE fmt)
endif()