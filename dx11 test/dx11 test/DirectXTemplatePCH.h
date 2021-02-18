//linker system, subsystem, windows

#include <windows.h>

// DirectX includes
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <atlbase.h>
// STL includes
#include <iostream>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")

/*

The SafeRelease function can be used to safely release a COM object and set the COM 
pointer to NULL. This function allows us 
to safely release a COM object even if 
it has already been released before. Since we will 
be releasing COM objects a lot in 
this application, this function will 
also allow us to create neater code.

*/
template<typename T>
inline void SafeRelease(T& ptr)
{
    if (ptr != NULL)
    {
        ptr->Release();
        ptr = NULL;
    }
}