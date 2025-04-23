#include <fstream>
#include <iostream>
#include "mesh.h"
#include "lattice.h"

//const char fileName[]
bool PolygonalMesh::LoadBinStl(const char fileName[])
{
	YsColor ysColor;
	ysColor.SetDoubleRGBA(0.8f, 0.8f, 0.8f, 1.0f);

	// With C++ standard library
	std::ifstream ifp(fileName,std::ios::binary);
	if(ifp.is_open())
	{
		CleanUp();

		unsigned char buf[80];
		ifp.read((char *)buf,80);

		int nTri;
		ifp.read((char *)&nTri,4);
		std::cout << nTri << " triangles.\n";

		for(int i=0; i<nTri; i++)
		{
			ifp.read((char *)buf,50);

			const float *t=(const float *)buf;
			VertexHandle vtHd[3];
			for(int j=0; j<3; ++j)
			{
				vtHd[j]=AddVertex(YsVec3(t[3+j*3],t[4+j*3],t[5+j*3]));
			}
			auto plHd=AddPolygon(3,vtHd);
			SetNormal(plHd,YsVec3(t[0],t[1],t[2]));
			SetColor(plHd,ysColor);
		}

		return true;
	}
	return false;
}

void PolygonalMesh::GetBoundingBox(YsVec3 &mn,YsVec3 &mx) const
{
	bool first=true;
	mn=YsVec3::Origin();
	mx=YsVec3::Origin();
	for(auto vtHd=FirstVertex(); NullVertex()!=vtHd; MoveToNext(vtHd))
	{
		auto pos=GetVertexPosition(vtHd);
		if(true==first)
		{
			mn=pos;
			mx=pos;
			first=false;
		}
		else
		{
			auto x=pos.x();
			auto y=pos.y();
			auto z=pos.z();

			mn.SetX(std::min<double>(mn.x(),x));
			mn.SetY(std::min<double>(mn.y(),y));
			mn.SetZ(std::min<double>(mn.z(),z));

			mx.SetX(std::max<double>(mx.x(),x));
			mx.SetY(std::max<double>(mx.y(),y));
			mx.SetZ(std::max<double>(mx.z(),z));
		}
	}
}
void PolygonalMesh::GetBoundingBox(YsVec3 bbx[2]) const
{
	GetBoundingBox(bbx[0],bbx[1]);
}

PolygonalMesh::OpenGLArrays PolygonalMesh::MakeVertexArrays(void) const
{
	OpenGLArrays arrays;
	for(auto plHd=FirstPolygon(); NullPolygon()!=plHd; MoveToNext(plHd))
	{
		auto plVtHd=GetPolygonVertex(plHd);
		for(size_t i=1; i+1<plVtHd.size(); ++i)
		{
			VertexHandle trVtHd[3]=
			{
				plVtHd[0],plVtHd[i],plVtHd[i+1]
			};
			YsVec3 trVtPos[3]=
			{
				GetVertexPosition(trVtHd[0]),
				GetVertexPosition(trVtHd[1]),
				GetVertexPosition(trVtHd[2]),
			};

			auto nom=GetNormal(plHd);
			auto col=GetColor(plHd);

			for(int j=0; j<3; ++j)
			{
				arrays.vtx.push_back(trVtPos[j].x());
				arrays.vtx.push_back(trVtPos[j].y());
				arrays.vtx.push_back(trVtPos[j].z());

				arrays.nom.push_back(nom.x());
				arrays.nom.push_back(nom.y());
				arrays.nom.push_back(nom.z());

				arrays.col.push_back(col.Rf());
				arrays.col.push_back(col.Gf());
				arrays.col.push_back(col.Bf());
				arrays.col.push_back(col.Af());

				arrays.edgeVtx.push_back(trVtPos[j].x());
				arrays.edgeVtx.push_back(trVtPos[j].y());
				arrays.edgeVtx.push_back(trVtPos[j].z());

				arrays.edgeVtx.push_back(trVtPos[(j + 1) % 3].x());
				arrays.edgeVtx.push_back(trVtPos[(j + 1) % 3].y());
				arrays.edgeVtx.push_back(trVtPos[(j + 1) % 3].z());

				arrays.edgeCol.push_back(0.0f);
				arrays.edgeCol.push_back(0.0f);
				arrays.edgeCol.push_back(0.0f);
				arrays.edgeCol.push_back(1.0f);

				arrays.edgeCol.push_back(0.0f);
				arrays.edgeCol.push_back(0.0f);
				arrays.edgeCol.push_back(0.0f);
				arrays.edgeCol.push_back(1.0f);
			}
		}
	}
	return arrays;
}

void PolygonalMesh::StitchVertexN2(void)
{
	for(auto plHd=FirstPolygon();
	    NullPolygon()!=plHd;
	    MoveToNext(plHd))
	{
		auto plVtHd=GetPolygonVertex(plHd);
		bool changed=false;
		for(auto &vtHd : plVtHd)
		{
			auto vtPos=GetVertexPosition(vtHd);
			for(auto vtHd2=FirstVertex();
			    NullVertex()!=vtHd2;
			    MoveToNext(vtHd2))
			{
				auto vtPos2=GetVertexPosition(vtHd2);
				if(vtPos==vtPos2 &&
				   GetSearchKey(vtHd2)<GetSearchKey(vtHd))
				{
					vtHd=vtHd2;
					changed=true;
				}
			}
		}
		if(true==changed)
		{
			SetPolygonVertex(plHd,plVtHd);
		}
	}
}

void PolygonalMesh::Stitch(void)
{
	if(GetNumVertices()==0)
	{
		return;
	}

	YsVec3 minmax[2];
	GetBoundingBox(minmax);

	auto diag=minmax[1]-minmax[0];

	double L=diag.GetLength();
	minmax[0]-=YsVec3(L/100.0,L/100.0,L/100.0);
	minmax[1]+=YsVec3(L/100.0,L/100.0,L/100.0);

	double V=diag.x()*diag.y()*diag.z();
	double v=V/double(GetNumVertices());
	double l=pow(v,1.0/3.0);

	int nx=1+int(diag.x()/l);
	int ny=1+int(diag.y()/l);
	int nz=1+int(diag.z()/l);

	Lattice3d <std::vector <VertexHandle> > ltc;
	ltc.Create(nx,ny,nz,minmax[0],minmax[1]);

	for(auto vtHd=FirstVertex();
	    vtHd!=NullVertex();
	    MoveToNext(vtHd))
	{
		auto pos=GetVertexPosition(vtHd);
		auto idx=ltc.GetBlockIndex(pos);
		if(true==ltc.IsInRange(idx))
		{
			ltc[idx].push_back(vtHd);
		}
		else
		{
			std::cout << "Lattice error!\n";
		}
	}

	for(auto plHd=FirstPolygon();
	    NullPolygon()!=plHd;
	    MoveToNext(plHd))
	{
		auto plVtHd=GetPolygonVertex(plHd);
		bool changed=false;
		for(auto &vtHd : plVtHd)
		{
			auto pos=GetVertexPosition(vtHd);
			auto idx=ltc.GetBlockIndex(pos);
			for(int i=0; i<27; ++i)
			{
				int dz=i/9;
				int dy=(i%9)/3;
				int dx=i%3;
				--dz;
				--dy;
				--dx;
				auto nei=idx;
				nei.AddX(dx);
				nei.AddY(dy);
				nei.AddZ(dz);
				if(true==ltc.IsInRange(nei))
				{
					for(auto neiVtHd : ltc[nei])
					{
						if(GetVertexPosition(neiVtHd)==pos &&
						   GetSearchKey(neiVtHd)<GetSearchKey(vtHd))
						{
							vtHd=neiVtHd;
							changed=true;
						}
					}
				}
			}
		}
		if(true==changed)
		{
			SetPolygonVertex(plHd,plVtHd);
		}
	}
}
