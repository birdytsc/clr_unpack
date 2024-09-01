#include "CommandLineArgs.h"

CommandLineArgs::CommandLineArgs(int argc, char* argv[])
{
    if (argc > 1 && argv[1][0] != '-') {
        firstArg = argv[1];
    }

    for (int i = !firstArg.empty() ? 2 : 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg[0] == '-') {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                parseArg(arg, argv[i + 1]);
                ++i;  // Skip the next argument as we've used it as a value
            }
            else {
                parseArg(arg);
            }
        }
    }
}

void CommandLineArgs::parseArg(const std::string& arg, const std::string& value)
{
    if (arg.substr(0, 2) == "--") {
        size_t pos = arg.find('=');
        if (pos != std::string::npos) {
            std::string key = arg.substr(2, pos - 2);
            std::string val = arg.substr(pos + 1);
            args[key] = val;
        }
        else {
            args[arg.substr(2)] = value.empty() ? "true" : value;
        }
    }
    else if (arg[0] == '-' && arg.length() == 2) {
        args[arg.substr(1)] = value.empty() ? "true" : value;
    }
}

bool CommandLineArgs::hasFirstArg()
{
    return !firstArg.empty();
}

std::string CommandLineArgs::getFirstArg()
{
    return firstArg;
}

bool CommandLineArgs::hasArg(const std::string& key)
{
    return args.find(key) != args.end();
}

std::string CommandLineArgs::getArg(const std::string& key, const std::string& defaultValue)
{
    auto it = args.find(key);
    return (it != args.end()) ? it->second : defaultValue;
}

int CommandLineArgs::getIntArg(const std::string& key, int defaultValue)
{
    auto it = args.find(key);
    if (it != args.end()) {
        try {
            return std::stoi(it->second);
        }
        catch (const std::exception& e) {
            std::cerr << "Error: Invalid integer value for argument '" << key << "'" << std::endl;
        }
    }
    return defaultValue;
}

double CommandLineArgs::getDoubleArg(const std::string& key, double defaultValue)
{
    auto it = args.find(key);
    if (it != args.end()) {
        try {
            return std::stod(it->second);
        }
        catch (const std::exception& e) {
            std::cerr << "Error: Invalid double value for argument '" << key << "'" << std::endl;
        }
    }
    return defaultValue;
}

void CommandLineArgs::printArgs()
{
    if (hasFirstArg()) {
        std::cout << "firstArg: " << getFirstArg() << std::endl;
    }
    for (const auto& pair : args) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
}