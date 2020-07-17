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

#include <cmath>
#include <iostream>
#include "CindyScriptParser.hpp"

float evaluateExpressionCdy(Json::Value &expr, std::map<std::string, float> &variables) {
    std::string exprType = expr["ctype"].asString();
    if (exprType == "void") {
        return 0.0f;
    } else if (exprType == "variable") {
        std::string variableName = expr["name"].asString();
        auto it = variables.find(variableName);
        if (it != variables.end()) {
            return it->second;
        }
        return 0.0f;
    } else if (exprType == "number") {
        return expr["value"]["real"].asFloat();
    } else if (exprType == "infix") {
        // Infix operators
        float lhs = evaluateExpressionCdy(expr["args"][0], variables);
        float rhs = evaluateExpressionCdy(expr["args"][1], variables);
        std::string operation = expr["oper"].asString();
        if (operation == "+") {
            return lhs + rhs;
        } else if (operation == "-") {
            return lhs - rhs;
        } else if (operation == "*") {
            return lhs * rhs;
        } else if (operation == "/") {
            return lhs / rhs;
        } else if (operation == "^") {
            return std::pow(lhs, rhs);
        } else if (operation == "=") {
            variables[expr["args"][0]["name"].asString()] = rhs;
            return rhs;
        } else if (operation == ";") {
            return rhs;
        }
    } else if (exprType == "function") {
        std::string operation = expr["oper"].asString();
        if (operation == "sqrt$1") {
            float argument = evaluateExpressionCdy(expr["args"][0], variables);
            return std::sqrt(argument);
        } else if (operation == "sin$1") {
            float argument = evaluateExpressionCdy(expr["args"][0], variables);
            return std::sin(argument);
        } else if (operation == "cos$1") {
            float argument = evaluateExpressionCdy(expr["args"][0], variables);
            return std::cos(argument);
        }
    }

    std::cerr << "Error in evaluateExpressionCdy: Unknown expression." << std::endl;
    return 0.0f;
}
