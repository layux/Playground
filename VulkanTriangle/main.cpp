// Complete, self‑contained hello‑triangle using GLFW + Vulkan 1.3
//  - vk‑bootstrap removes boiler‑plate for instance/device/swap‑chain
//  - VMA handles GPU allocations
//  - spdlog gives nice coloured output
//  - no vertex buffer: the three vertices are hard‑coded in the vertex shader
// Build with C++17.  Requires Vulkan SDK 1.3+, glfw 3.3+, vk‑bootstrap ≥0.7, VMA ≥3.0, spdlog ≥1.12
// -----------------------------------------------------------------------------

#include <spdlog/spdlog.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VkBootstrap.h"
#include "vk_mem_alloc.h"

#include <vector>
#include <fstream>
#include <stdexcept>
#include <array>

// --- helper ---------------------------------------------------------------
static std::vector<char> read_file(const char *path)
{
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f)
        throw std::runtime_error("cannot open file");
    size_t sz = f.tellg();
    std::vector<char> buf(sz);
    f.seekg(0);
    f.read(buf.data(), sz);
    return buf;
}

// --- minimal RAII container ----------------------------------------------
struct VulkanContext
{
    VkInstance instance{};
    VkPhysicalDevice gpu{};
    VkDevice device{};
    VkSurfaceKHR surface{};
    VkQueue gfxQ{};
    uint32_t gfxIndex{};

    VkSwapchainKHR swapchain{};
    VkFormat swpFormat{};
    VkExtent2D swpExtent{};
    std::vector<VkImage> images;
    std::vector<VkImageView> views;
    std::vector<VkFramebuffer> fbs;

    VkRenderPass renderPass{};
    VkPipelineLayout layout{};
    VkPipeline pipe{};
    VkCommandPool cmdPool{};
    std::vector<VkCommandBuffer> cmds;

    VkSemaphore imgAvail{};
    VkSemaphore renderDone{};
    VkFence inFlight{};

    VmaAllocator vma{};

    void destroy()
    {
        vkDeviceWaitIdle(device);
        vkDestroyFence(device, inFlight, nullptr);
        vkDestroySemaphore(device, renderDone, nullptr);
        vkDestroySemaphore(device, imgAvail, nullptr);
        vkDestroyCommandPool(device, cmdPool, nullptr);
        vkDestroyPipeline(device, pipe, nullptr);
        vkDestroyPipelineLayout(device, layout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);
        for (auto fb : fbs)
            vkDestroyFramebuffer(device, fb, nullptr);
        for (auto v : views)
            vkDestroyImageView(device, v, nullptr);
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        vmaDestroyAllocator(vma);
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }
};

// --- entry ----------------------------------------------------------------
int main()
{
    spdlog::set_pattern("[%^%l%$] %v");
    spdlog::info("Booting hello‑triangle …");

    // 1. GLFW window --------------------------------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *win = glfwCreateWindow(800, 600, "Triangle", nullptr, nullptr);

    VulkanContext ctx;

    // 2. Instance -----------------------------------------------------------
    auto instRet = vkb::InstanceBuilder{}
                       .set_app_name("hello_triangle")
                       .use_default_debug_messenger()
                       .request_validation_layers()
                       .require_api_version(1, 3, 0)
                       .build();
    if (!instRet)
        throw std::runtime_error(instRet.error().message());
    vkb::Instance vkbInst = instRet.value();
    ctx.instance = vkbInst.instance;

    // 3. Surface ------------------------------------------------------------
    if (glfwCreateWindowSurface(ctx.instance, win, nullptr, &ctx.surface) != VK_SUCCESS)
        throw std::runtime_error("failed to create surface");

    // 4. Device -------------------------------------------------------------
    auto physRet = vkb::PhysicalDeviceSelector{vkbInst}
                       .set_surface(ctx.surface)
                       .set_minimum_version(1, 3)
                       .select();
    if (!physRet)
        throw std::runtime_error(physRet.error().message());

    auto devRet = vkb::DeviceBuilder{physRet.value()}.build();
    if (!devRet)
        throw std::runtime_error(devRet.error().message());
    vkb::Device vkbDev = devRet.value();
    ctx.device = vkbDev.device;
    ctx.gpu = vkbDev.physical_device;
    ctx.gfxQ = vkbDev.get_queue(vkb::QueueType::graphics).value();
    ctx.gfxIndex = vkbDev.get_queue_index(vkb::QueueType::graphics).value();

    // 5. VMA allocator ------------------------------------------------------
    VmaAllocatorCreateInfo aci{};
    aci.instance = ctx.instance;
    aci.physicalDevice = ctx.gpu;
    aci.device = ctx.device;
    aci.vulkanApiVersion = VK_API_VERSION_1_3;
    vmaCreateAllocator(&aci, &ctx.vma);

    // 6. Swap‑chain ---------------------------------------------------------
    auto swpRet = vkb::SwapchainBuilder{vkbDev}.build();
    if (!swpRet)
        throw std::runtime_error(swpRet.error().message());
    vkb::Swapchain vkbSwp = swpRet.value();
    ctx.swapchain = vkbSwp.swapchain;
    ctx.swpFormat = vkbSwp.image_format;
    ctx.swpExtent = vkbSwp.extent;
    ctx.images = vkbSwp.get_images().value();
    ctx.views = vkbSwp.get_image_views().value();

    // 7. Render‑pass --------------------------------------------------------
    VkAttachmentDescription color{};
    color.format = ctx.swpFormat;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference ref{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription sub{};
    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub.colorAttachmentCount = 1;
    sub.pColorAttachments = &ref;
    VkRenderPassCreateInfo rpci{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    rpci.attachmentCount = 1;
    rpci.pAttachments = &color;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &sub;
    vkCreateRenderPass(ctx.device, &rpci, nullptr, &ctx.renderPass);

    // 8. Framebuffers -------------------------------------------------------
    ctx.fbs.resize(ctx.views.size());
    for (size_t i = 0; i < ctx.views.size(); ++i)
    {
        VkImageView att[] = {ctx.views[i]};
        VkFramebufferCreateInfo fbci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        fbci.renderPass = ctx.renderPass;
        fbci.attachmentCount = 1;
        fbci.pAttachments = att;
        fbci.width = ctx.swpExtent.width;
        fbci.height = ctx.swpExtent.height;
        fbci.layers = 1;
        vkCreateFramebuffer(ctx.device, &fbci, nullptr, &ctx.fbs[i]);
    }

    // 9. Pipeline -----------------------------------------------------------
    auto vert = read_file("triangle.vert.spv");
    auto frag = read_file("triangle.frag.spv");
    auto mk_shader = [&](auto &code, VkShaderStageFlagBits stage)
    {
        VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO}; ci.codeSize=code.size(); ci.pCode=reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule mod; vkCreateShaderModule(ctx.device,&ci,nullptr,&mod);
        VkPipelineShaderStageCreateInfo si{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}; si.stage=stage; si.module=mod; si.pName="main";
        return std::pair{si,mod};
    };
    auto [vs, vsMod] = mk_shader(vert, VK_SHADER_STAGE_VERTEX_BIT);
    auto [fs, fsMod] = mk_shader(frag, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineShaderStageCreateInfo stages[] = {vs, fs};

    VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkViewport vp{0, 0, (float)ctx.swpExtent.width, (float)ctx.swpExtent.height, 0, 1};
    VkRect2D sc{{0, 0}, ctx.swpExtent};
    VkPipelineViewportStateCreateInfo vpstate{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    vpstate.viewportCount = 1;
    vpstate.pViewports = &vp;
    vpstate.scissorCount = 1;
    vpstate.pScissors = &sc;
    VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;
    VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkPipelineColorBlendAttachmentState cbAtt{};
    cbAtt.colorWriteMask = 0xF;
    VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    cb.attachmentCount = 1;
    cb.pAttachments = &cbAtt;
    VkPipelineLayoutCreateInfo plci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    vkCreatePipelineLayout(ctx.device, &plci, nullptr, &ctx.layout);
    VkGraphicsPipelineCreateInfo gp{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    gp.stageCount = 2;
    gp.pStages = stages;
    gp.pVertexInputState = &vi;
    gp.pInputAssemblyState = &ia;
    gp.pViewportState = &vpstate;
    gp.pRasterizationState = &rs;
    gp.pMultisampleState = &ms;
    gp.pColorBlendState = &cb;
    gp.layout = ctx.layout;
    gp.renderPass = ctx.renderPass;
    vkCreateGraphicsPipelines(ctx.device, VK_NULL_HANDLE, 1, &gp, nullptr, &ctx.pipe);
    vkDestroyShaderModule(ctx.device, vsMod, nullptr);
    vkDestroyShaderModule(ctx.device, fsMod, nullptr);

    // 10. Command buffers ---------------------------------------------------
    VkCommandPoolCreateInfo pool{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool.queueFamilyIndex = ctx.gfxIndex;
    pool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(ctx.device, &pool, nullptr, &ctx.cmdPool);

    ctx.cmds.resize(ctx.fbs.size());
    VkCommandBufferAllocateInfo cba{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cba.commandPool = ctx.cmdPool;
    cba.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cba.commandBufferCount = (uint32_t)ctx.cmds.size();
    vkAllocateCommandBuffers(ctx.device, &cba, ctx.cmds.data());

    for (size_t i = 0; i < ctx.cmds.size(); ++i)
    {
        VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        vkBeginCommandBuffer(ctx.cmds[i], &bi);
        VkClearValue clear{{0.f, 0.f, 0.f, 1.f}};
        VkRenderPassBeginInfo rbi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        rbi.renderPass = ctx.renderPass;
        rbi.framebuffer = ctx.fbs[i];
        rbi.renderArea = {{0, 0}, ctx.swpExtent};
        rbi.clearValueCount = 1;
        rbi.pClearValues = &clear;
        vkCmdBeginRenderPass(ctx.cmds[i], &rbi, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(ctx.cmds[i], VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.pipe);
        vkCmdDraw(ctx.cmds[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(ctx.cmds[i]);
        vkEndCommandBuffer(ctx.cmds[i]);
    }

    // 11. Sync --------------------------------------------------------------
    VkSemaphoreCreateInfo sci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(ctx.device, &sci, nullptr, &ctx.imgAvail);
    vkCreateSemaphore(ctx.device, &sci, nullptr, &ctx.renderDone);
    VkFenceCreateInfo fci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(ctx.device, &fci, nullptr, &ctx.inFlight);

    // 12. Main‑loop ---------------------------------------------------------
    while (!glfwWindowShouldClose(win))
    {
        glfwPollEvents();
        vkWaitForFences(ctx.device, 1, &ctx.inFlight, VK_TRUE, UINT64_MAX);
        vkResetFences(ctx.device, 1, &ctx.inFlight);
        uint32_t imgIndex;
        vkAcquireNextImageKHR(ctx.device, ctx.swapchain, UINT64_MAX, ctx.imgAvail, VK_NULL_HANDLE, &imgIndex);
        VkPipelineStageFlags waitStage[]{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &ctx.imgAvail;
        submit.pWaitDstStageMask = waitStage;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &ctx.cmds[imgIndex];
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &ctx.renderDone;
        vkQueueSubmit(ctx.gfxQ, 1, &submit, ctx.inFlight);
        VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &ctx.renderDone;
        present.swapchainCount = 1;
        present.pSwapchains = &ctx.swapchain;
        present.pImageIndices = &imgIndex;
        vkQueuePresentKHR(ctx.gfxQ, &present);
    }

    // 13. Cleanup -----------------------------------------------------------
    ctx.destroy();
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}