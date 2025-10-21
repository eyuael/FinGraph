# Multi-stage build for Simulation Engine Service
FROM fingraph-cpp-base:latest as builder

# Switch to root for build
USER root

# Copy source code
COPY ../../simulation_engine /app/simulation_engine

# Create build directory
WORKDIR /app/simulation_engine/build

# Build the simulation engine library, CLI, and gRPC server
RUN cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    && make -j$(nproc)

# Production stage
FROM ubuntu:22.04

# Install runtime dependencies including gRPC
RUN apt-get update && apt-get install -y \
    curl \
    libgrpc++1 \
    libprotobuf23 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -u 1000 fingraph && mkdir -p /app && chown -R fingraph:fingraph /app
USER fingraph

# Copy built binaries from builder stage
COPY --from=builder /app/simulation_engine/build/fingraph_cli /app/fingraph_cli
COPY --from=builder /app/simulation_engine/build/fingraph_grpc_server /app/fingraph_grpc_server
COPY --from=builder /app/simulation_engine/build/libfingraph_simulation.a /app/libfingraph_simulation.a

# Copy test binaries if they exist
COPY --from=builder /app/simulation_engine/build/tests /app/tests 2>/dev/null || true

# Set working directory
WORKDIR /app

# Expose gRPC port
EXPOSE 50051

# Health check for gRPC server
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD ./fingraph_grpc_server --help || exit 1

# Run the gRPC server by default
CMD ["./fingraph_grpc_server", "--address", "0.0.0.0:50051"]