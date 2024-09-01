#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <optional>

class CommandLineArgs {
private:
    std::string firstArg;
    std::unordered_map<std::string, std::string> args;
    void parseArg(const std::string& arg, const std::string& value = "");

public:
    CommandLineArgs(int argc, char* argv[]);
    bool hasFirstArg();
    std::string getFirstArg();
    bool hasArg(const std::string& key);
    std::string getArg(const std::string& key, const std::string& defaultValue = "");
    int getIntArg(const std::string& key, int defaultValue = 0);
    double getDoubleArg(const std::string& key, double defaultValue = 0.0);
    void printArgs();
};