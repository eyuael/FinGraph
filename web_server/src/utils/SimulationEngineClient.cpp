#include "SimulationEngineClient.h"
#include <fstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <unistd.h>
#include <array>

SimulationEngineClient::SimulationEngineClient(std::string enginePath) 
    : enginePath_(std::move(enginePath)) {}

json SimulationEngineClient::runBacktest(
    const std::string& dataPath,
    const std::string& strategyName,
    const std::map<std::string, double>& strategyParams,
    double initialCash) {
    
    // Create temporary config file
    std::string configFile = createConfigFile(dataPath, strategyName, strategyParams, initialCash);
    
    // Run simulation engine
    std::string command = enginePath_ + " " + configFile;
    std::array<char, 128> buffer;
    std::string result;
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    // Clean up config file
    std::remove(configFile.c_str());
    
    // Parse and return result
    try {
        return json::parse(result);
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Failed to parse simulation engine output: " + result);
    }
}

std::string SimulationEngineClient::createConfigFile(
    const std::string& dataPath,
    const std::string& strategyName,
    const std::map<std::string, double>& strategyParams,
    double initialCash) {
    
    // Create temporary file
    std::string tempTemplate = "/tmp/fingraph_config_XXXXXX";
    std::vector<char> tempFile(tempTemplate.begin(), tempTemplate.end());
    tempFile.push_back('\0');
    int fid = mkstemp(tempFile.data());
    if (fid == -1) {
        throw std::runtime_error("Failed to create temporary config file");
    }
    close(fid);
    
    // Write config to file
    json config;
    config["dataPath"] = dataPath;
    config["strategy"] = strategyName;
    config["parameters"] = strategyParams;
    config["initialCash"] = initialCash;
    
    std::string filename(tempFile.data());

    std::ofstream file(filename);
    file << config.dump();
    file.close();
    
    return filename;
}