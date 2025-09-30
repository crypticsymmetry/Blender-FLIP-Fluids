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

#ifndef FLUIDENGINE_MSBG_PRESSURE_ADAPTER_H
#define FLUIDENGINE_MSBG_PRESSURE_ADAPTER_H

#include <vector>
#include "array3d.h"

class MACVelocityField;
class ParticleLevelSet;
class MeshLevelSet;
struct PressureSolverParameters;

/********************************************************************************
    MSBGPressureAdapter
    
    Bridge between existing FLIP Fluids solver and MSBG grid/solver.
    This is a skeleton implementation for single-level mode only.
********************************************************************************/

class MSBGPressureAdapter
{
public:
    MSBGPressureAdapter();
    ~MSBGPressureAdapter();

    // Initialize the MSBG grid and solver from domain parameters
    bool initializeFromDomain(const PressureSolverParameters& params,
                             const MACVelocityField& velocityField,
                             const ParticleLevelSet& liquidPLS,
                             const MeshLevelSet& solidLevelSet);

    // Upload right-hand side from negative divergence vector
    bool uploadRhsFromNegativeDivergence(const Array3d<double>& rhsGrid);

    // Upload right-hand side from a flat vector
    bool uploadRhsFromVector(const std::vector<double>& rhsVector,
                            const std::vector<int>& pressureCellIndices);

    // Solve the pressure system using MSBG
    bool solvePressure(int maxIterations, double tolerance, 
                      int& outIterations, double& outError);

    // Download solution to pressure grid
    bool downloadSolutionToPressureGrid(Array3d<float>& pressureGrid);

    // Download solution to a flat vector
    bool downloadSolutionToVector(std::vector<double>& solutionVector);

    // Check if MSBG is available and initialized
    bool isInitialized() const;

    // Get solver status message
    std::string getStatusMessage() const;

private:
    bool _isInitialized;
    int _isize, _jsize, _ksize;
    double _cellwidth;
    std::string _statusMessage;

    // Placeholder for MSBG grid/solver state
    // In a full implementation, this would hold MSBG objects
    void* _msbgGrid;
    void* _msbgSolver;
};

#endif
