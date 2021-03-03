#include "DirectXTemplatePCH.h"
#include <filesystem>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <unordered_map>
#include <algorithm>    // std::find
#include <string>
#include <WICTextureLoader.h> //https://github.com/microsoft/DirectXTK - used nuget to install the directX tool kit
#include <conio.h>
#include <future>

using namespace DirectX; // All of the functionsand types defined in the DirectXMath API are wrapped in the DirectX namespace

struct VertexPosColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    //XMFLOAT3 Color;
    XMFLOAT2 Tex;
    int TexLink;
};

SHORT a = 0;
bool launchedOPAModel = FALSE;
const LONG g_WindowWidth = 360;
const LONG g_WindowHeight = 360;
LPCSTR g_WindowClassName = ("DirectXWindowClass"); //window name
LPCSTR g_WindowName = "DirectX Template"; //
HWND g_WindowHandle = 0; //instance of window

const BOOL g_EnableVSync = TRUE; //zero screen tear

// Direct3D device and swap chain.
ID3D11Device* g_d3dDevice = nullptr; // allocates gpu resources
ID3D11DeviceContext* g_d3dDeviceContext = nullptr; //config rendering pipeline and draw geometry
IDXGISwapChain* g_d3dSwapChain = nullptr; //stores buffers that are used for rendering data - also determines how buffers are swapperd when the rendered image should be presented

// Render target view for the back buffer of the swap chain.
ID3D11RenderTargetView* g_d3dRenderTargetView = nullptr; //color of rendered image
// Depth/stencil view for use as a depth buffer.
ID3D11DepthStencilView* g_d3dDepthStencilView = nullptr; //depth and such of rendered image
// A texture to associate to the depth stencil view.
ID3D11Texture2D* g_d3dDepthStencilBuffer = nullptr;  //2d texture object that will be used to store the depth values  - stops overdrawn objects 

// Define the functionality of the depth/stencil stages.
ID3D11DepthStencilState* g_d3dDepthStencilState = nullptr; //store depth and stencil state
// Define the functionality of the rasterizer stage.
ID3D11RasterizerState* g_d3dRasterizerState = nullptr; //variable will be used to store rasterizer state
D3D11_VIEWPORT g_Viewport = { 0 }; //size of view port rectangle - multipul allows split screen multi-player


///this stuff is specific to test program - help from 3dgep.com

// Vertex buffer data
ID3D11InputLayout* g_d3dInputLayout; // order and type of data that vertex shader uses

ID3D11Buffer* g_d3dVertexBuffer;
std::vector<ID3D11Buffer*> g_d3dVertexBufferV; //store vertex data - color vertex


ID3D11Buffer* g_d3dIndexBuffer; // store index list - list of indices into the vertex buffer
std::vector<ID3D11Buffer*> g_d3dIndexBufferV;

// Shader data
ID3D11VertexShader* g_d3dVertexShader = nullptr; //vertex shader info - I am very lazy for offsets, so I'll us
ID3D11PixelShader* g_d3dPixelShader = nullptr; // pixel shader info
//buffer objects to hold/store data below
///////////////////Here we declare three constant buffers.Constant buffers are used to store shader variables that remain constant during current draw call.

std::vector<ID3D11ShaderResourceView*> textureV;
std::vector<ID3D11Resource*> textureT;

std::vector<ID3D11SamplerState*> sampler;

std::future<void> inputRelatedThread; //yes, I am making a thread DEDICATED to input checking - I really am fine with this

float eyePosX = 0;
float eyePosY = 0;
float eyePosZ = 1;

float modx = 0;

float mody = 0;

float modz = 0;

void makeSampler() {
    assert(g_d3dDevice);
    ID3D11SamplerState* tmpSample = nullptr;
    D3D11_SAMPLER_DESC tmpSampleDesc;

        tmpSampleDesc.Filter = D3D11_FILTER{ D3D11_FILTER_ANISOTROPIC };
        tmpSampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_MODE{ D3D11_TEXTURE_ADDRESS_WRAP };
        tmpSampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_MODE{ D3D11_TEXTURE_ADDRESS_WRAP };
        tmpSampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_MODE{ D3D11_TEXTURE_ADDRESS_WRAP };
        tmpSampleDesc.MipLODBias = 0;
        tmpSampleDesc.MaxAnisotropy = 8;
        tmpSampleDesc.ComparisonFunc = D3D11_COMPARISON_FUNC{ D3D11_COMPARISON_LESS };
    //tmpSampleDesc.BorderColor[0] =
        tmpSampleDesc.MinLOD = 1;
        tmpSampleDesc.MaxLOD = D3D11_FLOAT32_MAX;

    g_d3dDevice->CreateSamplerState(&tmpSampleDesc, &tmpSample);

    sampler.push_back(tmpSample);
}

void loadTex(std::wstring filePath) {
    //get usable shader feature level - YES I am running this everytime a texture is loaded, for now I may do some funny GPU stuff as a test, and I don't think the shader levels are equally supported among the gpu's I will swap with
    
    assert(g_d3dDevice);
    assert(g_d3dDeviceContext);
    
    ID3D11ShaderResourceView* trash_memV = nullptr; //fine to have a tmp
    ID3D11Resource* trash_memT = nullptr;



  //  std::vector<ID3D11ShaderResourceView*>& vecRefV = *textureV;
  //  std::vector<ID3D11Resource*>& vecRefT = *textureT; // I will let this get auto cleaned, really does not matter - derefrence to vector is cheap, so I'm not bothered to do this

    CreateWICTextureFromFile(g_d3dDevice, g_d3dDeviceContext, filePath.c_str(), &trash_memT, &trash_memV, GetFileSize(CreateFileA(LPCSTR(filePath.c_str()), GENERIC_READ,NULL,NULL,NULL,NULL, NULL), NULL)); //may need to change my size aquiring
    //I got the buffer resource since its cool for me to use

    textureV.push_back((trash_memV));
    textureT.push_back((trash_memT));


}

/*

constant buffer that stores the projection 
matrix of the camera and 
this shader variable only needs to be 
updated when the camera’s 
projection matrix is modified

*/

// Shader resources
enum ConstantBuffer
{
    CB_Application,
    CB_Frame,
    CB_Object,
    NumConstantBuffers
};

ID3D11Buffer* g_d3dConstantBuffers[NumConstantBuffers];

//

/*


    Application: The application level constant buffer stores variables that rarely change. 
    The contents of this constant buffer are being updated once during application startup 
    and perhaps are not updated again. An example of an application level shader variable is 
    the camera’s projection matrix. Usually the projection matrix is initialized once when the 
    render window is created and only needs to be updated if the dimensions of the render window are changed 
    (for example, if the window is resized).
    
    Frame: The frame level constant buffer stores variables that change each frame. An example of a frame level 
    shader variable would be the camera’s view matrix which changes whenever the camera moves. This variable only 
    needs to be updated once at the beginning of the render function and generally stays the same for all objects 
    rendered that frame.
    
    Object: The object level constant buffer stores variables that are different for every object being rendered. 
    An example of an object level shader variable is the object’s world matrix. Since each object in the scene 
    will probably have a different world matrix this shader variable needs to be updated for every separate draw call.


*/

// 
XMMATRIX g_WorldMatrix; //4x4 matrix to store cube of scene [later will not be cube]
XMMATRIX g_ViewMatrix; //once per frame stores a new camrea view matrix that transforms obj
XMMATRIX g_ProjectionMatrix; //store projection matrix of camrea; transform objects vertices
//


// Vertex data for cube.


//std::vector<VertexPosColor> g_Vertices;


std::vector<VertexPosColor> g_Vertices; /*=
{
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0 - indices, first is position, second is color
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
    { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
    { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
    { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
};
*/

//std::vector<UINT> g_Indicies; //texCoord

std::vector<XMFLOAT2> g_Normal; //texCoord

std::vector<UINT> g_Indicies; /*= //orginisation of indices to fomulate cube
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};*/
//

/////////////////////////////////////////

std::string GetLatestProfileBasic()
{
    assert(g_d3dDevice);

    // Query the current feature level:
    D3D_FEATURE_LEVEL featureLevel = g_d3dDevice->GetFeatureLevel(); //get usable shader feature level

    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_1:
    case D3D_FEATURE_LEVEL_11_0:
    {
        return "fx_5_0"; //11.1 and 11.0 are 5.0
    }
    break;
    case D3D_FEATURE_LEVEL_10_1:
    {
        return "fx_4_1";
    }
    break;
    case D3D_FEATURE_LEVEL_10_0:
    {
        return "fx_4_0";
    }
    break;
    case D3D_FEATURE_LEVEL_9_3:
    {
        return "fx_4_0_level_9_3";
    }
    break;
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
    {
        return "fx_4_0_level_9_1";
    }
    break;
    } // switch( featureLevel )

    return "";
}

void loadModel(std::string path) {
    //g_Vertices
    //VertexPosColor

    tinyobj::attrib_t attrib;// clear these values each read by reinitializing because I don't know if free and such works/how they work for these - rather do this for now since dead memory is a non-issue if it happens to exist 
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        throw std::runtime_error(warn + err);
    }


    //XMFLOAT2 tmpa;
    VertexPosColor tmpV;

    int i = 0;

    std::map<std::tuple<float, float, float>, int> b;

    for (const auto& shape : shapes) { // combine all faces into 1 model --> they are normally "seperate"
        i = 0;
        for (const auto& index : shape.mesh.indices) { //iterate through each indice in mesh


            tmpV.Position = {
                attrib.vertices[3 * index.vertex_index + 0], //since these are floats I multiply by 3?
                attrib.vertices[3 * index.vertex_index + 1] ,
                attrib.vertices[3 * index.vertex_index + 2] // move obj pos
            };

            tmpV.Tex = { // flip image vertically to fix model texture
                attrib.texcoords[2 * index.texcoord_index + 0], // this vector does not work if I do not set texcoods when making the obj
                /*1.0f - */attrib.texcoords[2 * index.texcoord_index + 1]
            };

/*            tmpV.Color = {

                attrib.colors[3 * index.vertex_index + 0], //since these are floats I multiply by 3?
                attrib.colors[3 * index.vertex_index + 1] ,
                attrib.colors[3 * index.vertex_index + 2] // move color pos

            };
            */
            tmpV.Normal = {

                attrib.normals[3 * index.vertex_index + 0], //since these are floats I multiply by 3?
                attrib.normals[3 * index.vertex_index + 1],
                attrib.normals[3 * index.vertex_index + 2]
                //attrib.normals[3 * index.vertex_index + 2] // move color pos

            };

            tmpV.TexLink = textureV.size();
            // count number of times a value appears in verticies array to make sure that it does not appear twice in the end result
            //uniqueVertices[tmpb] = static_cast<uint32_t>(g_Vertices.size());

            //g_Vertices.push_back(tmpV);
            //g_TexC.push_back(tmpT);

//any equal vertex pos must be turned into same indice
                //count = std::count(a.begin(), a.end(), tmpV.Position);



            if (b.count((std::make_tuple(tmpV.Position.x, tmpV.Position.y, tmpV.Position.z))) == 0) { //filter out duplicate verticies
                b[std::make_tuple(tmpV.Position.x, tmpV.Position.y, tmpV.Position.z)] = g_Vertices.size();
                g_Vertices.push_back(tmpV);
                //i++;
            }
            g_Indicies.push_back(b[std::make_tuple(tmpV.Position.x, tmpV.Position.y, tmpV.Position.z)]);

            //g_Vertices.push_back(tmpV);

                //g_Indicies.push_back(b[std::make_tuple(tmpV.Position.x, tmpV.Position.y, tmpV.Position.z)]); //push back point i associated with count -
       //     tempIndice.push_back(uniqueVertices[vertex]); //add to unique vertex unordered map
        }

    }

    // g_Indicies
    
}

// Forward declarations.
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam); //handle mouse, keyboard, and window events that are sent to application window

template< class ShaderClass >
ShaderClass* LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& profile); //template to load and compile shader

bool LoadContent(); //load resources for geometry
void UnloadContent(); //TODO: explain

void Update(float deltaTime); //update logic
void Render(); //render scene
void Cleanup(); //clean dx resources
//


/**
 * Initialize the application window.
 */
int InitApplication(HINSTANCE hInstance, int cmdShow)
{
    CoInitialize(NULL);
    WNDCLASSEX wndClass = { 0 };
    wndClass.cbSize = sizeof(WNDCLASSEX); //size in bytes of WNDCLASSX 
    wndClass.style = CS_HREDRAW | CS_VREDRAW; //if moveing window, adjust width, then height (HRe is width, VRe is height)
    wndClass.lpfnWndProc = &WndProc;//pointer to thing that handdles window event (WndProc is a var)
    wndClass.hInstance = hInstance;// handle to thing that owns window class
    //wndClass.hIcon = //class icon for top left icon thingy
    wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);// custom cursor
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);//class background brush to draw (can also be static color)
    wndClass.lpszMenuName = nullptr;// ..nothing means that window has no default menu - else we can make a menu
    wndClass.lpszClassName = CA2W(g_WindowClassName);//identify class name to make instance based off of

    if (!RegisterClassEx(&wndClass)) //make window class
    {
        return -1;
    }
//


    RECT windowRect = { 0, 0, g_WindowWidth, g_WindowHeight }; //client area 
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE); //adjust client area for overlapped window style in https://docs.microsoft.com/en-ca/windows/win32/winmsg/window-styles?redirectedfrom=MSDN

    g_WindowHandle = CreateWindowA(g_WindowClassName, //name of window to use as template - used to match registerClass
        g_WindowName, //window name for display in title bar
        WS_OVERLAPPEDWINDOW, //style of window - https://docs.microsoft.com/en-ca/windows/win32/winmsg/window-styles?redirectedfrom=MSDN
        CW_USEDEFAULT,  // initial x pos (left to right)
        CW_USEDEFAULT, // initial y pos (top to bottom)
        windowRect.right - windowRect.left, //width of window
        windowRect.bottom - windowRect.top, //height of window
        nullptr, //parent window handle; could be fun to make a child window for a second window split screen rendering for game [heh]
        nullptr, //window class template - menu
        hInstance, //handle to instance associated with window
        nullptr); // CREATESTRUCT structure - pointed to by the lParam param of the WM_CREATE message

    if (!g_WindowHandle)
    {
        return -1;
    }

    ShowWindow(g_WindowHandle, cmdShow);
    UpdateWindow(g_WindowHandle);

    return 0;
}


LRESULT CALLBACK WindowProc(
    _In_  HWND hwnd, //handle for window
    _In_  UINT uMsg, // event message
    _In_  WPARAM wParam, //some message stuff based on UMsg
    _In_  LPARAM lParam // wParam, but not wide text
);

//

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT paintStruct;
    HDC hDC;

    switch (message)
    {
    case WM_PAINT: // erase the window’s background, so paint hwnd over
    {
        hDC = BeginPaint(hwnd, &paintStruct);
        EndPaint(hwnd, &paintStruct);
    }
    break;
    case WM_DESTROY: //..kill program message
    {
        PostQuitMessage(0);
    }
    break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}



//Run function which will continue to execute until the user decides to quit

/**
 * The main application loop.
 */

void dupModelA() { //dup last gotten model
    
    //I am being very verbose here non purpose, rather than reusing a function I want control

    loadModel("./model/2.obj");
    loadTex(L"./tex/2.png");
    
    D3D11_BUFFER_DESC vertexBufferDesc; //describe buffer we will make
    ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));

    vertexBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_VERTEX_BUFFER; //how to bind buffer 

    vertexBufferDesc.ByteWidth = sizeof(VertexPosColor) * (g_Vertices.size()); //size of buffer --> make it the size of verticies*vertexPosColor [since vertex will have pos and color
    vertexBufferDesc.CPUAccessFlags = 0; // 0 means no CPU acsess

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT; //resource flag - 0 means none
    vertexBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

    D3D11_SUBRESOURCE_DATA resourceData; //data for buffer
    ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    resourceData.pSysMem = &g_Vertices[0]; //Vertex data for sub source

    g_d3dDevice->CreateBuffer(&vertexBufferDesc, &resourceData, &g_d3dVertexBuffer); //create buffer, using data settings struct, struct of data, and vertex buffer output - this is also used to create other buffer styles
    
    g_d3dVertexBufferV.push_back(g_d3dVertexBuffer);


    D3D11_BUFFER_DESC indexBufferDesc; //buffer obj
    ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC)); //alloc

    indexBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_INDEX_BUFFER; //type of buffer m8 - same logic as vertex
    indexBufferDesc.ByteWidth = sizeof(UINT) * (g_Indicies.size());
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

    resourceData.pSysMem = &g_Indicies[0]; //indice data for sub source

    g_d3dDevice->CreateBuffer(&indexBufferDesc, &resourceData, &g_d3dIndexBuffer); //make buffer

    g_d3dIndexBufferV.push_back(g_d3dIndexBuffer);



    OutputDebugStringA("\n");
}

void keyInputAsync() { //launch and leave it forever running in a spin lock

    while (true) {
        
        if (GetKeyState(VK_UP) & 0x8000) { //multipul keys can be pressed at once... do not care much

            mody = 0.1;//up

        }
        
        if (GetKeyState(VK_DOWN) & 0x8000) { //multipul keys can be pressed at once... do not care much

            mody = -0.1;//up

        }

        if (GetKeyState(VK_LEFT) & 0x8000) { //multipul keys can be pressed at once... do not care much

            modx = -0.1;//up

        }

        if (GetKeyState(VK_RIGHT) & 0x8000) { //multipul keys can be pressed at once... do not care much

            modx = 0.1;//up

        }

        if (GetKeyState(0x57) & 0x8000) { //multipul keys can be pressed at once... do not care much

            modz = -0.1;//up

        }

        if (GetKeyState(0x53) & 0x8000) { //multipul keys can be pressed at once... do not care much

            modz = 0.1;//up

        }

    }

}

int Run()
{
    MSG msg = { 0 };

    static DWORD previousTime = timeGetTime();
    static const float targetFramerate = 30.0f;
    static const float maxTimeStep = 1.0f / targetFramerate;
    
    //OutputDebugStringA("a");
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) // retrieved message should remove window message from queue - return false if nothing
        {
            TranslateMessage(&msg); //translate mesage to character message
            DispatchMessage(&msg); //dispatch message to appropriate window msg retrival feature -->  it will go to return func at end for returning to wParam which stores message
        }
        else //if no message
        {//some time thing
            DWORD currentTime = timeGetTime();
            float deltaTime = (currentTime - previousTime) / 1000.0f;
            previousTime = currentTime;

            // Cap the delta time to the max time step (useful if your 
            // debugging and you don't want the deltaTime value to explode.
            deltaTime = std::min<float>(deltaTime, maxTimeStep);

            auto keyA = GetKeyState(0x4D);
//            switch (GetMessage(&msg, g_WindowHandle,WM_KEYFIRST,WM_KEYLAST)) {

  //              OutputDebugStringA("yeet\n");

                if (keyA && launchedOPAModel == FALSE) {

                    dupModelA();
                 //   keyA = FALSE;
                    launchedOPAModel = TRUE;
                }

//            }

            Update(deltaTime);
            Render();
        }
    }

    return static_cast<int>(msg.wParam); //store message with wParam
}
//





/*
to do dx11:
*/

/*

    Create the device and swap chain,

    ^^ setup the swap chain description. The swap chain description 
    defines the size and number of render buffers that will 
    be used by the swap chain. It also associates the window 
    to the swap chain which determines where the final 
    image will be presented. The swap chain description 
    also defines the quality of anti-aliasing
    */

    /**
     * STEP 1: Initialize the DirectX device and swap chain.
     */
int InitDirectX(HINSTANCE hInstance, BOOL vSync)
{
    // A window handle must have been created already.
    assert(g_WindowHandle != 0);

    RECT clientRect;
    GetClientRect(g_WindowHandle, &clientRect); //get size of window

    // Compute the exact client dimensions. This will be used
    // to initialize the render targets for our swap chain.
    unsigned int clientWidth = clientRect.right - clientRect.left; //exact size of window width, by getting start and end pos of window position
    unsigned int clientHeight = clientRect.bottom - clientRect.top; //exact size of window height, by getting start and end pos of window position

    DXGI_SWAP_CHAIN_DESC swapChainDesc; //define swapchain
    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC)); //allocate swapchain memory size

    swapChainDesc.BufferCount = 1; //buffer count in swap chain - 2 images could be a good idea to render 2 seperate windows and states for unique split screen :P
    swapChainDesc.BufferDesc.Width = clientWidth; //width of window
    swapChainDesc.BufferDesc.Height = clientHeight; //height of window
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //buffer format of R G B A
    
    //define exact refresh rate to be static
    DXGI_RATIONAL refreshRateStatic; // 0/1 means unlimited, no vsync/fps lock -- I made it 30 for my laptop battery when coding on the go --> for testing on my mx250 over rx480 [battery] as well
    refreshRateStatic.Numerator = 30;
    refreshRateStatic.Denominator = 1;
    //
    
    swapChainDesc.BufferDesc.RefreshRate = refreshRateStatic; //refresh rate

    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //cpu access optio nfor back buffer:
    /*
    
    DXGI_USAGE_BACK_BUFFER: its a back buffer...

    DXGI_USAGE_READ_ONLY: use surface or resource for only read

    DXGI_USAGE_RENDER_TARGET_OUTPUT: use surface or resource as an output render target
    
    DXGI_USAGE_SHADER_INPUT: surface or resource as an input

    DXGI_USAGE_SHARED: share surface or resource

    DXGI_USAGE_UNORDERED_ACCESS: surface or resource for unordered access
    */


    swapChainDesc.OutputWindow = g_WindowHandle; //window to output swap chain to 
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // how to handle presentation buffer after on surface
    //DXGI_SWAP_EFFECT_SEQUENTIAL --> most efficent presentation; discard displayed buffer after IDXGISwapChain::Present is called
    //DXGI_SWAP_EFFECT_DISCARD --> present content of swap chain in order - keep buffer after ::Present is called

    swapChainDesc.Windowed = TRUE;
    
    //swapChainDesc.Flags; //https://docs.microsoft.com/en-ca/windows/win32/api/dxgi/ne-dxgi-dxgi_swap_chain_flag?redirectedfrom=MSDN <-- diffrent flags
    UINT createDeviceFlags = 0;
    //D3D11_CREATE_DEVICE_DEBUG is a debug layer to add extra checks
#if _DEBUG
    createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG; 
#endif

    // These are the feature levels that we will accept.
    D3D_FEATURE_LEVEL featureLevels[] = 
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    // This will be the feature level that 
    // is used to create our device and swap chain.
    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(  //create swap chain --> 
        nullptr, //A pointer to the video adapter to use --> nothing means default for program launch is used
        D3D_DRIVER_TYPE_HARDWARE, //direct 3d driver type: unknown [dunno], hardware [features in hardware], refrence [accuracy over speed], zero render ability driver, software [software driver - very slow], warp driver [9_1-10_1 support of high prof implment]
        nullptr, //dll for software rasterizer if software driver is used
        createDeviceFlags,  //device flags
        featureLevels, //feature level to try to acsess in order
        _countof(featureLevels), //number of elements
        D3D11_SDK_VERSION, //sdk version
        &swapChainDesc, //swap chain descriptor
        &g_d3dSwapChain, //pointer to swap chain obj used for rendering
        &g_d3dDevice, //adress to GPU device 
        &featureLevel, //return first supported feature in featureLevel array
        &g_d3dDeviceContext); //dunno
    //
    if (hr == E_INVALIDARG) //if making swap chain fails, try again with a diffrent feature level
    {
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
            nullptr, createDeviceFlags, &featureLevels[1], _countof(featureLevels) - 1,
            D3D11_SDK_VERSION, &swapChainDesc, &g_d3dSwapChain, &g_d3dDevice, &featureLevel,
            &g_d3dDeviceContext);
    }

    if (FAILED(hr))
    {
        return -1;
    }

    //////////////////////////////////////////
    // Next initialize the back buffer of the swap chain and associate it to a 
    // render target view.

    //STEP 2:  Create a render target view of the swap chain’s back buffer,

    ID3D11Texture2D* backBuffer;
    hr = g_d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer); //get pointer to the only buffer of swapchain (I currently made 1)
    if (FAILED(hr))
    {
        return -1;
    }

    hr = g_d3dDevice->CreateRenderTargetView( //make render view from buffer
        backBuffer, //buffer input
        nullptr,  //render target view description
        &g_d3dRenderTargetView); //pointer to target view port to send info to [need 2 seperate ones dealing with 2 seperate back buffers to render 2 viewports
    if (FAILED(hr))
    {
        return -1;
    }

    SafeRelease(backBuffer);


    //STEP 3: Create a Depth-Stencil Buffer

    // Create the depth buffer for use with the depth/stencil view.
    D3D11_TEXTURE2D_DESC depthStencilBufferDesc; //depth stencil buffer obj
    ZeroMemory(&depthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC)); //max texture memory size is allocated to buffer

    depthStencilBufferDesc.ArraySize = 1; //array of textures length (if array)
    depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL; //bind flags are vairous
    depthStencilBufferDesc.CPUAccessFlags = 0; // No CPU access required. 
    depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //texture format
    depthStencilBufferDesc.Width = clientWidth; //resolution of texture in texels width
    depthStencilBufferDesc.Height = clientHeight; //resolution of texture in texels height
    depthStencilBufferDesc.MipLevels = 1; //1 means multisampled
    
    ////multisampling parameters
    depthStencilBufferDesc.SampleDesc.Count = 1; 
    depthStencilBufferDesc.SampleDesc.Quality = 0;
    ////

    depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT; //how to read texture 
    /*
    D3D11_USAGE_DEFAULT:  read and write buffer with the gpu

    D3D11_USAGE_IMMUTABLE: read by gpu, and initialized when created

    D3D11_USAGE_DYNAMIC: GPU read, and cpu write - if updated every frame by cpu, it is good

    D3D11_USAGE_STAGING: resource that can be copied from cpu to gpu
    
    */

    //Step 4: Create a texture for the depth-stencil buffer,

    hr = g_d3dDevice->CreateTexture2D( //make 2d texture
        &depthStencilBufferDesc,  //pointer to 2d texture resource
        nullptr,  // sub data for 2d
        &g_d3dDepthStencilBuffer);  // output to stencil vuffer
    
    if (FAILED(hr))
    {
        return -1;
    }

    //Step 5: Create a depth-stencil view from the depth-stencil buffer,
    
    hr = g_d3dDevice->CreateDepthStencilView( //create depth stencil buffer view
        g_d3dDepthStencilBuffer,  // input depth stencil buffer
        nullptr, //input sub buffer - Pointer to a depth-stencil-view description --> 0 means mipmap 0 is used
        &g_d3dDepthStencilView); // output to depth stencil view

    if (FAILED(hr))
    {
        return -1;
    }

    //Step 6: Create a depth-stencil state object that defines the behaviour of the output merger stage

    // Setup depth/stencil state.
    D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
    ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

    depthStencilStateDesc.DepthEnable = TRUE; // enable depth testing
    depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; //choose to modify or not depth mask
    /*
    D3D11_DEPTH_WRITE_MASK_ALL: Turn on writes to the depth-stencil buffer.

    D3D11_DEPTH_WRITE_MASK_ZERO: Turn off writes to the depth-stencil buffer.
    */

    depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS; //depth data compairison or not -->  if the source data is less than the destination data (that is, the source data is closer to the eye), then the depth comparison passes --> render stuff
    depthStencilStateDesc.StencilEnable = FALSE; //stencil testing or not
    /*
    
    UINT8 StencilReadMask: Identify a portion of the depth-stencil buffer for reading stencil data.
    
    UINT8 StencilWriteMask: Identify a portion of the depth-stencil buffer for writing stencil data.
    
    D3D11_DEPTH_STENCILOP_DESC FrontFace: Identify how to use the results of the depth test and the stencil test for pixels whose surface normal is facing towards the camera (see D3D11_DEPTH_STENCILOP_DESC).
    
    D3D11_DEPTH_STENCILOP_DESC BackFace: Identify how to use the results of the depth test and the stencil test for pixels whose surface normal is facing away from the camera (see D3D11_DEPTH_STENCILOP_DESC).
    
    */


    hr = g_d3dDevice->CreateDepthStencilState( //make depth stencil state
        &depthStencilStateDesc, //descriptor of depth stencil
        &g_d3dDepthStencilState); //depth stencil state view to be outputed too

    //

    //Step 7:  Create a rasterizer state object that defines the behaviour of the rasterizer stage.

    // Setup rasterizer state.
    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

    rasterizerDesc.AntialiasedLineEnable = FALSE; //if MSAA is off: true turns on AA
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    /*
    D3D11_CULL_NONE: Always draw all triangles.
    D3D11_CULL_FRONT : Do not draw triangles that are front - facing.
    D3D11_CULL_BACK : Do not draw triangles that are back - facing.
    */

    rasterizerDesc.DepthBias = 0; //added value to depth - as simulated depth
    rasterizerDesc.DepthBiasClamp = 0.0f; //max for depth
    rasterizerDesc.DepthClipEnable = TRUE; //clip based on distance of obj 
    rasterizerDesc.FillMode = D3D11_FILL_SOLID; 
    /*
    
    D3D11_FILL_WIREFRAME: Draw lines connecting the vertices.
    D3D11_FILL_SOLID: Fill the triangles formed by the vertices.

    */


    rasterizerDesc.FrontCounterClockwise = FALSE; //if true, a triangle will be considered front if counter clock wise,  else opposite for false
    rasterizerDesc.MultisampleEnable = FALSE; //MSAA or not
    rasterizerDesc.ScissorEnable = FALSE; //cut renderview or not
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;//scalar on given pixel slope?

    // Create the rasterizer state object.
    hr = g_d3dDevice->CreateRasterizerState( //make rasterizer state
        &rasterizerDesc,  //rasterizer descriptor
        &g_d3dRasterizerState); //rasterizer state output

    if (FAILED(hr))
    {
        return -1;
    }

    //Step 8: Initialize the Viewport

        // Initialize the viewport to occupy the entire client area
    g_Viewport.Width = static_cast<float>(clientWidth); 
    g_Viewport.Height = static_cast<float>(clientHeight);
    g_Viewport.TopLeftX = 0.0f;
    g_Viewport.TopLeftY = 0.0f;
    g_Viewport.MinDepth = 0.0f;
    g_Viewport.MaxDepth = 1.0f;

    return 0;
}

/*

The vertex shader is responsible for transforming the incoming vertex position into clip-space 
as required by the rasterizer stage 

pixel shader is responsible for computing the final 
pixel color from the interpolated vertex attributes

*/

/*   -- https://www.3dgep.com/introduction-to-directx-11/#Introduction -- 
BASIC IDEA ON HOW SHADERS WORK:

the vertex shader might declare an output variable called out_color 
of type float4 which is associated to the COLOR semantic and the pixel 
shader declares an input variable called in_color of type float4 which 
is also associated to the COLOR semantic. This will cause the value of 
the out_color variable declared in the vertex shader to be connected to 
the value of the in_color variable in the pixel shader.


vertex shader says: [float4 out_color : COLOR] --> [float4 in_color : COLOR]
*/

/*

To load the shader at runtime, we will define a template function called LoadShader to load a shader from a file path

*/

// Get the latest profile for the specified shader type.
template< class ShaderClass >
std::string GetLatestProfile(); //template to get shader settings

template<>
std::string GetLatestProfile<ID3D11VertexShader>()
{
    assert(g_d3dDevice);

    // Query the current feature level:
    D3D_FEATURE_LEVEL featureLevel = g_d3dDevice->GetFeatureLevel(); //get usable shader feature level

    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_1:
    case D3D_FEATURE_LEVEL_11_0:
    {
        return "vs_5_0"; //11.1 and 11.0 are 5.0
    }
    break;
    case D3D_FEATURE_LEVEL_10_1:
    {
        return "vs_4_1";
    }
    break;
    case D3D_FEATURE_LEVEL_10_0:
    {
        return "vs_4_0";
    }
    break;
    case D3D_FEATURE_LEVEL_9_3:
    {
        return "vs_4_0_level_9_3";
    }
    break;
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
    {
        return "vs_4_0_level_9_1";
    }
    break;
    } // switch( featureLevel )

    return "";
}
////////////////////
template<>
std::string GetLatestProfile<ID3D11PixelShader>()
{
    assert(g_d3dDevice);

    // Query the current feature level:
    D3D_FEATURE_LEVEL featureLevel = g_d3dDevice->GetFeatureLevel(); //feature level to compile pixel shader 
    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_1:
    case D3D_FEATURE_LEVEL_11_0:
    {
        return "ps_5_0";
    }
    break;
    case D3D_FEATURE_LEVEL_10_1:
    {
        return "ps_4_1";
    }
    break;
    case D3D_FEATURE_LEVEL_10_0:
    {
        return "ps_4_0";
    }
    break;
    case D3D_FEATURE_LEVEL_9_3:
    {
        return "ps_4_0_level_9_3";
    }
    break;
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
    {
        return "ps_4_0_level_9_1";
    }
    break;
    }
    return "";
}
//////////////
template<>
std::string GetLatestProfile<ID3D11ComputeShader>()
{
    assert(g_d3dDevice);

    // Query the current feature level:
    D3D_FEATURE_LEVEL featureLevel = g_d3dDevice->GetFeatureLevel(); //feature level to compile pixel shader 
    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_1:
    case D3D_FEATURE_LEVEL_11_0:
    {
        return "cs_5_0";
    }
    break;
    case D3D_FEATURE_LEVEL_10_1:
    {
        return "cs_4_1";
    }
    break;
    case D3D_FEATURE_LEVEL_10_0:
    {
        return "cs_4_0";
    }
    break;
    case D3D_FEATURE_LEVEL_9_3: //I don't think these exist below
    {
        return "cs_4_0_level_9_3";
    }
    break;
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
    {
        return "cs_4_0_level_9_1";
    }
    break;
    }
    return "";
}
// --I can make special options for other shader types

/*
make shader object - LoadShader time:
*/

template< class ShaderClass >
ShaderClass* CreateShader(ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage); //generic shader creation, take in parameters [need to specify what later]

template<>
ID3D11VertexShader* CreateShader<ID3D11VertexShader>(ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage) //vertex shader shader type
{
    assert(g_d3dDevice);
    assert(pShaderBlob);

    ID3D11VertexShader* pVertexShader = nullptr;
    g_d3dDevice->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pVertexShader); //make a shader based on buffer, buffer size, classtype, and return to pshader object
    // create input here as well since blob is here

    D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, //  D3D11_INPUT_ELEMENT_DESC - vars is listed above 
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXLINK", 0, DXGI_FORMAT_R16_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    
    auto hr = g_d3dDevice->CreateInputLayout( //make input layout - global change to input Layout
        vertexLayoutDesc, //vertex shader - input assembler data
        _countof(vertexLayoutDesc), //number of elements
        pShaderBlob->GetBufferPointer(),  //vertex shader buffer
        pShaderBlob->GetBufferSize(), //vetex shader blob size 
        &g_d3dInputLayout); //input layout

    if (FAILED(hr))
    {
        OutputDebugStringW(L"failed input layout setup");
        //return false;
    }
    return pVertexShader;
}

////////////////////////////////

template<>
ID3D11PixelShader* CreateShader<ID3D11PixelShader>(ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage)
{
    assert(g_d3dDevice);
    assert(pShaderBlob);

    ID3D11PixelShader* pPixelShader = nullptr;
    g_d3dDevice->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pPixelShader); //pixel shader version of the vertex shader above

    return pPixelShader;
}

template<>
ID3D11ComputeShader* CreateShader<ID3D11ComputeShader>(ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage)
{
    assert(g_d3dDevice);
    assert(pShaderBlob);

    ID3D11ComputeShader* pComputeShader = nullptr;
    g_d3dDevice->CreateComputeShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pComputeShader); //pixel shader version of the vertex shader above

    return pComputeShader;
}


template< class ShaderClass >
ShaderClass* LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& _profile) //LoadShader class
{
    volatile auto a = std::filesystem::exists("./SimpleComputeShader.hlsl"); //debug test; this is not real project for much else than test and fun
    OutputDebugStringW(fileName.c_str());
    ID3DBlob* pShaderBlob = nullptr; 
    ID3DBlob* pErrorBlob = nullptr;
    ShaderClass* pShader = nullptr;

    std::string profile = _profile;
    if (profile == "latest")
    {
        profile = GetLatestProfile<ShaderClass>(); //get able shader profiles/settings
    }
    
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

#if _DEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif

    HRESULT hr = D3DCompileFromFile( //HLSL shader into a Binary Large Object (BLOB)  --> D3DCompileFromFile does this
        fileName.c_str(),  //shader path name
        nullptr, //array of shader macro's [try async compile later] - https://docs.microsoft.com/en-ca/windows/win32/api/d3dcommon/ns-d3dcommon-d3d_shader_macro?redirectedfrom=MSDN  --> stuff like async creation exists
        D3D_COMPILE_STANDARD_FILE_INCLUDE,  //read file realtive to current directory
        entryPoint.c_str(), //name of shader for execution point
        profile.c_str(),// set of shader features to compile against --> converted to c_str() because winAPI... 
        flags, //compile flags - https://docs.microsoft.com/en-ca/windows/win32/direct3dhlsl/d3dcompile-constants?redirectedfrom=MSDN  - like always use | to add more
        0, //effect compile flags - https://docs.microsoft.com/en-ca/windows/win32/direct3dhlsl/d3dcompile-effect-constants?redirectedfrom=MSDN 
        &pShaderBlob,// pointer to output shader Blob - compiled stuff
        &pErrorBlob);// error to Blob 

    if (FAILED(hr)) 
    {
        if (pErrorBlob) // if no blob, we free all data related to this
        {
            std::string errorMessage = (char*)pErrorBlob->GetBufferPointer(); //error message of when making shaderblob
            OutputDebugStringA(errorMessage.c_str()); //print string to visaul studio debug log - no need for a console

            SafeRelease(pShaderBlob); //clear mem of shader
            SafeRelease(pErrorBlob); //clear mem of error shader
        }

    //    return false;
    }

    pShader = CreateShader<ShaderClass>(pShaderBlob, nullptr); // if no crash I can make a shader using shader blob 

    SafeRelease(pShaderBlob); // no longer need shader mem
    SafeRelease(pErrorBlob); // no longer need shader mem

    return pShader;
}

//template< class ShaderClass >
//ShaderClass* LoadComputeShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& _profile) //LoadShader class
//{
//    //volatile auto a = std::filesystem::exists("./SimplePixelShader.hlsl"); //debug test; this is not real project for much else than test and fun
//    OutputDebugStringW(fileName.c_str());
//    ID3DBlob* pShaderBlob = nullptr;
//    ID3DBlob* pErrorBlob = nullptr;
//    ShaderClass* pShader = nullptr;
//
//    std::string profile = _profile;
//    if (profile == "latest")
//    {
//        profile = GetLatestProfile<ShaderClass>(); //get able shader profiles/settings
//    }
//
//    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
//
//#if _DEBUG
//    flags |= D3DCOMPILE_DEBUG;
//#endif
//
//    HRESULT hr = D3DCompileFromFile( //HLSL shader into a Binary Large Object (BLOB)  --> D3DCompileFromFile does this
//        fileName.c_str(),  //shader path name
//        nullptr, //array of shader macro's [try async compile later] - https://docs.microsoft.com/en-ca/windows/win32/api/d3dcommon/ns-d3dcommon-d3d_shader_macro?redirectedfrom=MSDN  --> stuff like async creation exists
//        D3D_COMPILE_STANDARD_FILE_INCLUDE,  //read file realtive to current directory
//        entryPoint.c_str(), //name of shader for execution point
//        profile.c_str(),// set of shader features to compile against --> converted to c_str() because winAPI... 
//        flags, //compile flags - https://docs.microsoft.com/en-ca/windows/win32/direct3dhlsl/d3dcompile-constants?redirectedfrom=MSDN  - like always use | to add more
//        0, //effect compile flags - https://docs.microsoft.com/en-ca/windows/win32/direct3dhlsl/d3dcompile-effect-constants?redirectedfrom=MSDN 
//        &pShaderBlob,// pointer to output shader Blob - compiled stuff
//        &pErrorBlob);// error to Blob 
//
//    if (FAILED(hr))
//    {
//        if (pErrorBlob) // if no blob, we free all data related to this
//        {
//            std::string errorMessage = (char*)pErrorBlob->GetBufferPointer(); //error message of when making shaderblob
//            OutputDebugStringA(errorMessage.c_str()); //print string to visaul studio debug log - no need for a console
//
//            SafeRelease(pShaderBlob); //clear mem of shader
//            SafeRelease(pErrorBlob); //clear mem of error shader
//        }
//
//        //    return false;
//    }
//
//    pShader = CreateShader<ShaderClass>(pShaderBlob, nullptr); // if no crash I can make a shader using shader blob 
//
//    SafeRelease(pShaderBlob); // no longer need shader mem
//    SafeRelease(pErrorBlob); // no longer need shader mem
//
//    return pShader;
//
//}


bool LoadContent()
{
    inputRelatedThread = std::async(std::launch::async, [] {
        keyInputAsync();
        });

    assert(g_d3dDevice);

    loadModel("./model/1.obj");
    loadTex(L"./tex/1.png");
    makeSampler();

    // Create an initialize the vertex buffer.
    D3D11_BUFFER_DESC vertexBufferDesc; //describe buffer we will make
    ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));

    //vertexBufferDesc.StructureByteStride --- if you have a struct buffer --> done to be efficent with zero memory allocation, but this allows allocation by stride

    vertexBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_VERTEX_BUFFER; //how to bind buffer 

    /* type of buffer:
    
    D3D11_BIND_VERTEX_BUFFER,
    D3D11_BIND_INDEX_BUFFER,
    D3D11_BIND_CONSTANT_BUFFER,
    D3D11_BIND_SHADER_RESOURCE,
    D3D11_BIND_STREAM_OUTPUT,
    D3D11_BIND_RENDER_TARGET,
    D3D11_BIND_DEPTH_STENCIL,
    D3D11_BIND_UNORDERED_ACCESS,
    D3D11_BIND_DECODER,
    D3D11_BIND_VIDEO_ENCODER
    
    */


    vertexBufferDesc.ByteWidth = sizeof(VertexPosColor) * (g_Vertices.size()); //size of buffer --> make it the size of verticies*vertexPosColor [since vertex will have pos and color
    vertexBufferDesc.CPUAccessFlags = 0; // 0 means no CPU acsess

    /*
    
    D3D11_CPU_ACCESS_WRITE       obvioussly write allowed by cpu to buffer
    D3D11_CPU_ACCESS_READ        obvioussly read allowed by cpu to buffer
    ^^ could use 2 over 1
    */

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT; //resource flag - 0 means none

    vertexBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

    /*https://docs.microsoft.com/en-ca/windows/win32/api/d3d11/ne-d3d11-d3d11_resource_misc_flag?redirectedfrom=MSDN
  D3D11_RESOURCE_MISC_GENERATE_MIPS,
  D3D11_RESOURCE_MISC_SHARED,
  D3D11_RESOURCE_MISC_TEXTURECUBE,
  D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS,
  D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS,
  D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
  D3D11_RESOURCE_MISC_RESOURCE_CLAMP,
  D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX,
  D3D11_RESOURCE_MISC_GDI_COMPATIBLE,
  D3D11_RESOURCE_MISC_SHARED_NTHANDLE,
  D3D11_RESOURCE_MISC_RESTRICTED_CONTENT,
  D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE,
  D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER,
  D3D11_RESOURCE_MISC_GUARDED,
  D3D11_RESOURCE_MISC_TILE_POOL,
  D3D11_RESOURCE_MISC_TILED,
  D3D11_RESOURCE_MISC_HW_PROTECTED

  ^^ This is lots

    
    */

    D3D11_SUBRESOURCE_DATA resourceData; //data for buffer
    ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    /*
    
    
    const void *pSysMem: A pointer to the data to initialize the buffer with.

    UINT SysMemPitch: The distance (in bytes) from the beginning of one line of a texture to the next line. System-memory pitch is used only for 2D and 3D texture data as it is has no meaning for the other resource types.

    UINT SysMemSlicePitch: The distance (in bytes) from the beginning of one depth level to the next. System-memory-slice pitch is only used for 3D texture data as it has no meaning for the other resource types.

    */
    
    //const VertexPosColor* tmpV = ;
    resourceData.pSysMem = &g_Vertices[0]; //Vertex data for sub source

    HRESULT hr = g_d3dDevice->CreateBuffer(&vertexBufferDesc, &resourceData, &g_d3dVertexBuffer); //create buffer, using data settings struct, struct of data, and vertex buffer output - this is also used to create other buffer styles
    if (FAILED(hr))
    {
        return false;
    }

    g_d3dVertexBufferV.push_back(g_d3dVertexBuffer);
    //////////////////////////////

    
    // Create and initialize the index buffer.
    D3D11_BUFFER_DESC indexBufferDesc; //buffer obj
    ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC)); //alloc

    indexBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_INDEX_BUFFER; //type of buffer m8 - same logic as vertex
    indexBufferDesc.ByteWidth = sizeof(UINT) * (g_Indicies.size());
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

    indexBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    
    resourceData.pSysMem = &g_Indicies[0]; //indice data for sub source

    hr = g_d3dDevice->CreateBuffer(&indexBufferDesc, &resourceData, &g_d3dIndexBuffer); //make buffer
    if (FAILED(hr))
    {
        return false;
    }
    
    g_d3dIndexBufferV.push_back(g_d3dIndexBuffer);
    //////////////////////

    // Create the constant buffers for the variables defined in the vertex shader.
    D3D11_BUFFER_DESC constantBufferDesc;
    ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC)); //mem alloc

    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; //const buffer
    constantBufferDesc.ByteWidth = sizeof(XMMATRIX); //size of matrix max
    constantBufferDesc.CPUAccessFlags = 0;
    constantBufferDesc.Usage = D3D11_USAGE_DEFAULT; //you can make this dynamic usage --> but we will use this [requires non dynamic]: ID3D11DeviceContext::UpdateSubresource  

    //I do not have Sub-Resource-data - since we allocate and reallocate this at a later date

    hr = g_d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &g_d3dConstantBuffers[CB_Application]); //make const buffer for application
    if (FAILED(hr))
    {
        return false;
    }
    hr = g_d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &g_d3dConstantBuffers[CB_Frame]); //make const buffer for frame
    if (FAILED(hr))
    {
        return false;
    }
    hr = g_d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &g_d3dConstantBuffers[CB_Object]); //make const buffer for object 
    if (FAILED(hr))
    {
        return false;
    }

    // we have 3 buffers of the same so we can modify object space, frame, and application space all seperately

    /*    
1.     Load and compile the shader at runtime.
2.     Load a precompiled shader object.
3.     Create a shader from a byte array.
    */

    // Load the shaders --> 
    g_d3dVertexShader = LoadShader<ID3D11VertexShader>(L"./SimpleVertexShader.hlsl", "SimpleVertexShader", "latest"); //load shader hlsl file named as object SimpleVertexShader
    g_d3dPixelShader = LoadShader<ID3D11PixelShader>(L"./SimplePixelShader.hlsl", "SimplePixelShader", "latest"); //load shader hlsl file named as object SimplePixelShader
    
    //ID3DBlob* vertexShaderBlob; //shader data blob
/*
#if _DEBUG
    LPCWSTR compiledVertexShaderObject = L"SimpleVertexShader_d.cso";
#else
    LPCWSTR compiledVertexShaderObject = L"SimpleVertexShader.cso";
#endif

    hr = D3DReadFileToBlob(compiledVertexShaderObject, &vertexShaderBlob); //shader txt data read from blob --> this is how to load from precompiled/was-compiled
    if (FAILED(hr))
    {
        return false;
    }

    hr = g_d3dDevice->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &g_d3dVertexShader); // make vertex shader --> vertexShaderBlob buffer pointer, vertexShaderBlob buffer size, class type [need none], and vertex shader obj with data vertex'es from earlier
    if (FAILED(hr))
    {
        return false;
    }
     //compile at run time
*/
    /*
     ID3D11InputLayout - defines how vertex data is attached to input assembler --> creates input layout is needed first [ID3D11Device::CreateInputLayout]
    
    ------------------------------------------------------

    const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs: An array of the input-assembler stage input 
    data types; each type is described by an element 
    description (see D3D11_INPUT_ELEMENT_DESC). This structure will be described in the next section.
    
    UINT NumElements: The number of input elements in the pInputElementDescs array.
    
    const void *pShaderBytecodeWithInputSignature: A pointer to the compiled shader. The compiled shader code 
    contains a input signature which is validated against the array of elements.
    
    SIZE_T BytecodeLength: The size in bytes of the pShaderBytecodeWithInputSignature array.
    
    ID3D11InputLayout **ppInputLayout: A pointer to the input-layout object 
    created (see ID3D11InputLayout).
    
    
    D3D11_INPUT_ELEMENT_DESC: 
    LPCSTR SemanticName: The HLSL semantic associated with this element in a shader input-signature.
    
    UINT SemanticIndex: The semantic index for the element. A semantic index modifies a semantic, with an integer index number. A semantic index is only needed in a case where there is more than one element with the same semantic.
    
    DXGI_FORMAT Format: The data type of the element data. See DXGI_FORMAT. For example, if the element describes a 4-component floating point vector, the Format flag would be set to DXGI_FORMAT_R32G32B32A32_FLOAT.
    
    UINT InputSlot: If using a single vertex buffer with interleaved vertex attributes then the input slot should always be 0. If using several packed vertex buffers where each vertex buffer contains the vertex data for a single vertex attribute, then the input slot is the index of the vertex buffer that is attached to the input assembler stage.
    
    UINT AlignedByteOffset: Offset (in bytes) between each element. Use D3D11_APPEND_ALIGNED_ELEMENT for convenience to define the current element directly after the previous one, including any packing if necessary.
    
    D3D11_INPUT_CLASSIFICATION InputSlotClass: Identifies the input data class for a single input slot. This member can have one of the following values [50]:
            D3D11_INPUT_PER_VERTEX_DATA: Input data is per-vertex data.
            D3D11_INPUT_PER_INSTANCE_DATA: Input data is per-instance data.
    

    UINT InstanceDataStepRate: The number of instances to draw using the same per-instance data before advancing in the buffer by one element. This value must be 0 for an element that contains per-vertex data (the slot class is set to D3D11_INPUT_PER_VERTEX_DATA).

    
    */

    // Create the input layout for the vertex shader.
/*    D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor,Position), D3D11_INPUT_PER_VERTEX_DATA, 0 }, //  D3D11_INPUT_ELEMENT_DESC - vars is listed above 
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor,Color), D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = g_d3dDevice->CreateInputLayout( //make input layout
        vertexLayoutDesc, //vertex shader - input assembler data
        _countof(vertexLayoutDesc), //number of elements
        vertexShaderBlob->GetBufferPointer(),  //vertex shader buffer
        vertexShaderBlob->GetBufferSize(), //vetex shader blob size 
        &g_d3dInputLayout); //input layout

    if (FAILED(hr))
    {
        return false; 
    }
    moved to when making shader directly
    
    SafeRelease(vertexShaderBlob); //vertex buffer for our cube contains two attributes: vertex pos and color
    */
    

    /////////////////
    
        // Load the compiled pixel shader. -- equivlent but for pixel as for shader
   // ID3DBlob* pixelShaderBlob;
    /*
#if _DEBUG
    LPCWSTR compiledPixelShaderObject = L"SimplePixelShader_d.cso";
#else
    LPCWSTR compiledPixelShaderObject = L"SimplePixelShader.cso";
#endif

    hr = D3DReadFileToBlob(compiledPixelShaderObject, &pixelShaderBlob);
    if (FAILED(hr))
    {
        return false;
    }

    hr = g_d3dDevice->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &g_d3dPixelShader);
    if (FAILED(hr))
    {
        return false;
    }

    SafeRelease(pixelShaderBlob);
    */
    
    //projection matrix:

    // Setup the projection matrix.
    RECT clientRect; //window size for projection matrix
    GetClientRect(g_WindowHandle, &clientRect); //get window corners

    // Compute the exact client dimensions.
    // This is required for a correct projection matrix.
    float clientWidth = static_cast<float>(clientRect.right - clientRect.left); //true width
    float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top); //true height

    g_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), clientWidth / clientHeight, 0.1f, 100.0f); //make camrea [left hand projection matrix] --> based on feild of view math, FovAngleY, AspectRatio, NearZ, FarZ
    //^^ other types exist

    g_d3dDeviceContext->UpdateSubresource( //change sub resource data on the fly for [can be done for constant buffer]
        g_d3dConstantBuffers[CB_Application],  // destination of sub resource - stored inside CB_application const buffer
        0,  /// destination of sub resource
        nullptr, // defines box that has detination of subresource data -- null means data is in subresource with no offset --> const buffer is modified position, so the const pDstBox cannot be used (since it is const)
        &g_ProjectionMatrix,  // pointer to fov source data
        0,  // row of source data [strut/array]
        0); // depth of source data [strut/array]

    return true;
}

//The Update Function

void Update(float deltaTime) //pass net time to pass to have a timer
{

    eyePosX += modx;
    eyePosY += mody;
    eyePosZ += modz;

    eyePosZ = abs(eyePosZ);

    

    XMVECTOR eyePosition = XMVectorSet(eyePosX, eyePosY, eyePosZ, 1); //eye pos
    XMVECTOR eyeDirection = XMVectorSet(0, 1, 0, 0); // cam focus --> add trignometric functions to allow this focus to adjust with rotation of camrea - 
    XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0); //dir up

    g_ViewMatrix = XMMatrixLookAtLH(eyePosition, eyeDirection, upDirection); //matrix LH pos
    g_d3dDeviceContext->UpdateSubresource(g_d3dConstantBuffers[CB_Frame], 0, nullptr, &g_ViewMatrix, 0, 0); //update subresource data of constant buffer


    static float angle = 0.0f;
    angle += 0.0f * deltaTime; //change cam angle based on time
    XMVECTOR rotationAxis = XMVectorSet(0, 1, 0, 0);

    g_WorldMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));
    g_d3dDeviceContext->UpdateSubresource(g_d3dConstantBuffers[CB_Object], 0, nullptr, &g_WorldMatrix, 0, 0);

    modx = 0;
    mody = 0;
    modz = 0;
}   

//clear old back-buffer
// Clear the color and depth buffers.
void Clear(const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil)
{
    g_d3dDeviceContext->ClearRenderTargetView(g_d3dRenderTargetView, clearColor); //clear render view
    g_d3dDeviceContext->ClearDepthStencilView(g_d3dDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepth, clearStencil); //clear stencil view
}
//

//called at end of drawing
void Present(bool vSync)
{
    if (vSync)
    {
        g_d3dSwapChain->Present(1, 0);
/*

https://www.3dgep.com/introduction-to-directx-11/#Introduction

^^^ ctrl f   -   UINT SyncInterval:

*/
    }
    else
    {
        g_d3dSwapChain->Present(0, 0);
    //options above
    }
}

//main render
void Render()
{
    assert(g_d3dDevice);
    assert(g_d3dDeviceContext);

    Clear(Colors::DarkOliveGreen, 1.0f, 0); //make background a color

    const UINT vertexStride = sizeof(VertexPosColor); //
    const UINT offset = 0; //

    

    for (int i = 0; i < g_d3dIndexBufferV.size(); i++) {
        g_d3dDeviceContext->IASetVertexBuffers( //bind vertex buffer to device context
            0, //first input slot for binding - each buffer extra is bounded to subsequent input slot // D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT-1 is max
            1, //vertex buffers in array, num of buffers //D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT – StartSlot is vertex buffer count

            //StartSlot argument should match the InputSlot of the 
            //D3D11_INPUT_ELEMENT_DESC elements that were configured 
            //in the LoadContent function.



            &g_d3dVertexBufferV[i], //pointer to array of vertex buffers [may not be array too]
            &vertexStride,  //ponter to array of stride values for each buffer
            &offset); //offset for each buffer in vertex buffer array
    }

    //g_d3dDeviceContext -> Map(g_d3dVertexBuffer,0, D3D11_MAP_WRITE, 0, &VMmap); //for now map to kill all later - will need to revamp this to make it additive later

    //g_d3dDeviceContext->Unmap(g_d3dVertexBuffer, 0);

    g_d3dDeviceContext->IASetInputLayout(
        g_d3dInputLayout);  //set input layout
    
    for (int i = 0; i < g_d3dIndexBufferV.size(); i++) {
        g_d3dDeviceContext->IASetIndexBuffer(
            g_d3dIndexBufferV[i], //index buffer array pointer
            DXGI_FORMAT_R16_UINT, //format of DXGI format
            0); //offset
    }

    g_d3dDeviceContext->IASetPrimitiveTopology( //primitive to load tri's
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ); // set to use as primitive topology tri list - some may need to be D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ


    /////////Setup the Vertex Shader Stage

    g_d3dDeviceContext->VSSetShader( //bound vertex shader to to shader stage as a whole
        g_d3dVertexShader, //pointer to shader to bind
        nullptr, //array of class instance - can be disabled
        0); //num of class instance above

    g_d3dDeviceContext->VSSetConstantBuffers( // bind constant buffer to shaderr stage
        0, // index of const buffer    --> // D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT – 1
        3, //number of buffers - obejct, frame, and application buffers
        g_d3dConstantBuffers); //arra of const buffer is given to device

    //setup compute shader:
 //   for (int i = 0; i < g_d3dComputeShader.size(); i++) {

    // Disable Compute Shader left over garbage from test build that I lazily removed to get a working proj

    ///////////Setup the Rasterizer Stage
    /*
    
    After the vertex shader stage but before the pixel shader stage comes the 
    rasterizer stage. The rasterizer stage is responsible 
    for interpolating the various vertex attributes output from the vertex shader 
    and invoking the pixel shader program for each screen pixel which is affected by the rendered geometry.
    https://www.3dgep.com/introduction-to-directx-11/#Introduction  
    */

    g_d3dDeviceContext->PSSetShader( //pixel state to bind to shader state
        g_d3dPixelShader,  // pointer to shader to bind
        nullptr, //array of class instance - can be disabled
        0); //number of instance

    for (int x = 0; x < textureV.size(); x++) { //may later fix to allow 2 models to be unique
        g_d3dDeviceContext->PSSetShaderResources(0+x, 1, &textureV[x]);
        
        //textureV[i]->GetDesc();
//        g_d3dDeviceContext->PSSetSamplers(0, 1, );
        //   g_d3dDeviceContext->Map(textureT[i], 0, D3D11_MAP{ D3D11_MAP_READ }, 0, 0);
    }

    g_d3dDeviceContext->PSSetSamplers(0, 1, &sampler[0]); //pass sampler to pixel sahder

    ///

    g_d3dDeviceContext->RSSetState(g_d3dRasterizerState); //set rasterizer state from deviceContext - InitDirectX 
    g_d3dDeviceContext->RSSetViewports( //set viewPort state from deviceContext 
        1, //view port count 
        &g_Viewport); //view port struct made previously

    //Setup the Pixel Shader Stage


    /////////////Setup the Output Merger Stage

    g_d3dDeviceContext->OMSetRenderTargets( //8 is max currently
        1, //1 render target 
        &g_d3dRenderTargetView, //setup array of render view  - can be null
        g_d3dDepthStencilView); //setup array of stencil view - can be null

    g_d3dDeviceContext->OMSetDepthStencilState(g_d3dDepthStencilState, 1); // bind stencil state after target?

    ////////////Draw The Cube
    
    g_d3dDeviceContext->DrawIndexed( //draw indice+vertex
        (g_Indicies.size()*2), //indice count - yes, this was an issue that needed *2...  
        0,  //start index location
        0); //base vertex location
    
    /*
    g_d3dDeviceContext->Draw( //draw indice+vertex
        g_Vertices.size(), //indice count
        g_Vertices[0].Position); //base vertex location
    */
    //  UINT VertexCount,
    //UINT StartVertexLocation

    ///////////Present

    Present(g_EnableVSync); //var to enable v_sync or not
}

void UnloadContent() //clean up
{
    SafeRelease(g_d3dConstantBuffers[CB_Application]);
    SafeRelease(g_d3dConstantBuffers[CB_Frame]);
    SafeRelease(g_d3dConstantBuffers[CB_Object]);
    SafeRelease(g_d3dIndexBuffer);
    SafeRelease(g_d3dVertexBuffer);
    SafeRelease(g_d3dInputLayout);
    SafeRelease(g_d3dVertexShader);
    SafeRelease(g_d3dPixelShader);
    CoUninitialize();
}

void Cleanup()
{
    SafeRelease(g_d3dDepthStencilView);
    SafeRelease(g_d3dRenderTargetView);
    SafeRelease(g_d3dDepthStencilBuffer);
    SafeRelease(g_d3dDepthStencilState);
    SafeRelease(g_d3dRasterizerState);
    SafeRelease(g_d3dSwapChain);
    SafeRelease(g_d3dDeviceContext);
    SafeRelease(g_d3dDevice);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{//TranslateMessage


    UNREFERENCED_PARAMETER(prevInstance);
    UNREFERENCED_PARAMETER(cmdLine);

    // Check for DirectX Math library support.
    if (!XMVerifyCPUSupport()) //crash if no DirectXMath support
    {
        MessageBox(nullptr, TEXT("Failed to verify DirectX Math library support."), TEXT("Error"), MB_OK);
        return -1;
    }

    if (InitApplication(hInstance, cmdShow) != 0) //make main window, else crash
    {
        MessageBox(nullptr, TEXT("Failed to create applicaiton window."), TEXT("Error"), MB_OK);
        return -1;
    }

    if (InitDirectX(hInstance, g_EnableVSync) != 0)
    {
        MessageBox(nullptr, TEXT("Failed to create DirectX device and swap chain."), TEXT("Error"), MB_OK);
        return -1;
    }

    if (!LoadContent())
    {
        MessageBox(nullptr, TEXT("Failed to load content."), TEXT("Error"), MB_OK); 
        return -1;
    }

    int returnCode = Run(); //start off main loop

    UnloadContent(); //clean muh trash up - heh heh heh - comedy gold
    Cleanup();

    return returnCode;
}


