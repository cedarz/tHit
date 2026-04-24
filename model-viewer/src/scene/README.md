# Scene Loader

This directory contains the scene loader ported from GLSL-PathTracer project.

## Files

- `loader_types.h` - Type definitions using glm types directly (glm::vec3, glm::mat4, etc.)
- `loader.h` / `loader.cpp` - Main scene file loader (supports custom scene file format)
- `scene_loader.h` / `scene_loader.cpp` - Simplified Scene class for storing loaded data
- `scene_converter.h` / `scene_converter.cpp` - Converter to transform loaded scene data to Model format

## Usage

In `model-viewer.cpp`, you can now use:

```cpp
// Load a scene file instead of a single model file
app.LoadSceneFromFile("path/to/scene.txt");
```

## Scene File Format

The loader supports a custom scene file format similar to GLSL-PathTracer:

```
material mat1 {
    color 1.0 1.0 1.0
    roughness 0.5
    metallic 0.0
    albedotexture texture.png
}

light {
    position 0.0 10.0 0.0
    emission 1.0 1.0 1.0
    type sphere
    radius 1.0
}

camera {
    position 0.0 0.0 5.0
    lookat 0.0 0.0 0.0
    fov 45.0
}

mesh {
    name mymesh
    file model.obj
    material mat1
    position 0.0 0.0 0.0
    scale 1.0 1.0 1.0
    rotation 0.0 0.0 0.0 1.0
}
```

## Notes

- GLTF support is not included in this port (would require additional dependencies)
- The loader converts loaded scene data to the existing Model format used by model-viewer
- Camera and lights are automatically updated from the scene file

