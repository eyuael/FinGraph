#pragma once

#include <string>
#include <vector>

namespace fingraph {

class DataService {
public:
    explicit DataService(std::string uploadDirectory);
    ~DataService() = default;

    // Saves an uploaded file and returns a unique dataId (the filename).
    std::string saveUploadedFile(const std::string& originalFilename, const std::string& fileContent);

    // Gets the full filesystem path for a given dataId.
    std::string getDataPath(const std::string& dataId) const;

    // Lists all available data files (dataIds).
    std::vector<std::string> listAvailableData() const;

private:
    std::string uploadDirectory_;
    
    // Helper to generate a unique filename to prevent overwrites.
    static std::string generateUniqueFilename(const std::string& originalFilename);
};

} // namespace fingraph