#include "VolumeRenderer.hpp"
#include <FAST/Data/Image.hpp>
#include <FAST/Utility.hpp>

namespace fast {

cl::Image2D VolumeRenderer::textureToCLimage(uint textureID, int width, int height, OpenCLDevice::pointer device, bool depth) {
    // TODO CL-GL interop
    int totalSize = width * height;
    if(!depth)
        totalSize *= 4;

    auto data = make_uninitialized_unique<float[]>(totalSize);
    glBindTexture(GL_TEXTURE_2D, textureID);
    if(depth) {
        glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data.get());
    } else {
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data.get());
    }
    auto image = cl::Image2D(
        device->getContext(),
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        depth ? cl::ImageFormat(CL_R, CL_FLOAT) : cl::ImageFormat(CL_RGBA, CL_FLOAT),
        width, height, 0,
        data.get()
    );

    return image;
}

void VolumeRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, bool mode2D) {
    // Get window/viewport size
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    const float aspectRatio = (float)viewport[2] / viewport[3];
    const Vector2i gridSize(aspectRatio*512, 512);

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    auto queue = device->getCommandQueue();

    // Get color data from the main FBO to use as input
    int mainFBO;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &mainFBO);
    int textureID;
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &textureID);
    cl::Image2D inputColor = textureToCLimage(textureID, viewport[2], viewport[3], device, false);
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &textureID);
    cl::Image2D inputDepth = textureToCLimage(textureID, viewport[2], viewport[3], device, true);

    // Create a FBO
    if(m_FBO == 0)
        glGenFramebuffers(1, &m_FBO);

    // Bind the framebuffer to render to it
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
    // TODO CL-GL interop

    auto image = cl::Image2D(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_RGBA, CL_FLOAT),
            gridSize.x(), gridSize.y()
    );
    auto mKernel = cl::Kernel(getOpenCLProgram(device), "volumeRender");
    mKernel.setArg(1, image);

    auto input = getInputData<Image>(0);
    auto access = input->getOpenCLImageAccess(ACCESS_READ, device);
    cl::Image3D *clImage = access->get3DImage();

    float density = 0.05f;
    float brightness = 1.0f;
    float transferOffset = 0.0f;
    float transferScale = 1.0f;

    // create transfer function texture
    float transferFunc[] = {
            0.0, 0.0, 0.0, 0.0,
            1.0, 0.0, 0.0, 1.0,
            1.0, 0.5, 0.0, 1.0,
            1.0, 1.0, 0.0, 1.0,
            0.0, 1.0, 0.0, 1.0,
            0.0, 1.0, 1.0, 1.0,
            0.0, 0.0, 1.0, 1.0,
            1.0, 0.0, 1.0, 1.0,
            0.0, 0.0, 0.0, 0.0,
    };

    auto d_transferFuncArray = cl::Image2D(
            device->getContext(),
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            cl::ImageFormat(CL_RGBA, CL_FLOAT),
            9, 1, 0,
            transferFunc
    );

    Affine3f modelMatrix = SceneGraph::getEigenAffineTransformationFromData(input);
    modelMatrix.scale(input->getSpacing());
    Matrix4f invViewMatrix = (viewingMatrix*modelMatrix.matrix()).inverse();

    auto inverseViewMatrixBuffer = cl::Buffer(
            device->getContext(),
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            16*sizeof(float),
            invViewMatrix.data()
    );
    // TODO probably don't need this:
    auto modelMatrixBuffer = cl::Buffer(
            device->getContext(),
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            16*sizeof(float),
            modelMatrix.data()
    );

    mKernel.setArg(0, *clImage);
    mKernel.setArg(2, d_transferFuncArray);
    mKernel.setArg(3, density);
    mKernel.setArg(4, brightness);
    mKernel.setArg(5, transferOffset);
    mKernel.setArg(6, transferScale);
    mKernel.setArg(7, inverseViewMatrixBuffer);
    mKernel.setArg(8, modelMatrixBuffer);
    mKernel.setArg(9, inputColor);
    mKernel.setArg(10, inputDepth);
    queue.enqueueNDRangeKernel(
            mKernel,
            cl::NullRange,
            cl::NDRange(gridSize.x(), gridSize.y()),
            cl::NullRange
    );


    // Attach texture to framebuffer
    if(m_texture == 0)
        glGenTextures(1, &m_texture);

    auto data = make_uninitialized_unique<float[]>(gridSize.x()*gridSize.y()*4);
    queue.enqueueReadImage(
            image,
            CL_TRUE,
            createOrigoRegion(),
            createRegion(gridSize.x(), gridSize.y(), 1),
            0, 0,
            data.get()
    );

    // Transfer texture data
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gridSize.x(), gridSize.y(), 0, GL_RGBA, GL_FLOAT, data.get());

    // Set texture to FBO
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

    // Blit/copy the framebuffer to the default framebuffer (window)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mainFBO);
    glBlitFramebuffer(0, 0, gridSize.x(), gridSize.y(), viewport[0], viewport[1], viewport[2], viewport[3], GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Reset framebuffer to default framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mainFBO);
}

VolumeRenderer::~VolumeRenderer() {
    if(m_FBO != -1)
        glDeleteFramebuffers(1, &m_FBO);
    if(m_texture != -1)
        glDeleteFramebuffers(1, &m_texture);
}

VolumeRenderer::VolumeRenderer() {
    createInputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/VolumeRenderer/VolumeRenderer.cl");
}

}