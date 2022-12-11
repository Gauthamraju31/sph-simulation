cmake_minimum_required( VERSION 3.10.2 )
project( sph-demo )

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)

find_package( OpenGL REQUIRED)
find_package( glfw3 REQUIRED )
find_package( OpenMP REQUIRED CXX )

include_directories( SYSTEM "external/glm")
include_directories ("include")

add_executable(
	sph-demo
	"src/world.cpp"
	"src/main.cpp")

target_sources( sph-demo
                PRIVATE
                    ${IMGUI_DIR}/imgui_demo.cpp
                    ${IMGUI_DIR}/imgui_draw.cpp
                    ${IMGUI_DIR}/imgui_tables.cpp
                    ${IMGUI_DIR}/imgui_widgets.cpp
                    ${IMGUI_DIR}/imgui.cpp

                PRIVATE
                    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
                )

target_include_directories( sph-demo
                            PUBLIC ${IMGUI_DIR}
                            PUBLIC ${IMGUI_DIR}/backends
                            )

target_compile_features(
    sph-demo
    PUBLIC cxx_std_11 )
target_link_libraries(
    sph-demo
    PUBLIC ${OPENGL_LIBRARY}
    PUBLIC ${OPENGL_gl_LIBRARY}
    PRIVATE glfw
    PRIVATE OpenMP::OpenMP_CXX 
    PUBLIC ${CMAKE_DL_LIBS})
