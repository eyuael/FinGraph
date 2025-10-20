# Multi-stage build for Simulation Engine Service
FROM fingraph-cpp-base:latest as builder

# Switch to root for build
USER root

# Copy source code
COPY ../../simulation_engine /app/simulation_engine

# Create build directory
WORKDIR /app/simulation_engine/build

# Build the simulation engine library and CLI
RUN cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    && make -j$(nproc)

# Production stage
FROM ubuntu:22.04

# Install runtime dependencies only
RUN apt-get update && apt-get install -y \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -u 1000 fingraph && mkdir -p /app && chown -R fingraph:fingraph /app
USER fingraph

# Copy built binaries from builder stage
COPY --from=builder /app/simulation_engine/build/fingraph_cli /app/fingraph_cli
COPY --from=builder /app/simulation_engine/build/libfingraph_simulation.a /app/libfingraph_simulation.a

# Copy test binaries if they exist
COPY --from=builder /app/simulation_engine/build/tests /app/tests 2>/dev/null || true

# Set working directory
WORKDIR /app

# Expose port (if running as service)
EXPOSE 8082

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD ./fingraph_cli --version || exit 1

# Run the CLI by default (can be overridden for service mode)
CMD ["./fingraph_cli", "--help"]