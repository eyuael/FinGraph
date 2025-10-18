#include "services/DataService.h"
#include <fstream>
#include <filesystem>
#include <random>
#include <sstream>
#include <iomanip>

namespace fingraph {

DataService::DataService(std::string uploadDirectory) : uploadDirectory_(std::move(uploadDirectory)) {
    // Ensure the upload directory exists
    std::filesystem::create_directories(uploadDirectory_);
}

std::string DataService::saveUploadedFile(const std::string& originalFilename, const std::string& fileContent) {
    std::string uniqueFilename = generateUniqueFilename(originalFilename);
    std::string fullPath = uploadDirectory_ + "/" + uniqueFilename;

    std::ofstream outFile(fullPath, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Failed to create file: " + fullPath);
    }

    outFile.write(fileContent.data(), fileContent.size());
    if (!outFile) {
        throw std::runtime_error("Failed to write to file: " + fullPath);
    }

    return uniqueFilename; // The uniqueFilename is our dataId
}

std::string DataService::getDataPath(const std::string& dataId) const {
    return uploadDirectory_ + "/" + dataId;
}

std::vector<std::string> DataService::listAvailableData() const {
    std::vector<std::string> dataIds;
    for (const auto& entry : std::filesystem::directory_iterator(uploadDirectory_)) {
        if (entry.is_regular_file()) {
            dataIds.push_back(entry.path().filename().string());
        }
    }
    return dataIds;
}

std::string DataService::generateUniqueFilename(const std::string& originalFilename) {
    // Simple unique filename generator using a random hex string
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; ++i) {
        ss << std::setw(2) << dis(gen);
    }

    std::filesystem::path p(originalFilename);
    return ss.str() + p.extension().string();
}

} // namespace fingraph