#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include "nate.h"

#include "mesh.h"
#include "fssimplewindow.h"
#include "ysclass.h"

void nate::helloWorld()
{
    std::cout << "Hello World!";
}

void nate::arrow(double el, YsVec3 org,  int direc)
{
    PolygonalMesh mesh;
    YsVec3 topV;
    YsVec3 r1V;
    YsVec3 l1V;
    YsVec3 r2V;
    YsVec3 l2V; 

    switch(direc)
    {
        case 0: //x-direction
        {
            //top vertex
            topV[0] = org[0]+el*0.03;
            topV[1] = org[1];
            topV[2] = org[2];    
            //right 1 vertex
            r1V[0] = org[0]+el*0.02;
            r1V[1] = org[1]+el*0.01;
            r1V[2] = org[2];
            //left 1 vertex
            l1V[0] = org[0]+el*0.02;
            l1V[1] = org[1]-el*0.01;
            l1V[2] = org[2];
            //right 2 vertex
            r2V[0] = org[0]+el*0.02;
            r2V[1] = org[1];
            r2V[2] = org[2]+el*0.01;
            //left 2 vertex
            l2V[0] = org[0]+el*0.02;
            l2V[1] = org[1];
            l2V[2] = org[2]-el*0.01;
            
            break;
        }
        case 1: //y-direction
        {
            topV[0] = org[0];
            topV[1] = org[1]+el*0.03;
            topV[2] = org[2];
            //right 1 vertex
            r1V[0] = org[0]+el*0.01;
            r1V[1] = org[1]+el*0.02;
            r1V[2] = org[2];
            //left 1 vertex
            l1V[0] = org[0]-el*0.01;
            l1V[1] = org[1]+el*0.02;
            l1V[2] = org[2];
            //right 2 vertex
            r2V[0] = org[0];
            r2V[1] = org[1]+el*0.02;
            r2V[2] = org[2]+el*0.01;
            //left 2 vertex
            l2V[0] = org[0];
            l2V[1] = org[1]+el*0.02;
            l2V[2] = org[2]-el*0.01;

            break;
        }
        case 2: //z-direction
        {
            topV[0] = org[0];
            topV[1] = org[1];
            topV[2] = org[2]+el*0.03;
            //right 1 vertex
            r1V[0] = org[0]+el*0.01;
            r1V[1] = org[1];
            r1V[2] = org[2]+el*0.02;
            //left 1 vertex
            l1V[0] = org[0]-el*0.01;
            l1V[1] = org[1];
            l1V[2] = org[2]+el*0.02;
            //right 2 vertex
            r2V[0] = org[0];
            r2V[1] = org[1]+el*0.01;
            r2V[2] = org[2]+el*0.02;
            //left 2 vertex
            l2V[0] = org[0];
            l2V[1] = org[1]-el*0.01;
            l2V[2] = org[2]+el*0.02;
            break;
        }
    }

    glBegin(GL_LINE_STRIP);
	glColor3f(0,0,0);

    //Main Line
    glVertex3dv(org);
    glVertex3dv(topV);
    //Right 1
    glVertex3dv(r1V);
    //Bottom 1
    glVertex3dv(l1V);
    //Left 1
    glVertex3dv(topV);
    //Right 2
    glVertex3dv(r2V);
    //Bottom 2
    glVertex3dv(l2V);
    //Left 2
    glVertex3dv(topV);

    glEnd();
    
}