/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2019, Christoph Neuhauser
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <thread>
#include <mutex>
#include <iostream>
#include <fstream>

#include "MarchingCubes.hpp"

const int _OPENCL_PLAT_ID_ = 0;

/**
 * Initializes OpenCL, creates a default device and a command queue.
 */
void MarchingCubesImpl::init()
{
    CLInterface::get()->initialize(CLContextInfo(_OPENCL_PLAT_ID_));
    CLInterface::get()->printInfo();

    context = CLInterface::get()->getContext();
    devices = CLInterface::get()->getDevices();
    computeProgram = CLInterface::get()->loadProgramFromSourceFiles({
        "cl/MarchingCubes.cl"
    });

#ifndef _PROFILING_CL_
    queue = cl::CommandQueue(context, devices[0]);
#else
    queue = cl::CommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);
#endif

    size_t maxWorkGroupSize;
    devices[0].getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &maxWorkGroupSize);

    // Set local work size.
    LOCAL_WORK_SIZE = cl::NDRange(64, 4, 1);
    assert(LOCAL_WORK_SIZE[0] * LOCAL_WORK_SIZE[1] * LOCAL_WORK_SIZE[2] <= maxWorkGroupSize);
}

void MarchingCubesImpl::quit()
{
}

/**
 * Writes a list of triangle points to an STL file called "out.stl".
 * @param trianglePoints
 */
/*void testWriteToStlFile(std::vector<glm::vec3> &trianglePoints) {
    std::ofstream file;
    file.open("out.stl");

    std::string fileContent = "solid printmesh\n\n";

    for (size_t i = 0; i < trianglePoints.size(); i += 3) {
        // Compute the facet normal (ignore stored normal data)
        glm::vec3 v0 = trianglePoints.at(i+0);
        glm::vec3 v1 = trianglePoints.at(i+1);
        glm::vec3 v2 = trianglePoints.at(i+2);
        glm::vec3 facetNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        fileContent += std::string() + "facet normal " + std::to_string(facetNormal.x) + " "
                + std::to_string(facetNormal.y) + " " + std::to_string(facetNormal.z) + "\n";
        fileContent += "\touter loop\n";
        for (size_t j = 0; j < 3; j++) {
            glm::vec3 vertex = trianglePoints.at(i+j);
            fileContent += "\t\tvertex " + std::to_string(vertex.x) + " " + std::to_string(vertex.y) + " "
                    + std::to_string(vertex.z) + "\n";
        }
        fileContent += "\tendloop\nendfacet\n";
    }

    fileContent += "\nendsolid printmesh\n";

    file << fileContent;
    file.close();
}*/

static std::mutex mcMutex;

/**
 * Uses the marching cubes algorithm to compute the iso surface of a scalar field approximated by a Cartesian grid.
 * @param nx The number of grid cells in x, y and z direction.
 * @param isoLevel The iso level of the iso surface to construct.
 * @param cartesianGrid The cartesian grid (i.e. a set of regularly arranged points mapped to scalar values).
 * @return The triangle vertex points of the iso surface.
 */
std::vector<glm::vec3> MarchingCubesImpl::marchingCubes(uint32_t nx, float isoLevel,
        const std::vector<CartesianGridCorner> &cartesianGrid)
{
    // Use a lock, as the OpenCL queue isn't multi-threaded and we don't need to handle multiple requests at once.
    std::lock_guard<std::mutex> lock(mcMutex);

    // Used for setting buffers to zero.
    uint32_t zeroUint = 0u;

    // The buffers containing the Cartesian grid data and the vertex counter.
    cl::Buffer cartesianGridBuffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof(CartesianGridCorner) * nx*nx*nx, (void *)&cartesianGrid.front());
    cl::Buffer vertexCounterBuffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            sizeof(uint32_t), (void *)&zeroUint);

    // The enqueue args specify the local and global work size. The global work size is paddes so that it is a multiple
    // of the local work size.
    cl::EnqueueArgs eargs(queue, cl::NullRange, CLInterface::get()->rangePadding3D(nx-1, nx-1, nx-1, LOCAL_WORK_SIZE),
            LOCAL_WORK_SIZE);

    // The kernel used for computing the number of vertices that get generated in a first pass.
    auto computeNumVertices = cl::KernelFunctor<cl::Buffer, cl::Buffer, unsigned int, float>(
            cl::Kernel(computeProgram, "computeNumVertices"));
    computeNumVertices(eargs, cartesianGridBuffer, vertexCounterBuffer, nx, isoLevel);

    // Read the number of vertices that get created.
    uint32_t numVertices = 0;
    queue.enqueueReadBuffer(vertexCounterBuffer, CL_FALSE, 0, sizeof(uint32_t), (void *)&numVertices);
    queue.finish();

    // Reset the counter to zero for the next pass (where we will reuse it).
    queue.enqueueWriteBuffer(vertexCounterBuffer, CL_FALSE, 0, sizeof(uint32_t), (void *)&zeroUint);
    queue.finish();

    if (numVertices == 0) {
        std::cout << "Mesh empty." << std::endl;
        return {};
    }

    // Create a vertex buffer large enough for storing all vertices that get generated by the MC algorithm.
    cl::Buffer vertexBuffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(glm::vec4) * numVertices);

    // Finally, launch the marching cubes algorithm.
    auto marchingCubes = cl::KernelFunctor<cl::Buffer, cl::Buffer, cl::Buffer, unsigned int, float>(
            cl::Kernel(computeProgram, "marchingCubes"));
    marchingCubes(eargs, cartesianGridBuffer, vertexBuffer, vertexCounterBuffer, nx, isoLevel);

    // Now, read the triangle vertices from the buffer on the GPU. On the GPU, float3 arrays get padded to float4
    // arrays. Thus, we directly use float4 arrays in OpenCL for the vertices and convert them to vec3 arrays for use
    // in our application.
    std::vector<glm::vec4> triangleVerticesVec4;
    triangleVerticesVec4.resize(numVertices);
    std::vector<glm::vec3> triangleVertices;
    triangleVertices.resize(numVertices);
    queue.enqueueReadBuffer(vertexBuffer, CL_FALSE, 0, sizeof(glm::vec4)*numVertices, (void *)&triangleVerticesVec4.front());
    queue.finish();
    #pragma omp parallel for
    for (uint32_t i = 0; i < numVertices; i++) {
        glm::vec4 vec4Obj = triangleVerticesVec4.at(i);
        triangleVertices.at(i) = glm::vec3(vec4Obj.x, vec4Obj.y, vec4Obj.z);
    }

    return triangleVertices;
}