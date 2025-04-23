#ifndef LATTICE_H_IS_INCLUDED
#define LATTICE_H_IS_INCLUDED

template <class T>
class Lattice3d
{
protected:
    int nx=0,ny=0,nz=0;
    std::vector <T> elem;
    YsVec3 min,max;
public:
    /*! Create a lattice with nx*ny*nz blocks.  To store nodal information, the length of the
        storage actually allocated will be (nx+1)*(ny+1)*(nz+1).  */
    void Create(int nx,int ny,int nz,const YsVec3 &min,const YsVec3 &max)
    {
		int L=(nx+1)*(ny+1)*(nz+1);
		elem.resize(L);
		this->nx=nx;
		this->ny=ny;
		this->nz=nz;
		this->min=min;
		this->max=max;
	}

    /*! Returns number of blocks in X,Y, and Z direction. */
    YSSIZE_T Nx(void) const
    {
		return nx;
	}
    YSSIZE_T Ny(void) const
	{
		return ny;
	}
    YSSIZE_T Nz(void) const
    {
		return nz;
	}
    /*! Returns the dimension of one block. */
    YsVec3 GetBlockDimension(void) const
    {
		auto dim=max-min;
		dim.DivX(double(nx));
		dim.DivY(double(ny));
		dim.DivZ(double(nz));
		return dim;
	}
    /*! Returns the minimum (x,y,z) of the block at (bx,by,bz). */
    YsVec3 GetBlockPosition(int ix,int iy,int iz) const
    {
		auto dim=GetBlockDimension();
		dim.MulX(double(ix));
		dim.MulY(double(iy));
		dim.MulZ(double(iz));
		dim+=min;
	}
    /*! Returns the index of the block that encloses the given position. */
    YsVec3i GetBlockIndex(const YsVec3 &pos) const
    {
		auto off=pos-min;
		auto dim=GetBlockDimension();
		off.DivX(dim.x());
		off.DivY(dim.y());
		off.DivZ(dim.z());
		return YsVec3i(off.x(),off.y(),off.z());
	}
    /*! Returns if the block index is within the valid range.
        The lattice elements can be nodal value or cell value.  To support the nodal values,
        this class allocates (nx+1)*(ny+1)*(nz+1) elems.  Therefore, the index is valid
        and this function returns true, if:
           0<=idx.x() && idx.x()<=nx && 0<=idx.y() && idx.y()<=ny && 0<=idx.z() && idx.z()<=nz. */
    bool IsInRange(const YsVec3i idx) const
    {
		return 0<=idx.x() && idx.x()<=nx && 0<=idx.y() && idx.y()<=ny && 0<=idx.z() && idx.z()<=nz;
	}
    /*! Returns a reference to the lattice element at the index. */
    T &operator[](const YsVec3i idx)
    {
		size_t i=idx.z()*(nx+1)*(ny+1)+idx.y()*(nx+1)+idx.x();
		return elem[i];
	}
    const T &operator[](const YsVec3i idx) const
    {
		size_t i=idx.z()*(nx+1)*(ny+1)+idx.y()*(nx+1)+idx.x();
		return elem[i];
	}
};

#endif
