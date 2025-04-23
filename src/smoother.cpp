#include "smoother.h"
#include <algorithm>
#include <cmath>

Smoother::Smoother(PolygonalMesh* meshPtr)
{
    mesh = meshPtr;
    ComputeCotangentWeights();
}

void Smoother::ComputeCotangentWeights()
{
    cotangentWeight.clear();

    for (auto plHd = mesh->FirstPolygon(); mesh->NullPolygon() != plHd; mesh->MoveToNext(plHd))
    {
        auto vtHd = mesh->GetPolygonVertex(plHd);
        if (vtHd.size() != 3) continue;

        YsVec3 pos[3] = {
            mesh->GetVertexPosition(vtHd[0]),
            mesh->GetVertexPosition(vtHd[1]),
            mesh->GetVertexPosition(vtHd[2])
        };

        for (int i = 0; i < 3; ++i)
        {
            int i0 = i;
            int i1 = (i + 1) % 3;
            int i2 = (i + 2) % 3;

            YsVec3 v0 = pos[i1] - pos[i0];
            YsVec3 v1 = pos[i2] - pos[i0];

            double dot = v0 * v1;
            double len0 = v0.GetLength();
            double len1 = v1.GetLength();
            double cosTheta = dot / (len0 * len1 + 1e-10);
            cosTheta = std::min(1.0, std::max(-1.0, cosTheta));
            double angle = acos(cosTheta);
            double cotangent = 1.0 / tan(angle);

            HasEdge::Edge e;
            e.edVtKey[0] = mesh->GetSearchKey(vtHd[i1]);
            e.edVtKey[1] = mesh->GetSearchKey(vtHd[i2]);

            cotangentWeight[e] += cotangent * 0.5;
        }
    }
}

YsVec3 Smoother::ComputeWeightedGradient(PolygonalMesh::VertexHandle vtHd) const
{
    YsVec3 grad = YsVec3::Origin();
    auto x_i = mesh->GetVertexPosition(vtHd);
    auto neighbors = mesh->GetConnectedVertex(vtHd);
    auto iKey = mesh->GetSearchKey(vtHd);

    double totalWeight = 0.0;

    for (auto nbr : neighbors)
    {
        auto jKey = mesh->GetSearchKey(nbr);
        HasEdge::Edge e;
        e.edVtKey[0] = iKey;
        e.edVtKey[1] = jKey;

        auto found = cotangentWeight.find(e);
        if (found != cotangentWeight.end())
        {
            double w = found->second;
            YsVec3 diff = x_i - mesh->GetVertexPosition(nbr);
            grad += w * diff;
            totalWeight += w;
        }
    }

    if (totalWeight > 0.0)
    {
        grad /= totalWeight;
    }

    return grad;
}

void Smoother::OptimizeGlobal(int iterations, double learningRate)
{
    for (int iter = 0; iter < iterations; ++iter)
    {
        std::unordered_map<unsigned int, YsVec3> gradMap;

        for (auto vtHd = mesh->FirstVertex(); mesh->NullVertex() != vtHd; mesh->MoveToNext(vtHd))
        {
            gradMap[mesh->GetSearchKey(vtHd)] = ComputeWeightedGradient(vtHd);
        }

        for (auto vtHd = mesh->FirstVertex(); mesh->NullVertex() != vtHd; mesh->MoveToNext(vtHd))
        {
            auto key = mesh->GetSearchKey(vtHd);
            auto pos = mesh->GetVertexPosition(vtHd);
            pos -= learningRate * gradMap[key];
            mesh->SetVertexPosition(vtHd, pos);
        }
    }
}

void Smoother::OptimizeWithinRadius(const YsVec3& center, double radius, int iterations, double learningRate)
{
    const double radius2 = radius * radius;
    for (int iter = 0; iter < iterations; ++iter)
    {
        std::unordered_map<unsigned int, YsVec3> gradMap;

        // Compute gradients for vertices within the sphere
        for (auto vtHd = mesh->FirstVertex(); mesh->NullVertex() != vtHd; mesh->MoveToNext(vtHd))
        {
            auto pos = mesh->GetVertexPosition(vtHd);
            if ((pos - center).GetSquareLength() <= radius2)
            {
                gradMap[mesh->GetSearchKey(vtHd)] = ComputeWeightedGradient(vtHd);
            }
        }

        // Update these vertices
        for (auto vtHd = mesh->FirstVertex(); mesh->NullVertex() != vtHd; mesh->MoveToNext(vtHd))
        {
            auto pos = mesh->GetVertexPosition(vtHd);
            if ((pos - center).GetSquareLength() <= radius2)
            {
                auto key = mesh->GetSearchKey(vtHd);
                pos -= learningRate * gradMap[key];
                mesh->SetVertexPosition(vtHd, pos);
            }
        }
    }
}

double Smoother::ComputeLoss() const
{
    double total = 0.0;

    for (auto vtHd = mesh->FirstVertex(); mesh->NullVertex() != vtHd; mesh->MoveToNext(vtHd))
    {
        auto grad = ComputeWeightedGradient(vtHd);
        total += grad.GetSquareLength();
    }

    return total;
}
