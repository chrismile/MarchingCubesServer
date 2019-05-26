//
// Created by christoph on 26.05.19.
//

#include "../CindyScriptParser.hpp"
#include "CartesianGrid.hpp"

std::vector<CartesianGridCorner> constructCartesianGridScalarField(const glm::vec3 &origin, float dx, uint32_t nx,
        Json::Value &scalarFunctionCdy, Json::Value &variables) {
    std::vector<CartesianGridCorner> cartesianGrid;
    cartesianGrid.resize(nx*nx*nx);
    std::map<std::string, float> variableMapGlobal;
    for (auto it = variables.begin(); it != variables.end(); it++) {
        variableMapGlobal[it.key().asString()] = it->asFloat();
    }

    // 1D scalar values at the grid points
    #pragma omp parallel for
    for (uint32_t i = 0; i < nx; i++) {
        for (uint32_t j = 0; j < nx; j++) {
            for (uint32_t k = 0; k < nx; k++) {
                std::map<std::string, float> variableMap = variableMapGlobal;
                CartesianGridCorner &gridCorner = cartesianGrid.at(i*nx*nx + j*nx + k);
                gridCorner.v.x = origin.x + k*dx;
                gridCorner.v.y = origin.y + j*dx;
                gridCorner.v.z = origin.z + i*dx;
                variableMap["x"] = gridCorner.v.x;
                variableMap["y"] = gridCorner.v.y;
                variableMap["z"] = gridCorner.v.z;
                gridCorner.f = evaluateExpressionCdy(scalarFunctionCdy["body"], variableMap);
            }
        }
    }

    return cartesianGrid;
}
