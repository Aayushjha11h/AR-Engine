#include "Language/Parser.h"
#include "Language/Interpreter.h"
#include <iostream>
#include <string>

int main() {
    const std::string source =
        "#EnemyBase Blueprint {\n"
        "    color: red\n"
        "    width: 48\n"
        "    height: 48\n"
        "    health: 3\n"
        "    patrol_speed: 60\n"
        "}\n"
        "#Skeleton Blueprint extends EnemyBase {\n"
        "    color: white\n"
        "    sprite: \"skeleton.png\"\n"
        "    patrol_speed: 90\n"
        "}\n"
        "#Ghost Blueprint extends EnemyBase {\n"
        "    color: cyan\n"
        "    health: 2\n"
        "    patrol_speed: 40\n"
        "}\n"
        "#Boss Blueprint extends EnemyBase {\n"
        "    width: 128\n"
        "    height: 128\n"
        "    health: 20\n"
        "    boss: true\n"
        "}\n"
        "#SkeletonA Skeleton at 200 400\n"
        "#FinalBoss Boss at 1200 400 {\n"
        "    health: 50\n"
        "    patrol_speed: 140\n"
        "}\n";

    ar::Lexer lexer(source);
    ar::Parser parser(lexer);
    if (!parser.Parse()) {
        std::cerr << parser.GetError() << std::endl;
        return 1;
    }

    ar::Interpreter interpreter;
    if (!interpreter.Load(parser.GetAST())) {
        std::cerr << interpreter.GetError() << std::endl;
        return 2;
    }

    const auto* skeleton = interpreter.GetBlueprint("Skeleton");
    if (!skeleton) {
        std::cerr << "Skeleton blueprint missing" << std::endl;
        return 3;
    }

    const auto* skeletonA = interpreter.GetObject("SkeletonA");
    if (!skeletonA) {
        std::cerr << "SkeletonA instance missing" << std::endl;
        return 4;
    }

    const auto* boss = interpreter.GetObject("FinalBoss");
    if (!boss) {
        std::cerr << "FinalBoss instance missing" << std::endl;
        return 5;
    }

    std::cout << skeleton->properties.at("color") << "|"
              << skeletonA->properties.at("patrol_speed") << "|"
              << skeletonA->properties.at("x") << "|"
              << skeletonA->properties.at("y") << "|"
              << boss->properties.at("health") << "|"
              << boss->properties.at("patrol_speed") << std::endl;
    return 0;
}
