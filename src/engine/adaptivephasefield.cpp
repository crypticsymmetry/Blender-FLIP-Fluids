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

#include <cmath>
#include <algorithm>
#include "adaptivephasefield.h"

AdaptivePhaseField::AdaptivePhaseField() {}

AdaptivePhaseField::AdaptivePhaseField(int i, int j, int k, double dx) {
    initialize(i, j, k, dx);
}

void AdaptivePhaseField::initialize(int i, int j, int k, double dx) {
    _isize = i;
    _jsize = j;
    _ksize = k;
    _dx = dx;

    _phaseField.initialize(_isize, _jsize, _ksize, _blockWidth, _levels, 0.0f);
}

void AdaptivePhaseField::configureSparseGrid(int blockWidth, int levels) {
    _blockWidth = blockWidth;
    _levels = levels;

    _phaseField.initialize(_isize, _jsize, _ksize, _blockWidth, _levels, 0.0f);
}


void AdaptivePhaseField::setParameters(float farDistance,
                                       int smoothingIterations,
                                       float smoothingTimeStep,
                                       int smoothingBandLayers,
                                       float velocityRefinementScale,
                                       float velocityBandExpansionScale,
                                       float alphaPhi,
                                       float densityThreshold) {
    _farDistance = farDistance;
    _smoothingIterations = smoothingIterations;
    _smoothingTimeStep = smoothingTimeStep;
    _smoothingBandLayers = smoothingBandLayers;
    _velocityRefinementScale = velocityRefinementScale;
    _velocityBandExpansionScale = velocityBandExpansionScale;
    _alphaPhi = alphaPhi;
    _densityThreshold = densityThreshold;
}

void AdaptivePhaseField::rebuildFromParticles(std::vector<vmath::vec3> &particles,
                                              std::vector<vmath::vec3> *velocities,
                                              double particleRadius,
                                              double dt) {
    _phaseField.clear();

    float bandRadius = _farDistance * (float)_dx;
    float baseRadius = (float)particleRadius;
    _lastParticleRadius = baseRadius;

    for (size_t pidx = 0; pidx < particles.size(); pidx++) {
        vmath::vec3 p = particles[pidx];
        float speed = 0.0f;
        if (velocities != nullptr && pidx < velocities->size()) {
            speed = velocities->at(pidx).length();
        }

        float velocityScale = std::max(0.0f, std::min(2.0f, (float)(speed * dt / _dx) * _velocityBandExpansionScale));
        float particleBandRadius = bandRadius * (1.0f + velocityScale);
        float maxDistance = baseRadius + particleBandRadius;
        float maxDistanceSq = maxDistance * maxDistance;
        int radiusInCells = (int)std::ceil(maxDistance / (float)_dx);
        int levelsToWrite = _getHierarchyLevelsForParticle(speed, dt);

        int i0 = (int)std::floor((p.x / _dx)) - radiusInCells;
        int j0 = (int)std::floor((p.y / _dx)) - radiusInCells;
        int k0 = (int)std::floor((p.z / _dx)) - radiusInCells;

        int i1 = i0 + 2 * radiusInCells;
        int j1 = j0 + 2 * radiusInCells;
        int k1 = k0 + 2 * radiusInCells;

        for (int k = k0; k <= k1; k++) {
            if (k < 0 || k >= _ksize) {
                continue;
            }
            for (int j = j0; j <= j1; j++) {
                if (j < 0 || j >= _jsize) {
                    continue;
                }
                for (int i = i0; i <= i1; i++) {
                    if (i < 0 || i >= _isize) {
                        continue;
                    }

                    vmath::vec3 gc((float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f);
                    gc *= (float)_dx;

                    float distanceSq = vmath::dot(gc - p, gc - p);
                    if (distanceSq > maxDistanceSq) {
                        continue;
                    }

                    float weight = _kernelWeight(distanceSq, maxDistance);
                    if (weight > 0.0f) {
                        _phaseField.setHierarchyAdd(i, j, k, weight, levelsToWrite);
                    }
                }
            }
        }
    }
}

void AdaptivePhaseField::sampleIntoDenseGrid(Array3d<float> &densePhi, Array3d<float> *densePhase) {
    float bandRadius = _farDistance * (float)_dx;
    float particleRadius = _lastParticleRadius > 0.0f ? _lastParticleRadius : (float)_dx * 0.5f;

    Array3d<float> phaseField(densePhi.width, densePhi.height, densePhi.depth, 0.0f);
    for (int k = 0; k < densePhi.depth; k++) {
        for (int j = 0; j < densePhi.height; j++) {
            for (int i = 0; i < densePhi.width; i++) {
                float rawDensity = _phaseField.sampleFine(i, j, k);
                float phase = _compressPhaseField(rawDensity);
                phaseField.set(i, j, k, phase);
            }
        }
    }

    Array3d<bool> activeMask(densePhi.width, densePhi.height, densePhi.depth, false);
    for (int k = 0; k < densePhi.depth; k++) {
        for (int j = 0; j < densePhi.height; j++) {
            for (int i = 0; i < densePhi.width; i++) {
                float phase = phaseField(i, j, k);
                if (phase > 0.0f && phase < 1.0f) {
                    activeMask.set(i, j, k, true);
                }
            }
        }
    }

    _expandActiveMask(activeMask);
    _smoothPhaseField(phaseField, activeMask);

    if (densePhase != nullptr &&
            densePhase->width == densePhi.width &&
            densePhase->height == densePhi.height &&
            densePhase->depth == densePhi.depth) {
        for (int k = 0; k < densePhi.depth; k++) {
            for (int j = 0; j < densePhi.height; j++) {
                for (int i = 0; i < densePhi.width; i++) {
                    densePhase->set(i, j, k, phaseField(i, j, k));
                }
            }
        }
    }

    for (int k = 0; k < densePhi.depth; k++) {
        for (int j = 0; j < densePhi.height; j++) {
            for (int i = 0; i < densePhi.width; i++) {
                densePhi.set(i, j, k, _phaseFieldToSignedDistance(phaseField(i, j, k), particleRadius, bandRadius));
            }
        }
    }
}

int AdaptivePhaseField::getActiveSparseBlockCount() {
    return _phaseField.getBlockCount();
}

float AdaptivePhaseField::_kernelWeight(float distanceSq, float supportRadius) const {
    if (supportRadius <= 0.0f) {
        return 0.0f;
    }

    float invSupportRadiusSq = 1.0f / (supportRadius * supportRadius);
    float q = 1.0f - distanceSq * invSupportRadiusSq;
    if (q <= 0.0f) {
        return 0.0f;
    }

    return q * q * q;
}

float AdaptivePhaseField::_compressPhaseField(float rawDensity) const {
    float compressedDensity = rawDensity - _densityThreshold;
    if (compressedDensity <= 0.0f) {
        return 0.0f;
    }

    float denom = std::max(_alphaPhi, 1e-6f);
    float phase = std::sqrt(compressedDensity / denom);
    return std::max(0.0f, std::min(1.0f, phase));
}

float AdaptivePhaseField::_phaseFieldToSignedDistance(float phaseField,
                                                      float particleRadius,
                                                      float bandRadius) const {
    phaseField = std::max(0.0f, std::min(1.0f, phaseField));
    float distance = (1.0f - phaseField) * (2.0f * particleRadius + bandRadius) - particleRadius;
    float maxDistance = bandRadius;
    if (distance > maxDistance) {
        distance = maxDistance;
    }
    if (distance < -particleRadius) {
        distance = -particleRadius;
    }
    return distance;
}

void AdaptivePhaseField::_smoothPhaseField(Array3d<float> &phaseField, Array3d<bool> &activeMask) {
    Array3d<float> temp(phaseField.width, phaseField.height, phaseField.depth, 0.0f);

    for (int iter = 0; iter < _smoothingIterations; iter++) {
        for (int k = 0; k < phaseField.depth; k++) {
            for (int j = 0; j < phaseField.height; j++) {
                for (int i = 0; i < phaseField.width; i++) {
                    float center = phaseField(i, j, k);
                    if (!activeMask(i, j, k)) {
                        temp.set(i, j, k, center);
                        continue;
                    }

                    float sum = 0.0f;
                    int count = 0;

                    if (i > 0) { sum += phaseField(i - 1, j, k); count++; }
                    if (i + 1 < phaseField.width) { sum += phaseField(i + 1, j, k); count++; }
                    if (j > 0) { sum += phaseField(i, j - 1, k); count++; }
                    if (j + 1 < phaseField.height) { sum += phaseField(i, j + 1, k); count++; }
                    if (k > 0) { sum += phaseField(i, j, k - 1); count++; }
                    if (k + 1 < phaseField.depth) { sum += phaseField(i, j, k + 1); count++; }

                    float laplacian = count > 0 ? (sum / (float)count - center) : 0.0f;
                    float smoothed = center + _smoothingTimeStep * laplacian;
                    temp.set(i, j, k, std::max(0.0f, std::min(1.0f, smoothed)));
                }
            }
        }

        phaseField = temp;
    }
}

void AdaptivePhaseField::_expandActiveMask(Array3d<bool> &activeMask) {
    for (int layer = 0; layer < _smoothingBandLayers; layer++) {
        Array3d<bool> expanded = activeMask;
        for (int k = 0; k < activeMask.depth; k++) {
            for (int j = 0; j < activeMask.height; j++) {
                for (int i = 0; i < activeMask.width; i++) {
                    if (activeMask(i, j, k)) {
                        continue;
                    }

                    bool hasActiveNeighbor = false;
                    if (i > 0 && activeMask(i - 1, j, k)) { hasActiveNeighbor = true; }
                    if (i + 1 < activeMask.width && activeMask(i + 1, j, k)) { hasActiveNeighbor = true; }
                    if (j > 0 && activeMask(i, j - 1, k)) { hasActiveNeighbor = true; }
                    if (j + 1 < activeMask.height && activeMask(i, j + 1, k)) { hasActiveNeighbor = true; }
                    if (k > 0 && activeMask(i, j, k - 1)) { hasActiveNeighbor = true; }
                    if (k + 1 < activeMask.depth && activeMask(i, j, k + 1)) { hasActiveNeighbor = true; }

                    if (hasActiveNeighbor) {
                        expanded.set(i, j, k, true);
                    }
                }
            }
        }
        activeMask = expanded;
    }
}

int AdaptivePhaseField::_getHierarchyLevelsForParticle(float speed, double dt) const {
    if (_levels <= 1) {
        return 1;
    }

    float normalizedSpeed = (float)(speed * dt / _dx) * _velocityRefinementScale;
    normalizedSpeed = std::max(0.0f, std::min(1.0f, normalizedSpeed));

    int minLevels = 1;
    int maxLevels = _levels;
    int levels = maxLevels - (int)std::round(normalizedSpeed * (float)(maxLevels - minLevels));
    return std::max(minLevels, std::min(maxLevels, levels));
}
