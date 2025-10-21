# Multi-stage build for Data Ingestion Service
FROM fingraph-cpp-base:latest as builder

# Switch to root for build
USER root

# Copy source code
COPY ../../data_ingestion /app/data_ingestion
COPY ../../simulation_engine /app/simulation_engine

# Create build directory
WORKDIR /app/data_ingestion/build

# Build the data ingestion HTTP server
RUN cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    && make -j$(nproc)

# Production stage
FROM ubuntu:22.04

# Install runtime dependencies including HTTP server dependencies
RUN apt-get update && apt-get install -y \
    libpq5 \
    libssl3 \
    curl \
    libcurl4 \
    libsqlite3-0 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -u 1000 fingraph && mkdir -p /app && chown -R fingraph:fingraph /app
USER fingraph

# Copy built binary from builder stage
COPY --from=builder /app/data_ingestion/build/data_ingestion_http_server /app/data_ingestion_http_server

# Copy configuration files
COPY --chown=fingraph:fingraph ../../data_ingestion/config /app/config

# Set working directory
WORKDIR /app

# Expose HTTP port
EXPOSE 8081

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8081/health || exit 1

# Run the HTTP server
CMD ["./data_ingestion_http_server", "--address", "0.0.0.0:8081"]