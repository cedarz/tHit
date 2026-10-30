#include "DepthPrePass.h"

DepthPrePass::SharedPtr DepthPrePass::create(const std::string& pass_name, Camera* camera)
{
    SharedPtr pDepth = SharedPtr(new DepthPrePass(pass_name));
    pDepth->camera = camera;
    return pDepth;
}

DepthPrePass::DepthPrePass(const std::string& pass_name) : RenderPass(pass_name)
{

}

void DepthPrePass::Execute()
{
}

void DepthPrePass::SetMeshes(std::vector<Mesh*> meshes, const glm::mat4& worldMatrix)
{
}
