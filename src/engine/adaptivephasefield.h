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

#ifndef FLUIDENGINE_ADAPTIVEPHASEFIELD_H
#define FLUIDENGINE_ADAPTIVEPHASEFIELD_H

#include <vector>
#include "array3d.h"
#include "vmath.h"
#include "multiresolution_sparse_block_grid.h"

class AdaptivePhaseField {
public:
    AdaptivePhaseField();
    AdaptivePhaseField(int i, int j, int k, double dx);

    void initialize(int i, int j, int k, double dx);
    void configureSparseGrid(int blockWidth, int levels);
    void setParameters(float farDistance,
                       int smoothingIterations,
                       float smoothingTimeStep,
                       int smoothingBandLayers,
                       float velocityRefinementScale,
                       float velocityBandExpansionScale,
                       float alphaPhi,
                       float densityThreshold);
    void rebuildFromParticles(std::vector<vmath::vec3> &particles,
                              std::vector<vmath::vec3> *velocities,
                              double particleRadius,
                              double dt);
    void sampleIntoDenseGrid(Array3d<float> &densePhi, Array3d<float> *densePhase = nullptr);

    int getActiveSparseBlockCount();

private:
    float _kernelWeight(float distanceSq, float supportRadius) const;
    float _compressPhaseField(float rawDensity) const;
    float _phaseFieldToSignedDistance(float phaseField, float particleRadius, float bandRadius) const;
    void _smoothPhaseField(Array3d<float> &phaseField, Array3d<bool> &activeMask);
    void _expandActiveMask(Array3d<bool> &activeMask);
    int _getHierarchyLevelsForParticle(float speed, double dt) const;

    int _isize = 0;
    int _jsize = 0;
    int _ksize = 0;
    double _dx = 0.0;

    int _blockWidth = 8;
    int _levels = 3;
    float _farDistance = 3.0f;
    int _smoothingIterations = 5;
    float _smoothingTimeStep = 0.05f;
    int _smoothingBandLayers = 2;
    float _velocityRefinementScale = 4.0f;
    float _velocityBandExpansionScale = 1.5f;
    float _alphaPhi = 1.0f;
    float _densityThreshold = 0.0f;

    MultiresolutionSparseBlockGrid _phaseField;
    float _lastParticleRadius = 0.0f;
};

#endif
