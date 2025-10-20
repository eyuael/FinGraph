# Multi-stage build for Web Server Service
FROM fingraph-cpp-base:latest as builder

# Switch to root for build
USER root

# Copy source code
COPY ../../web_server /app/web_server
COPY ../../simulation_engine /app/simulation_engine

# First, build the simulation engine library
WORKDIR /app/simulation_engine/build
RUN cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    && make -j$(nproc) fingraph_simulation

# Now build the web server
WORKDIR /app/web_server/build
RUN cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    && make -j$(nproc)

# Production stage
FROM ubuntu:22.04

# Install runtime dependencies only
RUN apt-get update && apt-get install -y \
    libssl3 \
    libpq5 \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -u 1000 fingraph && mkdir -p /app && chown -R fingraph:fingraph /app
USER fingraph

# Copy built binary from builder stage
COPY --from=builder /app/web_server/build/FinGraphWebServer /app/FinGraphWebServer

# Copy configuration files
COPY --chown=fingraph:fingraph ../../web_server/config /app/config

# Set working directory
WORKDIR /app

# Expose port
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/api/v1/health || exit 1

# Run the service
CMD ["./FinGraphWebServer"]