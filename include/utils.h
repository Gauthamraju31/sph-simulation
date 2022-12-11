#pragma once
#include <vector>
#include <cmath>
#include <unordered_map>
#include <glm/glm.hpp>
#include "constants.h"

// Hashmap the world particle spatial information & use it for collision detection
template <typename T>
class SpatialIndex
{
public:
    typedef std::vector<T *> NeighbourList;

    SpatialIndex(
        const unsigned int numBuckets, // number of hash buckets
        const float cellSize,          // grid cell size
        const bool twoDeeNeighbourhood  // true == 3x3 neighbourhood, false == 3x3x3
        )
        : mHashMap(numBuckets), mInvCellSize(1.0f / cellSize)
    {
        // initialize neighbour offsets
        for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j++)
                if (twoDeeNeighbourhood)
                    mOffsets.push_back(glm::ivec3(i, j, 0));
                else
                    for (int k = -1; k <= 1; k++)
                        mOffsets.push_back(glm::ivec3(i, j, k));
    }

    void Insert(const glm::vec3 &pos, T *thing)
    {
        mHashMap[Discretize(pos, mInvCellSize)].push_back(thing);
    }

    void Neighbours(const glm::vec3 &pos, NeighbourList &ret) const
    {
        const glm::ivec3 ipos = Discretize(pos, mInvCellSize);
        for (const auto &offset : mOffsets)
        {
            typename HashMap::const_iterator it = mHashMap.find(offset + ipos);
            if (it != mHashMap.end())
            {
                ret.insert(ret.end(), it->second.begin(), it->second.end());
            }
        }
    }

    void Clear()
    {
        mHashMap.clear();
    }

private:
    // "Optimized Spatial Hashing for Collision Detection of Deformable Objects"
    // Teschner, Heidelberger, et al.
    // returns a hash between 0 and 2^32-1
    struct TeschnerHash : std::unary_function<glm::ivec3, std::size_t>
    {
        std::size_t operator()(glm::ivec3 const &pos) const
        {
            const unsigned int p1 = 73856093;
            const unsigned int p2 = 19349663;
            const unsigned int p3 = 83492791;
            return size_t((pos.x * p1) ^ (pos.y * p2) ^ (pos.z * p3));
        };
    };

    // returns the indexes of the cell pos is in, assuming a cellSize grid
    // invCellSize is the inverse of the desired cell size
    static inline glm::ivec3 Discretize(const glm::vec3 &pos, const float invCellSize)
    {
        return glm::ivec3(glm::floor(pos * invCellSize));
    }

    typedef std::unordered_map<glm::ivec3, NeighbourList, TeschnerHash> HashMap;
    HashMap mHashMap;

    std::vector<glm::ivec3> mOffsets;

    const float mInvCellSize;
};
