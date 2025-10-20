#!/bin/bash

# FinGraph Complete Deployment Script
# This script builds, pushes, and deploys all FinGraph services

set -e

# Configuration
SERVICES=("data-ingestion" "web-server" "simulation-engine")
TAG="latest"
REGISTRY=""  # Set to your registry (e.g., docker.io/username)
BUILD=false
PUSH=false
DEPLOY=false
LOCAL=false
ENVIRONMENT="dev"
COMPOSE_FILE="docker-compose.yml"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
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

print_header() {
    echo -e "${BLUE}=== $1 ===${NC}"
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --build          Build all Docker images"
    echo "  --push           Push images to registry"
    echo "  --deploy         Deploy to production"
    echo "  --local          Deploy locally for development"
    echo "  --tag TAG        Set image tag (default: latest)"
    echo "  --registry REG   Set registry prefix"
    echo "  --env ENV        Set environment (dev|prod, default: dev)"
    echo "  --no-cache       Build without cache"
    echo "  --help           Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 --build --local                    # Build and run locally"
    echo "  $0 --build --push --tag v1.0.0       # Build and push with tag"
    echo "  $0 --deploy --env prod               # Deploy to production"
    echo "  $0 --build --push --deploy           # Build, push, and deploy"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --build)
            BUILD=true
            shift
            ;;
        --push)
            PUSH=true
            shift
            ;;
        --deploy)
            DEPLOY=true
            shift
            ;;
        --local)
            LOCAL=true
            BUILD=true
            shift
            ;;
        --tag)
            TAG="$2"
            shift 2
            ;;
        --registry)
            REGISTRY="$2/"
            shift 2
            ;;
        --env)
            ENVIRONMENT="$2"
            if [[ "$ENVIRONMENT" == "prod" ]]; then
                COMPOSE_FILE="docker-compose.prod.yml"
            fi
            shift 2
            ;;
        --no-cache)
            NO_CACHE="--no-cache"
            shift
            ;;
        --help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Validate arguments
if [[ "$BUILD" == false && "$PUSH" == false && "$DEPLOY" == false && "$LOCAL" == false ]]; then
    print_error "No action specified. Use --build, --push, --deploy, or --local"
    show_usage
    exit 1
fi

# Check if we're in the correct directory
if [[ ! -f "docker/scripts/deploy-all.sh" ]]; then
    print_error "Please run this script from the FinGraph root directory."
    exit 1
fi

# Check if Docker is running
if ! docker info > /dev/null 2>&1; then
    print_error "Docker is not running. Please start Docker and try again."
    exit 1
fi

print_header "FinGraph Deployment Script"
print_status "Environment: ${ENVIRONMENT}"
print_status "Tag: ${TAG}"
print_status "Registry: ${REGISTRY:-none}"
print_status "Compose File: ${COMPOSE_FILE}"

# Build all images
if [[ "$BUILD" == true ]]; then
    print_header "Building All Services"
    
    # Build base image first
    print_status "Building base image..."
    if ! docker build -t fingraph-cpp-base:latest -f docker/base/Dockerfile.cpp-base .; then
        print_error "Failed to build base image"
        exit 1
    fi
    
    # Build each service
    for service in "${SERVICES[@]}"; do
        print_status "Building ${service}..."
        if ! ./docker/scripts/build-${service}.sh --tag "${TAG}" --registry "${REGISTRY}" ${NO_CACHE}; then
            print_error "Failed to build ${service}"
            exit 1
        fi
    done
    
    print_status "All images built successfully!"
fi

# Push images to registry
if [[ "$PUSH" == true ]]; then
    print_header "Pushing Images to Registry"
    
    for service in "${SERVICES[@]}"; do
        print_status "Pushing ${service}..."
        if ! ./docker/scripts/build-${service}.sh --tag "${TAG}" --registry "${REGISTRY}" --push; then
            print_error "Failed to push ${service}"
            exit 1
        fi
    done
    
    print_status "All images pushed successfully!"
fi

# Deploy locally
if [[ "$LOCAL" == true ]]; then
    print_header "Deploying Locally"
    
    # Stop existing containers
    print_status "Stopping existing containers..."
    docker-compose -f "${COMPOSE_FILE}" down --remove-orphans || true
    
    # Build and start services
    print_status "Starting services..."
    if ! docker-compose -f "${COMPOSE_FILE}" up --build -d; then
        print_error "Failed to start services"
        exit 1
    fi
    
    # Wait for services to be healthy
    print_status "Waiting for services to be healthy..."
    sleep 10
    
    # Show status
    print_status "Service status:"
    docker-compose -f "${COMPOSE_FILE}" ps
    
    print_status "Local deployment completed!"
    print_status "Web Server: http://localhost:8080"
    print_status "Data Ingestion: http://localhost:8081"
    print_status "Simulation Engine: http://localhost:8082"
fi

# Deploy to production
if [[ "$DEPLOY" == true ]]; then
    print_header "Deploying to Production"
    
    # Check if environment file exists
    if [[ "$ENVIRONMENT" == "prod" && ! -f "docker/.env.prod" ]]; then
        print_error "Production environment file not found: docker/.env.prod"
        exit 1
    fi
    
    # Set environment file
    ENV_FILE=""
    if [[ -f "docker/.env.${ENVIRONMENT}" ]]; then
        ENV_FILE="--env-file docker/.env.${ENVIRONMENT}"
    fi
    
    # Deploy to remote server (placeholder for actual deployment logic)
    print_warning "Production deployment requires SSH access to Hetzner VPS"
    print_status "To deploy to Hetzner VPS, run:"
    print_status "  scp -r docker/ user@your-hetzner-server:/path/to/fingraph/"
    print_status "  ssh user@your-hetzner-server 'cd /path/to/fingraph && docker-compose -f ${COMPOSE_FILE} ${ENV_FILE} up -d'"
    
    # For now, just run docker-compose on current machine
    print_status "Running production compose file..."
    if ! docker-compose -f "${COMPOSE_FILE}" ${ENV_FILE} up -d; then
        print_error "Failed to deploy to production"
        exit 1
    fi
    
    print_status "Production deployment completed!"
fi

print_header "Deployment Summary"
if [[ "$BUILD" == true ]]; then
    print_status "✓ Images built"
fi
if [[ "$PUSH" == true ]]; then
    print_status "✓ Images pushed to registry"
fi
if [[ "$LOCAL" == true ]]; then
    print_status "✓ Local deployment completed"
fi
if [[ "$DEPLOY" == true ]]; then
    print_status "✓ Production deployment completed"
fi

print_status "FinGraph deployment process completed successfully!"