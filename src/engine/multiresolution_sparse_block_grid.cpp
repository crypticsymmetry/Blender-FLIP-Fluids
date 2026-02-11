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

#include <algorithm>
#include "multiresolution_sparse_block_grid.h"

MultiresolutionSparseBlockGrid::MultiresolutionSparseBlockGrid() {}

MultiresolutionSparseBlockGrid::MultiresolutionSparseBlockGrid(int i, int j, int k,
                                                               int blockWidth,
                                                               int levels,
                                                               float fillValue) {
    initialize(i, j, k, blockWidth, levels, fillValue);
}

void MultiresolutionSparseBlockGrid::initialize(int i, int j, int k,
                                                int blockWidth,
                                                int levels,
                                                float fillValue) {
    _isize = i;
    _jsize = j;
    _ksize = k;
    _levels = levels;
    _gridLevels.clear();
    _gridLevels.reserve(_levels);

    for (int level = 0; level < _levels; level++) {
        GridLevel l;
        l.scale = 1 << level;
        l.blockWidth = blockWidth;
        l.fillValue = fillValue;
        _gridLevels.push_back(l);
    }
}

void MultiresolutionSparseBlockGrid::clear() {
    for (size_t level = 0; level < _gridLevels.size(); level++) {
        _gridLevels[level].blocks.clear();
    }
}

void MultiresolutionSparseBlockGrid::fill(float value) {
    for (size_t level = 0; level < _gridLevels.size(); level++) {
        _gridLevels[level].fillValue = value;
        for (auto &blockItem : _gridLevels[level].blocks) {
            blockItem.second.values.fill(value);
        }
    }
}

void MultiresolutionSparseBlockGrid::setFine(int i, int j, int k, float value) {
    if (_gridLevels.empty() || !_isPointInRange(i, j, k)) {
        return;
    }

    GridLevel &fineLevel = _gridLevels[0];
    BlockKey key = _getBlockKey(i, j, k, 0);

    auto blockIt = fineLevel.blocks.find(key);
    if (blockIt == fineLevel.blocks.end()) {
        blockIt = fineLevel.blocks.emplace(key,
                                           BlockData(fineLevel.blockWidth, fineLevel.fillValue)).first;
    }

    GridIndex localIndex = _getLocalGridIndex(i, j, k, 0);
    blockIt->second.values.set(localIndex, value);
}

void MultiresolutionSparseBlockGrid::setHierarchyMin(int i, int j, int k, float value) {
    setHierarchyMin(i, j, k, value, (int)_gridLevels.size());
}


void MultiresolutionSparseBlockGrid::setHierarchyMin(int i, int j, int k, float value, int levelsToWrite) {
    if (_gridLevels.empty() || !_isPointInRange(i, j, k)) {
        return;
    }

    int maxLevel = std::max(1, std::min(levelsToWrite, (int)_gridLevels.size()));
    for (int level = 0; level < maxLevel; level++) {
        GridLevel &gridLevel = _gridLevels[level];
        BlockKey key = _getBlockKey(i, j, k, level);

        auto blockIt = gridLevel.blocks.find(key);
        if (blockIt == gridLevel.blocks.end()) {
            blockIt = gridLevel.blocks.emplace(key,
                                               BlockData(gridLevel.blockWidth, gridLevel.fillValue)).first;
        }

        GridIndex localIndex = _getLocalGridIndex(i, j, k, level);
        float current = blockIt->second.values(localIndex);
        if (value < current) {
            blockIt->second.values.set(localIndex, value);
        }
    }
}

float MultiresolutionSparseBlockGrid::sampleFine(int i, int j, int k) {
    if (_gridLevels.empty() || !_isPointInRange(i, j, k)) {
        return 0.0f;
    }

    for (size_t level = 0; level < _gridLevels.size(); level++) {
        GridLevel &gridLevel = _gridLevels[level];
        BlockKey key = _getBlockKey(i, j, k, (int)level);
        auto blockIt = gridLevel.blocks.find(key);
        if (blockIt == gridLevel.blocks.end()) {
            continue;
        }

        GridIndex localIndex = _getLocalGridIndex(i, j, k, (int)level);
        return blockIt->second.values(localIndex);
    }

    return _gridLevels.back().fillValue;
}

int MultiresolutionSparseBlockGrid::getBlockCount() const {
    if (_gridLevels.empty()) {
        return 0;
    }
    return (int)_gridLevels[0].blocks.size();
}

int MultiresolutionSparseBlockGrid::getBlockCount(int level) const {
    if (level < 0 || level >= (int)_gridLevels.size()) {
        return 0;
    }
    return (int)_gridLevels[level].blocks.size();
}

bool MultiresolutionSparseBlockGrid::_isPointInRange(int i, int j, int k) const {
    return i >= 0 && i < _isize && j >= 0 && j < _jsize && k >= 0 && k < _ksize;
}

MultiresolutionSparseBlockGrid::BlockKey
MultiresolutionSparseBlockGrid::_getBlockKey(int i, int j, int k, int level) const {
    const GridLevel &gridLevel = _gridLevels[level];
    int scale = gridLevel.scale;
    int blockWidth = gridLevel.blockWidth;
    int scaledI = i / scale;
    int scaledJ = j / scale;
    int scaledK = k / scale;

    return {
        scaledI / blockWidth,
        scaledJ / blockWidth,
        scaledK / blockWidth
    };
}

GridIndex MultiresolutionSparseBlockGrid::_getLocalGridIndex(int i, int j, int k, int level) const {
    const GridLevel &gridLevel = _gridLevels[level];
    int scale = gridLevel.scale;
    int blockWidth = gridLevel.blockWidth;
    int scaledI = i / scale;
    int scaledJ = j / scale;
    int scaledK = k / scale;

    return GridIndex(scaledI % blockWidth,
                     scaledJ % blockWidth,
                     scaledK % blockWidth);
}

std::size_t MultiresolutionSparseBlockGrid::BlockKeyHasher::operator()(const BlockKey &key) const {
    std::size_t h1 = (std::size_t)key.i * 73856093;
    std::size_t h2 = (std::size_t)key.j * 19349663;
    std::size_t h3 = (std::size_t)key.k * 83492791;
    return h1 ^ h2 ^ h3;
}
