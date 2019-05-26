//
// Created by christoph on 26.05.19.
//

#ifndef MARCHINGCUBESSERVER_CINDYSCRIPTPARSER_HPP
#define MARCHINGCUBESSERVER_CINDYSCRIPTPARSER_HPP

#include <string>
#include <map>
#include <json/json.h>

/**
 * Evaluates a CindyScript function returning a floating point number.
 * @param expr The CindyScript expression to evaluate (as a parsed source tree).
 * @param variables The variables to use as substitutions in the expressions.
 * @return The evaluated value.
 */
float evaluateExpressionCdy(Json::Value &expr, std::map<std::string, float> &variables);

#endif //MARCHINGCUBESSERVER_CINDYSCRIPTPARSER_HPP
