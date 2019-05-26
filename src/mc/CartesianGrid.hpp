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
#ifndef MARCHINGCUBESSERVER_CARTESIANGRID_HPP
#define MARCHINGCUBESSERVER_CARTESIANGRID_HPP

#include <vector>
#include <json/json.h>
#include <glm/glm.hpp>

struct CartesianGridCorner {
    // Corner position (xyz) and scalar value (w).
    glm::vec3 v;
    float f;
};

/**
 * Constructs a cartesian grid from a scalar field in 3D.
 * @param origin Origin of the cartesian grid.
 * @param dx Distance between two vertices in x direction (assuming dx = dy = dz).
 * @param nx The number of vertices in x direction (assuming nx = ny = nz).
 * @param scalarFunctionCdy A three-dimensional scalar field function vec3 -> Number.
 * The function is stored as a parsed source tree of a CindyScript function.
 * @param variables The free variables in the function to substitute.
 * @return The corners of the cartesian grid (with scalar values attached).
 */
std::vector<CartesianGridCorner> constructCartesianGridScalarField(const glm::vec3 &origin, float dx, uint32_t nx,
        Json::Value &scalarFunctionCdy, Json::Value &variables);

#endif //MARCHINGCUBESSERVER_CARTESIANGRID_HPP
