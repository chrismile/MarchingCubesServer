//
// Created by christoph on 26.05.19.
//

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
