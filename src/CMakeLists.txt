set(RadiosityTest_SOURCES
    BitSet.hpp
    Camera.cpp
    Camera.hpp
    CameraState.hpp
    File.hpp
    GenericMesh.cpp
    GenericMesh.hpp
    GpuAllocator.cpp
    GpuAllocator.hpp
    GpuBuffer.cpp
    GpuBuffer.hpp
    GpuProgram.cpp
    GpuProgram.hpp
    GpuTexture.cpp
    GpuTexture.hpp
    Light.cpp
    Light.hpp
    Lightmap.cpp
    Lightmap.hpp
    LightmapBuildProcess.cpp
    LightmapBuildProcess.hpp
    Main.cpp
    Mesh.hpp
    Object.hpp
    ObjectState.hpp
    Renderer.cpp
    Renderer.hpp
    Scene.cpp
    Scene.hpp
    SceneObject.cpp
    SceneObject.hpp
    VertexSpecification.cpp
    VertexSpecification.hpp
)

add_executable(RadiosityTest ${RadiosityTest_SOURCES})
target_link_libraries(RadiosityTest ${RadiosityTest_DEP_LIBS})
