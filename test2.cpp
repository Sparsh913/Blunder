#include <iostream>
#include "mesh.h"

int main(void)
{
    PolygonalMesh mesh;
    std::vector <PolygonalMesh::VertexHandle> vtHdArray;
    vtHdArray.push_back(mesh.AddVertex(YsVec3(-3,-3,-3)));
    vtHdArray.push_back(mesh.AddVertex(YsVec3( 3,-3,-3)));
    vtHdArray.push_back(mesh.AddVertex(YsVec3( 3,-3, 3)));
    vtHdArray.push_back(mesh.AddVertex(YsVec3(-3,-3, 3)));
    vtHdArray.push_back(mesh.AddVertex(YsVec3(-3, 3,-3)));
    vtHdArray.push_back(mesh.AddVertex(YsVec3( 3, 3,-3)));
    vtHdArray.push_back(mesh.AddVertex(YsVec3( 3, 3, 3)));
    vtHdArray.push_back(mesh.AddVertex(YsVec3(-3, 3, 3)));

    int plgVtx[6][4]=
    {
        {0,1,2,3},{7,6,5,4},{1,0,5,4},{2,1,5,6},{3,2,6,7},{0,3,7,4}
    };
    for(auto pv : plgVtx)
    {
        std::vector<PolygonalMesh::VertexHandle> plVtHd;
        plVtHd.push_back(vtHdArray[pv[0]]);
        plVtHd.push_back(vtHdArray[pv[1]]);
        plVtHd.push_back(vtHdArray[pv[2]]);
        plVtHd.push_back(vtHdArray[pv[3]]);
        mesh.AddPolygon(plVtHd);
    }


	for(size_t i=0; i<8; ++i)
	{
		auto found=mesh.FindPolygonsFromVertex(vtHdArray[i]);
		std::cout << "vertex " << i << ":";
		for(auto plHd : found)
		{
			std::cout << mesh.GetSearchKey(plHd) << " ";
		}
		std::cout << "\n";
	}

    return 0;
}
