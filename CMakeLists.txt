cmake_minimum_required(VERSION 3.22)
project(FreelyEngine VERSION 0.4.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(FREELY_BUILD_EDITOR   "Build the Freely Editor"          ON)
option(FREELY_BUILD_EXAMPLES "Build example applications"       ON)
option(FREELY_BUILD_RUNTIME  "Build standalone runtime"         ON)
option(FREELY_USE_VULKAN     "Enable Vulkan backend"            ON)
option(FREELY_SCRIPTING_LUA  "Enable Lua scripting"             ON)
option(FREELY_SCRIPTING_MONO "Enable C#/Mono scripting"         OFF)
option(FREELY_AUDIO_OPENAL   "Enable OpenAL audio backend"     ON)
option(FREELY_AUDIO_SDL      "Enable SDL audio backend"        ON)

# ─── Dependencies via FetchContent ────────────────────────────────────────────
include(FetchContent)

# GLFW
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.9
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# spdlog
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.13.0
)
FetchContent_MakeAvailable(spdlog)

# stb (header-only)
FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

# tinyobjloader
FetchContent_Declare(tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG        release
)
FetchContent_MakeAvailable(tinyobjloader)

# EnTT (ECS)
FetchContent_Declare(entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.16.0
)
FetchContent_MakeAvailable(entt)

# nlohmann/json
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(nlohmann_json)


# miniaudio (single-header audio library)
FetchContent_Declare(miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git
    GIT_TAG        0.11.21
)
FetchContent_MakeAvailable(miniaudio)
add_library(miniaudio_headers INTERFACE)
target_include_directories(miniaudio_headers INTERFACE ${miniaudio_SOURCE_DIR})

# Lua 5.4 (optional)
if(FREELY_SCRIPTING_LUA)
    FetchContent_Declare(lua
        GIT_REPOSITORY https://github.com/lua/lua.git
        GIT_TAG        v5.4.7
    )
    FetchContent_MakeAvailable(lua)
endif()

# Vulkan requirements
if(FREELY_USE_VULKAN)
    # Hint for SDK
    set(ENV{VULKAN_SDK} "C:/VulkanSDK/1.4.341.1")
    find_package(Vulkan REQUIRED)
    
    # VMA (header only)
    FetchContent_Declare(vma
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG        v3.1.0
    )
    FetchContent_MakeAvailable(vma)
endif()

# ─── glad (vendored) ─────────────────────────────────────────────────────────
add_library(glad STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/glad/src/glad.c
)
target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/glad/include
)

# ─── ImGui (vendored, docking branch) ────────────────────────────────────────
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/ImGui)
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

# ─── ImGuizmo (vendored) ─────────────────────────────────────────────────────
set(IMGUIZMO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/ImGuizmo)
add_library(imguizmo STATIC
    ${IMGUIZMO_DIR}/ImGuizmo.cpp
)
target_include_directories(imguizmo PUBLIC ${IMGUIZMO_DIR})
target_link_libraries(imguizmo PUBLIC imgui)

# ─── stb interface library ───────────────────────────────────────────────────
add_library(stb_headers INTERFACE)
target_include_directories(stb_headers INTERFACE ${stb_SOURCE_DIR})

# stb_truetype is part of the stb repo — no extra fetch needed.
# Font.cpp defines STB_TRUETYPE_IMPLEMENTATION so it just needs the include dir.
# Likewise stb_image is defined in Texture.cpp.

# ─── Freely Engine Library ───────────────────────────────────────────────────
file(GLOB_RECURSE ENGINE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/src/*.cpp
)
file(GLOB_RECURSE ENGINE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/include/*.h
)

add_library(FreelyEngine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
target_include_directories(FreelyEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/include
)
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
)

if(FREELY_USE_VULKAN)
    target_link_libraries(FreelyEngine PUBLIC Vulkan::Vulkan)
    
    # shaderc
    find_library(SHADERC_LIB NAMES shaderc_combined shadercd_combined PATHS "$ENV{VULKAN_SDK}/Lib")
    if(SHADERC_LIB)
        target_link_libraries(FreelyEngine PUBLIC ${SHADERC_LIB})
        target_include_directories(FreelyEngine PUBLIC "$ENV{VULKAN_SDK}/Include")
    else()
        message(WARNING "shaderc_combined not found in Vulkan SDK")
    endif()

    # VMA doesn't create a target by default if just fetched, we just include its dir
    target_include_directories(FreelyEngine PUBLIC ${vma_SOURCE_DIR}/include)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_RHI_VULKAN=1)
endif()
target_compile_definitions(FreelyEngine PUBLIC
    FREELY_VERSION_MAJOR=0
    FREELY_VERSION_MINOR=4
    FREELY_VERSION_PATCH=2
    FREELY_VERSION_STRING="0.4.2"
    GLM_ENABLE_EXPERIMENTAL
    FREELY_RENDERER3D=1
    FREELY_RENDERER2D=1
)

if(FREELY_SCRIPTING_LUA)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_SCRIPTING_LUA=1)
    # Lua is built as part of the engine
endif()

if(FREELY_SCRIPTING_MONO)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_SCRIPTING_MONO=1)
endif()

if(FREELY_AUDIO_OPENAL)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_AUDIO_OPENAL=1)
endif()

if(FREELY_AUDIO_SDL)
    target_compile_definitions(FreelyEngine PUBLIC FREELY_AUDIO_SDL=1)
endif()

# ─── Freely Editor ───────────────────────────────────────────────────────────
if(FREELY_BUILD_EDITOR)
    file(GLOB_RECURSE EDITOR_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/Editor/src/*.cpp
    )
    file(GLOB_RECURSE EDITOR_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/Editor/include/*.h
    )

    add_executable(FreelyEditor ${EDITOR_SOURCES} ${EDITOR_HEADERS})
    target_include_directories(FreelyEditor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Editor/include
    )
    target_link_libraries(FreelyEditor PRIVATE
        FreelyEngine
        imgui
        imguizmo
    )
    target_compile_definitions(FreelyEditor PRIVATE
        FREELY_EDITOR=1
    )

    if(WIN32)
        # IFileOpenDialog (ProjectManager native Open Project / Browse Folder dialogs)
        # needs COM: ole32 for Co* functions, uuid for the CLSID_/IID_ externs.
        target_link_libraries(FreelyEditor PRIVATE ole32 uuid)
    endif()

    # Copy font files to build output
    add_custom_command(TARGET FreelyEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/Fonts
            $<TARGET_FILE_DIR:FreelyEditor>/Fonts
    )
endif()

# ─── Freely Runtime (standalone player) ──────────────────────────────────────
if(FREELY_BUILD_RUNTIME)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/src/main.cpp)
        add_executable(FreelyRuntime
            ${CMAKE_CURRENT_SOURCE_DIR}/Runtime/src/main.cpp
        )
        target_link_libraries(FreelyRuntime PRIVATE FreelyEngine)
    endif()
endif()

# ─── Install ─────────────────────────────────────────────────────────────────
install(TARGETS FreelyEngine
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(DIRECTORY Engine/include/Freely
    DESTINATION include
)
if(FREELY_BUILD_EDITOR)
    install(TARGETS FreelyEditor DESTINATION bin)
endif()
