# MSBG Pressure Solver Integration

## Overview

This is a skeleton integration of MSBG (Multi-Scale Block Gauss-Seidel) pressure solver for the FLIP Fluids engine. This PR provides the build system scaffolding and adapter interface for MSBG integration, with stub implementations.

## Build Options

### Default Build (MSBG Disabled)
```bash
cmake ..
# or explicitly
cmake -DFLIP_USE_MSBG=OFF ..
```

The project builds normally without MSBG support. The MSBG adapter files are excluded from the build.

### Build with MSBG Support

To enable MSBG support:

```bash
cmake -DFLIP_USE_MSBG=ON ..
```

MSBG can be provided in two ways:

#### Option A: Vendored MSBG (Recommended for development)
Place MSBG source code in `third_party/msbg/` with a `CMakeLists.txt` that defines a `msbg` target.

```
third_party/
└── msbg/
    ├── CMakeLists.txt  # Must define target 'msbg'
    ├── include/
    └── src/
```

#### Option B: System-Installed MSBG
1. Install MSBG on your system
2. Either:
   - Set the `MSBG_ROOT` environment variable to point to MSBG installation, or
   - Ensure MSBG is in a standard location where `find_package(MSBG)` can find it

Example:
```bash
export MSBG_ROOT=/usr/local/msbg
cmake -DFLIP_USE_MSBG=ON ..
```

### Build Error Handling

If `FLIP_USE_MSBG=ON` but MSBG is not found, CMake will emit a clear error:
```
FLIP_USE_MSBG is ON but MSBG library not found.
Please install MSBG or set MSBG_ROOT environment variable,
or place MSBG source in third_party/msbg directory.
```

## MSBG Adapter API

The adapter is defined in:
- `src/engine/msbg_pressure_adapter.h` - Interface
- `src/engine/msbg_pressure_adapter.cpp` - Implementation (stub)

### Key Methods (Stub Implementations)

```cpp
class MSBGPressureAdapter {
public:
    // Initialize from domain parameters
    bool initializeFromDomain(const PressureSolverParameters& params,
                             const MACVelocityField& velocityField,
                             const ParticleLevelSet& liquidPLS,
                             const MeshLevelSet& solidLevelSet);

    // Upload right-hand side
    bool uploadRhsFromNegativeDivergence(const Array3d<double>& rhsGrid);
    bool uploadRhsFromVector(const std::vector<double>& rhsVector,
                            const std::vector<int>& pressureCellIndices);

    // Solve pressure system
    bool solvePressure(int maxIterations, double tolerance, 
                      int& outIterations, double& outError);

    // Download solution
    bool downloadSolutionToPressureGrid(Array3d<float>& pressureGrid);
    bool downloadSolutionToVector(std::vector<double>& solutionVector);

    // Status
    bool isInitialized() const;
    std::string getStatusMessage() const;
};
```

## Current Status

**This is a skeleton implementation only.** All methods contain stub implementations that:
- Compile successfully with `FLIP_USE_MSBG` defined or undefined
- Return appropriate status values
- Do not perform actual MSBG operations

## Next Steps (Future PRs)

1. Complete the adapter implementation to:
   - Create and configure MSBG grid structures
   - Map between FLIP Fluids data structures and MSBG format
   - Call actual MSBG solver routines
   - Extract and convert results back

2. Integrate the adapter into the pressure solver:
   - Add runtime option to choose between PCG and MSBG solvers
   - Wire the adapter into `PressureSolver::solve()`
   - Add performance benchmarks

3. Testing and validation:
   - Unit tests for the adapter
   - Integration tests with actual simulations
   - Performance comparisons

## Testing

To verify the skeleton builds correctly:

```bash
# Test without MSBG (default)
mkdir build && cd build
cmake ..
# Should configure successfully with message: "MSBG pressure solver support disabled"

# Test with MSBG (requires MSBG or mock in third_party/msbg)
mkdir build_msbg && cd build_msbg
cmake -DFLIP_USE_MSBG=ON ..
# Should configure successfully with message: "MSBG pressure solver support enabled"
```

For quick compilation test without full build:
```bash
cd src/engine
# Test without MSBG
g++ -std=c++17 -c -I. msbg_pressure_adapter.cpp

# Test with MSBG
g++ -std=c++17 -DFLIP_USE_MSBG -c -I. msbg_pressure_adapter.cpp
```

## License

This integration follows the same MIT License as the rest of the FLIP Fluids project.
