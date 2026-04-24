#ifndef PIPELINE_H
#define PIPELINE_H

#include <glad/gl.h>
#include "gl460/buffer.h"
#include "gl460/image_format.h"
#include "gl460/ShaderProgram.h"

#include <glm/glm.hpp>

#include <exception>
#include <functional>
#include <vector>
#include <memory>
#include <string>

using GfxContext = void;
using GfxLayoutType = GLenum;

struct GfxLayoutDesc {
    /// The name of the layout attribute. 
    ///
    /// @NOTE: This can be left blank for OpenGL.
    std::string name;

    /// The type of the layout.
    GfxLayoutType type;

    /// If this value is set to `0`, the layout will 
    /// be sent immediately to the shader. However, 
    /// if it is set to a value >= `1`, the layout 
    /// will be sent after the nth instance. 
    uint32_t instance_rate = 0;
};

//typedef struct VkGraphicsPipelineCreateInfo {
//    VkStructureType                                  sType;
//    const void* pNext;
//    VkPipelineCreateFlags                            flags;
//    uint32_t                                         stageCount;         // number of entries in the pStages array
//    const VkPipelineShaderStageCreateInfo* pStages;
//    const VkPipelineVertexInputStateCreateInfo* pVertexInputState;
//    const VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState;
//    const VkPipelineTessellationStateCreateInfo* pTessellationState;
//    const VkPipelineViewportStateCreateInfo* pViewportState;
//    const VkPipelineRasterizationStateCreateInfo* pRasterizationState;
//    const VkPipelineMultisampleStateCreateInfo* pMultisampleState;
//    const VkPipelineDepthStencilStateCreateInfo* pDepthStencilState;
//    const VkPipelineColorBlendStateCreateInfo* pColorBlendState;
//    const VkPipelineDynamicStateCreateInfo* pDynamicState;
//    VkPipelineLayout                                 layout;
//    VkRenderPass                                     renderPass;
//    uint32_t                                         subpass;
//    VkPipeline                                       basePipelineHandle;
//    int32_t                                          basePipelineIndex;
//} VkGraphicsPipelineCreateInfo;

struct GfxPipelineDesc {
    std::vector<ShaderStageDesc> stages;
    /// The vertex buffer to be used in a `draw_vertex` command.
    ///
    /// @NOTE: This buffer _must_ be set. It cannot be left a `nullptr`.
    /// Even if `draw_index` is being used.
    gl::Buffer* vertex_buffer = nullptr;

    /// The amount of vertices in the `vertex_buffer` to be drawn. 
    uint32_t vertices_count = 0;

    /// The index buffer to be used in a `draw_index` command.
    ///
    /// @NOTE: This buffer _must_ be set if a `draw_index` command is used.
    /// Otherwise, it can be left as `nullptr`.
    gl::Buffer* index_buffer = nullptr;

    /// The amount of indices in the `index_buffer` to be drawn.
    uint32_t indices_count = 0;

    /// Layout array up to `LAYOUT_ELEMENTS_MAX` describing each layout attribute.
    std::vector<GfxLayoutDesc> layout;

    /// The amount of layouts to be set in `layout`.
    uint32_t layout_count = 0;

    /// The draw mode of the entire pipeline.
    ///
    /// @NOTE: This can be changed at anytime before the draw command.
    uint32_t draw_mode;

    /// A flag to indicate if the pipeline can 
    /// or cannot write to the depth buffer. 
    ///
    /// @NOTE: By default, this value is `true`.
    bool depth_mask = true;

    /// The stencil reference value of the pipeline. 
    ///
    /// @NOTE: This is `1` by default.
    uint32_t stencil_ref = 1;

    /// The blend factor to be used in the pipeline. 
    ///
    /// @NOTE: This is `{0, 0, 0, 0}` by default.
    float blend_factor[4] = { 0, 0, 0, 0 };
};

struct GfxPipeline {
    GfxPipelineDesc desc = {};
    ShaderProgram shader_program;

    GfxContext* gfx = nullptr;

    uint32_t vertex_array;

    gl::Buffer* vertex_buffer = nullptr;
    uint32_t vertex_count = 0;

    gl::Buffer* index_buffer = nullptr;
    uint32_t index_count = 0;

    uint32_t draw_mode;
};

uint32_t get_layout_size(const GfxLayoutType layout);

uint32_t set_buffer_layout(uint32_t vao, const GfxLayoutDesc* layout, uint32_t layout_count);



std::shared_ptr<GfxPipeline> gfx_pipeline_create(GfxContext* gfx, const GfxPipelineDesc& desc);


std::shared_ptr<GfxPipeline> BuildGfxPipeline(const GfxPipelineDesc& desc);



#endif
