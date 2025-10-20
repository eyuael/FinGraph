# FinGraph Docker Deployment Guide

This directory contains all the Docker configuration files and scripts for deploying FinGraph services.

## 📁 Directory Structure

```
docker/
├── base/
│   └── Dockerfile.cpp-base          # Base C++ image with dependencies
├── data-ingestion/
│   └── Dockerfile                   # Data Ingestion service
├── web-server/
│   └── Dockerfile                   # Web Server service
├── simulation-engine/
│   └── Dockerfile                   # Simulation Engine service
├── nginx/
│   └── nginx.conf                   # Production Nginx configuration
├── scripts/
│   ├── build-data-ingestion.sh      # Build Data Ingestion image
│   ├── build-web-server.sh          # Build Web Server image
│   ├── build-simulation-engine.sh   # Build Simulation Engine image
│   └── deploy-all.sh                # Complete deployment script
├── docker-compose.yml               # Development environment
├── docker-compose.prod.yml          # Production environment
├── .env.example                     # Environment variables template
├── .env.dev                         # Development environment variables
└── README.md                        # This file
```

## 🚀 Quick Start

### Local Development

1. **Build and run all services locally:**
   ```bash
   ./docker/scripts/deploy-all.sh --local
   ```

2. **Access services:**
   - Web Server: http://localhost:8080
   - Data Ingestion: http://localhost:8081
   - Simulation Engine: http://localhost:8082
   - PostgreSQL: localhost:5432

### Production Deployment

1. **Prepare environment:**
   ```bash
   cp docker/.env.example docker/.env.prod
   # Edit docker/.env.prod with your production values
   ```

2. **Build and push images:**
   ```bash
   ./docker/scripts/deploy-all.sh --build --push --tag v1.0.0
   ```

3. **Deploy to production:**
   ```bash
   ./docker/scripts/deploy-all.sh --deploy --env prod
   ```

## 📋 Individual Service Commands

### Build Individual Services

```bash
# Data Ingestion
./docker/scripts/build-data-ingestion.sh --tag latest --push

# Web Server
./docker/scripts/build-web-server.sh --tag latest --push

# Simulation Engine
./docker/scripts/build-simulation-engine.sh --tag latest --push
```

### Docker Compose Commands

```bash
# Development
docker-compose up -d                    # Start services
docker-compose down                    # Stop services
docker-compose logs -f web-server       # View logs
docker-compose ps                      # Show status

# Production
docker-compose -f docker-compose.prod.yml up -d
docker-compose -f docker-compose.prod.yml down
```

## 🔧 Configuration

### Environment Variables

Key environment variables to configure:

- `POSTGRES_DB`, `POSTGRES_USER`, `POSTGRES_PASSWORD` - Database credentials
- `API_BASE_URL`, `API_KEY` - External API configuration
- `JWT_SECRET` - Security secret for authentication
- `LOG_LEVEL` - Logging level (DEBUG, INFO, WARN, ERROR)

### SSL Certificates

For production deployment, place SSL certificates in:
```
docker/nginx/ssl/
├── cert.pem
└── key.pem
```

## 🖥️ Hetzner VPS Setup

### Prerequisites

1. **Server Requirements:**
   - Ubuntu 22.04 LTS
   - Minimum 4GB RAM, 2 CPU cores
   - At least 40GB storage

2. **Install Docker:**
   ```bash
   curl -fsSL https://get.docker.com -o get-docker.sh
   sudo sh get-docker.sh
   sudo usermod -aG docker $USER
   ```

3. **Install Docker Compose:**
   ```bash
   sudo curl -L "https://github.com/docker/compose/releases/latest/download/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
   sudo chmod +x /usr/local/bin/docker-compose
   ```

### Deployment Steps

1. **Clone repository:**
   ```bash
   git clone <your-repo-url> /opt/fingraph
   cd /opt/fingraph
   ```

2. **Configure environment:**
   ```bash
   cp docker/.env.example docker/.env.prod
   nano docker/.env.prod  # Edit with your values
   ```

3. **Setup SSL certificates:**
   ```bash
   mkdir -p docker/nginx/ssl
   # Copy your SSL certificates to docker/nginx/ssl/
   ```

4. **Deploy:**
   ```bash
   ./docker/scripts/deploy-all.sh --build --deploy --env prod
   ```

### Firewall Configuration

```bash
sudo ufw allow 22/tcp      # SSH
sudo ufw allow 80/tcp      # HTTP
sudo ufw allow 443/tcp     # HTTPS
sudo ufw enable
```

### Monitoring

Check service status:
```bash
docker-compose -f docker-compose.prod.yml ps
docker-compose -f docker-compose.prod.yml logs -f
```

## 🔍 Troubleshooting

### Common Issues

1. **Build failures:**
   - Check Docker daemon is running
   - Verify sufficient disk space
   - Check network connectivity for package downloads

2. **Service startup failures:**
   - Check environment variables in `.env` files
   - Verify port availability
   - Check service logs: `docker-compose logs <service>`

3. **Database connection issues:**
   - Verify PostgreSQL is running: `docker-compose ps postgres`
   - Check database credentials in environment variables
   - Test connection: `docker-compose exec postgres psql -U fingraph -d fingraph`

### Health Checks

All services include health checks. Monitor with:
```bash
docker ps --format "table {{.Names}}\t{{.Status}}"
```

### Performance Tuning

1. **Resource limits:** Adjust memory/CPU limits in `docker-compose.prod.yml`
2. **Database tuning:** Configure PostgreSQL settings in production
3. **Nginx tuning:** Adjust worker connections and caching in `nginx.conf`

## 📊 Monitoring & Logging

### Logs

- Application logs: `docker/logs/`
- Nginx logs: `docker/logs/nginx/`
- Database logs: Docker container logs

### Metrics

Consider setting up:
- Prometheus for metrics collection
- Grafana for visualization
- Alertmanager for notifications

## 🔄 Updates

### Updating Services

1. **Build new images:**
   ```bash
   ./docker/scripts/deploy-all.sh --build --tag v1.1.0
   ```

2. **Deploy update:**
   ```bash
   docker-compose -f docker-compose.prod.yml pull
   docker-compose -f docker-compose.prod.yml up -d
   ```

### Database Migrations

Run migrations before updating:
```bash
docker-compose -f docker-compose.prod.yml exec postgres psql -U fingraph -d fingraph -c "SELECT version();"
```

## 🛡️ Security

1. **Regular updates:** Keep Docker and base images updated
2. **Secrets management:** Use Docker secrets or environment files
3. **Network security:** Use internal networks for service communication
4. **SSL/TLS:** Always use HTTPS in production
5. **Rate limiting:** Configure appropriate limits in Nginx

## 📞 Support

For issues:
1. Check logs: `docker-compose logs <service>`
2. Verify configuration: Check environment variables
3. Test connectivity: Check service health endpoints
4. Review this documentation for common solutions