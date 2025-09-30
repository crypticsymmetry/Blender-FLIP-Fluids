/*
MIT License

Copyright (C) 2025 Ryan L. Guy & Dennis Fassbaender

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "msbg_pressure_adapter.h"
#include "pressuresolver.h"
#include "macvelocityfield.h"
#include "particlelevelset.h"
#include "meshlevelset.h"

#ifdef FLIP_USE_MSBG
// When MSBG is available, include actual MSBG headers here
// #include <msbg/msbg.h>
#endif

/********************************************************************************
    MSBGPressureAdapter
********************************************************************************/

MSBGPressureAdapter::MSBGPressureAdapter() : 
    _isInitialized(false),
    _isize(0), _jsize(0), _ksize(0),
    _cellwidth(0.0),
    _statusMessage(""),
    _msbgGrid(nullptr),
    _msbgSolver(nullptr)
{
}

MSBGPressureAdapter::~MSBGPressureAdapter() {
#ifdef FLIP_USE_MSBG
    // Clean up MSBG resources when implemented
    if (_msbgGrid) {
        // delete static_cast<MSBGGrid*>(_msbgGrid);
        _msbgGrid = nullptr;
    }
    if (_msbgSolver) {
        // delete static_cast<MSBGSolver*>(_msbgSolver);
        _msbgSolver = nullptr;
    }
#endif
}

bool MSBGPressureAdapter::initializeFromDomain(
    const PressureSolverParameters& params,
    const MACVelocityField& velocityField,
    const ParticleLevelSet& liquidPLS,
    const MeshLevelSet& solidLevelSet)
{
#ifdef FLIP_USE_MSBG
    // Stub implementation - to be completed in follow-up PRs
    const_cast<MACVelocityField&>(velocityField).getGridDimensions(&_isize, &_jsize, &_ksize);
    _cellwidth = params.cellwidth;
    
    _statusMessage = "MSBG adapter initialized (stub)";
    _isInitialized = true;
    
    // Future implementation:
    // 1. Create MSBG grid with appropriate dimensions
    // 2. Set up fluid/solid cell markers from liquidSDF and solidLevelSet
    // 3. Initialize MSBG solver with grid
    // 4. Configure solver parameters
    
    return true;
#else
    _statusMessage = "MSBG support not compiled in";
    _isInitialized = false;
    return false;
#endif
}

bool MSBGPressureAdapter::uploadRhsFromNegativeDivergence(const Array3d<double>& rhsGrid)
{
#ifdef FLIP_USE_MSBG
    if (!_isInitialized) {
        _statusMessage = "Adapter not initialized";
        return false;
    }
    
    // Stub implementation - to be completed in follow-up PRs
    _statusMessage = "RHS uploaded from grid (stub)";
    
    // Future implementation:
    // 1. Convert Array3d to MSBG grid format
    // 2. Upload to MSBG solver's RHS
    
    return true;
#else
    _statusMessage = "MSBG support not compiled in";
    return false;
#endif
}

bool MSBGPressureAdapter::uploadRhsFromVector(
    const std::vector<double>& rhsVector,
    const std::vector<int>& pressureCellIndices)
{
#ifdef FLIP_USE_MSBG
    if (!_isInitialized) {
        _statusMessage = "Adapter not initialized";
        return false;
    }
    
    // Stub implementation - to be completed in follow-up PRs
    _statusMessage = "RHS uploaded from vector (stub)";
    
    // Future implementation:
    // 1. Map flat vector to MSBG grid using pressureCellIndices
    // 2. Upload to MSBG solver's RHS
    
    return true;
#else
    _statusMessage = "MSBG support not compiled in";
    return false;
#endif
}

bool MSBGPressureAdapter::solvePressure(
    int maxIterations, 
    double tolerance,
    int& outIterations,
    double& outError)
{
#ifdef FLIP_USE_MSBG
    if (!_isInitialized) {
        _statusMessage = "Adapter not initialized";
        return false;
    }
    
    // Stub implementation - to be completed in follow-up PRs
    outIterations = 0;
    outError = 0.0;
    _statusMessage = "Pressure solved with MSBG (stub)";
    
    // Future implementation:
    // 1. Configure MSBG solver with maxIterations and tolerance
    // 2. Call MSBG solve
    // 3. Extract iterations and error from MSBG
    
    return true;
#else
    _statusMessage = "MSBG support not compiled in";
    return false;
#endif
}

bool MSBGPressureAdapter::downloadSolutionToPressureGrid(Array3d<float>& pressureGrid)
{
#ifdef FLIP_USE_MSBG
    if (!_isInitialized) {
        _statusMessage = "Adapter not initialized";
        return false;
    }
    
    // Stub implementation - to be completed in follow-up PRs
    _statusMessage = "Solution downloaded to grid (stub)";
    
    // Future implementation:
    // 1. Extract pressure solution from MSBG
    // 2. Convert to Array3d format
    // 3. Copy to pressureGrid
    
    return true;
#else
    _statusMessage = "MSBG support not compiled in";
    return false;
#endif
}

bool MSBGPressureAdapter::downloadSolutionToVector(std::vector<double>& solutionVector)
{
#ifdef FLIP_USE_MSBG
    if (!_isInitialized) {
        _statusMessage = "Adapter not initialized";
        return false;
    }
    
    // Stub implementation - to be completed in follow-up PRs
    _statusMessage = "Solution downloaded to vector (stub)";
    
    // Future implementation:
    // 1. Extract pressure solution from MSBG
    // 2. Convert to flat vector format
    // 3. Copy to solutionVector
    
    return true;
#else
    _statusMessage = "MSBG support not compiled in";
    return false;
#endif
}

bool MSBGPressureAdapter::isInitialized() const
{
    return _isInitialized;
}

std::string MSBGPressureAdapter::getStatusMessage() const
{
    return _statusMessage;
}
