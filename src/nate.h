#ifndef NATE_IS_INCLUDED
#define NATE_IS_INCLUDED

#include <list>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <ysclass.h>
#include "mesh.h"

class nate
{
public:
    void helloWorld(void);
    void arrow(double el, YsVec3 org, int direc);
};

#endif