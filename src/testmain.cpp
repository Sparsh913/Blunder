#include <iostream>
#include "mesh.h"

int main(void)
{
	PolygonalMesh mesh;
	PolygonalMesh::VertexHandle vtHd[3]=
	{
		mesh.AddVertex(YsVec3(1.0,2.0,3.0)),
		mesh.AddVertex(YsVec3(4.0,5.0,6.0)),
		mesh.AddVertex(YsVec3(-1.0,-3.0,-5.0)),
	};

	int id=0;
	for(auto vtHd=mesh.FirstVertex();
		mesh.NullVertex()!=vtHd;
		mesh.MoveToNext(vtHd))
	{
		std::cout << "Vtx:" << id << " " << mesh.GetVertexPosition(vtHd).Txt() << "\n";
		++id;
	}

	mesh.AddPolygon(3,vtHd);

	id=0;
	for(auto plHd=mesh.FirstPolygon();
		mesh.NullPolygon()!=plHd;
		mesh.MoveToNext(plHd))
	{
		std::cout << "Polygon:" << id << "\n";
		for(auto hd : mesh.GetPolygonVertex(plHd))
		{
			auto pos=mesh.GetVertexPosition(hd);
			std::cout << pos.Txt() << "\n";
		}
		++id;
	}

	return 0;
}
