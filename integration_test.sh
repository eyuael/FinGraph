#!/bin/bash

# FinGraph Integration Test Script
# Tests the gRPC-based microservices architecture

set -e

echo "🚀 Starting FinGraph Integration Tests"
echo "===================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test functions
test_service() {
    local service_name=$1
    local port=$2
    local endpoint=$3
    
    echo -n "Testing $service_name on port $port... "
    
    if curl -f -s "http://localhost:$port$endpoint" > /dev/null 2>&1; then
        echo -e "${GREEN}✓ PASS${NC}"
        return 0
    else
        echo -e "${RED}✗ FAIL${NC}"
        return 1
    fi
}

test_grpc_service() {
    local service_name=$1
    local port=$2
    
    echo -n "Testing $service_name gRPC on port $port... "
    
    # Simple port check for gRPC (would need grpcurl for proper testing)
    if nc -z localhost $port 2>/dev/null; then
        echo -e "${GREEN}✓ PASS${NC}"
        return 0
    else
        echo -e "${RED}✗ FAIL${NC}"
        return 1
    fi
}

# Build and start services
echo -e "${YELLOW}📦 Building services...${NC}"
docker-compose -f docker/docker-compose.yml build --no-cache

echo -e "${YELLOW}🚀 Starting services...${NC}"
docker-compose -f docker/docker-compose.yml up -d

# Wait for services to be ready
echo -e "${YELLOW}⏳ Waiting for services to be ready...${NC}"
sleep 30

# Run tests
echo -e "\n${YELLOW}🧪 Running integration tests...${NC}"

FAILED_TESTS=0

# Test Data Ingestion Service
echo -e "\n${YELLOW}Testing Data Ingestion Service:${NC}"
if ! test_service "Data Ingestion" 8081 "/health"; then
    ((FAILED_TESTS++))
fi

# Test Simulation Engine gRPC Service
echo -e "\n${YELLOW}Testing Simulation Engine gRPC Service:${NC}"
if ! test_grpc_service "Simulation Engine" 50051; then
    ((FAILED_TESTS++))
fi

# Test Web Server
echo -e "\n${YELLOW}Testing Web Server:${NC}"
if ! test_service "Web Server" 8080 "/api/v1/health"; then
    ((FAILED_TESTS++))
fi

# Test API endpoints
echo -e "\n${YELLOW}Testing API Endpoints:${NC}"

# Test strategies endpoint
echo -n "Testing strategies endpoint... "
if curl -f -s "http://localhost:8080/api/v1/strategies" > /dev/null 2>&1; then
    echo -e "${GREEN}✓ PASS${NC}"
else
    echo -e "${RED}✗ FAIL${NC}"
    ((FAILED_TESTS++))
fi

# Test data list endpoint
echo -n "Testing data list endpoint... "
if curl -f -s "http://localhost:8081/api/v1/data/list" > /dev/null 2>&1; then
    echo -e "${GREEN}✓ PASS${NC}"
else
    echo -e "${RED}✗ FAIL${NC}"
    ((FAILED_TESTS++))
fi

# Test backtest submission
echo -n "Testing backtest submission... "
BACKTEST_RESPONSE=$(curl -s -X POST "http://localhost:8080/api/v1/backtest" \
    -H "Content-Type: application/json" \
    -d '{
        "dataId": "test_data",
        "strategy": "MovingAverage",
        "initialCash": 10000,
        "parameters": {"short_window": 10, "long_window": 20}
    }' || echo "")

if [[ $BACKTEST_RESPONSE == *"job_id"* ]]; then
    echo -e "${GREEN}✓ PASS${NC}"
    JOB_ID=$(echo $BACKTEST_RESPONSE | grep -o '"job_id":"[^"]*"' | cut -d'"' -f4)
    echo "  Job ID: $JOB_ID"
else
    echo -e "${RED}✗ FAIL${NC}"
    ((FAILED_TESTS++))
fi

# Test service communication
echo -e "\n${YELLOW}Testing Service Communication:${NC}"

# Check if services can reach each other
echo -n "Testing web server to data ingestion... "
if docker exec fingraph-web-server curl -f -s "http://data-ingestion:8081/health" > /dev/null 2>&1; then
    echo -e "${GREEN}✓ PASS${NC}"
else
    echo -e "${RED}✗ FAIL${NC}"
    ((FAILED_TESTS++))
fi

echo -n "Testing web server to simulation engine... "
if docker exec fingraph-web-server nc -z simulation-engine 50051 2>/dev/null; then
    echo -e "${GREEN}✓ PASS${NC}"
else
    echo -e "${RED}✗ FAIL${NC}"
    ((FAILED_TESTS++))
fi

# Results
echo -e "\n${YELLOW}📊 Test Results:${NC}"
echo "===================================="

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}🎉 All tests passed!${NC}"
    echo -e "${GREEN}✅ Integration test successful${NC}"
    EXIT_CODE=0
else
    echo -e "${RED}❌ $FAILED_TESTS test(s) failed${NC}"
    echo -e "${RED}❌ Integration test failed${NC}"
    EXIT_CODE=1
fi

# Show logs for debugging
echo -e "\n${YELLOW}📋 Service Logs (last 10 lines):${NC}"
echo "===================================="

echo -e "${YELLOW}Data Ingestion:${NC}"
docker logs --tail 10 fingraph-data-ingestion 2>/dev/null || echo "No logs available"

echo -e "\n${YELLOW}Simulation Engine:${NC}"
docker logs --tail 10 fingraph-simulation-engine 2>/dev/null || echo "No logs available"

echo -e "\n${YELLOW}Web Server:${NC}"
docker logs --tail 10 fingraph-web-server 2>/dev/null || echo "No logs available"

# Cleanup
echo -e "\n${YELLOW}🧹 Cleaning up...${NC}"
docker-compose -f docker/docker-compose.yml down

echo -e "\n${YELLOW}🏁 Integration test complete${NC}"
echo "===================================="

exit $EXIT_CODE