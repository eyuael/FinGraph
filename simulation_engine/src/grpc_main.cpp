#include <iostream>
#include <memory>
#include <string>
#include <csignal>
#include "fingraph/SimulationEngineServer.h"

// Global server instance for signal handling
std::unique_ptr<fingraph::SimulationEngineServer> g_server;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down gracefully..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    // Set up signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Parse command line arguments
    std::string server_address = "0.0.0.0:50051";
    size_t max_concurrent_jobs = 4;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--address" && i + 1 < argc) {
            server_address = argv[++i];
        } else if (arg == "--max-jobs" && i + 1 < argc) {
            max_concurrent_jobs = std::stoul(argv[++i]);
        } else if (arg == "--help") {
            std::cout << "FinGraph Simulation Engine gRPC Server" << std::endl;
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --address <addr>  Server address (default: 0.0.0.0:50051)" << std::endl;
            std::cout << "  --max-jobs <num>  Maximum concurrent jobs (default: 4)" << std::endl;
            std::cout << "  --help           Show this help message" << std::endl;
            return 0;
        }
    }
    
    try {
        // Create and configure the server
        g_server = std::make_unique<fingraph::SimulationEngineServer>(max_concurrent_jobs);
        
        std::cout << "Starting FinGraph Simulation Engine gRPC Server..." << std::endl;
        std::cout << "Server address: " << server_address << std::endl;
        std::cout << "Max concurrent jobs: " << max_concurrent_jobs << std::endl;
        
        // Start the server
        if (!g_server->start(server_address)) {
            std::cerr << "Failed to start server" << std::endl;
            return 1;
        }
        
        std::cout << "Server started successfully!" << std::endl;
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