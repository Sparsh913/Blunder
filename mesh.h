#ifndef POLYGONALMESH_IS_INCLUDED
#define POLYGONALMESH_IS_INCLUDED

#include <list>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <ysclass.h>
#include <string>
#include <fstream>
#include <cstring>


class HasEdge
{
public:
	class Edge
	{
	public:
		unsigned int edVtKey[2];
		bool operator==(Edge e) const
		{
			return (edVtKey[0]==e.edVtKey[0] && edVtKey[1]==e.edVtKey[1]) ||
			       (edVtKey[0]==e.edVtKey[1] && edVtKey[1]==e.edVtKey[0]);
		}
		bool operator!=(Edge e) const
		{
			return (edVtKey[0]!=e.edVtKey[0] || edVtKey[1]!=e.edVtKey[1]) &&
			       (edVtKey[0]!=e.edVtKey[1] || edVtKey[1]!=e.edVtKey[0]);
		}
	};
};

template <>
struct std::hash <HasEdge::Edge>
{
	size_t operator()(const HasEdge::Edge &edge) const
	{
		auto H=std::max(edge.edVtKey[0],edge.edVtKey[1]);
		auto L=std::min(edge.edVtKey[0],edge.edVtKey[1]);
		return H*11+L*7;
	}
};

class PolygonalMesh : public HasEdge
{
private:
	unsigned int searchKeySeed=1;
	class HasSearchKey
	{
	public:
		unsigned int searchKey;
	};

protected:
    class Vertex : public HasSearchKey
    {
    public:
        YsVec3 pos;
    };
private:
    mutable std::list <Vertex> vtxList;
public:
    class VertexHandle
    {
    friend class PolygonalMesh;
    private:
        std::list <Vertex>::iterator vtxPtr;
    public:
        inline bool operator==(const VertexHandle &vtHd) const
        {
			return vtxPtr==vtHd.vtxPtr;
		}
        inline bool operator!=(const VertexHandle &vtHd) const
        {
			return vtxPtr!=vtHd.vtxPtr;
		}
    };
    VertexHandle AddVertex(const YsVec3 &pos)
    {
		Vertex newVtx;
		newVtx.searchKey=searchKeySeed++;
		newVtx.pos=pos;
		vtxList.push_back(newVtx);

		auto last=vtxList.end();
		--last;

		VertexHandle vtHd;
		vtHd.vtxPtr=last;

		return vtHd;
	}
    inline VertexHandle NullVertex(void) const
    {
		VertexHandle vtHd;
		vtHd.vtxPtr=vtxList.end();
		return vtHd;
	}
    YsVec3 GetVertexPosition(VertexHandle vtHd) const
    {
		return vtHd.vtxPtr->pos;
	}
	void SetVertexPosition(VertexHandle vtHd,const YsVec3 &pos)
	{
		vtHd.vtxPtr->pos=pos;
	}
	unsigned int GetSearchKey(VertexHandle vtHd) const
	{
		return vtHd.vtxPtr->searchKey;
	}
	size_t GetNumVertices(void) const
	{
		return vtxList.size();
	}
	VertexHandle FirstVertex(void) const
	{
		VertexHandle vtHd;
		vtHd.vtxPtr=vtxList.begin();
		return vtHd;
	}
	bool MoveToNext(VertexHandle &vtHd) const
	{
		if(vtHd.vtxPtr!=vtxList.end())
		{
			++vtHd.vtxPtr;
		}
		else
		{
			vtHd.vtxPtr=vtxList.begin();
		}
		return vtHd.vtxPtr!=vtxList.end();
	}



protected:
    class Polygon : public HasSearchKey
    {
    public:
		YsVec3 nom;
        std::vector <VertexHandle> vtHd;
		YsColor col;
    };
private:
    mutable std::list <Polygon> plgList;
public:
    class PolygonHandle
    {
    friend class PolygonalMesh;
    private:
        std::list <Polygon>::iterator plgPtr;
    public:
        PolygonHandle(){};  // C++11 PolygonHandle()=default;
        inline bool operator==(const PolygonHandle &plHd) const
        {
			return plHd.plgPtr==plgPtr;
		}
        inline bool operator!=(const PolygonHandle &plHd) const
        {
			return plHd.plgPtr!=plgPtr;
		}
    };
    PolygonHandle AddPolygon(int nPlVt,const VertexHandle plVtHd[])
    {
		Polygon newPlg;
		for(auto i=0; i<nPlVt; ++i)
		{
			newPlg.vtHd.push_back(plVtHd[i]);
		}

		plgList.push_back(newPlg);
		auto last=plgList.end();
		--last;

		PolygonHandle plHd;
		plHd.plgPtr=last;

		RegisterPolygon(plHd);

		return plHd;
	}
    PolygonHandle AddPolygon(const std::vector <VertexHandle> &plVtHd)
    {
		Polygon newPlg;
		newPlg.searchKey=searchKeySeed++;
		newPlg.vtHd=plVtHd;

		plgList.push_back(newPlg);
		auto last=plgList.end();
		--last;

		PolygonHandle plHd;
		plHd.plgPtr=last;

		RegisterPolygon(plHd);

		return plHd;
	}
	void SetNormal(PolygonHandle plHd,const YsVec3 &nom)
	{
		plHd.plgPtr->nom=nom;
	}
	YsVec3 GetNormal(PolygonHandle plHd) const
	{
		return plHd.plgPtr->nom;
	}
	void SetColor(PolygonHandle plHd,YsColor col)
	{
		plHd.plgPtr->col=col;
	}
	YsColor GetColor(PolygonHandle plHd) const
	{
		return plHd.plgPtr->col;
	}
    inline PolygonHandle NullPolygon(void) const
    {
		PolygonHandle plHd;
		plHd.plgPtr=plgList.end();
		return plHd;
	}
	size_t GetPolygonNumVertices(PolygonHandle plHd) const
	{
		return plHd.plgPtr->vtHd.size();
	}
    const std::vector <VertexHandle> GetPolygonVertex(PolygonHandle plHd) const
    {
		return plHd.plgPtr->vtHd;
	}
	void SetPolygonVertex(PolygonHandle plHd,const std::vector <VertexHandle> newPlVtHd)
	{
		UnregisterPolygon(plHd);
		plHd.plgPtr->vtHd=newPlVtHd;
		RegisterPolygon(plHd);
	}
	int GetPolygonVertex(const VertexHandle *&plVtHd,PolygonHandle plHd) const
	{
		plVtHd=plHd.plgPtr->vtHd.data();
		return plHd.plgPtr->vtHd.size();
	}
	unsigned int GetSearchKey(PolygonHandle plHd) const
	{
		return plHd.plgPtr->searchKey;
	}
	size_t GetNumPolygons(void) const
	{
		return plgList.size();
	}
	PolygonHandle FirstPolygon(void) const
	{
		PolygonHandle plHd;
		plHd.plgPtr=plgList.begin();
		return plHd;
	}
	bool MoveToNext(PolygonHandle &plHd) const
	{
		if(plHd.plgPtr!=plgList.end())
		{
			++plHd.plgPtr;
		}
		else
		{
			plHd.plgPtr=plgList.begin();
		}
		return plHd.plgPtr!=plgList.end();
	}



private:
	std::unordered_map <unsigned int,std::vector <PolygonHandle> > vtxToPlg;
	std::unordered_map <Edge,std::vector <PolygonHandle> > edgeToPlg;

	void RegisterPolygon(PolygonHandle plHd)
	{
		for(auto vtHd : plHd.plgPtr->vtHd)
		{
			auto vtKey=GetSearchKey(vtHd);
			vtxToPlg[vtKey].push_back(plHd);
		}
		for(int i=0; i<plHd.plgPtr->vtHd.size(); ++i)
		{
			Edge e;
			e.edVtKey[0]=GetSearchKey(plHd.plgPtr->vtHd[i]);
			e.edVtKey[1]=GetSearchKey(plHd.plgPtr->vtHd[(i+1)%plHd.plgPtr->vtHd.size()]);
			edgeToPlg[e].push_back(plHd);
		}
	}
	void UnregisterPolygon(PolygonHandle plHd)
	{
		for(auto vtHd : plHd.plgPtr->vtHd)
		{
			auto vtKey=GetSearchKey(vtHd);
			auto found=vtxToPlg.find(vtKey);
			if(vtxToPlg.end()!=found)
			{
				for(int i=found->second.size()-1; 0<=i; --i)
				{
					if(found->second[i]==plHd)
					{
						found->second[i]=found->second.back();
						found->second.pop_back();
					}
				}
			}
			else
			{
				std::cout << "Integrity!\n";
			}
		}
		for(int i=0; i<plHd.plgPtr->vtHd.size(); ++i)
		{
			Edge e;
			e.edVtKey[0]=GetSearchKey(plHd.plgPtr->vtHd[i]);
			e.edVtKey[1]=GetSearchKey(plHd.plgPtr->vtHd[(i+1)%plHd.plgPtr->vtHd.size()]);
			auto found=edgeToPlg.find(e);
			if(edgeToPlg.end()!=found)
			{
				for(int i=found->second.size()-1; 0<=i; --i)
				{
					if(found->second[i]==plHd)
					{
						found->second[i]=found->second.back();
						found->second.pop_back();
					}
				}
			}
			else
			{
				std::cout << "Integrity!\n";
			}
		}
	}

public:
	std::vector <PolygonHandle> FindPolygonsFromVertex(VertexHandle vtHd) const
	{
		auto vtKey=GetSearchKey(vtHd);
		auto found=vtxToPlg.find(vtKey);
		if(found!=vtxToPlg.end())
		{
			return found->second;
		}
		else
		{
			return std::vector <PolygonHandle>();
		}
	}

	std::vector <VertexHandle> GetConnectedVertex(VertexHandle fromVtHd) const
	{
		std::vector <VertexHandle> connVtHd;
		for(auto plHd : FindPolygonsFromVertex(fromVtHd))
		{
			auto plVtHd=GetPolygonVertex(plHd);
			for(int i=0; i<plVtHd.size(); ++i)
			{
				if(fromVtHd==plVtHd[i])
				{
					VertexHandle C[2]=
					{
						plVtHd[(i+1)%plVtHd.size()],
						plVtHd[(i+plVtHd.size()-1)%plVtHd.size()]
					};
					for(auto c : C)
					{
						if(connVtHd.end()==std::find(connVtHd.begin(),connVtHd.end(),c))
						{
							connVtHd.push_back(c);
						}
					}
				}
			}
		}
		return connVtHd;
	}

	std::vector <PolygonHandle> FindPolygonsFromEdgePiece(
	    VertexHandle edVtHd0,VertexHandle edVtHd1) const
	{
		std::vector <PolygonHandle> plHdArray;
		return plHdArray;
	}

	PolygonHandle GetNeighborPolygon(PolygonHandle plHd,int i) const
	{
		auto plVtHd=GetPolygonVertex(plHd);
		if(i<plVtHd.size())
		{
			Edge e;
			e.edVtKey[0]=GetSearchKey(plVtHd[i]);
			e.edVtKey[1]=GetSearchKey(plVtHd[(i+1)%plVtHd.size()]);
			auto found=edgeToPlg.find(e);
			if(edgeToPlg.end()!=found && 2==found->second.size())
			{
				if(found->second[0]==plHd)
				{
					return found->second[1];
				}
				else if(found->second[1]==plHd)
				{
					return found->second[0];
				}
				else
				{
					std::cout << "GetNeighborPolygon found a broken table.\n";
				}
			}
		}
		return NullPolygon();
	}


public:
	void CleanUp(void)
	{
		// Will implement.
	}


public:
    bool LoadBinStl(const char fileName[]);


public:
    void GetBoundingBox(YsVec3 &mn,YsVec3 &mx) const;
    void GetBoundingBox(YsVec3 bbx[2]) const;


public:
	class OpenGLArrays
	{
	public:
		std::vector <float> vtx;
		std::vector <float> nom;
		std::vector <float> col;

		std::vector <float> edgeVtx, edgeCol;
	};
	OpenGLArrays MakeVertexArrays(void) const; // Gabriel +1



public:
	void StitchVertexN2(void);
	void Stitch(void);
	
public:
	struct Triangle {
		std::vector <float> normal;
		std::vector <std::vector <float> > trvtx;
	};

	void SaveMesh(void)
	{
		std::vector <std::vector <float> > vertices;
		std::vector <std::vector <float> > triangleNormals;
		std::vector<std::vector<std::vector<float>>> triangleVertices;
		for (auto plHd = FirstPolygon(); plHd != NullPolygon(); MoveToNext(plHd))
		{
			auto verts = GetPolygonVertex(plHd);
			std::vector<YsVec3> pos;
			for (int j = 0; j < 3; j++)
			{
				pos.push_back(GetVertexPosition(verts[j]));
			}
			//float a = pos[0].x();
			//float b = pos[0].y();
			//float c = pos[0].z();

			for (int i = 0; i < 3; i++)
			{
				float a = pos[i].x();
				float b = pos[i].y();
				float c = pos[i].z();

				std::vector <float> vectorList = { a,b,c };
				/*
				auto find = std::find(vertices.begin(),vertices.end(),vectorList);
				if (find == vertices.end())
					{
						vertices.push_back(vectorList);
					}
				*/
				//auto point = pos[i];
				vertices.push_back(vectorList);

				//vertices.push_back(2);
			}
			auto normalPos = GetNormal(plHd);
			float nx = normalPos.x();
			float ny = normalPos.y();
			float nz = normalPos.z();

			std::vector <float> normalVector = { nx,ny,nz };
			std::vector <int> temp = { 3,2,1 };

			auto vA = vertices[vertices.size() - 3];
			auto vB = vertices[vertices.size() - 2];
			auto vC = vertices[vertices.size() - 1];


			YsVec3 e1 = YsVec3(vB[0] - vA[0], vB[1] - vA[1], vB[2] - vA[2]);
			YsVec3 e2 = YsVec3(vC[0] - vA[0], vC[1] - vA[1], vC[2] - vA[2]);
			auto cross = YsVec3(e1.y() * e2.z() - e1.z() * e2.y(), e1.z() * e2.x() - e1.x() * e2.z(), e1.x() * e2.y() - e1.y() * e2.x());
			auto dot = cross.x() * nx + cross.y() * ny + cross.z() * nz;
			if (dot < 0)
			{
				temp = { 3,1,2 };
			}




			std::vector <std::vector <float>> triangleVtx = { vertices[vertices.size() - temp[0]],vertices[vertices.size() - temp[1]],vertices[vertices.size() - temp[2]] };
			triangleNormals.push_back(normalVector);
			//printf("%f %f %f\n",triangleNormals[triangleNormals.size()-1][0],triangleNormals[triangleNormals.size()-1][1],triangleNormals[triangleNormals.size()-1][2]);
			triangleVertices.push_back(triangleVtx);


			//int vtxId = mesh.GetSearchKey(vtHd);
			//vtxIdToHandle[vtxId] = vtHd;
		}
		/*
		std::string input;
		std::cout << "\nInput STL file path: ";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::getline(std::cin >> std::ws, input);
		const char* cstr = input.c_str();
		std::ofstream stlFile(cstr, std::ios::binary);
		*/
		std::string input;
		std::cout << "\nInput STL file name (must include .stl): ";
		std::cin >> input;
		//std::getline(std::cin, input);
		const char* cstr = input.c_str();
		std::ofstream stlFile(cstr, std::ios::binary);
		
		
		
		if (stlFile.is_open())
		{
			// Write header (80 bytes)
			char header[80] = {};  // Zero-initialized
			strncpy(header, "Cube Model", 79);
			stlFile.write(header, 80);

			// Write number of triangles (4 bytes)
			uint32_t numTriangles = static_cast<uint32_t>(triangleNormals.size());
			stlFile.write(reinterpret_cast<const char*>(&numTriangles), 4);

			// Write triangle data
			/*
			for (const auto& triangle : triangles) {
				stlFile.write(reinterpret_cast<const char*>(&triangle.normal), sizeof(triangle.normal));
				for (int i = 0; i < 3; ++i) {
					stlFile.write(reinterpret_cast<const char*>(&triangle.vertices[i]), sizeof(triangle.vertices[i]));
				}
				unsigned short attributeByteCount = 0;
				stlFile.write(reinterpret_cast<const char*>(&attributeByteCount), sizeof(attributeByteCount));
			}
			*/
			for (int i = 0; i < triangleNormals.size(); i++)
			{
				stlFile.write(reinterpret_cast<const char*>(triangleNormals[i].data()), 12);
				for (int j = 0; j < 3; j++) {
					stlFile.write(reinterpret_cast<const char*>(triangleVertices[i][j].data()), 12);
				}
				uint16_t attributeByteCount = 0;
				stlFile.write(reinterpret_cast<const char*>(&attributeByteCount), 2);
			}

			// Close the file
			stlFile.close();
			std::cout << "STL file exported successfully." << std::endl;
		}
		else
		{
			std::cerr << "Error opening file for writing." << std::endl;
		}
	}

public:
	int currentPrimitive = -1;

	bool primitives(int val)
	{
		if (val == 0)
		{
			if(true!=LoadBinStl("../../Resource/Primitive/sphere.stl"))
			{
				std::cout << "STL read error\n";
				return false;
			}
			currentPrimitive = 0;
			return true;
		}
		
		else if (val == 1)
		{
			if(true!=LoadBinStl("../../Resource/Primitive/plate.stl"))
			{
				std::cout << "STL read error\n";
				return false;
			}
			currentPrimitive = 1;
			return true;
		}
		else if (val == 2)
		{
			if(true!=LoadBinStl("../../Resource/Primitive/cube.stl"))
			{
				std::cout << "STL read error\n";
				return false;
			}
			currentPrimitive = 2;
			return true;
		}
		else if (val == 3)
		{
			std::string input;
			std::cout << "\nInput STL file path: ";
			std::getline(std::cin, input);
			const char* cstr = input.c_str();
			if(true!=LoadBinStl(cstr))
			{
				std::cout << "STL read error\n";
				return false;
			}
			currentPrimitive = 3;
			return true;
		}
		/*
		else if (val == 3)
		{
			char filename[256] = "";
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;
			ofn.lpstrFile = filename;
			ofn.nMaxFile = sizeof(filename);
			ofn.lpstrFilter = "All Files (*.*)\0*.*\0Text Files (*.txt)\0*.txt\0";
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			if (GetOpenFileName(&ofn) == TRUE) {
				std::cout << "Selected file: " << filename << std::endl;
			} else {
				std::cout << "File selection cancelled or error occurred." << std::endl;
			}	
			if(true!=LoadBinStl(filename))
			{
				std::cout << "STL read error\n";
				return 0;
			}
			
		}
		*/
	}

};

#endif

