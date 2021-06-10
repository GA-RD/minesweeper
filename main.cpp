#include <windows.h>
#include <ctime>
#include <gl/gl.h>

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

#define mapH 10
#define mapW 10

class TCell{
public:
    bool mine;
    bool flag;
    bool open;
    int cntAround;
};

TCell map[mapW][mapH];
int mines;
int closedCell;
bool failed;

bool IsCellInMap(int x, int y){
    return (x>=0) && (y>=0) && (x<mapW) && (y<mapH);
}

void ScreeToOpenGL(HWND hwnd, int x, int y, float *ox, float *oy){
    RECT rct;
    GetClientRect(hwnd, &rct);
    *ox = x/float(rct.right)*mapW;
    *oy = mapH-y/float(rct.bottom)*mapH;
}

void Game_New(){

    srand(time(NULL));
    memset(map,0,sizeof(map));

    int mines = 20;
    closedCell = mapW*mapH;
    failed=false;

    for(int i=0;i<mines;++i){
        int x = rand() % mapW;
        int y = rand() % mapH;
        if(map[x][y].mine) i--;
        else {
            map[x][y].mine = true;

            for(int dx = -1; dx<2; ++dx)
                for(int dy = -1; dy<2; ++dy)
                   if(IsCellInMap(x+dx,y+dy))
                      map[x+dx][y+dy].cntAround++;
        }
    }
}

void Line(float x1,float y1,float x2,float y2){
        glVertex2f(x1,y1);
        glVertex2f(x2,y2);
    }

void ShowCount(int a){
    glLineWidth(4);
    glColor3f(1.000, 0.271, 0.000);
    glBegin(GL_LINES);
       if((a!=1) && (a!=4)) Line(0.3,0.85,0.7,0.85);
       if((a!=0) && (a!=1) && (a!=7)) Line(0.3,0.5,0.7,0.5);
       if((a!=4) && (a!=1) && (a!=7)) Line(0.3,0.15,0.7,0.15);

       if((a!=5) && (a!=6)) Line(0.7,0.5,0.7,0.85);
       if(a!=2) Line(0.7,0.5,0.7,0.15);

       if((a!=2) && (a!=1) && (a!=3) && (a!=7)) Line(0.3,0.5,0.3,0.85);
       if((a==2) || (a==0) || (a==6) || (a==8)) Line(0.3,0.5,0.3,0.15);
    glEnd();
}

void ShowMine(){
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0,0,0);
       glVertex2f(0.3, 0.3);
       glVertex2f(0.7, 0.3);
       glVertex2f(0.7, 0.7);
       glVertex2f(0.3, 0.7);
    glEnd();
}

void ShowField(){
    glBegin(GL_TRIANGLE_STRIP);
       glColor3f(1.000, 0.271, 0.000); glVertex2f(0, 1);
       glColor3f(0.545, 0.000, 0.000); glVertex2f(1, 1);
       glColor3f(1.000, 0.549, 0.000); glVertex2f(0, 0);
       glColor3f(1.000, 0.271, 0.000); glVertex2f(1, 0);
    glEnd();
}

void ShowFieldOpen(){
    glBegin(GL_TRIANGLE_STRIP);
       glColor3f(0.000, 0.392, 0.000); glVertex2f(0, 1);
       glColor3f(0.000, 0.192, 0.000); glVertex2f(1, 1);
       glColor3f(0.000, 0.502, 0.000); glVertex2f(0, 0);
       glColor3f(0.000, 0.392, 0.000); glVertex2f(1, 0);
    glEnd();
}

void ShowFlag(){
    glBegin(GL_TRIANGLES);
       glColor3f(0.545, 0.000, 0.000);
       glVertex2f(0.25, 0.75);
       glVertex2f(0.85, 0.5);
       glVertex2f(0.25, 0.25);
    glEnd();

    glLineWidth(5);
    glBegin(GL_LINES);
      glColor3f(0,0,0);
      glVertex2f(0.25, 0.75);
      glVertex2f(0.25, 0);
    glEnd();
}

void OpenFields(int x, int y){
    if((!IsCellInMap(x,y)) || map[x][y].open) return;
    map[x][y].open=true;
    closedCell--;

    if(map[x][y].cntAround == 0)
        for(int dx = -1; dx < 2; ++dx)
            for(int dy = -1; dy<2; ++dy)
               OpenFields(x+dx, y+dy);
    if(map[x][y].mine){
        failed = true;
        for(int j = 0; j<mapH;++j)
            for(int i = 0; i<mapW;++i)
                map[i][j].open = true;
    }
}

void Game_Show(){
    glLoadIdentity();
    glScalef(2.0/mapW,2.0/mapH,1);
    glTranslatef(-mapW*0.5,-mapH*0.5,0);

    for(int j = 0;  j < mapH; ++j){
        for(int i = 0; i < mapW; ++i){

            glPushMatrix();
            glTranslatef(i,j,0);

            if(map[i][j].open){
               ShowFieldOpen();
               if(map[i][j].mine) ShowMine();
               else if (map[i][j].cntAround > 0)
                     ShowCount(map[i][j].cntAround);
             } else {
                 ShowField();
                 if(map[i][j].flag) ShowFlag();
            }
            glPopMatrix();
        }
    }
}


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          700,
                          700,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

    Game_New();

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            if(mines == closedCell) Game_New();

            Game_Show();

            SwapBuffers(hDC);

            Sleep (1);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_LBUTTONDOWN:
            {
                if(failed) Game_New();
                else{
                POINTFLOAT pf;
                ScreeToOpenGL(hwnd, LOWORD(lParam), HIWORD(lParam), &pf.x, &pf.y);
                int x = int(pf.x);
                int y = int(pf.y);
                if(IsCellInMap(x,y) && !(map[x][y].flag))
                    OpenFields(x,y);
                }
            } break;

        case WM_RBUTTONDOWN:
            {
                POINTFLOAT pf;
                ScreeToOpenGL(hwnd, LOWORD(lParam), HIWORD(lParam), &pf.x, &pf.y);
                int x = int(pf.x);
                int y = int(pf.y);
                if(IsCellInMap(x,y)) map[x][y].flag = !map[x][y].flag;
            } break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

