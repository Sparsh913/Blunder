#ifndef SMOOTHER_IS_INCLUDED
#define SMOOTHER_IS_INCLUDED

#include "mesh.h"
#include <unordered_map>


class Smoother
{
public:
    Smoother(PolygonalMesh* meshPtr);

    void OptimizeGlobal(int iterations, double learningRate);
    void OptimizeWithinRadius(const YsVec3& center, double radius, int iterations, double learningRate);
    double ComputeLoss() const;

private:
    PolygonalMesh* mesh;

    std::unordered_map<HasEdge::Edge, double> cotangentWeight;

    void ComputeCotangentWeights();
    YsVec3 ComputeWeightedGradient(PolygonalMesh::VertexHandle vtHd) const;
};

#endif