#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <string>
#include "fssimplewindow.h"
#include "ysglfontdata.h"



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
        void UpdateTrueProperty()
        {
            Property *temp = ptr->propPtr;
            *temp = ptr->property;
        }
        void SetAddress(Property * address)
        {
            newPanel->propPtr = address;
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
Container<float>::PanelHandle PickedPnlHd(
    Container<float> &cont,
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
        Container<float>::PanelHandle nullHd;
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
void RenderContainer(const Container <float> &cont,
                     typename Container<float>::PanelHandle highlightHover,
                     typename Container<float>::PanelHandle highlightSelect)
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

    // fit in pname
    glColor3f(0.35, 0.35, 0.35);
    glBegin(GL_TRIANGLE_FAN);
    int nameLen = cont.longestname;
    glVertex2i(cont.posX - 6*nameLen - cont.padding, cont.posY);
    glVertex2i(cont.posX, cont.posY);
    glVertex2i(cont.posX, cont.posY + cont.height);
    glVertex2i(cont.posX - 6*nameLen - cont.padding, cont.posY + cont.height);
    glEnd();
    
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
    sprintf(splash, "@ Blunder");
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
    
    // Instructions
    glColor3f(1,1,1);
    char splash[256];
    sprintf(splash, "? Instructions - Controls");
    glRasterPos2i(cont.posX + cont.panelHeight/2, cont.posY - 5);
    YsGlDrawFontBitmap6x8(splash);

    // X
    glColor3f(1,1,1);
    char xbutton[256];
    sprintf(xbutton, "x");
    glRasterPos2i(cont.posX + cont.width - cont.panelHeight/2 - 3, cont.posY - 5);
    YsGlDrawFontBitmap6x8(xbutton);
}

int main(void)
{
    Container <float> container;
    float ImportantValue = 10;
    // To add function to panel do container.Insert(Value, PropName, tooltip, Value address)
    // mesh.GetVertexPosition(selectedVtHd)
 
    container.Insert(ImportantValue, "X", "tooltip ex", &ImportantValue);
    container.Insert(ImportantValue, "Y", "tooltip ex", &ImportantValue);
    container.Insert(ImportantValue, "Z", "tooltip ex", &ImportantValue);
    container.Insert(ImportantValue, "Step Size", "tooltip ex", &ImportantValue);
	
    // Container Dimensions
    int containerWidth = winWid / 5;
    container.GetLongestNameLen();
    container.ResizePanel(20);
    container.ResizeContainerWidth(containerWidth);
    container.setPos(winWid - containerWidth, 0);
    container.AutoResize();

    Container <bool> checkbox;
    bool ImportantToggle = false;
	checkbox.Insert(false, "Wireframe", "tooltip ex", &ImportantToggle);
    checkbox.Insert(false, "Smoothing", "tooltip ex", &ImportantToggle);

    // Checkbox Dimensions
    int checkboxWid = winWid / 5;
    checkbox.GetLongestNameLen();
    checkbox.ResizePanel(20);
    checkbox.ResizeContainerWidth(checkboxWid);
    checkbox.setPos(winWid - checkboxWid, container.height);
    checkbox.AutoResize();

    Container <bool> splashscreen;
    bool showStartup = true;
    bool showHelpPage = false;
    bool primativeSphere = false;
    bool primativeCube = false;
    bool primativeTriangle = false;
    bool promptUserInput = false;
    // Splashscreen Dimensions
    splashscreen.Insert(false, "1. Sphere", "", &primativeSphere);
    splashscreen.Insert(false, "2. Cube", "", &primativeCube);
    splashscreen.Insert(false, "3. Triangle", "", &primativeTriangle);
    splashscreen.Insert(false, "4. User Input File Path", "", &promptUserInput);
    splashscreen.Insert(false, "? Show Help Guide", "", &showHelpPage);
    int splashscreenWid = winWid / 3;
    splashscreen.GetLongestNameLen();
    splashscreen.ResizePanel(20);
    splashscreen.setPadding(50);
    splashscreen.ResizeContainerWidth(splashscreenWid);
    splashscreen.setPos(winWid/2 - splashscreenWid, winHei/2 - splashscreen.height);
    splashscreen.AutoResize();


    bool selected = false;
    bool locked = false;
    int mouseXOnClick;
    Container<float>::PanelHandle selectedPnHd;
    Container<bool>::PanelHandle selectedPnHdCb;
    Container<bool>::PanelHandle selectedPnHdSs;
    FsOpenWindow(0,0,winWid,winHei,1);
    for(;;)
    {
        FsPollDevice();
		auto key=FsInkey();
		if(FSKEY_ESC==key)
		{
			break;
		}

        int lb,mb,rb,mx,my;
		auto evt=FsGetMouseEvent(lb,mb,rb,mx,my);
        // Check if mouse over panel
        auto pickedPnHdSs = PickedPnlHdBoolSs(splashscreen,mx,my); 
        auto pickedPnHdCb = PickedPnlHdBool(checkbox,mx,my); 
		auto pickedPnHd = PickedPnlHd(container,mx,my);
        if(evt == 1){selected=true;}
        // Left mouse button up
        else if(evt == 2) 
        {
            selected=false;
            locked = false;
            if (selectedPnHd.IsNotNull())
            {
                selectedPnHd.SetProperty(container.GetTempProperty(selectedPnHd));
                selectedPnHd.UpdateTrueProperty();
                selectedPnHd.MakeNull();
            }
            else if (selectedPnHdCb.IsNotNull())
            {
                selectedPnHdCb.SetProperty(checkbox.GetTempProperty(selectedPnHdCb));
                selectedPnHdCb.UpdateTrueProperty();
                selectedPnHdCb.MakeNull();
            }
            else if (showStartup && selectedPnHdSs.IsNotNull())
            {
                selectedPnHdSs.SetProperty(splashscreen.GetTempProperty(selectedPnHdSs));
                selectedPnHdSs.UpdateTrueProperty();
                selectedPnHdSs.MakeNull();
                if(!showHelpPage){showStartup = false;} // Close startup
            }

            std::cout << ImportantToggle << std::endl;
            std::cout << ImportantValue << std::endl;
        }

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
        // Left button down - ON SPLASHSCREEN
        if(showStartup && !showHelpPage && true==pickedPnHdSs.IsNotNull() && selected && !locked) 
		{
            selectedPnHdSs = pickedPnHdSs;
            selectedPnHdSs.SetTempProperty(!splashscreen.GetProperty(selectedPnHdSs)); 
            locked = true;
		}
        if(showHelpPage && evt == 2 &&
            mx >= splashscreen.posX + splashscreen.width - splashscreen.panelHeight &&
            mx < splashscreen.posX + splashscreen.width &&
            my >= splashscreen.posY - splashscreen.panelHeight &&
            my < splashscreen.posY)
        {
            showHelpPage = false;   
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

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        RenderContainer(container, pickedPnHd, selectedPnHd);
        RenderContainerCheckbox(checkbox, pickedPnHdCb, selectedPnHdCb);
        if(true==showStartup && !showHelpPage){RenderContainerSplash(splashscreen, pickedPnHdSs, selectedPnHdSs);}
        if(true==showHelpPage){RenderHelpScreen(splashscreen);}

        FsSwapBuffers();
    }

    return 0;
}
