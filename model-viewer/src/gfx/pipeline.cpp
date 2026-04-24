#include "gfx/pipeline.h"

uint32_t get_layout_size(const GfxLayoutType layout)
{
    return 12;
}

uint32_t set_buffer_layout(uint32_t vao, const GfxLayoutDesc* layout, uint32_t layout_count) {

    uint32_t stride = 0;

    for (uint32_t i = 0; i < layout_count; i++) {
        stride += get_layout_size(layout[i].type);
    }

    uint32_t offset = 0;
    uint32_t semantic_index = 0;

    for (uint32_t i = 0; i < layout_count; i++) {
        /// @NOTE: A "semantic" is the inner value of an attribute. 
        /// For example, the semantic of a 'Mat4' is just a `Vec4` or, rather, 
        /// 4 `Vec4`s. In that case `semantic_count` would be `4` and `semantic_size`
        /// would be `16` since it's just a `FLOAT4` under the hood.

        // Different configuration if the current layout is a semantic or not
        /*if (is_semantic_attrib(layout[i].type)) {
            semantic_index = set_semantic_attrib(vao, layout[i], semantic_index, &offset);
        }
        else {
            set_vertex_attrib(vao, layout[i], semantic_index, &offset);
        }*/

        semantic_index += 1;
    }

    return stride;
}

std::shared_ptr<GfxPipeline> gfx_pipeline_create(GfxContext* gfx, const GfxPipelineDesc& desc) {
    std::shared_ptr<GfxPipeline> pipe = std::make_shared<GfxPipeline>();

    pipe->desc = desc;
    pipe->gfx = gfx;

    // VAO init
    glCreateVertexArrays(1, &pipe->vertex_array);

    // Layout init 
    uint32_t stride = set_buffer_layout(pipe->vertex_array, desc.layout.data(), desc.layout_count);

    // VBO init
    pipe->vertex_buffer = desc.vertex_buffer;
    pipe->vertex_count = desc.vertices_count;

    glVertexArrayVertexBuffer(pipe->vertex_array, 0, pipe->vertex_buffer->native_handle(), 0, stride);

    // EBO init
    if (desc.index_buffer) {
        pipe->index_buffer = desc.index_buffer;
        pipe->index_count = desc.indices_count;

        glVertexArrayElementBuffer(pipe->vertex_array, pipe->index_buffer->native_handle());
    }

    // Set the draw mode for the whole pipeline
    pipe->draw_mode = desc.draw_mode;

    return pipe;
}

std::shared_ptr<GfxPipeline> BuildGfxPipeline(const GfxPipelineDesc& desc) {
    std::shared_ptr<GfxPipeline> pipeline = std::make_shared< GfxPipeline>();
    pipeline->shader_program.load(desc.stages);

    return pipeline;
}
