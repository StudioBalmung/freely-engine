cmake_minimum_required(VERSION 3.22)
project(FreelyEngine VERSION 0.4.4 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Convenience root 
set(FREELY_ROOT    ${CMAKE_CURRENT_SOURCE_DIR})
set(THIRDPARTY_DIR ${FREELY_ROOT}/ThirdParty)

# Options
option(FREELY_BUILD_EDITOR   "Build the Freely Editor"          ON)
option(FREELY_BUILD_EXAMPLES "Build example applications"       ON)
option(FREELY_BUILD_RUNTIME  "Build standalone runtime"         ON)
option(FREELY_USE_VULKAN     "Enable Vulkan backend"            ON)
option(FREELY_SCRIPTING_LUA  "Enable Lua scripting"             ON)
option(FREELY_SCRIPTING_MONO "Enable C#/Mono scripting"         OFF)
option(FREELY_AUDIO_OPENAL   "Enable OpenAL audio backend"      ON)
option(FREELY_AUDIO_SDL      "Enable SDL audio backend"         ON)
option(FREELY_USE_BGFX       "Enable bgfx render backend"       OFF)
option(FREELY_USE_MIMALLOC   "Use mimalloc as global allocator" ON)

# Suppress warnings in third-party code 
# Prevents noisy build output from vendored libraries.
if(MSVC)
    add_compile_options(/W0)
endif()

# FetchContent - only for libs NOT vendored 
include(FetchContent)

# EnTT (ECS) - vendored in ThirdParty
add_subdirectory(${THIRDPARTY_DIR}/EnTT EXCLUDE_FROM_ALL)

# nlohmann/json - vendored in ThirdParty
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRDPARTY_DIR}/Json EXCLUDE_FROM_ALL)

# Vendored: GLM 
add_subdirectory(${THIRDPARTY_DIR}/glm EXCLUDE_FROM_ALL)

# Vendored: GLFW 
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRDPARTY_DIR}/glfw EXCLUDE_FROM_ALL)

# Vendored: spdlog 
set(SPDLOG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRDPARTY_DIR}/spdlog EXCLUDE_FROM_ALL)

# Vendored: stb (header-only) 
add_library(stb_headers INTERFACE)
target_include_directories(stb_headers INTERFACE ${THIRDPARTY_DIR}/stb)

# Vendored: tinyobjloader 
set(TINYOBJLOADER_BUILD_TEST_LOADER OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRDPARTY_DIR}/tinyobjloader EXCLUDE_FROM_ALL)

# Vendored: miniaudio (single-header) 
add_library(miniaudio_headers INTERFACE)
target_include_directories(miniaudio_headers INTERFACE ${THIRDPARTY_DIR}/miniaudio)

# Vendored: Jolt Physics 
if(EXISTS ${THIRDPARTY_DIR}/Jolt/CMakeLists.txt)
    set(JOLT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(${THIRDPARTY_DIR}/Jolt EXCLUDE_FROM_ALL)
endif()

# Vendored: Box2D 
set(BOX2D_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_DOCS    OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRDPARTY_DIR}/Box2D EXCLUDE_FROM_ALL)

# Vendored: glad (generated glad2 output) 
# The ThirdParty/glad2 folder contains the glad Python generator itself.
# The actual pre-generated glad C/H files live in ThirdParty/OpenGL.
# Use the OpenGL vendored glad if present, otherwise generate a minimal one inline.
if(EXISTS ${THIRDPARTY_DIR}/OpenGL/src/glad.c)
    add_library(glad STATIC
        ${THIRDPARTY_DIR}/OpenGL/src/glad.c
    )
    target_include_directories(glad PUBLIC
        ${THIRDPARTY_DIR}/OpenGL/include
    )
else()
    # Use Box2D's vendored glad loader as it is already pre-generated and avoids Jinja2 python dependency
    add_library(glad STATIC
        ${THIRDPARTY_DIR}/Box2D/extern/glad/src/glad.c
    )
    target_include_directories(glad PUBLIC
        ${THIRDPARTY_DIR}/Box2D/extern/glad/include
    )
endif()

# Vendored: ImGui (docking branch, vendored in Thirdparty/ImGui) 
set(IMGUI_DIR ${THIRDPARTY_DIR}/ImGui)
if(EXISTS ${IMGUI_DIR}/imgui.cpp)
    add_library(imgui STATIC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC glfw glad)
endif()

# Vendored: ImGuizmo 
set(IMGUIZMO_DIR ${THIRDPARTY_DIR}/ImGuizmo)
if(EXISTS ${IMGUIZMO_DIR}/ImGuizmo.cpp AND TARGET imgui)
    add_library(imguizmo STATIC ${IMGUIZMO_DIR}/ImGuizmo.cpp)
    target_include_directories(imguizmo PUBLIC ${IMGUIZMO_DIR})
    target_link_libraries(imguizmo PUBLIC imgui)
endif()

# Vendored: mimalloc 
if(FREELY_USE_MIMALLOC)
    set(MI_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(MI_BUILD_SHARED   OFF CACHE BOOL "" FORCE)
    set(MI_BUILD_STATIC   ON  CACHE BOOL "" FORCE)
    set(MI_BUILD_OBJECT   OFF CACHE BOOL "" FORCE)
    add_subdirectory(${THIRDPARTY_DIR}/mimalloc EXCLUDE_FROM_ALL)
endif()

# Vendored: xxHash (header-only) 
add_library(xxhash INTERFACE)
target_include_directories(xxhash INTERFACE ${THIRDPARTY_DIR}/xxHash)
target_compile_definitions(xxhash INTERFACE XXH_STATIC_LINKING_ONLY)

# Vendored: zstd 
# zstd's top-level CMakeLists is a super-build wrapper; use build/cmake directly.
set(ZSTD_BUILD_PROGRAMS  OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_TESTS     OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_CONTRIB   OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_SHARED    OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_STATIC    ON  CACHE BOOL "" FORCE)
if(EXISTS ${THIRDPARTY_DIR}/zstd/build/cmake/CMakeLists.txt)
    add_subdirectory(${THIRDPARTY_DIR}/zstd/build/cmake EXCLUDE_FROM_ALL)
endif()

# Vendored: libPNG 
# libPNG requires zlib; use bundled zlib vendored copy or system zlib.
set(PNG_BUILD_ZLIB    OFF CACHE BOOL "" FORCE)
set(PNG_SHARED        OFF CACHE BOOL "" FORCE)
set(PNG_STATIC        ON  CACHE BOOL "" FORCE)
set(PNG_TESTS         OFF CACHE BOOL "" FORCE)
set(PNG_TOOLS         OFF CACHE BOOL "" FORCE)
if(EXISTS ${THIRDPARTY_DIR}/zlib/CMakeLists.txt)
    # Build zlib from vendored copy and hint libpng to it
    set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    add_subdirectory(${THIRDPARTY_DIR}/zlib EXCLUDE_FROM_ALL)
    set(ZLIB_LIBRARY   zlibstatic CACHE STRING "" FORCE)
    set(ZLIB_INCLUDE_DIR ${THIRDPARTY_DIR}/zlib CACHE PATH "" FORCE)
    set(ZLIB_ROOT      ${THIRDPARTY_DIR}/zlib CACHE PATH "" FORCE)
else()
    find_package(ZLIB REQUIRED)
endif()
add_subdirectory(${THIRDPARTY_DIR}/libPNG EXCLUDE_FROM_ALL)

# Vendored: Ogg 
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRDPARTY_DIR}/Ogg EXCLUDE_FROM_ALL)

# Vendored: OpenAL-Soft 
if(FREELY_AUDIO_OPENAL)
    set(ALSOFT_EXAMPLES   OFF CACHE BOOL "" FORCE)
    set(ALSOFT_TESTS      OFF CACHE BOOL "" FORCE)
    set(ALSOFT_UTILS      OFF CACHE BOOL "" FORCE)
    set(ALSOFT_INSTALL    OFF CACHE BOOL "" FORCE)
    set(ALSOFT_ENABLE_MODULES OFF CACHE BOOL "" FORCE)
    set(LIBTYPE           STATIC CACHE STRING "" FORCE)
    add_subdirectory(${THIRDPARTY_DIR}/OpenAL EXCLUDE_FROM_ALL)
endif()

# Vendored: SDL3
if(FREELY_AUDIO_SDL)
    set(SDL_SHARED        OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC        ON  CACHE BOOL "" FORCE)
    set(SDL_TESTS         OFF CACHE BOOL "" FORCE)
    set(SDL_INSTALL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
    add_subdirectory(${THIRDPARTY_DIR}/SDL3 EXCLUDE_FROM_ALL)
endif()

# Vendored: SFML 
# SFML is not used by Freely Engine and causes FetchContent downloads over the network.
# Keeping it disabled.
# set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# set(SFML_BUILD_DOC      OFF CACHE BOOL "" FORCE)
# set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
# set(SFML_USE_SYSTEM_DEPS OFF CACHE BOOL "" FORCE)
# add_subdirectory(${THIRDPARTY_DIR}/SFML EXCLUDE_FROM_ALL)

# Vendored: bgfx (manual static library) 
# bgfx uses GENie/premake - no CMakeLists provided. Build a minimal static lib
# from the amalgamated source so the engine can optionally use it as an RHI.
if(FREELY_USE_BGFX)
    set(BGFX_DIR  ${THIRDPARTY_DIR}/bgfx)
    set(BX_DIR    ${THIRDPARTY_DIR}/bgfx/3rdparty)   # bx lives alongside bgfx

    # bimg and bx are bgfx companion repos - check if they are present alongside
    # as sibling directories (common bgfx workspace layout).
    set(BIMG_DIR  ${THIRDPARTY_DIR}/bimg)
    set(BX_REAL   ${THIRDPARTY_DIR}/bx)

    # Collect bgfx core sources
    file(GLOB BGFX_SOURCES
        ${BGFX_DIR}/src/bgfx.cpp
        ${BGFX_DIR}/src/vertexlayout.cpp
        ${BGFX_DIR}/src/topology.cpp
        ${BGFX_DIR}/src/shader.cpp
        ${BGFX_DIR}/src/renderer_noop.cpp
        ${BGFX_DIR}/src/debug_renderdoc.cpp
    )
    # Add platform-specific renderers
    if(WIN32)
        list(APPEND BGFX_SOURCES
            ${BGFX_DIR}/src/renderer_d3d11.cpp
            ${BGFX_DIR}/src/renderer_d3d12.cpp
            ${BGFX_DIR}/src/renderer_gl.cpp
            ${BGFX_DIR}/src/dxgi.cpp
            ${BGFX_DIR}/src/nvapi.cpp
            ${BGFX_DIR}/src/glcontext_wgl.cpp
        )
    elseif(APPLE)
        list(APPEND BGFX_SOURCES
            ${BGFX_DIR}/src/renderer_gl.cpp
            ${BGFX_DIR}/src/renderer_mtl.cpp
            ${BGFX_DIR}/src/glcontext_egl.cpp
        )
    elseif(UNIX)
        list(APPEND BGFX_SOURCES
            ${BGFX_DIR}/src/renderer_gl.cpp
            ${BGFX_DIR}/src/glcontext_egl.cpp
        )
    endif()
    if(FREELY_USE_VULKAN)
        list(APPEND BGFX_SOURCES ${BGFX_DIR}/src/renderer_vk.cpp)
    endif()
    if(EMSCRIPTEN)
        list(APPEND BGFX_SOURCES
            ${BGFX_DIR}/src/renderer_webgpu.cpp
            ${BGFX_DIR}/src/glcontext_html5.cpp
        )
    endif()

    add_library(bgfx STATIC ${BGFX_SOURCES})
    target_include_directories(bgfx PUBLIC
        ${BGFX_DIR}/include
        ${BGFX_DIR}/3rdparty
        ${BGFX_DIR}/3rdparty/khronos
    )
    # bgfx internal includes needed during compilation
    target_include_directories(bgfx PRIVATE
        ${BGFX_DIR}/src
    )
    target_compile_definitions(bgfx PUBLIC
        BGFX_CONFIG_DEBUG=$<IF:$<CONFIG:Debug>,1,0>
    )
    if(WIN32)
        target_compile_definitions(bgfx PUBLIC
            _WIN32_WINNT=0x0601
            NOMINMAX
            WIN32_LEAN_AND_MEAN
        )
        target_link_libraries(bgfx PUBLIC d3d11 d3d12 dxgi dxguid)
    endif()
    if(APPLE)
        target_compile_options(bgfx PRIVATE -x objective-c++)
    endif()
    if(MSVC)
        target_compile_options(bgfx PRIVATE /bigobj /W0)
    else()
        target_compile_options(bgfx PRIVATE -Wno-everything)
    endif()
endif()

# Vendored: FBX (minimal open-source parser) 
# ThirdParty/FBX/src contains a lightweight FBX reader (no Autodesk SDK required).
set(FBX_DIR ${THIRDPARTY_DIR}/FBX/src)
if(EXISTS ${FBX_DIR}/CMakeLists.txt)
    add_subdirectory(${FBX_DIR} fbx_build EXCLUDE_FROM_ALL)
elseif(EXISTS ${FBX_DIR}/fbxdocument.cpp)
    add_library(fbx_parser STATIC
        ${FBX_DIR}/fbxdocument.cpp
        ${FBX_DIR}/fbxnode.cpp
        ${FBX_DIR}/fbxproperty.cpp
        ${FBX_DIR}/fbxutil.cpp
    )
    target_include_directories(fbx_parser PUBLIC ${FBX_DIR})
    if(MSVC)
        target_compile_options(fbx_parser PRIVATE /W0)
    else()
        target_compile_options(fbx_parser PRIVATE -w)
    endif()
endif()

# ─── Lua 5.4 (optional, vendored if present, FetchContent fallback) ───────────
if(FREELY_SCRIPTING_LUA)
    set(LUA_DIR ${THIRDPARTY_DIR}/Lua)
    if(EXISTS ${LUA_DIR}/CMakeLists.txt)
        add_subdirectory(${LUA_DIR} EXCLUDE_FROM_ALL)
    else()
        # Fallback to finding system installed Lua to avoid Git download
        find_package(Lua)
        if(LUA_FOUND)
            add_library(lua INTERFACE)
            target_include_directories(lua INTERFACE ${LUA_INCLUDE_DIR})
            target_link_libraries(lua INTERFACE ${LUA_LIBRARIES})
        else()
            message(WARNING "Lua not found locally or on the system. Disabling FREELY_SCRIPTING_LUA.")
            set(FREELY_SCRIPTING_LUA OFF CACHE BOOL "" FORCE)
        endif()
    endif()
endif()

# Vulkan 
if(FREELY_USE_VULKAN AND NOT EMSCRIPTEN)
    find_package(Vulkan REQUIRED)

    # Vulkan Memory Allocator (header-only) - look in KTX vendor or Vulkan vendor folder
    if(EXISTS ${THIRDPARTY_DIR}/KTX/other_include/vma/vk_mem_alloc.h)
        set(VMA_DIR ${THIRDPARTY_DIR}/KTX/other_include/vma)
    else()
        set(VMA_DIR ${THIRDPARTY_DIR}/Vulkan)
    endif()

    if(EXISTS ${VMA_DIR}/vk_mem_alloc.h)
        add_library(vma INTERFACE)
        target_include_directories(vma INTERFACE ${VMA_DIR})
    else()
        message(FATAL_ERROR "Vulkan Memory Allocator (vk_mem_alloc.h) not found in ThirdParty. FetchContent fallback disabled.")
    endif()
endif()

# Freely Engine Library 
file(GLOB_RECURSE ENGINE_SOURCES
    ${FREELY_ROOT}/Engine/src/*.cpp
)
file(GLOB_RECURSE ENGINE_HEADERS
    ${FREELY_ROOT}/Engine/include/*.h
)

add_library(FreelyEngine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
target_include_directories(FreelyEngine PUBLIC
    ${FREELY_ROOT}/Engine/include
)

# Core required dependencies 
target_link_libraries(FreelyEngine PUBLIC
    glfw
    glad
    glm::glm
    spdlog::spdlog
    EnTT::EnTT
    nlohmann_json::nlohmann_json
    stb_headers
    tinyobjloader
    miniaudio_headers
    xxhash
)

# Box2D 
if(TARGET box2d)
    target_link_libraries(FreelyEngine PUBLIC box2d)
endif()

# Jolt 
if(TARGET Jolt)
    target_link_libraries(FreelyEngine PUBLIC Jolt)
endif()

# mimalloc 
if(FREELY_USE_MIMALLOC AND TARGET mimalloc-static)
    target_link_libraries(FreelyEngine PUBLIC mimalloc-static)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_USE_MIMALLOC=1)
endif()

# zstd 
if(TARGET libzstd_static)
    target_link_libraries(FreelyEngine PUBLIC libzstd_static)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_HAS_ZSTD=1)
endif()

# libPNG 
if(TARGET png_static)
    target_link_libraries(FreelyEngine PUBLIC png_static)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_HAS_LIBPNG=1)
endif()

# Ogg 
if(TARGET Ogg::ogg)
    target_link_libraries(FreelyEngine PUBLIC Ogg::ogg)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_HAS_OGG=1)
elseif(TARGET ogg)
    target_link_libraries(FreelyEngine PUBLIC ogg)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_HAS_OGG=1)
endif()

# OpenAL 
if(FREELY_AUDIO_OPENAL AND TARGET OpenAL::OpenAL)
    target_link_libraries(FreelyEngine PUBLIC OpenAL::OpenAL)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_AUDIO_OPENAL=1)
elseif(FREELY_AUDIO_OPENAL AND TARGET OpenAL)
    target_link_libraries(FreelyEngine PUBLIC OpenAL)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_AUDIO_OPENAL=1)
endif()

# SDL3 
if(FREELY_AUDIO_SDL AND TARGET SDL3::SDL3-static)
    target_link_libraries(FreelyEngine PUBLIC SDL3::SDL3-static)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_AUDIO_SDL=1)
elseif(FREELY_AUDIO_SDL AND TARGET SDL3-static)
    target_link_libraries(FreelyEngine PUBLIC SDL3-static)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_AUDIO_SDL=1)
endif()

# SFML 
if(TARGET SFML::Audio)
    target_link_libraries(FreelyEngine PUBLIC SFML::Audio SFML::Network SFML::Window SFML::System)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_HAS_SFML=1)
endif()

# bgfx 
if(FREELY_USE_BGFX AND TARGET bgfx)
    target_link_libraries(FreelyEngine PUBLIC bgfx)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_RHI_BGFX=1)
endif()

# FBX parser 
if(TARGET fbx_parser)
    target_link_libraries(FreelyEngine PUBLIC fbx_parser)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_HAS_FBX=1)
endif()

# Vulkan 
if(FREELY_USE_VULKAN AND NOT EMSCRIPTEN)
    target_link_libraries(FreelyEngine PUBLIC Vulkan::Vulkan)

    # shaderc
    find_library(SHADERC_LIB NAMES shaderc_combined shadercd_combined
                 PATHS "$ENV{VULKAN_SDK}/Lib" "$ENV{VULKAN_SDK}/lib")
    if(SHADERC_LIB)
        target_link_libraries(FreelyEngine PUBLIC ${SHADERC_LIB})
        target_include_directories(FreelyEngine PUBLIC "$ENV{VULKAN_SDK}/Include")
    else()
        message(WARNING "shaderc_combined not found in Vulkan SDK")
    endif()

    if(TARGET vma)
        target_link_libraries(FreelyEngine PUBLIC vma)
    else()
        target_include_directories(FreelyEngine PUBLIC ${vma_SOURCE_DIR}/include)
    endif()
    target_compile_definitions(FreelyEngine PUBLIC FREELY_RHI_VULKAN=1)
endif()

# Compile definitions 
target_compile_definitions(FreelyEngine PUBLIC
    FREELY_VERSION_MAJOR=0
    FREELY_VERSION_MINOR=4
    FREELY_VERSION_PATCH=3
    FREELY_VERSION_STRING="0.4.3"
    GLM_ENABLE_EXPERIMENTAL
    FREELY_RENDERER3D=1
    FREELY_RENDERER2D=1
)

if(FREELY_SCRIPTING_LUA)
    if(TARGET lua)
        target_link_libraries(FreelyEngine PUBLIC lua)
    endif()
    target_compile_definitions(FreelyEngine PUBLIC FREELY_SCRIPTING_LUA=1)
endif()
if(FREELY_SCRIPTING_MONO)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_SCRIPTING_MONO=1)
endif()

# Emscripten-specific flags for WebGL/WASM
if(EMSCRIPTEN)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_PLATFORM_WEB=1)
    set_target_properties(FreelyEngine PROPERTIES
        COMPILE_FLAGS "-s USE_WEBGL2=1 -s FULL_ES3=1"
    )
endif()

# Freely Editor 
if(FREELY_BUILD_EDITOR AND TARGET imgui)
    file(GLOB_RECURSE EDITOR_SOURCES
        ${FREELY_ROOT}/Editor/src/*.cpp
    )
    file(GLOB_RECURSE EDITOR_HEADERS
        ${FREELY_ROOT}/Editor/include/*.h
    )

    add_executable(FreelyEditor ${EDITOR_SOURCES} ${EDITOR_HEADERS})
    target_include_directories(FreelyEditor PRIVATE
        ${FREELY_ROOT}/Editor/include
    )
    target_link_libraries(FreelyEditor PRIVATE
        FreelyEngine
        imgui
    )
    if(TARGET imguizmo)
        target_link_libraries(FreelyEditor PRIVATE imguizmo)
    endif()
    target_compile_definitions(FreelyEditor PRIVATE FREELY_EDITOR=1)

    if(WIN32)
        # IFileOpenDialog needs COM
        target_link_libraries(FreelyEditor PRIVATE ole32 uuid)
    endif()

    # Copy font files to build output
    if(EXISTS ${FREELY_ROOT}/Fonts)
        add_custom_command(TARGET FreelyEditor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${FREELY_ROOT}/Fonts
                $<TARGET_FILE_DIR:FreelyEditor>/Fonts
        )
    endif()
endif()

# Freely Runtime (standalone player) 
if(FREELY_BUILD_RUNTIME)
    if(EXISTS ${FREELY_ROOT}/Runtime/src/main.cpp)
        add_executable(FreelyRuntime
            ${FREELY_ROOT}/Runtime/src/main.cpp
        )
        target_link_libraries(FreelyRuntime PRIVATE FreelyEngine)

        # Emscripten: output as .html with embedded assets
        if(EMSCRIPTEN)
            set_target_properties(FreelyRuntime PROPERTIES SUFFIX ".html")
            target_link_options(FreelyRuntime PRIVATE
                -s ALLOW_MEMORY_GROWTH=1
                -s USE_WEBGL2=1
                -s FULL_ES3=1
                -s WASM=1
                --preload-file ${FREELY_ROOT}/Assets@/Assets
            )
        endif()
    endif()
endif()

# Install 
install(TARGETS FreelyEngine
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(DIRECTORY Engine/include/Freely
    DESTINATION include
)
if(FREELY_BUILD_EDITOR AND TARGET FreelyEditor)
    install(TARGETS FreelyEditor DESTINATION bin)
endif()
