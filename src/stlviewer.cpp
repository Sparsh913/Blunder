#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <string>
#include "fssimplewindow.h"
#include "ysclass.h"
#include "mesh.h"
#include "glutil.h"
#include "laplacian_editing.h"
#include "smoother.h"
#include "ysglfontdata.h"
#include "nate.h"


const double epsilon = 1e-6;

const int winWid = 800;
const int winHei = 600;

// Panel - can contain subpanels
// subpanels, vector of pointers
// contains subclass property,
template<class Property>
class Container
{
private:
    class Panel
    {
    friend class Container <Property>;
    enum
    {
        TYPE_DISPLAY, // Display information, non interactable (info)
        TYPE_SLIDER_LIMITED, // Slider with bounds             (min max)
        TYPE_SLIDER_UNLIMITED, // Slider with no bounds        (-inf inf)
        TYPE_CHECKBOX, // For bool type properties             (On Off)
        TYPE_DROPDOWN_SELECTION // For enum type properties    (A, B, C, D)
    };

    public:
        // Each panel holds a panel type, and a property 
        int panelType = TYPE_DISPLAY;
        std::string propName;
        std::string tooltip;
        
        // Property to be displayed/editted
        Property property;
        Property tempProperty;
        Property *propPtr;
    };
public:
    // Container holds array of panels
    std::vector <Panel *> panelVec;
    int posX = 400;
    int posY = 0;
    int padding = 20;
    int width = 100;
    int panelHeight = 20;
    int height = panelVec.size() * panelHeight;

public:
    // Pointer to panel and its property
    class PanelHandle
    {
    friend class Container <Property>;
    private:
        Panel *ptr=nullptr;
    public:
        bool IsNotNull(void) const
		{
			return nullptr!=ptr;
		}
        bool IsNull(void) const
        {
            return nullptr==ptr;
        }
        void MakeNull(void)
        {
            ptr = nullptr;
        }
        bool operator==(PanelHandle incoming) const
		{
			return incoming.ptr==ptr;
		}
		bool operator!=(PanelHandle incoming) const
		{
			return incoming.ptr!=ptr;
		}
        void SetProperty(Property newprop)
        {
            ptr->property = newprop;
            ptr->tempProperty = newprop;
        }
        void SetTempProperty(Property newprop)
        {
            ptr->tempProperty = newprop;
        }
        void SetAddress(Property * address)
        {
            ptr->propPtr = address;
        }
        void UpdateTrueProperty()
        {
            Property *temp = ptr->propPtr;
            *temp = ptr->property;
        }
    };

public:
    int longestname = 0;

public:
    void Delete(Panel *ptr)
    {
        ptr->subpanels.clear();
    }
    // Resizing functions
    void ResizeContainer(int wid, int hei)
    {
        height = hei;
        width = wid;
    }
    void ResizeContainerWidth(int wid)
    {
        width = wid;
    }
    void AutoResize()
    {
        height = panelVec.size() * (panelHeight + padding/3) + 2* padding;
    }
    void ResizePanel(int hei)
    {
        panelHeight = hei;
    }
    void setPadding(int pad)
    {
        padding = pad;
    }
    void setPos(int x, int y)
    {
        posX = x;
        posY = y;
    }
public:
    PanelHandle PtrToHandle(Panel *ptr) const
    {
        PanelHandle hd;
        hd.ptr=ptr;
        return hd;
    }
    PanelHandle Insert(const Property &property, 
                           std::string pn, std::string tt, Property *address)
    {
        auto *newPanel = new Panel;
        newPanel->property = property;
        newPanel->tempProperty = property;
        newPanel->propName = pn;
        newPanel->tooltip = tt;
        newPanel->propPtr = address;
        panelVec.emplace_back(newPanel);

        AutoResize();
        return PtrToHandle(newPanel);
    }
    const Property &GetProperty(PanelHandle hd) const
    {
        return hd.ptr->property;
    }
    const Property &GetTempProperty(PanelHandle hd) const
    {
        return hd.ptr->tempProperty;
    }
    void GetLongestNameLen() 
    {
        int len = 0;
        char name[256];
        for(int i = 0; i < panelVec.size(); i++)
        {
            sprintf(name, "%s", panelVec[i]->propName.c_str());
            if(std::strlen(name) > len)
            {
                len = std::strlen(name);
            }
        }
        //std::cout << len;
        longestname = len;
    }
};

// Find picked panel
Container<double>::PanelHandle PickedPnlHd(
    Container<double> &cont,
    int mx, int my)
    {
        int wid,hei;
        FsGetWindowSize(wid,hei);
        // Container coordinates
        int x0 = cont.posX + cont.padding;
        int x1 = cont.posX + cont.width - cont.padding;
        int y0 = cont.posY + cont.padding;
        int y1 = cont.posY + cont.padding + cont.panelHeight;
        int yPad = cont.padding/3 + cont.panelHeight;

        // Check if a panel is selected
        for(int i = 0; i < cont.panelVec.size(); i++)
        {
            if(x0<=mx && mx<x1 && y0+(yPad*i)<=my && my<y1+(yPad*i))
            {
                // take pointer to panelhandle
                return cont.PtrToHandle(cont.panelVec[i]);
            }
        }
        Container<double>::PanelHandle nullHd;
		return nullHd;
    }

Container<bool>::PanelHandle PickedPnlHdBool(
    Container<bool> &cont,
    int mx, int my)
    {
        int wid,hei;
        FsGetWindowSize(wid,hei);
        // Container coordinates
        int x0 = cont.posX + cont.width/2 - cont.panelHeight/2;
        int x1 = cont.posX + cont.width/2 + cont.panelHeight/2;
        int y0 = cont.posY + cont.padding;
        int y1 = cont.posY + cont.padding + cont.panelHeight;
        int yPad = cont.padding/3 + cont.panelHeight;

        // Check if a panel is selected
        for(int i = 0; i < cont.panelVec.size(); i++)
        {
            if(x0<=mx && mx<x1 && y0+(yPad*i)<=my && my<y1+(yPad*i))
            {
                // take pointer to panelhandle
                return cont.PtrToHandle(cont.panelVec[i]);
            }
        }
        Container<bool>::PanelHandle nullHd;
        return nullHd;
    }

// Find picked panel
Container<bool>::PanelHandle PickedPnlHdBoolSs(
    Container<bool> &cont,
    int mx, int my)
{
    int wid,hei;
    FsGetWindowSize(wid,hei);
    // Container coordinates
    int x0 = cont.posX + cont.padding;
    int x1 = cont.posX + cont.width - cont.padding;
    int y0 = cont.posY + cont.padding;
    int y1 = cont.posY + cont.padding + cont.panelHeight;
    int yPad = cont.padding/3 + cont.panelHeight;

    // Check if a panel is selected
    for(int i = 0; i < cont.panelVec.size(); i++)
    {
        if(x0<=mx && mx<x1 && y0+(yPad*i)<=my && my<y1+(yPad*i))
        {
            // take pointer to panelhandle
            return cont.PtrToHandle(cont.panelVec[i]);
        }
    }
    Container<bool>::PanelHandle nullHd;
    return nullHd;
}

// Draw container FLOAT
void RenderContainer(const Container <double> &cont,
                     typename Container<double>::PanelHandle highlightHover,
                     typename Container<double>::PanelHandle highlightSelect)
{
    int x0 = cont.posX + cont.padding;
    int x1 = cont.posX + cont.width - cont.padding;
    int y0 = cont.posY + cont.padding;
    int y1 = cont.posY + cont.padding + cont.panelHeight;
    int yPad = cont.padding/3 + cont.panelHeight;
    int idx = 0;
    
    // Check if a panel is selected
    for(int i = 0; i < cont.panelVec.size(); i++)
    {
        if(highlightSelect.IsNotNull() && cont.PtrToHandle(cont.panelVec[i])==highlightSelect) // Highlight selected
        {
            glColor3f(0.75,0.75,1.0);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2i(x0,y0+(yPad*i));
            glVertex2i(x1,y0+(yPad*i));
            glVertex2i(x1,y1+(yPad*i));
            glVertex2i(x0,y1+(yPad*i));
            glEnd();
            glColor3f(0,0,0);
        }
        else if(cont.PtrToHandle(cont.panelVec[i])==highlightHover) // Highlight hovered panel
        {
            // take pointer to panelhandle
            glColor3f(0.65,0.65,0.65);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2i(x0,y0+(yPad*i));
            glVertex2i(x1,y0+(yPad*i));
            glVertex2i(x1,y1+(yPad*i));
            glVertex2i(x0,y1+(yPad*i));
            glEnd();
            glColor3f(0,0,0);
        }
        else // draw other panels
        {
            glColor3f(0.5, 0.5, 0.5);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2i(x0,y0+(yPad*i));
            glVertex2i(x1,y0+(yPad*i));
            glVertex2i(x1,y1+(yPad*i));
            glVertex2i(x0,y1+(yPad*i));
            glEnd();
            glColor3f(0,0,0);
        }

        int cx = (x0+x1)/2;
        int cy = y0 + (yPad*i) + cont.panelHeight/2;
        
        // write property value
        glColor3f(1,1,1);
        char str[256];
        sprintf(str, "%.2f", cont.panelVec[i]->tempProperty);
        glRasterPos2i(cx - 6*std::strlen(str)/2, cy + 4);
        YsGlDrawFontBitmap6x8(str);

        // write property name
        char name[256];
        sprintf(name, "%s", cont.panelVec[i]->propName.c_str());
        glColor3f(1,1,1);
        glRasterPos2i(cx - 6*std::strlen(name) - cont.width/2, cy + 4);
        YsGlDrawFontBitmap6x8(name);
    }

    // Draw container
    glColor3f(0.35, 0.35, 0.35);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2i(cont.posX, cont.posY);
    glVertex2i(cont.posX + cont.width, cont.posY);
    glVertex2i(cont.posX + cont.width, cont.posY + cont.height);
    glVertex2i(cont.posX, cont.posY + cont.height);
    glEnd();

    // fit in pname
    glColor3f(0.35, 0.35, 0.35);
    glBegin(GL_TRIANGLE_FAN);
    int nameLen = cont.longestname;
    glVertex2i(cont.posX - 6*nameLen - cont.padding, cont.posY);
    glVertex2i(cont.posX, cont.posY);
    glVertex2i(cont.posX, cont.posY + cont.height);
    glVertex2i(cont.posX - 6*nameLen - cont.padding, cont.posY + cont.height);
    glEnd();

    // window name background
    glColor3f(0.15, 0.15, 0.15);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2i(cont.posX - 6*nameLen - cont.padding, cont.posY);
    glVertex2i(cont.posX + cont.width, cont.posY);
    glVertex2i(cont.posX + cont.width, cont.posY - cont.panelHeight);
    glVertex2i(cont.posX - 6*nameLen - cont.padding, cont.posY - cont.panelHeight);
    glEnd();
    
    // write property value
    glColor3f(1,1,1);
    char splash[256];
    sprintf(splash, "Transform");
    glRasterPos2i(cont.posX + cont.panelHeight/2, cont.posY - 5);
    YsGlDrawFontBitmap6x8(splash);

    // X
    glColor3f(1,1,1);
    char xbutton[256];
    sprintf(xbutton, "x");
    glRasterPos2i(cont.posX + cont.width - cont.panelHeight/2 - 3, cont.posY - 5);
    YsGlDrawFontBitmap6x8(xbutton);
}

// Draw container BOOL
void RenderContainerCheckbox(const Container <bool> &cont,
    typename Container<bool>::PanelHandle highlightHover,
    typename Container<bool>::PanelHandle highlightSelect)
{
    int x0 = cont.posX + cont.width/2 - cont.panelHeight/2;
    int x1 = cont.posX + cont.width/2 + cont.panelHeight/2;
    int y0 = cont.posY + cont.padding;
    int y1 = cont.posY + cont.padding + cont.panelHeight;
    int yPad = cont.padding/3 + cont.panelHeight;
    int idx = 0;

    // Check if a panel is selected
    for(int i = 0; i < cont.panelVec.size(); i++)
    {
        if(highlightSelect.IsNotNull() && cont.PtrToHandle(cont.panelVec[i])==highlightSelect) // Highlight selected
        {
            glColor3f(0.75,0.75,1.0);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2i(x0,y0+(yPad*i));
            glVertex2i(x1,y0+(yPad*i));
            glVertex2i(x1,y1+(yPad*i));
            glVertex2i(x0,y1+(yPad*i));
            glEnd();
            glColor3f(0,0,0);
        }
        else if(cont.PtrToHandle(cont.panelVec[i])==highlightHover) // Highlight hovered panel
        {
            // take pointer to panelhandle
            glColor3f(0.65,0.65,0.65);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2i(x0,y0+(yPad*i));
            glVertex2i(x1,y0+(yPad*i));
            glVertex2i(x1,y1+(yPad*i));
            glVertex2i(x0,y1+(yPad*i));
            glEnd();
            glColor3f(0,0,0);
        }
        else // draw other panels
        {
            glColor3f(0.5, 0.5, 0.5);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2i(x0,y0+(yPad*i));
            glVertex2i(x1,y0+(yPad*i));
            glVertex2i(x1,y1+(yPad*i));
            glVertex2i(x0,y1+(yPad*i));
            glEnd();
            glColor3f(0,0,0);
        }

        int cx = (x0+x1)/2;
        int cy = y0 + (yPad*i) + cont.panelHeight/2;

        // write property value
        glColor3f(1,1,1);
        char str[256];
        if (cont.panelVec[i]->property){sprintf(str, "X");}
        else                           {sprintf(str, " ");} 

        glRasterPos2i(cx - 6*std::strlen(str)/2 + 1, cy + 4);
        YsGlDrawFontBitmap6x8(str);

        // write property name
        char name[256];
        sprintf(name, "%s", cont.panelVec[i]->propName.c_str());
        glColor3f(1,1,1);
        glRasterPos2i(cx - 6*std::strlen(name) - cont.width/2, cy + 4);
        YsGlDrawFontBitmap6x8(name);
    }

    // Draw container
    glColor3f(0.35, 0.35, 0.35);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2i(cont.posX, cont.posY);
    glVertex2i(cont.posX + cont.width, cont.posY);
    glVertex2i(cont.posX + cont.width, cont.posY + cont.height);
    glVertex2i(cont.posX, cont.posY + cont.height);
    glEnd();

    // fit in pname
    glColor3f(0.35, 0.35, 0.35);
    glBegin(GL_TRIANGLE_FAN);
    int nameLen = cont.longestname;
    glVertex2i(cont.posX - 6*nameLen - cont.padding, cont.posY);
    glVertex2i(cont.posX, cont.posY);
    glVertex2i(cont.posX, cont.posY + cont.height);
    glVertex2i(cont.posX - 6*nameLen - cont.padding, cont.posY + cont.height);
    glEnd();
}

void RenderContainerSplash(const Container <bool> &cont,
    typename Container<bool>::PanelHandle highlightHover,
    typename Container<bool>::PanelHandle highlightSelect)
{
    int x0 = cont.posX + cont.padding;
    int x1 = cont.posX + cont.width - cont.padding;
    int y0 = cont.posY + cont.padding;
    int y1 = cont.posY + cont.padding + cont.panelHeight;
    int yPad = cont.padding/3 + cont.panelHeight;
    int idx = 0;

    // Draw container
    glColor3f(0.35, 0.35, 0.35);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2i(cont.posX, cont.posY);
    glVertex2i(cont.posX + cont.width, cont.posY);
    glVertex2i(cont.posX + cont.width, cont.posY + cont.height);
    glVertex2i(cont.posX, cont.posY + cont.height);
    glEnd();

    glColor3f(0.15, 0.15, 0.15);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2i(cont.posX, cont.posY);
    glVertex2i(cont.posX + cont.width, cont.posY);
    glVertex2i(cont.posX + cont.width, cont.posY - cont.panelHeight);
    glVertex2i(cont.posX, cont.posY - cont.panelHeight);
    glEnd();
    
    // write property value
    glColor3f(1,1,1);
    char splash[256];
    sprintf(splash, "@ Blunder - Select a primative to start");
    glRasterPos2i(cont.posX + cont.panelHeight/2, cont.posY - 5);
    YsGlDrawFontBitmap6x8(splash);

    // Check if a panel is selected
    for(int i = 0; i < cont.panelVec.size(); i++)
    {
        if(highlightSelect.IsNotNull() && cont.PtrToHandle(cont.panelVec[i])==highlightSelect) // Highlight selected
        {
            glColor3f(0.75,0.75,1.0);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2i(x0,y0+(yPad*i));
            glVertex2i(x1,y0+(yPad*i));
            glVertex2i(x1,y1+(yPad*i));
            glVertex2i(x0,y1+(yPad*i));
            glEnd();
            glColor3f(0,0,0);
        }
        else if(cont.PtrToHandle(cont.panelVec[i])==highlightHover) // Highlight hovered panel
        {
            // take pointer to panelhandle
            glColor3f(0.65,0.65,0.65);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2i(x0,y0+(yPad*i));
            glVertex2i(x1,y0+(yPad*i));
            glVertex2i(x1,y1+(yPad*i));
            glVertex2i(x0,y1+(yPad*i));
            glEnd();
            glColor3f(0,0,0);
        }
        else // draw other panels
        {
            glColor3f(0.5, 0.5, 0.5);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2i(x0,y0+(yPad*i));
            glVertex2i(x1,y0+(yPad*i));
            glVertex2i(x1,y1+(yPad*i));
            glVertex2i(x0,y1+(yPad*i));
            glEnd();
            glColor3f(0,0,0);
        }

        int cx = (x0+x1)/2;
        int cy = y0 + (yPad*i) + cont.panelHeight/2;
        
        // write property value
        glColor3f(1,1,1);
        char str[256];
        sprintf(str, "%s", cont.panelVec[i]->propName.c_str());
        glRasterPos2i(cx - 6*std::strlen(str)/2, cy + 4);
        YsGlDrawFontBitmap6x8(str);
    }
}

void RenderHelpScreen(const Container <bool> &cont)
{
    int x0 = cont.posX + cont.padding - cont.width/2;
    int x1 = cont.posX + cont.width - cont.padding + cont.width/2;
    int y0 = cont.posY + cont.padding;
    int y1 = cont.posY + cont.padding + cont.panelHeight;
    int yPad = cont.padding/3 + cont.panelHeight;
    int idx = 0;

    // Draw container
    glColor3f(0.35, 0.35, 0.35);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2i(x0, cont.posY);
    glVertex2i(x1, cont.posY);
    glVertex2i(x1, cont.posY + cont.height);
    glVertex2i(x0, cont.posY + cont.height);
    glEnd();

    glColor3f(0.15, 0.15, 0.15);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2i(x0, cont.posY);
    glVertex2i(x1, cont.posY);
    glVertex2i(x1, cont.posY - cont.panelHeight);
    glVertex2i(x0, cont.posY - cont.panelHeight);
    glEnd();
    
    // Instructions
    glColor3f(1,1,1);
    char splash[256];
    sprintf(splash, "? Instructions");
    glRasterPos2i(cont.posX + cont.panelHeight/2, cont.posY - 5);
    YsGlDrawFontBitmap6x8(splash);

    glColor3f(1,1,1);
    char xBox[256] = "x";
    glRasterPos2i(cont.posX + cont.width - cont.panelHeight/2 - 3, cont.posY - cont.panelHeight/2 + 4);
    YsGlDrawFontBitmap6x8(xBox);

    // X
    glColor3f(1,1,1);
    char info0[256] = "Click on a vertex to select it\n";
    char info1[256] = "X,Y,Z - Move selected vertex along respective positive axis\n";
    char info2[256] = "Ctrl + X,Y,Z - Move selected vertex in negative direction\n";
    char info3[256] = "S - Toggle local brush smoothing mode\n";
    char info4[256] = "Scroll Wheel - Adjust brush radius\n";
    char info5[256] = "R - Reset to original positions\n";
    char info6[256] = "C - Clear vertex selection\n";
    char info7[256] = "Arrow keys - Rotate view\n";
    char info8[256] = "G/M - Zoom in/out\n";
    char info9[256] = "ESC - Exit\n";
    char info10[256] = "Save - Ctrl + S\n";
    glRasterPos2i(cont.posX - cont.width/4, cont.posY + cont.panelHeight/2 + 0);
    YsGlDrawFontBitmap6x8(info0);
    glRasterPos2i(cont.posX - cont.width/4, cont.posY + cont.panelHeight/2 + 9);
    YsGlDrawFontBitmap6x8(info1);
    glRasterPos2i(cont.posX - cont.width/4, cont.posY + cont.panelHeight/2 + 18);
    YsGlDrawFontBitmap6x8(info2);
    glRasterPos2i(cont.posX - cont.width/4, cont.posY + cont.panelHeight/2 + 27);
    YsGlDrawFontBitmap6x8(info3);
    glRasterPos2i(cont.posX - cont.width/4, cont.posY + cont.panelHeight/2 + 36);
    YsGlDrawFontBitmap6x8(info4);
    glRasterPos2i(cont.posX - cont.width/4, cont.posY + cont.panelHeight/2 + 45);
    YsGlDrawFontBitmap6x8(info5);
    glRasterPos2i(cont.posX - cont.width/4, cont.posY + cont.panelHeight/2 + 54);
    YsGlDrawFontBitmap6x8(info6);
    glRasterPos2i(cont.posX - cont.width/4, cont.posY + cont.panelHeight/2 + 63);
    YsGlDrawFontBitmap6x8(info7);
    glRasterPos2i(cont.posX - cont.width/4, cont.posY + cont.panelHeight/2 + 72);
    YsGlDrawFontBitmap6x8(info8);
    glRasterPos2i(cont.posX - cont.width/4, cont.posY + cont.panelHeight/2 + 81);
    YsGlDrawFontBitmap6x8(info9);
    glRasterPos2i(cont.posX - cont.width/4, cont.posY + cont.panelHeight/2 + 90);
    YsGlDrawFontBitmap6x8(info10);
}

void WindowCoordToLine(YsVec3 pos[2], int wx, int wy, const YsMatrix4x4& modelView, const YsMatrix4x4& projection)
{
    int wid, hei;
    FsGetWindowSize(wid, hei);

    pos[0] = WindowToViewPort(wid, hei, wx, wy);
    pos[1] = pos[0];

    pos[0].SetZ(-1.0);
    pos[1].SetZ(1.0);

    projection.MulInverse(pos[0], pos[0], 1.0);
    projection.MulInverse(pos[1], pos[1], 1.0);

    modelView.MulInverse(pos[0], pos[0], 1.0);
    modelView.MulInverse(pos[1], pos[1], 1.0);
}

PolygonalMesh::PolygonHandle PickedPlHd(
    const PolygonalMesh& mesh,
    int mx, int my,
    const YsMatrix4x4& modelView, const YsMatrix4x4& projection)
{
    YsVec3 pos[2];
    WindowCoordToLine(pos, mx, my, modelView, projection);

    double nearDist = 0.0;
    PolygonalMesh::PolygonHandle nearPlHd = mesh.NullPolygon();

    for (auto plHd = mesh.FirstPolygon(); mesh.NullPolygon() != plHd; mesh.MoveToNext(plHd))
    {
        auto plVtHd = mesh.GetPolygonVertex(plHd);
        if (3 <= plVtHd.size())
        {
            YsVec3 trVtPos[3] =
            {
                mesh.GetVertexPosition(plVtHd[0]),
                mesh.GetVertexPosition(plVtHd[1]),
                mesh.GetVertexPosition(plVtHd[2]),
            };

            YsVec3 nom = (trVtPos[1] - trVtPos[0]) ^ (trVtPos[2] - trVtPos[0]);
            auto Lnom = nom.GetLength();
            if (epsilon <= Lnom)
            {
                nom /= Lnom;

                YsPlane pln(trVtPos[0], nom);
                YsVec3 itsc;
                if (YSOK == pln.GetIntersection(itsc, pos[0], pos[1] - pos[0]))
                {
                    if (0.0 <= (itsc - pos[0]) * (pos[1] - pos[0]))
                    {
                        auto side = YsCheckInsidePolygon3(itsc, 3, trVtPos);
                        if (YSINSIDE == side)
                        {
                            double dist = (pos[0] - itsc).GetSquareLength();
                            if (mesh.NullPolygon() == nearPlHd ||
                                dist < nearDist)
                            {
                                nearPlHd = plHd;
                                nearDist = dist;
                            }
                        }
                    }
                }
            }
        }
    }
    return nearPlHd;
}

PolygonalMesh::VertexHandle PickedVtHd(
    const PolygonalMesh& mesh,
    int mx, int my,
    const YsMatrix4x4& modelView, const YsMatrix4x4& projection)
{
    double nearDist = 0;
    PolygonalMesh::VertexHandle nearVtHd = mesh.NullVertex();

    int wid, hei;
    FsGetWindowSize(wid, hei);

    for (auto vtHd = mesh.FirstVertex();
        mesh.NullVertex() != vtHd;
        mesh.MoveToNext(vtHd))
    {
        auto pos = mesh.GetVertexPosition(vtHd);
        auto posv = projection * modelView * pos;
        auto posw = ViewPortToWindow(wid, hei, posv);

        auto dist = posv.z();
        if (-1.0 <= dist)
        {
            auto dx = posw.x() - mx;
            auto dy = posw.y() - my;
            if (-5 <= dx && dx <= 5 && -5 <= dy && dy <= 5)
            {
                if (mesh.NullVertex() == nearVtHd ||
                    dist < nearDist)
                {
                    nearVtHd = vtHd;
                    nearDist = dist;
                }
            }
        }
    }

    return nearVtHd;
}

void DrawSphere(const YsVec3& center, double radius, int slices, int stacks)
{
    for (int i = 0; i <= stacks; ++i)
    {
        double lat0 = YsPi * (-0.5 + double(i - 1) / stacks);
        double z0 = radius * sin(lat0);
        double zr0 = radius * cos(lat0);

        double lat1 = YsPi * (-0.5 + double(i) / stacks);
        double z1 = radius * sin(lat1);
        double zr1 = radius * cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j)
        {
            double lng = 2 * YsPi * double(j - 1) / slices;
            double x = cos(lng);
            double y = sin(lng);

            glVertex3d(center.x() + x * zr0, center.y() + y * zr0, center.z() + z0);
            glVertex3d(center.x() + x * zr1, center.y() + y * zr1, center.z() + z1);
        }
        glEnd();
    }
}

int main()
{ 
    PolygonalMesh mesh;
    nate n;

    // Brush tool state
    bool brushActive = false;
    bool brushModeActive = false;
    YsVec3 brushCenter = YsVec3::Origin();

    // Select primitive or input file path
    std::cout << "Select a primitive to start\n";
    std::cout << "    1. Sphere\n";
    std::cout << "    2. Cube\n";
    std::cout << "    3. Triangle\n";
    std::cout << "    4. User input file path\n";

    /////////////////////////////////////// Splashscreen Setup /////////////////
    Container <bool> splashscreen;
    bool showStartup = true;
    bool showHelpPage = false;
    bool primativeSphere = false;
    bool primativeCube = false;
    bool primativeTriangle = false;
    bool promptUserInput = false;
    // Splashscreen Dimensions
    splashscreen.Insert(false, "1. Sphere", "", &primativeSphere);
    splashscreen.Insert(false, "2. Plate", "", &primativeCube);
    splashscreen.Insert(false, "3. Cube", "", &primativeTriangle);
    splashscreen.Insert(false, "4. User Input File Path", "", &promptUserInput);
    auto helpPnHd = splashscreen.Insert(false, "? Show Help Guide", "", &showHelpPage);
    int splashscreenWid = winWid / 3;
    splashscreen.GetLongestNameLen();
    splashscreen.ResizePanel(20);
    splashscreen.setPadding(50);
    splashscreen.ResizeContainerWidth(splashscreenWid);
    splashscreen.setPos(winWid/2 - splashscreenWid/2, winHei/2 - splashscreen.height);
    splashscreen.AutoResize();


    bool selected = false;
    bool locked = false;
    int mouseXOnClick;
    Container<bool>::PanelHandle selectedPnHdSs;
    ////////////////////////////////////////////////////////////////////////////

    FsOpenWindow(0,0,winWid,winHei,1);
    glClearColor(0.9529, 0.9373, 0.925,1);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1, 1);
    for(;;)
    {
        FsPollDevice();
		auto key=FsInkey();
		if(FSKEY_ESC==key)
		{
			return 0;
		}

        int lb,mb,rb,mx,my;
		auto evt=FsGetMouseEvent(lb,mb,rb,mx,my);
        // Check if mouse over panel
        auto pickedPnHdSs = PickedPnlHdBoolSs(splashscreen,mx,my); 
        if(evt == 1){selected=true;}
        // Left mouse button up
        else if(evt == 2) 
        {
            selected=false;
            locked = false;
            if (showStartup && selectedPnHdSs.IsNotNull())
            {
                selectedPnHdSs.SetProperty(splashscreen.GetTempProperty(selectedPnHdSs));
                selectedPnHdSs.UpdateTrueProperty();
                selectedPnHdSs.MakeNull();
                if(!showHelpPage){showStartup = false; break;} // Close startup
            }
        }

        // Left button down - ON SPLASHSCREEN
        if(showStartup && !showHelpPage && true==pickedPnHdSs.IsNotNull() && selected && !locked) 
		{
            selectedPnHdSs = pickedPnHdSs;
            selectedPnHdSs.SetTempProperty(!splashscreen.GetProperty(selectedPnHdSs)); 
            locked = true;
		}
        if(showHelpPage && selected && !locked &&
            mx >= splashscreen.posX + splashscreen.width - splashscreen.panelHeight &&
            mx < splashscreen.posX + splashscreen.width &&
            my >= splashscreen.posY - splashscreen.panelHeight &&
            my < splashscreen.posY)
        {
            helpPnHd.SetTempProperty(false);
            helpPnHd.SetProperty(false);
            helpPnHd.UpdateTrueProperty();
        }

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        if(true==showStartup && !showHelpPage){RenderContainerSplash(splashscreen, pickedPnHdSs, selectedPnHdSs);}
        if(true==showHelpPage){RenderHelpScreen(splashscreen);}

        FsSwapBuffers();
    }

    ////////////////////////////// Start Screen Closed /////////////////////////
    for(;;)
    {
        if (primativeSphere)
        {
            FsChangeToProgramDir();
            if (!mesh.primitives(0))
            {
                return 1;
            }
            break;
        }
        else if (primativeCube)
        {
            FsChangeToProgramDir();
            if (!mesh.primitives(1))
            {
                return 1;
            }
            break;
        }
        else if (primativeTriangle)
        {
            FsChangeToProgramDir();
            if (!mesh.primitives(2))
            {
                return 1;
            }
            break;
        }
        else if (promptUserInput)
        {
            //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ignore anything left in cin
            if (!mesh.primitives(3))
            {
                return 1;
            }
            FsChangeToProgramDir();
            break;
        }
    }


    mesh.Stitch();

    // Create LaplacianEditor instance
    LaplacianEditor editor(&mesh);

    std::cout << "Number of Vertices " << mesh.GetNumVertices() << "\n";
    std::cout << "Number of Polygons " << mesh.GetNumPolygons() << "\n";

    auto vtxArrays = mesh.MakeVertexArrays();

    YsVec3 bbx[2];
    mesh.GetBoundingBox(bbx);
    auto l = (bbx[1] - bbx[0]).GetLength();

    std::cout << "bbx0: " << bbx[0].Txt() << "\n";
    std::cout << "bbx1: " << bbx[1].Txt() << "\n";

    // Brush parameters
    double brushStep = l/200;
    double brushRadius = l/5;

    YsVec3 poi = (bbx[0] + bbx[1]) / 2.0;;
    double cameraDist = l / (2.0 * tan(YsPi / 8));
    YsMatrix3x3 cameraOrientation;
    cameraOrientation.LoadIdentity();

    // Home position
    YsVec3 Home_bbx[2];
    Home_bbx[0] = bbx[0];
    Home_bbx[1] = bbx[1];
    auto Home_l = (Home_bbx[1] - Home_bbx[0]).GetLength();

    YsVec3 Home_poi = (Home_bbx[0] + Home_bbx[1]) / 2.0;
    double Home_cameraDist = Home_l / (2.0 * tan(YsPi / 8));
    YsMatrix3x3 Home_cameraOrientation = cameraOrientation;
    bool HomewardBound = false;

    // Mouse movement
    int prevMx = 0;
    int prevMy = 0;
    int prevRbState = 0;

    // Laplacian editing
    YsVec3 pickedLine[2] = { YsVec3::Origin(),YsVec3::Origin() };
    YsVec3 highlightPos = YsVec3::Origin();
    auto selectedVtHd = mesh.NullVertex();
    std::vector <PolygonalMesh::VertexHandle> connVtHd;
    bool vertexSelected = false;
    const double deformationLambda = 1000.0; // Higher lambda for stronger constraints
    double movement_step = l/200;

    // take the step size from user input
    // std::cout << "Enter movement step size: ";
    // std::cin >> movement_step;
    // if (movement_step <= 0) {
    //     std::cerr << "Invalid step size. Using default value of 1.0." << std::endl;
    //     movement_step = 1.0;
    // }
    // std::cout << "Movement step size: " << movement_step << std::endl;

    std::cout << "Controls:\n";
    std::cout << "  Click on a vertex to select it\n";
    std::cout << "  X,Y,Z - Move selected vertex along respective positive axis\n";
    std::cout << "  Ctrl + X,Y,Z - Move selected vertex in negative direction\n";
    std::cout << "  S - Toggle local brush smoothing mode\n";
    std::cout << "  Scroll Wheel - Adjust brush radius\n";
    std::cout << "  R - Reset to original positions\n";
    std::cout << "  C - Clear vertex selection\n";
    std::cout << "  Arrow keys - Rotate view\n";
    std::cout << "  G/M - Zoom in/out\n";
    std::cout << "  ESC - Exit\n";

    double deltaX = 0.0;
    double deltaY = 0.0;
    double deltaZ = 0.0;
    Container <double> container;
    container.Insert(deltaX, "X", "tooltip ex", &deltaX);
    container.Insert(deltaY, "Y", "tooltip ex", &deltaY);
    container.Insert(deltaZ, "Z", "tooltip ex", &deltaZ);
    container.Insert(movement_step, "Step Size", "tooltip ex", &movement_step);
    
    // Container Dimensions
    int containerWidth = winWid / 5;
    container.GetLongestNameLen();
    container.ResizePanel(20);
    container.ResizeContainerWidth(containerWidth);
    container.setPos(winWid - containerWidth - 2, container.panelHeight + 2);
    container.AutoResize();

    Container <bool> checkbox;
	checkbox.Insert(HomewardBound, "Homeward", "tooltip ex", &HomewardBound);
    checkbox.Insert(brushModeActive, "Smoothing", "tooltip ex", &brushModeActive);

    // Checkbox Dimensions
    int checkboxWid = winWid / 5;
    checkbox.GetLongestNameLen();
    checkbox.ResizePanel(20);
    checkbox.ResizeContainerWidth(checkboxWid);
    checkbox.setPos(winWid - checkboxWid - 2, container.height);
    checkbox.AutoResize();

    Container<double>::PanelHandle selectedPnHd;
    Container<bool>::PanelHandle selectedPnHdCb;
    selected = false;
    locked = false;

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1, 1);

    for (;;)
    {
        FsPollDevice();

        auto key = FsInkey();
        if (FSKEY_ESC == key)
        {
            break;
        }

        // Transform first, then allow picking and brushing on the new world matrices
        // rotate with mouse
        int lbState, mbState, rbState, mx2, my2;
        FsGetMouseState(lbState, mbState, rbState, mx2, my2);

        double diffX = (my2 - prevMy);
        double diffY = (mx2 - prevMx);

        if (rbState)
        {
            double sensitivity = YsPi / 800.0;
            cameraOrientation.RotateYZ(-diffX * sensitivity);
            cameraOrientation.RotateXZ(+diffY * sensitivity);
        }

        // pan with mouse
        if (mbState)
        {
            const double panSpeed = cameraDist * 0.002;
            YsVec3 camRight = cameraOrientation * YsVec3(1.0, 0.0, 0.0);
            YsVec3 camUp = cameraOrientation * YsVec3(0.0, 1.0, 0.0);
            poi += (camUp * (+diffX) + camRight * (-diffY)) * panSpeed;
        }
        prevMx = mx2;
        prevMy = my2;
        prevRbState = rbState;

        bool ctrlDown = (FsGetKeyState(FSKEY_CTRL) != 0);

        if (ctrlDown && FsGetKeyState(FSKEY_S) != 0)
        {
            mesh.SaveMesh();
            continue;
        }

        switch (key)
        {
        case FSKEY_X:
            if (mesh.NullVertex() != selectedVtHd && editor.hasSelection()) {
                if (ctrlDown) {
                    // Move in negative X
                    editor.moveSelectedX(-movement_step);
                }
                else {
                    // Move in positive X
                    editor.moveSelectedX(movement_step);
                }
                editor.applyDeformation(deformationLambda);
                vtxArrays = mesh.MakeVertexArrays();
            }
            break;

        case FSKEY_Y:
            if (mesh.NullVertex() != selectedVtHd && editor.hasSelection()) {
                if (ctrlDown) {
                    // Move in negative Y
                    editor.moveSelectedY(-movement_step);
                }
                else {
                    // Move in positive Y
                    editor.moveSelectedY(movement_step);
                }
                editor.applyDeformation(deformationLambda);
                vtxArrays = mesh.MakeVertexArrays();
            }
            break;

        case FSKEY_Z:
            if (mesh.NullVertex() != selectedVtHd && editor.hasSelection()) {
                if (ctrlDown) {
                    // Move in negative Z
                    editor.moveSelectedZ(-movement_step);
                }
                else {
                    // Move in positive Z
                    editor.moveSelectedZ(movement_step);
                }
                editor.applyDeformation(deformationLambda);
                vtxArrays = mesh.MakeVertexArrays();
            }
            break;

        case FSKEY_A:
            if (editor.hasSelection()) {
                editor.applyDeformation(deformationLambda);
                vtxArrays = mesh.MakeVertexArrays();
            }
            break;

        case FSKEY_R:
            editor.resetToOriginal();
            vtxArrays = mesh.MakeVertexArrays();
            break;

        case FSKEY_C:
            editor.clearSelection();
            selectedVtHd = mesh.NullVertex();
            connVtHd.clear();
            vertexSelected = false;
            vtxArrays = mesh.MakeVertexArrays();
            break;

        case FSKEY_H:
            HomewardBound = true;
            break;

        case FSKEY_S:
            // Toggle brush tool
            brushModeActive = !brushModeActive;
            if (brushModeActive)
            {
                std::cout << "Brush mode ON: click and hold on mesh to smooth.\n";
            }
            else
            {
                std::cout << "Brush mode OFF.\n";
            }
            break;

        case FSKEY_WHEELUP:
            if (ctrlDown && brushModeActive)
            {
                brushRadius += brushStep;
                std::cout << "Brush radius: " << brushRadius << "\n";
            }
            else
            {
                cameraDist /= 1.01;
                if (cameraDist < 1.0)
                {
                    cameraDist = 1.0;
                }
            }
            break;

        case FSKEY_WHEELDOWN:
            if (ctrlDown && brushModeActive)
            {
                brushRadius = std::max(0.01, brushRadius - brushStep);
                std::cout << "Brush radius: " << brushRadius << "\n";
            }
            else
            {
                cameraDist *= 1.01;
            }
            break;
        }

        // Compute aspect
        int wid, hei;
        FsGetWindowSize(wid, hei);
        double aspect = double(wid) / double(hei);

        YsMatrix4x4 modelView;
        {
            auto cameraInv = cameraOrientation;
            cameraInv.Invert();
            modelView.Translate(0, 0, -cameraDist);
            modelView *= cameraInv;
            modelView.Translate(-poi);
        }
        YsMatrix4x4 projection = MakePerspective(45.0, aspect, cameraDist * 0.01, cameraDist + l);

        // Brush intersection detection
        brushActive = false;
        int lb, mb, rb, mx, my;
        auto evt = FsGetMouseEvent(lb, mb, rb, mx, my);

        /////////////////////////// Select UI //////////////////////////////////
        auto pickedPnHdCb = PickedPnlHdBool(checkbox,mx,my); 
		auto pickedPnHd = PickedPnlHd(container,mx,my);
        ////////////////////////////////////////////////////////////////////////

        if (brushModeActive)
        {
            WindowCoordToLine(pickedLine, mx, my, modelView, projection);
            auto hoverPlHd = PickedPlHd(mesh, mx, my, modelView, projection);
            if (mesh.NullPolygon() != hoverPlHd)
            {
                auto plVtHd = mesh.GetPolygonVertex(hoverPlHd);
                YsVec3 v0 = mesh.GetVertexPosition(plVtHd[0]);
                YsVec3 v1 = mesh.GetVertexPosition(plVtHd[1]);
                YsVec3 v2 = mesh.GetVertexPosition(plVtHd[2]);
                YsPlane pln(v0, (v1 - v0) ^ (v2 - v0));
                YsVec3 itsc;
                if (YSOK == pln.GetIntersection(itsc, pickedLine[0], pickedLine[1] - pickedLine[0]))
                {
                    brushCenter = itsc;
                    brushActive = true;
                }
            }
        }


        ////////////////////////////// Mouse up ////////////////////////////////
        if(evt == 1){selected=true;}
        else if(evt == 2) // FSMOUSEEVENT_LBUTTONUP
        {
            //std::cout << "mouseup";
            selected=false;
            locked = false;
            if (selectedPnHd.IsNotNull())
            {
                selectedPnHd.SetProperty(container.GetTempProperty(selectedPnHd));
                selectedPnHd.UpdateTrueProperty();
                // Apply transformation
                if (mesh.NullVertex() != selectedVtHd && editor.hasSelection())
                {
                    editor.moveSelectedX(*container.panelVec[0]->propPtr);
                    editor.moveSelectedY(*container.panelVec[1]->propPtr);
                    editor.moveSelectedZ(*container.panelVec[2]->propPtr);
                    editor.applyDeformation(deformationLambda);
                    vtxArrays = mesh.MakeVertexArrays();
                    selectedPnHd.SetProperty(0);
                }
                
                // Reset transformation
                selectedPnHd.UpdateTrueProperty();
                selectedPnHd.MakeNull();
            }
            else if (selectedPnHdCb.IsNotNull())
            {
                selectedPnHdCb.SetProperty(checkbox.GetTempProperty(selectedPnHdCb));
                selectedPnHdCb.UpdateTrueProperty();
                selectedPnHdCb.MakeNull();
                std::cout << brushModeActive << std::endl;
            }
        }
        ////////////////////////////////////////////////////////////////////////

        /////////////////////////////////// UI /////////////////////////////////
        // Left button down - ON SLIDER
        if(true==pickedPnHd.IsNotNull() && selected && !locked) 
		{
            selectedPnHd = pickedPnHd;
            locked = true;
            mouseXOnClick = mx;
		}
        // Left button down - ON CHECKBOX
        if(true==pickedPnHdCb.IsNotNull() && selected && !locked) 
		{
            selectedPnHdCb = pickedPnHdCb;
            selectedPnHdCb.SetTempProperty(!checkbox.GetProperty(selectedPnHdCb)); 
            locked = true;
		}

        // drag left
        if(selectedPnHd.IsNotNull() && selected && mx+10 < mouseXOnClick)
        {
            float dx = (float)(mouseXOnClick - mx)/100;
            selectedPnHd.SetTempProperty(container.GetProperty(selectedPnHd) - dx); 
            //std::cout << propertyTemp << std::endl;
        }
        // drag right
        else if(selectedPnHd.IsNotNull() && selected && mx-10 > mouseXOnClick)
        {
            float dx = (float)(mx - mouseXOnClick)/100;
            selectedPnHd.SetTempProperty(container.GetProperty(selectedPnHd) + dx);
            //std::cout << propertyTemp << std::endl;
        }
        ////////////////////////////////////////////////////////////////////////

        if (evt == FSMOUSEEVENT_LBUTTONDOWN)
        {
            if (!brushModeActive && !locked) // if not in brush mode, select vertex to move
            {
                auto pickedVtHd = PickedVtHd(mesh, mx, my, modelView, projection);
                editor.clearSelection();
                if (mesh.NullVertex() != pickedVtHd) // if a vertex is selected
                {
                    highlightPos = mesh.GetVertexPosition(pickedVtHd);
                    selectedVtHd = pickedVtHd;
                    connVtHd = mesh.GetConnectedVertex(pickedVtHd);
                    editor.selectVertex(mesh.GetVertexPosition(pickedVtHd), 0.1);
                    editor.createAnchors(pickedVtHd, 5.0);
                    std::cout << "Vertex selected. Use X,Y,Z keys to move it.\n";
                    vtxArrays = mesh.MakeVertexArrays();
                }
                else // if no vertex is selected (clicked at a empty space)
                {
                    editor.clearSelection();
                    selectedVtHd = mesh.NullVertex();
                    connVtHd.clear();
                    vertexSelected = false;
                    vtxArrays = mesh.MakeVertexArrays();
                }
            }
        }

        // if in brush mode, continue to smooth if LB is held down
        if (brushModeActive)
        {
            int lbState, mbState, rbState, mx2, my2;
            FsGetMouseState(lbState, mbState, rbState, mx2, my2);
            if (lbState && brushActive)
            {
                Smoother localSmoother(&mesh);
                localSmoother.OptimizeWithinRadius(brushCenter, brushRadius, 10, 0.01);
                vtxArrays = mesh.MakeVertexArrays();
            }
        }

        // Check for arrow key rotation
        if (0 != FsGetKeyState(FSKEY_UP))
        {
            cameraOrientation.RotateYZ(YsPi / 60.0);
        }
        if (0 != FsGetKeyState(FSKEY_DOWN))
        {
            cameraOrientation.RotateYZ(-YsPi / 60.0);
        }
        if (0 != FsGetKeyState(FSKEY_LEFT))
        {
            cameraOrientation.RotateXZ(YsPi / 60.0);
        }
        if (0 != FsGetKeyState(FSKEY_RIGHT))
        {
            cameraOrientation.RotateXZ(-YsPi / 60.0);
        }

        // Set home
        if (HomewardBound == true)
        {
            l = Home_l;
            poi = Home_poi;
            cameraDist = Home_cameraDist;
            cameraOrientation = Home_cameraOrientation;
            HomewardBound = false;
        }

        //Rendering portion

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        double glProjection[16];
        projection.GetOpenGlCompatibleMatrix(glProjection);
        glMultMatrixd(glProjection);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        GLfloat lightDir[] = { 0,1 / sqrt(2.0f),1 / sqrt(2.0f),0 };
        glLightfv(GL_LIGHT0, GL_POSITION, lightDir);
        glEnable(GL_COLOR_MATERIAL);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        double glModelView[16];
        modelView.GetOpenGlCompatibleMatrix(glModelView);
        glMultMatrixd(glModelView);

        // Model rendering
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_FLOAT, 0, vtxArrays.col.data());
        glNormalPointer(GL_FLOAT, 0, vtxArrays.nom.data());
        glVertexPointer(3, GL_FLOAT, 0, vtxArrays.vtx.data());
        glDrawArrays(GL_TRIANGLES, 0, vtxArrays.vtx.size() / 3);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);

        // Wireframe rendering
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_FLOAT, 0, vtxArrays.edgeCol.data());
        glVertexPointer(3, GL_FLOAT, 0, vtxArrays.edgeVtx.data());
        glDrawArrays(GL_LINES, 0, vtxArrays.edgeVtx.size() / 3);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);

        // Draw brush sphere
        if (brushActive && brushModeActive)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4d(0.0, 0.0, 1.0, 0.3);
            DrawSphere(brushCenter, brushRadius, 16, 16);
            glDisable(GL_BLEND);
        }

        // Draw selected vertex and connected vertices
        if (mesh.NullVertex() != selectedVtHd) {
            glPointSize(8);
            glBegin(GL_POINTS);
            glColor3f(1, 0, 0); // Red for selected vertex
            glVertex3dv(mesh.GetVertexPosition(selectedVtHd));
            glEnd();

            glPointSize(5);
            glBegin(GL_POINTS);
            glColor3f(0, 1, 0); // Green for connected vertices
            for (auto vtHd : connVtHd) {
                glVertex3dv(mesh.GetVertexPosition(vtHd));
            }
            glEnd();

            n.arrow(Home_l, mesh.GetVertexPosition(selectedVtHd), 0);
            n.arrow(Home_l, mesh.GetVertexPosition(selectedVtHd), 1);
            n.arrow(Home_l, mesh.GetVertexPosition(selectedVtHd), 2);

            // Draw anchored vertices if any
            // glPointSize(3);
            // glBegin(GL_POINTS);
            // glColor3f(0, 0, 1); // Blue for anchored vertices
            // for (auto anchor : editor.getAnchorVertices()) {
            //     for (auto vtHd = mesh.FirstVertex(); vtHd != mesh.NullVertex(); mesh.MoveToNext(vtHd)) {
            //         if (mesh.GetSearchKey(vtHd) == anchor) {
            //             glVertex3dv(mesh.GetVertexPosition(vtHd));
            //             break;
            //         }
            //     }
            // }
            // glEnd();
        }
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glOrtho(0, winWid, winHei, 0, -1, 1); // Set up orthographic projection

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        //////////////////////////// UI //////////////////////////////////////
        RenderContainer(container, pickedPnHd, selectedPnHd);
        RenderContainerCheckbox(checkbox, pickedPnHdCb, selectedPnHdCb);
        //////////////////////////////////////////////////////////////////////

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, wid, hei, 0, -1, 1);

        glViewport(0, 0, wid, hei);

        FsSwapBuffers();

        FsSleep(5);
    }
    return 0;
}
