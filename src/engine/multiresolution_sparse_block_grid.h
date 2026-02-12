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

#ifndef FLUIDENGINE_MULTIRESOLUTION_SPARSE_BLOCK_GRID_H
#define FLUIDENGINE_MULTIRESOLUTION_SPARSE_BLOCK_GRID_H

#include <unordered_map>
#include <vector>
#include "array3d.h"

class MultiresolutionSparseBlockGrid {
public:
    MultiresolutionSparseBlockGrid();
    MultiresolutionSparseBlockGrid(int i, int j, int k, int blockWidth, int levels, float fillValue);

    void initialize(int i, int j, int k, int blockWidth, int levels, float fillValue);
    void clear();
    void fill(float value);

    void setFine(int i, int j, int k, float value);
    void setHierarchyMin(int i, int j, int k, float value);
    void setHierarchyMin(int i, int j, int k, float value, int levelsToWrite);
    void setHierarchyAdd(int i, int j, int k, float value);
    void setHierarchyAdd(int i, int j, int k, float value, int levelsToWrite);
    float sampleFine(int i, int j, int k);

    int getBlockCount() const;
    int getBlockCount(int level) const;

private:
    struct BlockKey {
        int i = 0;
        int j = 0;
        int k = 0;

        bool operator==(const BlockKey &other) const {
            return i == other.i && j == other.j && k == other.k;
        }
    };

    struct BlockKeyHasher {
        std::size_t operator()(const BlockKey &key) const;
    };

    struct BlockData {
        Array3d<float> values;

        BlockData() {}
        BlockData(int width, float fillValue) : values(width, width, width, fillValue) {}
    };

    struct GridLevel {
        int scale = 1;
        int blockWidth = 8;
        float fillValue = 3.402823466e+38F;
        std::unordered_map<BlockKey, BlockData, BlockKeyHasher> blocks;
    };

    bool _isPointInRange(int i, int j, int k) const;
    BlockKey _getBlockKey(int i, int j, int k, int level) const;
    GridIndex _getLocalGridIndex(int i, int j, int k, int level) const;

    int _isize = 0;
    int _jsize = 0;
    int _ksize = 0;
    int _levels = 1;
    std::vector<GridLevel> _gridLevels;
};

#endif
