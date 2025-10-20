#!/bin/bash

# FinGraph Simulation Engine Build Script
# This script builds and optionally pushes the Simulation Engine Docker image

set -e

# Configuration
IMAGE_NAME="fingraph/simulation-engine"
TAG="latest"
REGISTRY=""  # Set to your registry (e.g., docker.io/username)
PUSH=false
BUILD_CONTEXT=".."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --tag)
            TAG="$2"
            shift 2
            ;;
        --registry)
            REGISTRY="$2/"
            shift 2
            ;;
        --push)
            PUSH=true
            shift
            ;;
        --no-cache)
            NO_CACHE="--no-cache"
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --tag TAG        Set image tag (default: latest)"
            echo "  --registry REG   Set registry prefix"
            echo "  --push           Push image after build"
            echo "  --no-cache       Build without cache"
            echo "  --help           Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Full image name
FULL_IMAGE_NAME="${REGISTRY}${IMAGE_NAME}:${TAG}"

print_status "Building FinGraph Simulation Engine..."
print_status "Image: ${FULL_IMAGE_NAME}"

# Check if Docker is running
if ! docker info > /dev/null 2>&1; then
    print_error "Docker is not running. Please start Docker and try again."
    exit 1
fi

# Check if we're in the correct directory
if [[ ! -f "docker/simulation-engine/Dockerfile" ]]; then
    print_error "Dockerfile not found. Please run this script from the FinGraph root directory."
    exit 1
fi

# Build the base image first
print_status "Building base image..."
if ! docker build -t fingraph-cpp-base:latest -f docker/base/Dockerfile.cpp-base .; then
    print_error "Failed to build base image"
    exit 1
fi

# Build the Simulation Engine image
print_status "Building Simulation Engine image..."
if ! docker build ${NO_CACHE} \
    -t "${FULL_IMAGE_NAME}" \
    -f docker/simulation-engine/Dockerfile \
    "${BUILD_CONTEXT}"; then
    print_error "Failed to build Simulation Engine image"
    exit 1
fi

print_status "Build completed successfully!"
print_status "Image: ${FULL_IMAGE_NAME}"

# Show image information
print_status "Image details:"
docker images "${FULL_IMAGE_NAME}"

# Test the built image
print_status "Testing built image..."
if docker run --rm "${FULL_IMAGE_NAME}" --version; then
    print_status "Image test passed!"
else
    print_warning "Image test failed, but build completed"
fi

# Push if requested
if [[ "${PUSH}" == "true" ]]; then
    print_status "Pushing image to registry..."
    if docker push "${FULL_IMAGE_NAME}"; then
        print_status "Image pushed successfully!"
    else
        print_error "Failed to push image"
        exit 1
    fi
fi

# Tag as latest if different tag was used
if [[ "${TAG}" != "latest" ]]; then
    LATEST_IMAGE_NAME="${REGISTRY}${IMAGE_NAME}:latest"
    print_status "Tagging as latest..."
    docker tag "${FULL_IMAGE_NAME}" "${LATEST_IMAGE_NAME}"
    
    if [[ "${PUSH}" == "true" ]]; then
        print_status "Pushing latest tag..."
        docker push "${LATEST_IMAGE_NAME}"
    fi
fi

print_status "Simulation Engine build process completed!"