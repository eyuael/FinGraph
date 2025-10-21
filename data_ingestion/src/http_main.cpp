#include <iostream>
#include <memory>
#include <string>
#include <csignal>
#include "fingraph/DataIngestionServer.h"

// Global server instance for signal handling
std::unique_ptr<fingraph::DataIngestionServer> g_server;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down gracefully..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
}

void printUsage(const char* program_name) {
    std::cout << "FinGraph Data Ingestion HTTP Server" << std::endl;
    std::cout << "Usage: " << program_name << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --address <addr>  Server address (default: 0.0.0.0:8081)" << std::endl;
    std::cout << "  --database <path> Database file path (default: fingraph_data.db)" << std::endl;
    std::cout << "  --help           Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "API Endpoints:" << std::endl;
    std::cout << "  POST /api/v1/data/upload     Upload market data" << std::endl;
    std::cout << "  GET  /api/v1/data/list       List available data" << std::endl;
    std::cout << "  GET  /api/v1/data/{id}       Get data info" << std::endl;
    std::cout << "  GET  /api/v1/data/{id}/preview Preview data" << std::endl;
    std::cout << "  GET  /api/v1/data/{id}/download Download data" << std::endl;
    std::cout << "  DELETE /api/v1/data/{id}      Delete data" << std::endl;
    std::cout << "  POST /api/v1/data/fetch      Fetch from external API" << std::endl;
    std::cout << "  GET  /health                 Health check" << std::endl;
}

int main(int argc, char* argv[]) {
    // Set up signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Parse command line arguments
    std::string server_address = "0.0.0.0:8081";
    std::string database_path = "fingraph_data.db";
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--address" && i + 1 < argc) {
            server_address = argv[++i];
        } else if (arg == "--database" && i + 1 < argc) {
            database_path = argv[++i];
        } else if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
    }
    
    try {
        // Create and configure the server
        g_server = std::make_unique<fingraph::DataIngestionServer>(database_path);
        
        std::cout << "Starting FinGraph Data Ingestion HTTP Server..." << std::endl;
        std::cout << "Server address: " << server_address << std::endl;
        std::cout << "Database path: " << database_path << std::endl;
        
        // Start the server
        if (!g_server->start(server_address)) {
            std::cerr << "Failed to start server" << std::endl;
            return 1;
        }
        
        std::cout << "Server started successfully!" << std::endl;
        std::cout << "API documentation available at: http://" << server_address << "/docs" << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        
        // Wait for the server to finish
        g_server->wait();
        
        std::cout << "Server shutdown complete" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}