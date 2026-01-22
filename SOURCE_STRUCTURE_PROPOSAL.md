# Source Code Structure Proposal

## Current Structure

```
simple-utcd/
├── include/simple-utcd/
│   └── [various headers]
├── src/simple-utcd/  # Implementation
└── CMakeLists.txt     # Single build configuration
```

## Proposed Structure

### Modular Directory Structure (Recommended)

```
simple-utcd/
├── src/
│   ├── core/                    # Shared core (Production base)
│   │   ├── utc/
│   │   ├── config/
│   │   └── utils/
│   ├── production/              # Production-specific features
│   │   ├── security/
│   │   └── features/
│   ├── enterprise/              # Enterprise-specific features
│   │   ├── ha/
│   │   ├── management/
│   │   └── integrations/
│   └── datacenter/              # Datacenter-specific features
│       ├── cluster/
│       ├── multi-site/
│       └── cloud/
├── include/
│   └── simple-utcd/
│       ├── core/
│       ├── production/
│       ├── enterprise/
│       └── datacenter/
├── main/
│   ├── production.cpp
│   ├── enterprise.cpp
│   └── datacenter.cpp
└── CMakeLists.txt               # Version-aware build
```

## Build Commands

```bash
# Build Production
cmake -DBUILD_VERSION=production ..
make

# Build Enterprise  
cmake -DBUILD_VERSION=enterprise ..
make

# Build Datacenter
cmake -DBUILD_VERSION=datacenter ..
make
```
