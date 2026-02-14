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
#include <thread>
#include "adaptivephasefield.h"
#include "threadutils.h"

namespace {

template <class Func>
void _parallelForKRange(int ksize, std::size_t workItems, Func &&fn) {
    if (ksize <= 0) {
        return;
    }

    int numCPU = ThreadUtils::getMaxThreadCount();
    int numthreads = std::max(1, std::min(numCPU, ksize));
    if (numthreads <= 1 || workItems < 32768) {
        fn(0, ksize);
        return;
    }

    std::vector<std::thread> threads(numthreads);
    std::vector<int> intervals = ThreadUtils::splitRangeIntoIntervals(0, ksize, numthreads);
    for (int tidx = 0; tidx < numthreads; tidx++) {
        threads[tidx] = std::thread([&, tidx]() {
            fn(intervals[tidx], intervals[tidx + 1]);
        });
    }
    for (int tidx = 0; tidx < numthreads; tidx++) {
        threads[tidx].join();
    }
}

}

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
                                              double dt,
                                              std::vector<int> *particleLevels) {
    _phaseField.clear();

    float baseRadius = (float)particleRadius;
    _lastParticleRadius = baseRadius;

    auto depositParticleRange = [&](int startidx, int endidx, MultiresolutionSparseBlockGrid *phaseFieldGrid) {
        for (int pidx = startidx; pidx < endidx; pidx++) {
            vmath::vec3 p = particles[pidx];
            float speed = 0.0f;
            if (velocities != nullptr && pidx < (int)velocities->size()) {
                speed = velocities->at(pidx).length();
            }

            int particleLevel = 0;
            if (particleLevels != nullptr && pidx < (int)particleLevels->size()) {
                particleLevel = std::max(0, std::min(8, particleLevels->at(pidx)));
            }
            float particleRadiusScale = std::pow(2.0f, (float)particleLevel);
            float particleBaseRadius = baseRadius * particleRadiusScale;
            float velocityScale = std::max(0.0f, std::min(1.0f, (float)(speed * dt / _dx) * _velocityBandExpansionScale));
            // Keep phase support local (about one particle radius) to avoid
            // long-range pre-contact coupling artifacts in dynamics.
            float supportRadius = particleBaseRadius * (1.0f + 0.35f * velocityScale);
            supportRadius = std::max(supportRadius, 0.5f * (float)_dx);
            float supportRadiusSq = supportRadius * supportRadius;
            int radiusInCells = (int)std::ceil(supportRadius / (float)_dx);
            int levelsToWrite = _getHierarchyLevelsForParticle(speed, dt);

            int ci = (int)std::floor(p.x / _dx);
            int cj = (int)std::floor(p.y / _dx);
            int ck = (int)std::floor(p.z / _dx);
            int i0 = std::max(0, ci - radiusInCells);
            int j0 = std::max(0, cj - radiusInCells);
            int k0 = std::max(0, ck - radiusInCells);
            int i1 = std::min(_isize - 1, ci + radiusInCells);
            int j1 = std::min(_jsize - 1, cj + radiusInCells);
            int k1 = std::min(_ksize - 1, ck + radiusInCells);

            for (int k = k0; k <= k1; k++) {
                float z = ((float)k + 0.5f) * (float)_dx;
                float dz = z - p.z;
                float dz2 = dz * dz;
                for (int j = j0; j <= j1; j++) {
                    float y = ((float)j + 0.5f) * (float)_dx;
                    float dy = y - p.y;
                    float dy2 = dy * dy;
                    for (int i = i0; i <= i1; i++) {
                        float x = ((float)i + 0.5f) * (float)_dx;
                        float dx = x - p.x;
                        float distanceSq = dx * dx + dy2 + dz2;
                        if (distanceSq > supportRadiusSq) {
                            continue;
                        }

                        float weight = _kernelWeight(distanceSq, supportRadius);
                        if (weight > 1e-8f) {
                            phaseFieldGrid->setHierarchyAdd(i, j, k, weight, levelsToWrite);
                        }
                    }
                }
            }
        }
    };

    int numCPU = ThreadUtils::getMaxThreadCount();
    int numthreads = std::max(1, std::min(numCPU, (int)particles.size()));
    if (numthreads <= 1 || particles.size() < 4000) {
        depositParticleRange(0, (int)particles.size(), &_phaseField);
        return;
    }

    std::vector<MultiresolutionSparseBlockGrid> threadPhaseFields(numthreads);
    for (int tidx = 0; tidx < numthreads; tidx++) {
        threadPhaseFields[tidx].initialize(_isize, _jsize, _ksize, _blockWidth, _levels, 0.0f);
    }

    std::vector<std::thread> threads(numthreads);
    std::vector<int> intervals = ThreadUtils::splitRangeIntoIntervals(0, (int)particles.size(), numthreads);
    for (int tidx = 0; tidx < numthreads; tidx++) {
        threads[tidx] = std::thread([&, tidx]() {
            depositParticleRange(intervals[tidx], intervals[tidx + 1], &threadPhaseFields[tidx]);
        });
    }

    for (int tidx = 0; tidx < numthreads; tidx++) {
        threads[tidx].join();
    }

    for (int tidx = 0; tidx < numthreads; tidx++) {
        _phaseField.addFrom(threadPhaseFields[tidx]);
    }
}

void AdaptivePhaseField::sampleIntoDenseGrid(Array3d<float> &densePhi, Array3d<float> *densePhase) {
    float bandRadius = _farDistance * (float)_dx;
    float particleRadius = _lastParticleRadius > 0.0f ? _lastParticleRadius : (float)_dx * 0.5f;
    std::size_t workItems = (std::size_t)densePhi.width * (std::size_t)densePhi.height * (std::size_t)densePhi.depth;

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
    _parallelForKRange(densePhi.depth, workItems, [&](int kstart, int kend) {
        for (int k = kstart; k < kend; k++) {
            for (int j = 0; j < densePhi.height; j++) {
                for (int i = 0; i < densePhi.width; i++) {
                    float phase = phaseField(i, j, k);
                    if (phase > 0.0f && phase < 1.0f) {
                        activeMask.set(i, j, k, true);
                    }
                }
            }
        }
    });

    _expandActiveMask(activeMask);
    _smoothPhaseField(phaseField, activeMask);

    if (densePhase != nullptr &&
            densePhase->width == densePhi.width &&
            densePhase->height == densePhi.height &&
            densePhase->depth == densePhi.depth) {
        _parallelForKRange(densePhi.depth, workItems, [&](int kstart, int kend) {
            for (int k = kstart; k < kend; k++) {
                for (int j = 0; j < densePhi.height; j++) {
                    for (int i = 0; i < densePhi.width; i++) {
                        densePhase->set(i, j, k, phaseField(i, j, k));
                    }
                }
            }
        });
    }

    _parallelForKRange(densePhi.depth, workItems, [&](int kstart, int kend) {
        for (int k = kstart; k < kend; k++) {
            for (int j = 0; j < densePhi.height; j++) {
                for (int i = 0; i < densePhi.width; i++) {
                    densePhi.set(i, j, k, _phaseFieldToSignedDistance(phaseField(i, j, k), particleRadius, bandRadius));
                }
            }
        }
    });
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
    if (_smoothingIterations <= 0) {
        return;
    }

    Array3d<float> temp(phaseField.width, phaseField.height, phaseField.depth, 0.0f);
    std::size_t workItems = (std::size_t)phaseField.width * (std::size_t)phaseField.height * (std::size_t)phaseField.depth;
    Array3d<float> *source = &phaseField;
    Array3d<float> *destination = &temp;

    for (int iter = 0; iter < _smoothingIterations; iter++) {
        _parallelForKRange(phaseField.depth, workItems, [&](int kstart, int kend) {
            for (int k = kstart; k < kend; k++) {
                for (int j = 0; j < phaseField.height; j++) {
                    for (int i = 0; i < phaseField.width; i++) {
                        float center = source->get(i, j, k);
                        if (!activeMask(i, j, k)) {
                            destination->set(i, j, k, center);
                            continue;
                        }

                        float sum = 0.0f;
                        int count = 0;

                        if (i > 0) { sum += source->get(i - 1, j, k); count++; }
                        if (i + 1 < phaseField.width) { sum += source->get(i + 1, j, k); count++; }
                        if (j > 0) { sum += source->get(i, j - 1, k); count++; }
                        if (j + 1 < phaseField.height) { sum += source->get(i, j + 1, k); count++; }
                        if (k > 0) { sum += source->get(i, j, k - 1); count++; }
                        if (k + 1 < phaseField.depth) { sum += source->get(i, j, k + 1); count++; }

                        float laplacian = count > 0 ? (sum / (float)count - center) : 0.0f;
                        float smoothed = center + _smoothingTimeStep * laplacian;
                        destination->set(i, j, k, std::max(0.0f, std::min(1.0f, smoothed)));
                    }
                }
            }
        });
        std::swap(source, destination);
    }

    if (source != &phaseField) {
        phaseField = *source;
    }
}

void AdaptivePhaseField::_expandActiveMask(Array3d<bool> &activeMask) {
    if (_smoothingBandLayers <= 0) {
        return;
    }

    Array3d<bool> tempMask(activeMask.width, activeMask.height, activeMask.depth, false);
    Array3d<bool> *source = &activeMask;
    Array3d<bool> *destination = &tempMask;
    std::size_t workItems = (std::size_t)activeMask.width * (std::size_t)activeMask.height * (std::size_t)activeMask.depth;

    for (int layer = 0; layer < _smoothingBandLayers; layer++) {
        _parallelForKRange(activeMask.depth, workItems, [&](int kstart, int kend) {
            for (int k = kstart; k < kend; k++) {
                for (int j = 0; j < activeMask.height; j++) {
                    for (int i = 0; i < activeMask.width; i++) {
                        if (source->get(i, j, k)) {
                            destination->set(i, j, k, true);
                            continue;
                        }

                        bool hasActiveNeighbor = false;
                        if (i > 0 && source->get(i - 1, j, k)) { hasActiveNeighbor = true; }
                        if (i + 1 < activeMask.width && source->get(i + 1, j, k)) { hasActiveNeighbor = true; }
                        if (j > 0 && source->get(i, j - 1, k)) { hasActiveNeighbor = true; }
                        if (j + 1 < activeMask.height && source->get(i, j + 1, k)) { hasActiveNeighbor = true; }
                        if (k > 0 && source->get(i, j, k - 1)) { hasActiveNeighbor = true; }
                        if (k + 1 < activeMask.depth && source->get(i, j, k + 1)) { hasActiveNeighbor = true; }

                        destination->set(i, j, k, hasActiveNeighbor);
                    }
                }
            }
        });
        std::swap(source, destination);
    }

    if (source != &activeMask) {
        activeMask = *source;
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
