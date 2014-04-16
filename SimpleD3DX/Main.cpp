//broken as of 4/16/2014! i am not going to update this any further.

#include <windows.h>
#include <windowsx.h>


#include <d3d11.h>
#include <d3dx11.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#include "Struct.h"

#define DISPLAY_WIDTH	800
#define DISPLAY_HEIGHT	600


//GLOBAL declarations
IDXGISwapChain *swapchain; //the pointer to the buffer strategy (swap chain)
ID3D11Device *device;         //the pointer to the Direct3D device's interface
ID3D11DeviceContext *devicecon;  //the pointer to the Direct3D device context
ID3D11RenderTargetView *buffBack; //pointer to the back buffer

ID3D11VertexShader *cVertexShader;
ID3D11PixelShader *cPixelShader;

ID3D11Buffer *pVBuffer; //pointer to vertex buffer

ID3D11InputLayout *inputLayout;

void SD3D_StartX(HWND hWnd); //starts the Direct3D engine (hWnd = handle for the active window)
void SD3D_DestroyX(void); //cleans up the Direct3D environment after the application needs to close
void SD3D_Render(void);  //renders one SD3D frame
void SD3D_StartPipeline(void);
void SD3D_StartGraphics(void);

//callback for messages sent to the window
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
   // wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    RECT wr = {0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindowEx(NULL,
                          L"WindowClass",
                          L"SimpleD3D",
                          WS_OVERLAPPEDWINDOW,
						  DISPLAY_WIDTH,
                          DISPLAY_HEIGHT,
                          wr.right - wr.left,
                          wr.bottom - wr.top,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hWnd, nCmdShow);

    //start D3D
	SD3D_StartX(hWnd);


    MSG msg;

	while(TRUE)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) //exit program
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if(msg.message == WM_QUIT)
                break;
        }
        
        //GAME LOOP CODE HERE! THIS IS IMPORTANT WAKE UP PEOPLE
		SD3D_Render();

    }

    //destroy D3D
	SD3D_DestroyX();

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } break;
    }

    return DefWindowProc (hWnd, message, wParam, lParam);
}




void SD3D_StartX(HWND hWnd) 
{

	
	DXGI_SWAP_CHAIN_DESC swapinfo; //make a storage variable for information about the 
								   //swapchain structure
	ZeroMemory(&swapinfo, sizeof(DXGI_SWAP_CHAIN_DESC)); //clear the whole swapchain of data 
														 //quickly and all at once

	swapinfo.BufferCount = 1; //back buffers
	swapinfo.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //32 bit color scheme
	swapinfo.BufferDesc.Height = DISPLAY_HEIGHT; //size of buffer
	swapinfo.BufferDesc.Width = DISPLAY_WIDTH; //size of buffer
	swapinfo.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //draw to the back buffer
	swapinfo.OutputWindow = hWnd; //that output will be our active window handler (hWnd)
	swapinfo.SampleDesc.Count = 4; //MSAA count of multisampling
	swapinfo.Windowed = true; //Use a window, not fullscreen!
	swapinfo.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


	//Create the DX3D device and swap chain for real now

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, 
								  D3D11_SDK_VERSION, &swapinfo, &swapchain, &device, NULL, 
								  &devicecon);

	ID3D11Texture2D *sBackBuffer;
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&sBackBuffer);

    //Create an area in video memory to render to
    device->CreateRenderTargetView(sBackBuffer, NULL, &buffBack);
    sBackBuffer->Release();

    //make the render target the back buffer
    devicecon->OMSetRenderTargets(1, &buffBack, NULL);

	//setup a structure called a viewport to determine where the -1, -1 coordinates equate
    D3D11_VIEWPORT sViewPort;
    ZeroMemory(&sViewPort, sizeof(D3D11_VIEWPORT));

    sViewPort.TopLeftX = 0;
    sViewPort.TopLeftY = 0;
	sViewPort.Width = DISPLAY_WIDTH;
    sViewPort.Height = DISPLAY_HEIGHT;

	devicecon->RSSetViewports(1, &sViewPort); //actually activates the viewport structure
	SD3D_StartPipeline();
	SD3D_StartGraphics();
}

void SD3D_DestroyX(void)
{
	//clean up the swap chain and Direct3D device
	cPixelShader->Release();
	cVertexShader->Release();
	swapchain->SetFullscreenState(false, NULL);
	swapchain->Release();
	device->Release();
	devicecon->Release();
}


void SD3D_Render(void)
{
	float clearColor[4] = { 0.3f, 0.5f, 0.3f, 0.875f };
	devicecon->ClearRenderTargetView(buffBack, clearColor);


	
        UINT stride = sizeof(SD3DVertex);
        UINT offset = 0;
        devicecon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);

        
        devicecon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        
        devicecon->Draw(3, 0);
	
	//Render to the back buffer here

	//======================================================
	swapchain->Present(0, 0);
}


void SD3D_StartPipeline(void)
{
	//compiles shaders

	ID3D10Blob *vertexShader, *pixelShader;

	// /res/shaders.hlsl or just shaders.hlsl?
	D3DX11CompileFromFile(L"shaders.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &vertexShader, 0, 0);
	D3DX11CompileFromFile(L"shaders.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &pixelShader, 0, 0);

	//store the compiled shaders into the shader objects that are globally declared
	device->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), NULL, &cVertexShader);
	device->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), NULL, &cPixelShader);

	//set the shaders on the device context
	devicecon->VSSetShader(cVertexShader, 0, 0);
	devicecon->PSSetShader(cPixelShader, 0, 0);

	D3D11_INPUT_ELEMENT_DESC vertexStruct[] = //tell Direct3D the instructions for interpreting the SD3DVertex struct
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	device->CreateInputLayout(vertexStruct, 2, vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), &inputLayout);
	devicecon->IASetInputLayout(inputLayout);
}

void SD3D_StartGraphics(void)
{
	//float red[4] =   { 1.0f, 0.0f, 0.0f, 1.0f   };
	//float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f   };
	//float blue[4] =  { 0.0f, 0.0f, 1.0f, 1.0f   };


	SD3DVertex triangleTest[] =
	{
		{0.0f, 0.5f, 0.0f, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)},
		{0.45f, -0.5, 0.0f, D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
		{-0.45f, -0.5f, 0.0f, D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f)}
	};

	
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc)); //zero out the buffer's description

	bufferDesc.Usage = D3D11_USAGE_DYNAMIC; //both the GPU and CPU can change this?
	bufferDesc.ByteWidth = sizeof(SD3DVertex) + 3;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //let Direct3D have write access to the CPU for copying to the video memory

	device->CreateBuffer(&bufferDesc, NULL, &pVBuffer); //create the buffer

	D3D11_MAPPED_SUBRESOURCE mappedSource;
	devicecon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSource); //allow access to the buffer ("map" it)
	memcpy(mappedSource.pData, triangleTest, sizeof(triangleTest)); //copy the memory
	devicecon->Unmap(pVBuffer, NULL);


}
