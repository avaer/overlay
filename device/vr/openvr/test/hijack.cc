#include <deque>
#include <map>
// #include <set>
#include <string>
#include <algorithm>
#include <iomanip>

#define CINTERFACE
#define D3D11_NO_HELPERS
// #include <windows.h>
#include <D3D11_4.h>
#include <d3dcompiler.h>
// #include <wrl.h>
// #include <dcomp.h>

#include "device/vr/openvr/test/hijack.h"
#include "device/vr/openvr/test/out.h"
#include "device/vr/openvr/test/glcontext.h"
#include "device/vr/openvr/test/offsets.h"
#include "device/vr/detours/detours.h"
#include "third_party/openvr/src/headers/openvr.h"

#define EGL_EGLEXT_PROTOTYPES

#include "third_party/khronos/EGL/egl.h"
#include "third_party/khronos/EGL/eglext.h"
#include "third_party/khronos/EGL/eglext_angle.h"

#include "device/vr/openvr/test/glcontext.h"

// externs
extern Hijacker *g_hijacker;
// extern uint64_t *pFrameCount;
extern bool isChrome;
extern Offsets *g_offsets;

constexpr unsigned int viewportWidth = 500;
constexpr unsigned int viewportHeight = 500;

char kHijacker_RegisterSurface[] = "IVRCompositor::kIVRCompositor_RegisterSurface";
char kHijacker_TryBindSurface[] = "IVRCompositor::kIVRCompositor_TryBindSurface";
char kHijacker_GetSharedEyeTexture[] = "IVRCompositor::kIVRCompositor_GetSharedEyeTexture";

// void LocalGetDXGIOutputInfo(int32_t *pAdaterIndex);
void ProxyGetDXGIOutputInfo(int32_t *pAdaterIndex);
void ProxyGetRecommendedRenderTargetSize(uint32_t *pWidth, uint32_t *pHeight);
void ProxyGetProjectionRaw(vr::EVREye eye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom);
float ProxyGetFloat(const char *pchSection, const char *pchSettingsKey, vr::EVRSettingsError *peError);

namespace hijacker {

// constants
const char *depthVsh = R"END(#version 100
precision highp float;

attribute vec2 position;
attribute vec2 uv;
varying vec2 vUv;

void main() {
  vUv = uv;
  gl_Position = vec4(position.xy, 0., 1.);
}
)END";
const char *depthFsh = R"END(#version 100
precision highp float;

varying vec2 vUv;
uniform sampler2D tex;

void main() {
  gl_FragColor = texture2D(tex, vUv);
  // gl_FragColor = vec4(vec3(texture2D(tex, vUv).r), 0.0);
  // gl_FragColor.r += 1.0;
  // gl_FragColor = vec4(0.0);
}
)END";
const char *hlsl = R"END(
//------------------------------------------------------------//
// Structs
//------------------------------------------------------------//
// Vertex shader OUT struct
struct VS_OUTPUT
{
   float4 Position: SV_POSITION;
   float2 Uv: TEXCOORD0;
};
//------------------------------------------------------------//
// Pixel Shader OUT struct
struct PS_OUTPUT
{
	float4 Color : SV_Target0;
};
struct PS_OUTPUT_COPY
{
	float4 Color : SV_Target0;
};
//------------------------------------------------------------//

//------------------------------------------------------------//
// Textures / samplers
//------------------------------------------------------------//
//Texture
Texture2D QuadTexture : register(ps, t0);
SamplerState QuadTextureSampler {
  MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};
//------------------------------------------------------------//

VS_OUTPUT vs_main(float2 inPos : POSITION, float2 inTex : TEXCOORD0)
{
  VS_OUTPUT Output;
  Output.Position = float4(inPos, 0, 1);
  Output.Uv = inTex;
  return Output;
}

PS_OUTPUT ps_main_blit(VS_OUTPUT IN)
{
  PS_OUTPUT result;
  result.Color = QuadTexture.Sample(QuadTextureSampler, IN.Uv);
  // result.Color = float4(1, 0, 0, 1);
  return result;
}

//------------------------------------------------------------//
)END";

// dx
// client
ID3D11Device5 *hijackerDevice = nullptr;
ID3D11DeviceContext4 *hijackerContext = nullptr;
HANDLE hijackerInteropDevice = NULL;

// gl
// front
int glPhase = 0;
GLuint depthResolveTexId = 0;
GLuint depthTexId = 0;
GLuint depthResolveFbo = 0;
GLuint depthShFbo = 0;
ID3D11Texture2D *depthTex = nullptr;
GLuint depthVao = 0;
GLsizei depthSamples = 0;
GLenum depthInternalformat = 0;
GLuint depthProgram = 0;
// client
ID3D11Texture2D *clientDepthTex = nullptr;
HANDLE clientDepthEvent = NULL;
HANDLE clientDepthHandleLatched = NULL;

// dx + gl
// front
uint32_t depthWidth = 0;
uint32_t depthHeight = 0;
size_t fenceValue = 0;
HANDLE frontSharedDepthHandle = NULL;
std::vector<ID3D11Fence *> fenceCache;

void checkDetourError(const char *label, LONG error) {
  if (error) {
    getOut() << "detour error " << label << " " << (void *)error << std::endl;
    abort();
  }
}

decltype(eglGetCurrentDisplay) *EGL_GetCurrentDisplay = nullptr;
decltype(eglChooseConfig) *EGL_ChooseConfig = nullptr;
decltype(eglCreatePbufferFromClientBuffer) *EGL_CreatePbufferFromClientBuffer = nullptr;
decltype(eglBindTexImage) *EGL_BindTexImage = nullptr;
decltype(eglQueryString) *EGL_QueryString = nullptr;
decltype(eglQueryDisplayAttribEXT) *EGL_QueryDisplayAttribEXT = nullptr;
decltype(eglQueryDeviceAttribEXT) *EGL_QueryDeviceAttribEXT = nullptr;
decltype(eglGetError) *EGL_GetError = nullptr;

ID3D11Buffer *vertexBuffer = nullptr;
ID3D11Buffer *indexBuffer = nullptr;
ID3DBlob *vsBlob = nullptr;
ID3D11VertexShader *vsShader = nullptr;
ID3DBlob *psBlob = nullptr;
ID3D11PixelShader *psShader = nullptr;
ID3D11InputLayout *vertexLayout = nullptr;
ID3D11Texture2D *viewportFrontShTex = nullptr;
IDXGIResource1 *viewportFrontShDXGIRes = nullptr;
ID3D11Resource *viewportFrontShD3D11Res = nullptr;
ID3D11Texture2D *viewportBackShTex = nullptr;
IDXGIResource1 *viewportBackShDXGIRes = nullptr;
ID3D11Resource *viewportBackShD3D11Res = nullptr;
IDXGIResource1 *viewportBackShRes = nullptr;
ID3D11Fence *viewportFence = nullptr;
size_t viewportFenceValue = 0;
HANDLE viewportShHandle = NULL;
ID3D11RenderTargetView *viewportRtv = nullptr;

HANDLE latchedShEyeTex = NULL;
ID3D11ShaderResourceView *eyeShaderResourceView = nullptr;

ID3D11InfoQueue *infoQueue = nullptr;
void InfoQueueLog() {
  if (infoQueue) {
    UINT64 numStoredMessages = infoQueue->lpVtbl->GetNumStoredMessagesAllowedByRetrievalFilter(infoQueue);
    for (UINT64 i = 0; i < numStoredMessages; i++) {
      size_t messageSize = 0;
      HRESULT hr = infoQueue->lpVtbl->GetMessage(
        infoQueue,
        i,
        nullptr,
        &messageSize
      );
      if (SUCCEEDED(hr)) {
        D3D11_MESSAGE *message = (D3D11_MESSAGE *)malloc(messageSize);
        
        hr = infoQueue->lpVtbl->GetMessage(
          infoQueue,
          i,
          message,
          &messageSize
        );
        if (SUCCEEDED(hr)) {
          // if (message->Severity <= D3D11_MESSAGE_SEVERITY_WARNING) {
            getOut() << "info: " << message->Severity << " " << std::string(message->pDescription, message->DescriptionByteLength) << std::endl;
          // }
        } else {
          getOut() << "failed to get info queue message size: " << (void *)hr << std::endl;
        }
        
        free(message);
      } else {
        getOut() << "failed to get info queue message size: " << (void *)hr << std::endl;
      }
    }
    infoQueue->lpVtbl->ClearStoredMessages(infoQueue);
  }
}
void initBlitShader() {
  HRESULT hr;

  ID3D11Device *deviceBasic;
  ID3D11DeviceContext *contextBasic;
  D3D_FEATURE_LEVEL featureLevels[] = {
    D3D_FEATURE_LEVEL_11_1
  };
  hr = D3D11CreateDevice(
    // adapter, // pAdapter
    NULL, // pAdapter
    D3D_DRIVER_TYPE_HARDWARE, // DriverType
    NULL, // Software
    0, // D3D11_CREATE_DEVICE_DEBUG, // Flags
    featureLevels, // pFeatureLevels
    ARRAYSIZE(featureLevels), // FeatureLevels
    D3D11_SDK_VERSION, // SDKVersion
    &deviceBasic, // ppDevice
    NULL, // pFeatureLevel
    &contextBasic // ppImmediateContext
  );
  if (SUCCEEDED(hr)) {
    // nothing
  } else {
    getOut() << "opengl texture dx device creation failed " << (void *)hr << std::endl;
    abort();
  }
  
  hr = deviceBasic->lpVtbl->QueryInterface(deviceBasic, IID_ID3D11Device5, (void **)&hijackerDevice);
  if (SUCCEEDED(hr)) {
    // nothing
  } else {
    getOut() << "hijacker device5 query failed" << std::endl;
    // abort();
  }

  hr = contextBasic->lpVtbl->QueryInterface(contextBasic, IID_ID3D11DeviceContext4, (void **)&hijackerContext);
  if (SUCCEEDED(hr)) {
    // nothing
  } else {
    getOut() << "hijacker context4 query failed" << std::endl;
    // abort();
  }

  g_hijacker->hijackDx(contextBasic);
  
  float vertices[] = { // xyuv
    -1, -1, 0, 1,
    -1, 1, 0, 0,
    1, -1, 1, 1,
    1, 1, 1, 0
  };
  int indices[] = {
    0, 1, 2,
    3, 2, 1
  };

  {
    D3D11_BUFFER_DESC bd{};
    D3D11_SUBRESOURCE_DATA InitData{};

    // ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(float) * (2+2) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    // ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;
    
    // ID3D11Buffer* vertexBuffer = nullptr;
    hr = hijackerDevice->lpVtbl->CreateBuffer(hijackerDevice, &bd, &InitData, &vertexBuffer);
    if(FAILED(hr)) {
      getOut() << "Unable to create vertex buffer: " << (void *)hr << std::endl;
      abort();   
    }
    // m_VB.Set(vertexBuffer);
  }
  // getOut() << "init render 2" << std::endl;
  {
    D3D11_BUFFER_DESC bd{};
    D3D11_SUBRESOURCE_DATA InitData{};

    // ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(int) * 6;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    // ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;

    // ID3D11Buffer* indexBuffer = nullptr;
    hr = hijackerDevice->lpVtbl->CreateBuffer(hijackerDevice, &bd, &InitData, &indexBuffer);
    if(FAILED(hr)) {
      getOut() << "Unable to create index buffer: " << (void *)hr << std::endl;
      abort();
    }
    // m_IB.Set(indexBuffer);
  }
  {
    ID3DBlob *errorBlob = nullptr;
    hr = D3DCompile(
      hlsl,
      strlen(hlsl),
      "vs.hlsl",
      nullptr,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      "vs_main",
      "vs_5_0",
      D3DCOMPILE_ENABLE_STRICTNESS,
      0,
      &vsBlob,
      &errorBlob
    );
    if (FAILED(hr)) {
      if (errorBlob != nullptr) {
        getOut() << "vs compilation failed: " << (char*)errorBlob->lpVtbl->GetBufferPointer(errorBlob) << std::endl;
        abort();
      }
    }
    ID3D11ClassLinkage *linkage = nullptr;
    hr = hijackerDevice->lpVtbl->CreateVertexShader(hijackerDevice, vsBlob->lpVtbl->GetBufferPointer(vsBlob), vsBlob->lpVtbl->GetBufferSize(vsBlob), linkage, &vsShader);
    if (FAILED(hr)) {
      getOut() << "vs create failed: " << (void *)hr << std::endl;
      abort();
    }
  }
  // getOut() << "init render 5" << std::endl;
  {
    ID3DBlob *errorBlob = nullptr;
    hr = D3DCompile(
      hlsl,
      strlen(hlsl),
      "ps.hlsl",
      nullptr,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      "ps_main_blit",
      "ps_5_0",
      D3DCOMPILE_ENABLE_STRICTNESS,
      0,
      &psBlob,
      &errorBlob
    );
    // getOut() << "init render 6 1" << std::endl;
    if (FAILED(hr)) {
      if (errorBlob != nullptr) {
        getOut() << "ps compilation failed: " << (char*)errorBlob->lpVtbl->GetBufferPointer(errorBlob) << std::endl;
        abort();
      }
    }
    
    // getOut() << "init render 7 1" << std::endl;

    ID3D11ClassLinkage *linkage = nullptr;
    hr = hijackerDevice->lpVtbl->CreatePixelShader(hijackerDevice, psBlob->lpVtbl->GetBufferPointer(psBlob), psBlob->lpVtbl->GetBufferSize(psBlob), linkage, &psShader);
    if (FAILED(hr)) {
      getOut() << "ps create failed: " << (void *)hr << std::endl;
      abort();
    }
  }
  {
    D3D11_INPUT_ELEMENT_DESC PositionTextureVertexLayout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(PositionTextureVertexLayout);
    hr = hijackerDevice->lpVtbl->CreateInputLayout(hijackerDevice, PositionTextureVertexLayout, numElements, vsBlob->lpVtbl->GetBufferPointer(vsBlob), vsBlob->lpVtbl->GetBufferSize(vsBlob), &vertexLayout);
    if (FAILED(hr)) {
      getOut() << "vertex layout create failed: " << (void *)hr << std::endl;
      abort();
    }
  }
  {
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = viewportWidth;
    desc.Height = viewportHeight;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    // desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

    hr = hijackerDevice->lpVtbl->CreateTexture2D(
      hijackerDevice,
      &desc,
      NULL,
      &viewportFrontShTex
    );
    if (FAILED(hr)) {
      getOut() << "failed to create viewport texture: " << (void *)hr << std::endl;
      abort();
    }

    {
      hr = viewportFrontShTex->lpVtbl->QueryInterface(viewportFrontShTex, IID_IDXGIResource1, (void **)&viewportFrontShDXGIRes);
      if (FAILED(hr)) {
        getOut() << "failed to query viewport front dxgi resource: " << (void *)hr << std::endl;
        abort();
      }
      
      hr = viewportFrontShDXGIRes->lpVtbl->GetSharedHandle(viewportFrontShDXGIRes, &viewportShHandle);
      if (FAILED(hr)) {
        getOut() << "failed to query viewport front shared handle: " << (void *)hr << std::endl;
        abort();
      }
      
      hr = viewportFrontShTex->lpVtbl->QueryInterface(viewportFrontShTex, IID_ID3D11Resource, (void **)&viewportFrontShD3D11Res);
      if (FAILED(hr)) {
        getOut() << "failed to query viewport front d3d11 resource: " << (void *)hr << std::endl;
        abort();
      }
      
      hr = hijackerDevice->lpVtbl->CreateFence(
        hijackerDevice,
        0, // value
        D3D11_FENCE_FLAG_NONE, // flags
        IID_ID3D11Fence, // interface
        (void **)&viewportFence // out
      );
      if (FAILED(hr)) {
        getOut() << "failed to create viewport fence" << std::endl;
        abort();
      }
    }

    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
    renderTargetViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc.Texture2D.MipSlice = 0;
    hr = hijackerDevice->lpVtbl->CreateRenderTargetView(
      hijackerDevice,
      viewportFrontShD3D11Res,
      &renderTargetViewDesc,
      &viewportRtv
    );
    if (FAILED(hr)) {
      getOut() << "failed to create viewport rtv: " << (void *)hr << std::endl;
      abort();
    }
  }

  deviceBasic->lpVtbl->Release(deviceBasic);
  contextBasic->lpVtbl->Release(contextBasic);

  // InfoQueueLog();
}
void blitEyeView(ID3D11ShaderResourceView *eyeShaderResourceView) {
  // set new
  UINT stride = sizeof(float) * 4; // xyuv
  UINT offset = 0;
  hijackerContext->lpVtbl->IASetPrimitiveTopology(hijackerContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  hijackerContext->lpVtbl->IASetInputLayout(hijackerContext, vertexLayout);
  hijackerContext->lpVtbl->IASetVertexBuffers(hijackerContext, 0, 1, &vertexBuffer, &stride, &offset);
  hijackerContext->lpVtbl->IASetIndexBuffer(hijackerContext, indexBuffer, DXGI_FORMAT_R32_UINT, 0);
  hijackerContext->lpVtbl->VSSetShader(hijackerContext, vsShader, nullptr, 0);
  hijackerContext->lpVtbl->PSSetShader(hijackerContext, psShader, nullptr, 0);

  ID3D11ShaderResourceView *localShaderResourceViews[1] = {
    eyeShaderResourceView,
  };
  hijackerContext->lpVtbl->PSSetShaderResources(hijackerContext, 0, ARRAYSIZE(localShaderResourceViews), localShaderResourceViews);

  ID3D11RenderTargetView *localRenderTargetViews[1] = {
    viewportRtv,
  };
  hijackerContext->lpVtbl->OMSetRenderTargets(hijackerContext, ARRAYSIZE(localRenderTargetViews), localRenderTargetViews, nullptr);

  D3D11_VIEWPORT viewport{
    0, // TopLeftX,
    0, // TopLeftY,
    viewportWidth, // Width,
    viewportHeight, // Height,
    0, // MinDepth,
    1 // MaxDepth
  };
  hijackerContext->lpVtbl->RSSetViewports(hijackerContext, 1, &viewport);

  // draw
  hijackerContext->lpVtbl->DrawIndexed(hijackerContext, 6, 0, 0);

  ++viewportFenceValue;
  hijackerContext->lpVtbl->Signal(hijackerContext, viewportFence, viewportFenceValue);
}

ID3D11Resource *backbufferShRes = nullptr;
HANDLE backbufferShHandle = NULL;
D3D11_TEXTURE2D_DESC backbufferDesc{};
template<typename T>
void presentSwapChain(T *swapChain) {
  // getOut() << "present swap chain 1" << std::endl;

  DXGI_SWAP_CHAIN_DESC desc;
  swapChain->lpVtbl->GetDesc(swapChain, &desc);
  const DXGI_MODE_DESC &modeDesc = desc.BufferDesc;
  if (modeDesc.Width >= 500 && modeDesc.Height >= 500) {
    if (!hijackerDevice) {
      initBlitShader();
    }
  
    ID3D11Resource *res;
    HRESULT hr = swapChain->lpVtbl->GetBuffer(swapChain, 0, IID_ID3D11Resource, (void **)&res);
    if (FAILED(hr)) {
      getOut() << "get_dxgi_backbuffer: GetBuffer failed" << std::endl;
    }
    
    ID3D11Texture2D *tex;
    hr = res->lpVtbl->QueryInterface(res, IID_ID3D11Texture2D, (void **)&tex);
    if (FAILED(hr)) {
      getOut() << "failed to query backbuffer texture: " << (void *)hr << std::endl;
      abort();
    }

    ID3D11Device *device;
    tex->lpVtbl->GetDevice(tex, &device);

    /* ID3D11Device5 *device5;
    hr = device->lpVtbl->QueryInterface(device, IID_ID3D11Device5, (void **)&device5);
    if (FAILED(hr)) {
      getOut() << "failed to query backbuffer device5 : " << (void *)hr << std::endl;
      abort();
    } */
    
    hr = device->lpVtbl->QueryInterface(device, IID_ID3D11InfoQueue, (void **)&infoQueue);
    if (SUCCEEDED(hr)) {
      infoQueue->lpVtbl->PushEmptyStorageFilter(infoQueue);
      InfoQueueLog();
    } else {
      getOut() << "info queue query failed" << std::endl;
      // abort();
    }
    
    ID3D11DeviceContext *context;
    device->lpVtbl->GetImmediateContext(device, &context);
    
    ID3D11DeviceContext4 *context4;
    hr = context->lpVtbl->QueryInterface(context, IID_ID3D11DeviceContext4, (void **)&context4);
    if (FAILED(hr)) {
      getOut() << "failed to query backbuffer context4: " << (void *)hr << std::endl;
      abort();
    }

    D3D11_TEXTURE2D_DESC desc;
    tex->lpVtbl->GetDesc(tex, &desc);
    
    if (!backbufferShHandle || backbufferDesc.Width != desc.Width || backbufferDesc.Height != desc.Height) {
      backbufferDesc = desc;

      desc.MipLevels = 1;
      desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
      desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_SHARED;

      ID3D11Texture2D *backbufferShTex;
      hr = device->lpVtbl->CreateTexture2D(
        device,
        &desc,
        NULL,
        &backbufferShTex
      );
      if (FAILED(hr)) {
        getOut() << "failed to create backbuffer texture: " << (void *)hr << std::endl;
        abort();
      }
      
      hr = backbufferShTex->lpVtbl->QueryInterface(backbufferShTex, IID_ID3D11Resource, (void **)&backbufferShRes);
      if (FAILED(hr)) {
        getOut() << "failed to query backbuffer d3d11 resource: " << (void *)hr << std::endl;
        abort();
      }
      
      IDXGIResource1 *dxgiResource;
      hr = backbufferShTex->lpVtbl->QueryInterface(backbufferShTex, IID_IDXGIResource1, (void **)&dxgiResource);
      if (FAILED(hr)) {
        getOut() << "failed to query backbuffer dxgi resource: " << (void *)hr << std::endl;
        abort();
      }
      
      hr = dxgiResource->lpVtbl->GetSharedHandle(dxgiResource, &backbufferShHandle);
      if (FAILED(hr)) {
        getOut() << "failed to query backbuffer shared handle: " << (void *)hr << std::endl;
        abort();
      }
    }

    context->lpVtbl->CopyResource(
      context,
      backbufferShRes,
      res
    );

    g_hijacker->fnp.call<
      kHijacker_RegisterSurface,
      int,
      HANDLE
    >(backbufferShHandle);
    
    HANDLE shEyeTexHandle = g_hijacker->fnp.call<
      kHijacker_GetSharedEyeTexture,
      HANDLE
    >();
    if (latchedShEyeTex != shEyeTexHandle) {
      if (eyeShaderResourceView) {
        eyeShaderResourceView->lpVtbl->Release(eyeShaderResourceView);
        eyeShaderResourceView = nullptr;
      }
      latchedShEyeTex = shEyeTexHandle;
      
      ID3D11Resource *shEyeTexResource;
      HRESULT hr = hijackerDevice->lpVtbl->OpenSharedResource(hijackerDevice, shEyeTexHandle, IID_ID3D11Resource, (void**)&shEyeTexResource);
      if (FAILED(hr)) {
        getOut() << "failed to unpack shared eye texture handle: " << (void *)hr << " " << (void *)shEyeTexHandle << std::endl;
        abort();
      }

      D3D11_SHADER_RESOURCE_VIEW_DESC eyeSrv{};
      eyeSrv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      eyeSrv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      eyeSrv.Texture2D.MostDetailedMip = 0;
      eyeSrv.Texture2D.MipLevels = 1;
      
      hr = hijackerDevice->lpVtbl->CreateShaderResourceView(
        hijackerDevice,
        shEyeTexResource,
        &eyeSrv,
        &eyeShaderResourceView
      );
      if (FAILED(hr)) {
        getOut() << "failed to eye shader resource view: " << (void *)hr << std::endl;
        abort();
      }

      shEyeTexResource->lpVtbl->Release(shEyeTexResource);
    }
    if (eyeShaderResourceView) {
      InfoQueueLog();

      blitEyeView(eyeShaderResourceView);
      context4->lpVtbl->Wait(context4, viewportFence, viewportFenceValue);
      
      if (!viewportBackShD3D11Res) {
        hr = device->lpVtbl->OpenSharedResource(device, viewportShHandle, IID_IDXGIResource1, (void**)&viewportBackShDXGIRes);
        if (FAILED(hr)) {
          getOut() << "failed to unpack viewport share texture handle: " << (void *)hr << " " << (void *)viewportShHandle << std::endl;
          abort();
        }
        
        hr = viewportBackShDXGIRes->lpVtbl->QueryInterface(viewportBackShDXGIRes, IID_ID3D11Texture2D, (void **)&viewportBackShTex);
        if (FAILED(hr)) {
          getOut() << "failed to get viewport back texture: " << (void *)hr << std::endl;
          abort();
        }
        
        hr = viewportBackShDXGIRes->lpVtbl->QueryInterface(viewportBackShDXGIRes, IID_ID3D11Resource, (void **)&viewportBackShD3D11Res);
        if (FAILED(hr)) {
          getOut() << "failed to get viewport back resource: " << (void *)hr << std::endl;
          abort();
        }
      }

      /* D3D11_TEXTURE2D_DESC desc;
      viewportBackShTex->lpVtbl->GetDesc(viewportBackShTex, &desc);
      
      getOut() << "copy region " <<
        desc.Width << " " << desc.Height << " " <<
        desc.MipLevels << " " << desc.ArraySize << " " <<
        desc.SampleDesc.Count << " " << desc.SampleDesc.Quality << " " <<
        desc.Format << " " <<
        desc.Usage << " " << desc.BindFlags << " " << desc.CPUAccessFlags << " " << desc.MiscFlags << " " <<
        std::endl; */

      /* D3D11_BOX srcBox{};
      srcBox.left = 0;
      srcBox.right = viewportWidth;
      srcBox.top = 0;
      srcBox.bottom = viewportHeight;
      srcBox.front = 0;
      srcBox.back = 1; */
      context->lpVtbl->CopySubresourceRegion(
        context,
        res, // dst
        0, // dst sub
        0, // dst x
        desc.Height - viewportHeight, // dst y
        0, // dst z
        viewportBackShD3D11Res, // src
        0, // src sub
        NULL
      );
    }

    res->lpVtbl->Release(res);
    tex->lpVtbl->Release(tex);
    device->lpVtbl->Release(device);
    // device5->lpVtbl->Release(device5);
    context->lpVtbl->Release(context);
    context4->lpVtbl->Release(context4);
  }
}
HRESULT (STDMETHODCALLTYPE *RealPresent)(
  IDXGISwapChain *This,
  UINT SyncInterval,
  UINT Flags
) = nullptr;
HRESULT STDMETHODCALLTYPE MinePresent(
  IDXGISwapChain *This,
  UINT SyncInterval,
  UINT Flags
) {
  // getOut() << "present0" << std::endl;
  if (isChrome) {
    presentSwapChain(This);
  }
  return RealPresent(This, SyncInterval, Flags);
}
HRESULT (STDMETHODCALLTYPE *RealPresent1)(
  IDXGISwapChain1 *This,
  UINT                          SyncInterval,
  UINT                          PresentFlags,
  const DXGI_PRESENT_PARAMETERS *pPresentParameters
) = nullptr;
HRESULT STDMETHODCALLTYPE MinePresent1(
  IDXGISwapChain1 *This,
  UINT                          SyncInterval,
  UINT                          PresentFlags,
  const DXGI_PRESENT_PARAMETERS *pPresentParameters
) {
  if (isChrome) {
    presentSwapChain(This);
  }
  if (pPresentParameters) {
    getOut() << "present1 " << pPresentParameters->DirtyRectsCount << std::endl;
    
    // DXGI_PRESENT_PARAMETERS presentParameters = *pPresentParameters;
    // presentParameters.DirtyRectsCount = 0;
    // presentParameters.pDirtyRects = nullptr;
    return RealPresent1(This, SyncInterval, PresentFlags, pPresentParameters);
  } else {
    getOut() << "present1 blank" << std::endl;
    return RealPresent1(This, SyncInterval, PresentFlags, pPresentParameters);
  }
}
/* HWND (STDMETHODCALLTYPE *RealCreateWindowExA)( 
  DWORD     dwExStyle,
  LPCSTR    lpClassName,
  LPCSTR    lpWindowName,
  DWORD     dwStyle,
  int       X,
  int       Y,
  int       nWidth,
  int       nHeight,
  HWND      hWndParent,
  HMENU     hMenu,
  HINSTANCE hInstance,
  LPVOID    lpParam
) = nullptr;
HWND STDMETHODCALLTYPE MineCreateWindowExA(
  DWORD     dwExStyle,
  LPCSTR    lpClassName,
  LPCSTR    lpWindowName,
  DWORD     dwStyle,
  int       X,
  int       Y,
  int       nWidth,
  int       nHeight,
  HWND      hWndParent,
  HMENU     hMenu,
  HINSTANCE hInstance,
  LPVOID    lpParam
) {
  getOut() << "RealCreateWindowExA " << (void *)dwExStyle << " " << (void *)dwStyle << std::endl;
  // dwExStyle |= WS_EX_TOOLWINDOW;
  return RealCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}
HWND (STDMETHODCALLTYPE *RealCreateWindowExW)( 
  DWORD     dwExStyle,
  LPCWSTR   lpClassName,
  LPCWSTR   lpWindowName,
  DWORD     dwStyle,
  int       X,
  int       Y,
  int       nWidth,
  int       nHeight,
  HWND      hWndParent,
  HMENU     hMenu,
  HINSTANCE hInstance,
  LPVOID    lpParam
) = nullptr;
HWND STDMETHODCALLTYPE MineCreateWindowExW(
  DWORD     dwExStyle,
  LPCWSTR   lpClassName,
  LPCWSTR   lpWindowName,
  DWORD     dwStyle,
  int       X,
  int       Y,
  int       nWidth,
  int       nHeight,
  HWND      hWndParent,
  HMENU     hMenu,
  HINSTANCE hInstance,
  LPVOID    lpParam
) {
  getOut() << "RealCreateWindowExW " << (void *)dwExStyle << " " << (void *)dwStyle << std::endl;
  // dwExStyle |= WS_EX_TOOLWINDOW;
  return RealCreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
} */
/* HRESULT (STDMETHODCALLTYPE *RealCreateTargetForHwnd)(
  IDCompositionDesktopDevice *This,
  HWND                hwnd,
  BOOL                topmost,
  IDCompositionTarget **target
) = nullptr;
HRESULT STDMETHODCALLTYPE MineCreateTargetForHwnd(
  IDCompositionDesktopDevice *This,
  HWND                hwnd,
  BOOL                topmost,
  IDCompositionTarget **target
) {
  getOut() << "RealCreateTargetForHwnd " << (void *)hwnd << std::endl;
  return RealCreateTargetForHwnd(This, hwnd, topmost, target);
} */
/* HRESULT (STDMETHODCALLTYPE *RealD3D11CreateDeviceAndSwapChain)(
  IDXGIAdapter               *pAdapter,
  D3D_DRIVER_TYPE            DriverType,
  HMODULE                    Software,
  UINT                       Flags,
  const D3D_FEATURE_LEVEL    *pFeatureLevels,
  UINT                       FeatureLevels,
  UINT                       SDKVersion,
  const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
  IDXGISwapChain             **ppSwapChain,
  ID3D11Device               **ppDevice,
  D3D_FEATURE_LEVEL          *pFeatureLevel,
  ID3D11DeviceContext        **ppImmediateContext
) = nullptr;
HRESULT STDMETHODCALLTYPE MineD3D11CreateDeviceAndSwapChain(
  IDXGIAdapter               *pAdapter,
  D3D_DRIVER_TYPE            DriverType,
  HMODULE                    Software,
  UINT                       Flags,
  const D3D_FEATURE_LEVEL    *pFeatureLevels,
  UINT                       FeatureLevels,
  UINT                       SDKVersion,
  const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
  IDXGISwapChain             **ppSwapChain,
  ID3D11Device               **ppDevice,
  D3D_FEATURE_LEVEL          *pFeatureLevel,
  ID3D11DeviceContext        **ppImmediateContext
) {
  getOut() << "create device and swap chain" << std::endl;
  return RealD3D11CreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
}
HRESULT (STDMETHODCALLTYPE *RealD3D11CreateDevice)(
  IDXGIAdapter            *pAdapter,
  D3D_DRIVER_TYPE         DriverType,
  HMODULE                 Software,
  UINT                    Flags,
  const D3D_FEATURE_LEVEL *pFeatureLevels,
  UINT                    FeatureLevels,
  UINT                    SDKVersion,
  ID3D11Device            **ppDevice,
  D3D_FEATURE_LEVEL       *pFeatureLevel,
  ID3D11DeviceContext     **ppImmediateContext
) = nullptr;
HRESULT STDMETHODCALLTYPE MineD3D11CreateDevice(
  IDXGIAdapter            *pAdapter,
  D3D_DRIVER_TYPE         DriverType,
  HMODULE                 Software,
  UINT                    Flags,
  const D3D_FEATURE_LEVEL *pFeatureLevels,
  UINT                    FeatureLevels,
  UINT                    SDKVersion,
  ID3D11Device            **ppDevice,
  D3D_FEATURE_LEVEL       *pFeatureLevel,
  ID3D11DeviceContext     **ppImmediateContext
) {
  getOut() << "create device" << std::endl;
  return RealD3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
} */

void (STDMETHODCALLTYPE *RealOMGetRenderTargets)(
  ID3D11DeviceContext *This,
  UINT                   NumViews,
  ID3D11RenderTargetView **ppRenderTargetViews,
  ID3D11DepthStencilView **ppDepthStencilView
) = nullptr;
void STDMETHODCALLTYPE MineOMGetRenderTargets(
  ID3D11DeviceContext *This,
  UINT                   NumViews,
  ID3D11RenderTargetView **ppRenderTargetViews,
  ID3D11DepthStencilView **ppDepthStencilView
) {
  RealOMGetRenderTargets(This, NumViews, ppRenderTargetViews, ppDepthStencilView);
}
void (STDMETHODCALLTYPE *RealOMSetRenderTargets)(
  ID3D11DeviceContext *This,
  UINT                   NumViews,
  ID3D11RenderTargetView * const *ppRenderTargetViews,
  ID3D11DepthStencilView *pDepthStencilView
) = nullptr;
void STDMETHODCALLTYPE MineOMSetRenderTargets(
  ID3D11DeviceContext *This,
  UINT                   NumViews,
  ID3D11RenderTargetView * const *ppRenderTargetViews,
  ID3D11DepthStencilView *pDepthStencilView
) {
  // getOut() << "RealOMSetRenderTargets" << std::endl;
  RealOMSetRenderTargets(This, NumViews, ppRenderTargetViews, pDepthStencilView);
}
void (STDMETHODCALLTYPE *RealOMSetDepthStencilState)(
  ID3D11DeviceContext *This,
  ID3D11DepthStencilState *pDepthStencilState,
  UINT                    StencilRef
) = nullptr;
void STDMETHODCALLTYPE MineOMSetDepthStencilState(
  ID3D11DeviceContext *This,
  ID3D11DepthStencilState *pDepthStencilState,
  UINT                    StencilRef
) {
  /* getOut() << "set depth stencil state " <<
    D3D11_COMPARISON_NEVER << " " <<
    D3D11_COMPARISON_LESS << " " <<
    D3D11_COMPARISON_EQUAL << " " <<
    D3D11_COMPARISON_LESS_EQUAL << " " <<
    D3D11_COMPARISON_GREATER << " " <<
    D3D11_COMPARISON_NOT_EQUAL << " " <<
    D3D11_COMPARISON_GREATER_EQUAL << " " <<
    D3D11_COMPARISON_ALWAYS << " " <<
    std::endl; */
  /* if (pDepthStencilState) {
    D3D11_DEPTH_STENCIL_DESC desc;
    pDepthStencilState->lpVtbl->GetDesc(pDepthStencilState, &desc);

    getOut() << "depth state " <<
      // (void *)This << " " <<
      desc.DepthEnable << " " <<
      desc.DepthWriteMask << " " <<
      desc.DepthFunc << " " <<
      desc.StencilEnable << " " <<
      (void *)desc.StencilReadMask << " " <<
      (void *)desc.StencilWriteMask << " " <<
      desc.FrontFace.StencilFailOp << " " << desc.FrontFace.StencilDepthFailOp << " " << desc.FrontFace.StencilPassOp << " " << desc.FrontFace.StencilFunc << " " <<
      desc.BackFace.StencilFailOp << " " << desc.BackFace.StencilDepthFailOp << " " << desc.BackFace.StencilPassOp << " " << desc.BackFace.StencilFunc << " " <<
      std::endl;
  } */
  RealOMSetDepthStencilState(This, pDepthStencilState, StencilRef);
}
void (STDMETHODCALLTYPE *RealDraw)(
  ID3D11DeviceContext *This,
  UINT VertexCount,
  UINT StartVertexLocation
) = nullptr;
void STDMETHODCALLTYPE MineDraw(
  ID3D11DeviceContext *This,
  UINT VertexCount,
  UINT StartVertexLocation
) {
  TRACE("Hijack", [&]() { getOut() << "Draw " << VertexCount << std::endl; });
  RealDraw(This, VertexCount, StartVertexLocation);
}
void (STDMETHODCALLTYPE *RealDrawAuto)(
  ID3D11DeviceContext *This
) = nullptr;
void STDMETHODCALLTYPE MineDrawAuto(
  ID3D11DeviceContext *This
) {
  TRACE("Hijack", [&]() { getOut() << "DrawAuto" << std::endl; });
  RealDrawAuto(This);
}
void (STDMETHODCALLTYPE *RealDrawIndexed)(
  ID3D11DeviceContext *This,
  UINT IndexCount,
  UINT StartIndexLocation,
  INT  BaseVertexLocation
) = nullptr;
void STDMETHODCALLTYPE MineDrawIndexed(
  ID3D11DeviceContext *This,
  UINT IndexCount,
  UINT StartIndexLocation,
  INT  BaseVertexLocation
) {
  TRACE("Hijack", [&]() { getOut() << "DrawIndexed " << IndexCount << std::endl; });
  RealDrawIndexed(This, IndexCount, StartIndexLocation, BaseVertexLocation);
}
void (STDMETHODCALLTYPE *RealDrawIndexedInstanced)(
  ID3D11DeviceContext *This,
  UINT IndexCountPerInstance,
  UINT InstanceCount,
  UINT StartIndexLocation,
  INT  BaseVertexLocation,
  UINT StartInstanceLocation
) = nullptr;
void STDMETHODCALLTYPE MineDrawIndexedInstanced(
  ID3D11DeviceContext *This,
  UINT IndexCountPerInstance,
  UINT InstanceCount,
  UINT StartIndexLocation,
  INT  BaseVertexLocation,
  UINT StartInstanceLocation
) {
  TRACE("Hijack", [&]() { getOut() << "DrawIndexedInstanced " << IndexCountPerInstance << " " << InstanceCount << std::endl; });
  RealDrawIndexedInstanced(This, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}
void (STDMETHODCALLTYPE *RealDrawIndexedInstancedIndirect)(
  ID3D11DeviceContext *This,
  ID3D11Buffer *pBufferForArgs,
  UINT         AlignedByteOffsetForArgs
) = nullptr;
void STDMETHODCALLTYPE MineDrawIndexedInstancedIndirect(
  ID3D11DeviceContext *This,
  ID3D11Buffer *pBufferForArgs,
  UINT         AlignedByteOffsetForArgs
) {
  TRACE("Hijack", [&]() { getOut() << "DrawIndexedInstancedIndirect" << std::endl; });
  RealDrawIndexedInstancedIndirect(This, pBufferForArgs, AlignedByteOffsetForArgs);
}
void (STDMETHODCALLTYPE *RealDrawInstanced)(
  ID3D11DeviceContext *This,
  UINT VertexCountPerInstance,
  UINT InstanceCount,
  UINT StartVertexLocation,
  UINT StartInstanceLocation
) = nullptr;
void STDMETHODCALLTYPE MineDrawInstanced(
  ID3D11DeviceContext *This,
  UINT VertexCountPerInstance,
  UINT InstanceCount,
  UINT StartVertexLocation,
  UINT StartInstanceLocation
) {
  TRACE("Hijack", [&]() { getOut() << "DrawInstanced " << VertexCountPerInstance << " " << InstanceCount << std::endl; });
  RealDrawInstanced(This, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}
void (STDMETHODCALLTYPE *RealDrawInstancedIndirect)(
  ID3D11DeviceContext *This,
  ID3D11Buffer *pBufferForArgs,
  UINT         AlignedByteOffsetForArgs
) = nullptr;
void STDMETHODCALLTYPE MineDrawInstancedIndirect(
  ID3D11DeviceContext *This,
  ID3D11Buffer *pBufferForArgs,
  UINT         AlignedByteOffsetForArgs
) {
  TRACE("Hijack", [&]() { getOut() << "DrawInstancedIndirect" << std::endl; });
  RealDrawInstancedIndirect(This, pBufferForArgs, AlignedByteOffsetForArgs);
}
HRESULT (STDMETHODCALLTYPE *RealCreateShaderResourceView)(
  ID3D11Device *This,
  ID3D11Resource                        *pResource,
  const D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc,
  ID3D11ShaderResourceView              **ppSRView
) = nullptr;
HRESULT STDMETHODCALLTYPE MineCreateShaderResourceView(
  ID3D11Device *This,
  ID3D11Resource                        *pResource,
  const D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc,
  ID3D11ShaderResourceView              **ppSRView
) {
  TRACE("Hijack", [&]() { getOut() << "CreateShaderResourceView" << std::endl; });
  D3D11_RESOURCE_DIMENSION dim;
  pResource->lpVtbl->GetType(pResource, &dim);
  if (dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
    ID3D11Texture2D *tex;
    HRESULT hr = pResource->lpVtbl->QueryInterface(pResource, IID_ID3D11Texture2D, (void **)&tex);
    if (FAILED(hr)) {
      getOut() << "failed to get hijack shader resource view texture: " << (void *)hr << std::endl;
      abort();
    }

    D3D11_TEXTURE2D_DESC desc;
    tex->lpVtbl->GetDesc(tex, &desc);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = *pDesc;
    bool changed = false;
    if (srvDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM && desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM) {
      getOut() << "bad shader resource view " << desc.Width << " " << desc.Height << " " << srvDesc.Texture2D.MostDetailedMip << " " << srvDesc.Texture2D.MipLevels << std::endl;
      srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      // srvDesc.Texture2D.MostDetailedMip = 0;
      // srvDesc.Texture2D.MipLevels = 1;
      changed = true;
    }
    
    tex->lpVtbl->Release(tex);
    
    *ppSRView = nullptr;
    hr = RealCreateShaderResourceView(This, pResource, &srvDesc, ppSRView);
    if (changed) {
      getOut() << "bad shader resource view result " << (void *)pResource << " " << (void *)*ppSRView << std::endl;
    }
    return hr;
  } else {
    return RealCreateShaderResourceView(This, pResource, pDesc, ppSRView);
  }
}
HRESULT (STDMETHODCALLTYPE *RealCreateRenderTargetView)(
  ID3D11Device *This,
  ID3D11Resource                      *pResource,
  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
  ID3D11RenderTargetView              **ppRTView
) = nullptr;
HRESULT STDMETHODCALLTYPE MineCreateRenderTargetView(
  ID3D11Device *This,
  ID3D11Resource                      *pResource,
  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
  ID3D11RenderTargetView              **ppRTView
) {
  TRACE("Hijack", [&]() { getOut() << "CreateRenderTargetView" << std::endl; });
  D3D11_RESOURCE_DIMENSION dim;
  pResource->lpVtbl->GetType(pResource, &dim);
  if (dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
    ID3D11Texture2D *tex;
    HRESULT hr = pResource->lpVtbl->QueryInterface(pResource, IID_ID3D11Texture2D, (void **)&tex);
    if (FAILED(hr)) {
      getOut() << "failed to get hijack render target view texture: " << (void *)hr << std::endl;
      abort();
    }
    
    D3D11_TEXTURE2D_DESC desc;
    tex->lpVtbl->GetDesc(tex, &desc);

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = *pDesc;
    bool changed = false;
    if (rtvDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM && desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM) {
      getOut() << "bad render target view " << desc.Width << " " << desc.Height << " " << rtvDesc.Texture2D.MipSlice << std::endl;
      rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      // rtvDesc.Texture2D.MipSlice = 0;
      changed = true;
    }
    
    tex->lpVtbl->Release(tex);
    
    *ppRTView = nullptr;
    hr = RealCreateRenderTargetView(This, pResource, &rtvDesc, ppRTView);
    if (changed) {
      getOut() << "bad render target view result " << desc.Width << " " << desc.Height << " " << rtvDesc.Texture2D.MipSlice << " " << (void *)pResource << " " << (void *)*ppRTView << std::endl;
    }
    return hr;
  } else {
    return RealCreateRenderTargetView(This, pResource, pDesc, ppRTView);
  }
}
void (STDMETHODCALLTYPE *RealPSSetShaderResources)(
  ID3D11DeviceContext *This,
  UINT                     StartSlot,
  UINT                     NumViews,
  ID3D11ShaderResourceView * const *ppShaderResourceViews
) = nullptr;
void STDMETHODCALLTYPE MinePSSetShaderResources(
  ID3D11DeviceContext *This,
  UINT                     StartSlot,
  UINT                     NumViews,
  ID3D11ShaderResourceView * const *ppShaderResourceViews
) {
  TRACE("Hijack", [&]() { getOut() << "RealPSSetShaderResources" << std::endl; });
  /* for (UINT i = 0; i < NumViews; i++) {
    ID3D11ShaderResourceView *srv = ppShaderResourceViews[i];
    
    if (srv) {
      ID3D11Texture2D *depthTex = nullptr;
      ID3D11Resource *depthTexResource = nullptr;
      srv->lpVtbl->GetResource(srv, &depthTexResource);
      HRESULT hr = depthTexResource->lpVtbl->QueryInterface(depthTexResource, IID_ID3D11Texture2D, (void **)&depthTex);
      if (FAILED(hr)) {
        getOut() << "failed to get srv texture resource: " << (void *)hr << std::endl;
        abort();
      }
      
      D3D11_TEXTURE2D_DESC desc;
      depthTex->lpVtbl->GetDesc(depthTex, &desc);
      
      getOut() << "bind shader resource " << i << " " << NumViews << " " << (void *)srv << " " << desc.Width << " " << desc.Height << " " << desc.Format << std::endl;
      
      depthTex->lpVtbl->Release(depthTex);
      depthTexResource->lpVtbl->Release(depthTexResource);
    }
  } */
  return RealPSSetShaderResources(This, StartSlot, NumViews, ppShaderResourceViews);
}
HRESULT (STDMETHODCALLTYPE *RealCreateDepthStencilView)(
  ID3D11Device *This,
  ID3D11Resource                      *pResource,
  const D3D11_DEPTH_STENCIL_VIEW_DESC *pDesc,
  ID3D11DepthStencilView              **ppDepthStencilView
) = nullptr;
HRESULT STDMETHODCALLTYPE MineCreateDepthStencilView(
  ID3D11Device *This,
  ID3D11Resource                      *pResource,
  const D3D11_DEPTH_STENCIL_VIEW_DESC *pDesc,
  ID3D11DepthStencilView              **ppDepthStencilView
) {
  ID3D11Texture2D *depthTex;
  HRESULT hr = pResource->lpVtbl->QueryInterface(pResource, IID_ID3D11Texture2D, (void **)&depthTex);
  if (FAILED(hr)) {
    getOut() << "failed to get hijack depth texture resource: " << (void *)hr << std::endl;
    abort();
  }
  
  D3D11_TEXTURE2D_DESC desc;
  depthTex->lpVtbl->GetDesc(depthTex, &desc);
  
  getOut() << "CreateDepthStencilView " << (void *)pResource <<
    desc.Width << " " << desc.Height << " " <<
    desc.MipLevels << " " << desc.ArraySize << " " <<
    desc.SampleDesc.Count << " " << desc.SampleDesc.Quality << " " <<
    desc.Format << " " <<
    desc.Usage << " " << desc.BindFlags << " " << desc.CPUAccessFlags << " " << desc.MiscFlags << " " <<
    std::endl;
  
  depthTex->lpVtbl->Release(depthTex);

  return RealCreateDepthStencilView(This, pResource, pDesc, ppDepthStencilView);
}
void (STDMETHODCALLTYPE *RealClearRenderTargetView)(
  ID3D11DeviceContext *This,
  ID3D11RenderTargetView *pRenderTargetView,
  const FLOAT         ColorRGBA[4]
) = nullptr;
void STDMETHODCALLTYPE MineClearRenderTargetView(
  ID3D11DeviceContext *This,
  ID3D11RenderTargetView *pRenderTargetView,
  const FLOAT         ColorRGBA[4]
) {
  TRACE("Hijack", [&]() { getOut() << "ClearRenderTargetView" << std::endl; });
  // getOut() << "ClearRenderTargetView " << (void *)pRenderTargetView << std::endl;

  /* {
    ID3D11Texture2D *depthTex = nullptr;
    ID3D11Resource *depthTexResource = nullptr;
    pRenderTargetView->lpVtbl->GetResource(pRenderTargetView, &depthTexResource);
    HRESULT hr = depthTexResource->lpVtbl->QueryInterface(depthTexResource, IID_ID3D11Texture2D, (void **)&depthTex);
    if (FAILED(hr)) {
      getOut() << "failed to get rtv resource: " << (void *)hr << std::endl;
      abort();
    }
    
    D3D11_TEXTURE2D_DESC desc;
    depthTex->lpVtbl->GetDesc(depthTex, &desc);
    
    getOut() << "clear render target view " << (void *)depthTexResource << " " << (void *)pRenderTargetView << " " << desc.Width << " " << desc.Height << " " << desc.Format << std::endl;
    
    depthTex->lpVtbl->Release(depthTex);
    depthTexResource->lpVtbl->Release(depthTexResource);
  } */
  
  RealClearRenderTargetView(This, pRenderTargetView, ColorRGBA);
}
void (STDMETHODCALLTYPE *RealClearDepthStencilView)(
  ID3D11DeviceContext *This,
  ID3D11DepthStencilView *pDepthStencilView,
  UINT                   ClearFlags,
  FLOAT                  Depth,
  UINT8                  Stencil
) = nullptr;
void STDMETHODCALLTYPE MineClearDepthStencilView(
  ID3D11DeviceContext *This,
  ID3D11DepthStencilView *pDepthStencilView,
  UINT                   ClearFlags,
  FLOAT                  Depth,
  UINT8                  Stencil
) {
  TRACE("Hijack", [&]() { getOut() << "ClearDepthStencilView" << std::endl; });
  RealClearDepthStencilView(This, pDepthStencilView, ClearFlags, Depth, Stencil);
}
void (STDMETHODCALLTYPE *RealClearState)(
  ID3D11DeviceContext *This
) = nullptr;
void STDMETHODCALLTYPE MineClearState(
  ID3D11DeviceContext *This
) {
  TRACE("Hijack", [&]() { getOut() << "ClearState" << std::endl; });
  RealClearState(This);
}
void (STDMETHODCALLTYPE *RealClearView)(
  ID3D11DeviceContext1 *This,
  ID3D11View       *pView,
  const FLOAT   Color[4],
  const D3D11_RECT *pRect,
  UINT             NumRects
) = nullptr;
void STDMETHODCALLTYPE MineClearView(
  ID3D11DeviceContext1 *This,
  ID3D11View       *pView,
  const FLOAT   Color[4],
  const D3D11_RECT *pRect,
  UINT             NumRects
) {
  TRACE("Hijack", [&]() { getOut() << "ClearView" << std::endl; });
  RealClearView(This, pView, Color, pRect, NumRects);
}
void (STDMETHODCALLTYPE *RealCopyResource)(
  ID3D11DeviceContext1 *This,
  ID3D11Resource *pDstResource,
  ID3D11Resource *pSrcResource
) = nullptr;
void STDMETHODCALLTYPE MineCopyResource(
  ID3D11DeviceContext1 *This,
  ID3D11Resource *pDstResource,
  ID3D11Resource *pSrcResource
) {
  TRACE("Hijack", [&]() { getOut() << "RealCopyResource " << (void *)pDstResource << " " << (void *)pSrcResource << std::endl; });
  RealCopyResource(This, pDstResource, pSrcResource);
}
void (STDMETHODCALLTYPE *RealCopySubresourceRegion)(
  ID3D11DeviceContext1 *This,
  ID3D11Resource  *pDstResource,
  UINT            DstSubresource,
  UINT            DstX,
  UINT            DstY,
  UINT            DstZ,
  ID3D11Resource  *pSrcResource,
  UINT            SrcSubresource,
  const D3D11_BOX *pSrcBox
) = nullptr;
void STDMETHODCALLTYPE MineCopySubresourceRegion(
  ID3D11DeviceContext1 *This,
  ID3D11Resource  *pDstResource,
  UINT            DstSubresource,
  UINT            DstX,
  UINT            DstY,
  UINT            DstZ,
  ID3D11Resource  *pSrcResource,
  UINT            SrcSubresource,
  const D3D11_BOX *pSrcBox
) {
  TRACE("Hijack", [&]() { getOut() << "RealCopySubresourceRegion " << (void *)pDstResource << " " << (void *)pSrcResource << std::endl; });
  RealCopySubresourceRegion(This, pDstResource, DstSubresource, DstX, DstY, DstZ, pSrcResource, SrcSubresource, pSrcBox);
}
void (STDMETHODCALLTYPE *RealResolveSubresource)(
  ID3D11DeviceContext1 *This,
  ID3D11Resource *pDstResource,
  UINT           DstSubresource,
  ID3D11Resource *pSrcResource,
  UINT           SrcSubresource,
  DXGI_FORMAT    Format
) = nullptr;
void STDMETHODCALLTYPE MineResolveSubresource(
  ID3D11DeviceContext1 *This,
  ID3D11Resource *pDstResource,
  UINT           DstSubresource,
  ID3D11Resource *pSrcResource,
  UINT           SrcSubresource,
  DXGI_FORMAT    Format
) {
  TRACE("Hijack", [&]() { getOut() << "ResolveSubresource " << (void *)pDstResource << " " << (void *)pSrcResource << std::endl; });
  RealResolveSubresource(This, pDstResource, DstSubresource, pSrcResource, SrcSubresource, Format);
}
void (STDMETHODCALLTYPE *RealUpdateSubresource)(
  ID3D11DeviceContext1 *This,
  ID3D11Resource  *pDstResource,
  UINT            DstSubresource,
  const D3D11_BOX *pDstBox,
  const void      *pSrcData,
  UINT            SrcRowPitch,
  UINT            SrcDepthPitch
) = nullptr;
void STDMETHODCALLTYPE MineUpdateSubresource(
  ID3D11DeviceContext1 *This,
  ID3D11Resource  *pDstResource,
  UINT            DstSubresource,
  const D3D11_BOX *pDstBox,
  const void      *pSrcData,
  UINT            SrcRowPitch,
  UINT            SrcDepthPitch
) {
  RealUpdateSubresource(This, pDstResource, DstSubresource, pDstBox, pSrcData, SrcRowPitch, SrcDepthPitch);
}
HRESULT (STDMETHODCALLTYPE *RealCreateBuffer)(
  ID3D11Device *This,
  const D3D11_BUFFER_DESC      *pDesc,
  const D3D11_SUBRESOURCE_DATA *pInitialData,
  ID3D11Buffer                 **ppBuffer
) = nullptr;
HRESULT STDMETHODCALLTYPE MineCreateBuffer(
  ID3D11Device *This,
  const D3D11_BUFFER_DESC      *pDesc,
  const D3D11_SUBRESOURCE_DATA *pInitialData,
  ID3D11Buffer                 **ppBuffer
) {
  return RealCreateBuffer(This, pDesc, pInitialData, ppBuffer);
}
std::map<ID3D11Resource *, std::pair<void *, size_t>> cbufs;
HRESULT (STDMETHODCALLTYPE *RealMap)(
  ID3D11DeviceContext1 *This,
  ID3D11Resource           *pResource,
  UINT                     Subresource,
  D3D11_MAP                MapType,
  UINT                     MapFlags,
  D3D11_MAPPED_SUBRESOURCE *pMappedResource
) = nullptr;
HRESULT STDMETHODCALLTYPE MineMap(
  ID3D11DeviceContext1 *This,
  ID3D11Resource           *pResource,
  UINT                     Subresource,
  D3D11_MAP                MapType,
  UINT                     MapFlags,
  D3D11_MAPPED_SUBRESOURCE *pMappedResource
) {
  HRESULT hr = RealMap(This, pResource, Subresource, MapType, MapFlags, pMappedResource);
  /* if (SUCCEEDED(hr)) {
    D3D11_RESOURCE_DIMENSION dim;
    pResource->lpVtbl->GetType(pResource, &dim);

    if (dim == D3D11_RESOURCE_DIMENSION_BUFFER) {
      ID3D11Buffer *buffer;
      HRESULT hr = pResource->lpVtbl->QueryInterface(pResource, IID_ID3D11Buffer, (void **)&buffer);

      if (SUCCEEDED(hr)) {
        D3D11_BUFFER_DESC desc;
        buffer->lpVtbl->GetDesc(buffer, &desc);

        if (desc.ByteWidth < PROJECTION_MATRIX_SEARCH_SIZE) {
          cbufs[pResource] = std::pair<void *, size_t>(pMappedResource->pData, desc.ByteWidth);
        }
        
        buffer->lpVtbl->Release(buffer);
      } else {
        getOut() << "failed to get cbuf buffer view " << (void *)hr << std::endl;
        abort();
      }
    }
  } */
  return hr;
}
void (STDMETHODCALLTYPE *RealUnmap)(
  ID3D11DeviceContext1 *This,
  ID3D11Resource *pResource,
  UINT           Subresource
) = nullptr;
void STDMETHODCALLTYPE MineUnmap(
  ID3D11DeviceContext1 *This,
  ID3D11Resource *pResource,
  UINT           Subresource
) {
  RealUnmap(This, pResource, Subresource);
}
HRESULT (STDMETHODCALLTYPE *RealCreateTexture2D)(
  ID3D11Device *This,
  const D3D11_TEXTURE2D_DESC   *pDesc,
  const D3D11_SUBRESOURCE_DATA *pInitialData,
  ID3D11Texture2D              **ppTexture2D
) = nullptr;
HRESULT STDMETHODCALLTYPE MineCreateTexture2D(
  ID3D11Device *This,
  const D3D11_TEXTURE2D_DESC   *pDesc,
  const D3D11_SUBRESOURCE_DATA *pInitialData,
  ID3D11Texture2D              **ppTexture2D
) {
  // getOut() << "bind surface try client 1 " << pDesc->Width << " " << pDesc->Height << std::endl;
  HANDLE surfaceShHandle = g_hijacker->fnp.call<
    kHijacker_TryBindSurface,
    HANDLE,
    D3D11_TEXTURE2D_DESC
  >(*pDesc);
  // getOut() << "bind surface try client 2 " << (void *)surfaceShHandle << std::endl;
  if (surfaceShHandle) {
    ID3D11Resource *surfaceRes;
    HRESULT hr = This->lpVtbl->OpenSharedResource(This, surfaceShHandle, IID_ID3D11Resource, (void**)&surfaceRes);
    if (FAILED(hr)) {
      getOut() << "failed to unpack surface texture handle: " << (void *)hr << " " << (void *)surfaceShHandle << std::endl;
      abort();
    }

    hr = surfaceRes->lpVtbl->QueryInterface(surfaceRes, IID_ID3D11Texture2D, (void **)ppTexture2D);
    if (FAILED(hr)) {
      getOut() << "failed to unpack surface texture: " << (void *)hr << std::endl;
      abort();
    }

    surfaceRes->lpVtbl->Release(surfaceRes);
    
    return hr;
  } else {
    auto hr = RealCreateTexture2D(This, pDesc, pInitialData, ppTexture2D);
    /* const D3D11_TEXTURE2D_DESC &desc = *pDesc;
    getOut() << "create texture 2d normal " <<
      (void *)(*ppTexture2D) << " " <<
      desc.Width << " " << desc.Height << " " <<
      desc.MipLevels << " " << desc.ArraySize << " " <<
      desc.SampleDesc.Count << " " << desc.SampleDesc.Quality << " " <<
      desc.Format << " " <<
      desc.Usage << " " << desc.BindFlags << " " << desc.CPUAccessFlags << " " << desc.MiscFlags << " " <<
      std::endl; */
    return hr;
  }
}
HRESULT (STDMETHODCALLTYPE *RealCreateRasterizerState)(
  ID3D11Device *This,
  const D3D11_RASTERIZER_DESC *pRasterizerDesc,
  ID3D11RasterizerState       **ppRasterizerState
) = nullptr;
HRESULT STDMETHODCALLTYPE MineCreateRasterizerState(
  ID3D11Device *This,
  const D3D11_RASTERIZER_DESC *pRasterizerDesc,
  ID3D11RasterizerState       **ppRasterizerState
) {
  // *ppRasterizerState = nullptr;
  return RealCreateRasterizerState(This, pRasterizerDesc, ppRasterizerState);
  // getOut() << "RealCreateRasterizerState " << (void *)(*ppRasterizerState) << " " << pRasterizerDesc->DepthBias << " " << pRasterizerDesc->DepthBiasClamp << " " << pRasterizerDesc->SlopeScaledDepthBias << " " << pRasterizerDesc->DepthClipEnable << std::endl;
  // return hr;
}
void (STDMETHODCALLTYPE *RealRSSetState)(
  ID3D11DeviceContext1 *This,
  ID3D11RasterizerState *pRasterizerState
) = nullptr;
void STDMETHODCALLTYPE MineRSSetState(
  ID3D11DeviceContext1 *This,
  ID3D11RasterizerState *pRasterizerState
) {
  // getOut() << "RealRSSetState " << (void *)pRasterizerState << std::endl;
  return RealRSSetState(This, pRasterizerState);
}

BOOL (STDMETHODCALLTYPE *RealGlGetIntegerv)(
  GLenum pname,
 	GLint * data
) = nullptr;
BOOL (STDMETHODCALLTYPE *RealGlGetFramebufferAttachmentParameteriv)(
  GLenum target,
 	GLenum attachment,
 	GLenum pname,
 	GLint *params
) = nullptr;

void (STDMETHODCALLTYPE *RealGlViewport)(
  GLint x,
 	GLint y,
 	GLsizei width,
 	GLsizei height
) = nullptr;
void (STDMETHODCALLTYPE *RealGlGenFramebuffers)(
  GLsizei n,
 	GLuint * framebuffers
) = nullptr;
void STDMETHODCALLTYPE MineGlGenFramebuffers(
  GLsizei n,
 	GLuint * framebuffers
) {
  RealGlGenFramebuffers(n, framebuffers);
  TRACE("Hijack", [&]() { getOut() << "glGenFramebuffers " << n << " " << framebuffers[0] << " " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl; });
}
void (STDMETHODCALLTYPE *RealGlBindFramebuffer)(
  GLenum target,
 	GLuint framebuffer
) = nullptr;
void STDMETHODCALLTYPE MineGlBindFramebuffer(
  GLenum target,
 	GLuint framebuffer
) {
  TRACE("Hijack", [&]() { getOut() << "glBindFramebuffer " << target << " " << framebuffer << " " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl; });
  RealGlBindFramebuffer(target, framebuffer);
}
void (STDMETHODCALLTYPE *RealGlGenRenderbuffers)(
  GLsizei n,
 	GLuint * renderbuffers
) = nullptr;
void STDMETHODCALLTYPE MineGlGenRenderbuffers(
  GLsizei n,
 	GLuint * renderbuffers
) {
  EGLDisplay display = EGL_GetCurrentDisplay();
  if (display == EGL_NO_DISPLAY) {
    getOut() << "failed to get EGL display" << std::endl;
    abort();
  }
  
  intptr_t egl_device_ptr = 0;
  intptr_t device_ptr = 0;
  getOut() << "hijack dx from gl " << (void *)EGL_QueryDisplayAttribEXT << " " << EGL_QueryDeviceAttribEXT << std::endl;
  EGLBoolean ok = (*((decltype(eglQueryDisplayAttribEXT) *)EGL_QueryDisplayAttribEXT))(display, EGL_DEVICE_EXT, &egl_device_ptr);
  // getOut() << "get device renderbuffers 2 " << (void *)egl_device_ptr << " " << (void *)ok << std::endl;
  ok = EGL_QueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(egl_device_ptr), EGL_D3D11_DEVICE_ANGLE, &device_ptr);
  // getOut() << "get device renderbuffers 3 " << (void *)device_ptr << " " << (void *)ok << " " << GetLastError() << std::endl;
  ID3D11Device *d3d11_device = reinterpret_cast<ID3D11Device *>(device_ptr);
  ID3D11DeviceContext *context;
  d3d11_device->lpVtbl->GetImmediateContext(d3d11_device, &context);
  // getOut() << "get device renderbuffers 4 " << (void *)context << " " << GetLastError() << std::endl;
  g_hijacker->hijackDx(context);
  // getOut() << "get device renderbuffers 5" << std::endl; */
  
  RealGlGenRenderbuffers(n, renderbuffers);
  TRACE("Hijack", [&]() { getOut() << "glGenRenderbuffers " << n << " " << renderbuffers[0] << " " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl; });
}
void (STDMETHODCALLTYPE *RealGlBindRenderbuffer)(
  GLenum target,
 	GLuint renderbuffer
) = nullptr;
void STDMETHODCALLTYPE MineGlBindRenderbuffer(
  GLenum target,
 	GLuint renderbuffer
) {
  TRACE("Hijack", [&]() { getOut() << "glBindRenderbuffer " << target << " " << renderbuffer << " " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl; });
  RealGlBindRenderbuffer(target, renderbuffer);
}
void (STDMETHODCALLTYPE *RealGlFramebufferTexture2D)(
  GLenum target,
 	GLenum attachment,
 	GLenum textarget,
 	GLuint texture,
 	GLint level
) = nullptr;
void STDMETHODCALLTYPE MineGlFramebufferTexture2D(
  GLenum target,
 	GLenum attachment,
 	GLenum textarget,
 	GLuint texture,
 	GLint level
) {
  TRACE("Hijack", [&]() { getOut() << "glFramebufferTexture2D " << target << " " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl; });
  RealGlFramebufferTexture2D(target, attachment, textarget, texture, level);
}
void (STDMETHODCALLTYPE *RealGlFramebufferTexture2DMultisampleEXT)(
  GLenum target,
  GLenum attachment,
  GLenum textarget,
  GLuint texture, 
  GLint level,
  GLsizei samples
) = nullptr;
void STDMETHODCALLTYPE MineGlFramebufferTexture2DMultisampleEXT(
  GLenum target,
  GLenum attachment,
  GLenum textarget,
  GLuint texture, 
  GLint level,
  GLsizei samples
) {
  glPhase = 1;
  TRACE("Hijack", [&]() { getOut() << "glFramebufferTexture2DMultisampleEXT " << target << " " << attachment << " " << textarget << " " << texture << " " << level << " " << samples << " " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl; });
  RealGlFramebufferTexture2DMultisampleEXT(target, attachment, textarget, texture, level, samples);
}
void (STDMETHODCALLTYPE *RealGlFramebufferRenderbuffer)(
  GLenum target,
 	GLenum attachment,
 	GLenum renderbuffertarget,
 	GLuint renderbuffer
) = nullptr;
void STDMETHODCALLTYPE MineGlFramebufferRenderbuffer(
  GLenum target,
 	GLenum attachment,
 	GLenum renderbuffertarget,
 	GLuint renderbuffer
) {
  TRACE("Hijack", [&]() { getOut() << "glFramebufferRenderbuffer " << target << " " << attachment << " " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl; });
  RealGlFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}
void (STDMETHODCALLTYPE *RealGlRenderbufferStorage)(
  GLenum target,
 	GLenum internalformat,
 	GLsizei width,
 	GLsizei height
) = nullptr;
void (STDMETHODCALLTYPE *RealGlRenderbufferStorageMultisampleEXT)(
  GLenum target,
  GLsizei samples,
  GLenum internalformat,
  GLsizei width,
  GLsizei height
) = nullptr;
void STDMETHODCALLTYPE MineGlRenderbufferStorageMultisampleEXT(
  GLenum target,
  GLsizei samples,
  GLenum internalformat,
  GLsizei width,
  GLsizei height
) {
  /* depthSamples = samples;
  depthInternalformat = internalformat;
  depthWidth = width;
  depthHeight = height; */
  TRACE("Hijack", [&]() { getOut() << "glRenderbufferStorageMultisampleEXT " << target << " " << samples << " " << internalformat << " " << width << " " << height << " " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl; });
  // RealGlRenderbufferStorage(target, internalformat, width, height);
  RealGlRenderbufferStorageMultisampleEXT(target, samples, internalformat, width, height);
}
void (STDMETHODCALLTYPE *RealGlDiscardFramebufferEXT)(
  GLenum target, 
  GLsizei numAttachments, 
  const GLenum *attachments
) = nullptr;
void STDMETHODCALLTYPE MineGlDiscardFramebufferEXT(
  GLenum target, 
  GLsizei numAttachments, 
  const GLenum *attachments
) {
  TRACE("Hijack", [&]() { getOut() << "glDiscardFramebufferEXT" << std::endl; });
  RealGlDiscardFramebufferEXT(target, numAttachments, attachments);
}
void (STDMETHODCALLTYPE *RealGlDiscardFramebufferEXTContextANGLE)(
  GLenum target, 
  GLsizei numAttachments, 
  const GLenum *attachments
) = nullptr;
void STDMETHODCALLTYPE MineGlDiscardFramebufferEXTContextANGLE(
  GLenum target, 
  GLsizei numAttachments, 
  const GLenum *attachments
) {
  TRACE("Hijack", [&]() { getOut() << "glDiscardFramebufferEXTContextANGLE" << std::endl; });
  RealGlDiscardFramebufferEXTContextANGLE(target, numAttachments, attachments);
}
void (STDMETHODCALLTYPE *RealGlInvalidateFramebuffer)(
  GLenum target,
 	GLsizei numAttachments,
 	const GLenum *attachments
) = nullptr;
void STDMETHODCALLTYPE MineGlInvalidateFramebuffer(
  GLenum target,
 	GLsizei numAttachments,
 	const GLenum *attachments
) {
  TRACE("Hijack", [&]() { getOut() << "glInvalidateFramebuffer" << std::endl; });
  RealGlInvalidateFramebuffer(target, numAttachments, attachments);
}
void (STDMETHODCALLTYPE *RealDiscardFramebufferEXT)(
  GLenum target,
  GLsizei count,
  const GLenum* attachments
) = nullptr;
void STDMETHODCALLTYPE MineDiscardFramebufferEXT(
  GLenum target,
  GLsizei count,
  const GLenum* attachments
) {
  TRACE("Hijack", [&]() { getOut() << "DiscardFramebufferEXT" << std::endl; });
  RealDiscardFramebufferEXT(target, count, attachments);
}
void (STDMETHODCALLTYPE *RealGlGenTextures)(
  GLsizei n,
 	GLuint * textures
) = nullptr;
/* void STDMETHODCALLTYPE MineGlGenTextures(
  GLsizei n,
 	GLuint * textures
) {
  RealGlGenTextures(n, textures);
  getOut() << "RealGlGenTextures" << n << " " << textures[0] << " " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl;
} */
void (STDMETHODCALLTYPE *RealGlTexImage2D)(
  GLenum target,
 	GLint level,
 	GLint internalformat,
 	GLsizei width,
 	GLsizei height,
 	GLint border,
 	GLenum format,
 	GLenum type,
 	const void * data
) = nullptr;
void (STDMETHODCALLTYPE *RealGlTexParameteri)(
  GLenum target,
 	GLenum pname,
 	GLint param
) = nullptr;
void (STDMETHODCALLTYPE *RealGlBindTexture)(
  GLenum target,
 	GLuint texture
) = nullptr;
/* void STDMETHODCALLTYPE MineGlBindTexture(
  GLenum target,
 	GLuint texture
) {
  getOut() << "RealGlBindTexture " << target << " " << texture << std::endl;
  RealGlBindTexture(target, texture);
} */
void (STDMETHODCALLTYPE *RealGlReadPixels)(
  GLint x,
 	GLint y,
 	GLsizei width,
 	GLsizei height,
 	GLenum format,
 	GLenum type,
 	void * data
) = nullptr;
void (STDMETHODCALLTYPE *RealGlTexStorage2DMultisample)(
  GLenum target,
 	GLsizei samples,
 	GLenum internalformat,
 	GLsizei width,
 	GLsizei height,
 	GLboolean fixedsamplelocations
) = nullptr;
GLenum (STDMETHODCALLTYPE *RealGlGetError)(
) = nullptr;
void (STDMETHODCALLTYPE *RealGlRequestExtensionANGLE)(
  const GLchar *extension
) = nullptr;
void STDMETHODCALLTYPE MineGlRequestExtensionANGLE(
  const GLchar *extension
) {
  TRACE("Hijack", [&]() { getOut() << "RealGlRequestExtensionANGLE " << extension << std::endl; });
  RealGlRequestExtensionANGLE(extension);
}
void (STDMETHODCALLTYPE *RealGlDeleteTextures)(
  GLsizei n,
 	const GLuint * textures
) = nullptr;
void STDMETHODCALLTYPE MineGlDeleteTextures(
  GLsizei n,
 	const GLuint * textures
) {
  RealGlDeleteTextures(n, textures);
  if (n > 0 && textures != NULL) {
    TRACE("Hijack", [&]() { getOut() << "RealGlDeleteTextures " << n << " " << textures[0] << " " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl; });
  } else {
    TRACE("Hijack", [&]() { getOut() << "RealGlDeleteTextures " << n << std::endl; });
  }
}
void (STDMETHODCALLTYPE *RealGlFenceSync)(
  GLenum condition,
 	GLbitfield flags
) = nullptr;
void STDMETHODCALLTYPE MineGlFenceSync(
  GLenum condition,
 	GLbitfield flags
) {
  TRACE("Hijack", [&]() { getOut() << "RealGlFenceSync" << std::endl; });
  RealGlFenceSync(condition, flags);
}
void (STDMETHODCALLTYPE *RealGlDeleteSync)(
  GLsync sync
) = nullptr;
void STDMETHODCALLTYPE MineGlDeleteSync(
  GLsync sync
) {
  TRACE("Hijack", [&]() { getOut() << "RealGlDeleteSync" << std::endl; });
  RealGlDeleteSync(sync);
}
void (STDMETHODCALLTYPE *RealGlWaitSync)(
  GLsync sync,
 	GLbitfield flags,
 	GLuint64 timeout
) = nullptr;
void STDMETHODCALLTYPE MineGlWaitSync(
  GLsync sync,
 	GLbitfield flags,
 	GLuint64 timeout
) {
  TRACE("Hijack", [&]() { getOut() << "RealGlWaitSync" << std::endl; });
  RealGlWaitSync(sync, flags, timeout);
}
void (STDMETHODCALLTYPE *RealGlClientWaitSync)(
  GLsync sync,
 	GLbitfield flags,
 	GLuint64 timeout
) = nullptr;
void STDMETHODCALLTYPE MineGlClientWaitSync(
  GLsync sync,
 	GLbitfield flags,
 	GLuint64 timeout
) {
  TRACE("Hijack", [&]() { getOut() << "RealGlClientWaitSync" << std::endl; });
  RealGlClientWaitSync(sync, flags, timeout);
}
GLenum (STDMETHODCALLTYPE *RealGlDrawElements)(
  GLenum mode,
 	GLsizei count,
 	GLenum type,
 	const void * indices
) = nullptr;
void (STDMETHODCALLTYPE *RealGlClearColor)(
  GLclampf red,
 	GLclampf green,
 	GLclampf blue,
 	GLclampf alpha
) = nullptr;
void STDMETHODCALLTYPE MineGlClearColor(
  GLclampf red,
 	GLclampf green,
 	GLclampf blue,
 	GLclampf alpha
) {
  if (glPhase == 2 && red == 0 && blue == 0 && green == 0 && alpha == 0) {
    glPhase = 3;
  } else {
    glPhase = 0;
  }
  TRACE("Hijack", [&]() { getOut() << "RealGlClearColor " << red << " " << green << " " << blue << " " << alpha << " " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl; });
  RealGlClearColor(red, green, blue, alpha);
}
void (STDMETHODCALLTYPE *RealGlColorMask)(
  GLboolean red,
 	GLboolean green,
 	GLboolean blue,
 	GLboolean alpha
) = nullptr;
void STDMETHODCALLTYPE MineGlColorMask(
  GLboolean red,
 	GLboolean green,
 	GLboolean blue,
 	GLboolean alpha
) {
  if (glPhase == 1 && red == 1 && blue == 1 && green == 1 && alpha == 1) {
    glPhase = 2;
  } else {
    glPhase = 0;
  }
  TRACE("Hijack", [&]() { getOut() << "RealGlColorMask " << (int)red << " " << (int)green << " " << (int)blue << " " << (int)alpha << " " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl; });
  RealGlColorMask(red, green, blue, alpha);
}
void (STDMETHODCALLTYPE *RealGlFlush)(
) = nullptr;
void handleGlClearHack() {
  if (glPhase == 3) {
    if (!glActiveTexture) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glActiveTexture = (decltype(glActiveTexture))GetProcAddress(libGlesV2, "glActiveTexture");
    }
    if (!glTexStorage2DMultisample) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glTexStorage2DMultisample = (decltype(glTexStorage2DMultisample))GetProcAddress(libGlesV2, "glTexStorage2DMultisample");
    }
    if (!glFramebufferTexture2DMultisampleEXT) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glFramebufferTexture2DMultisampleEXT = (decltype(glFramebufferTexture2DMultisampleEXT))GetProcAddress(libGlesV2, "glFramebufferTexture2DMultisampleEXT");
    }
    if (!glGenFramebuffers) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glGenFramebuffers = (decltype(glGenFramebuffers))GetProcAddress(libGlesV2, "glGenFramebuffers");
    }
    if (!glBindFramebuffer) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glBindFramebuffer = (decltype(glBindFramebuffer))GetProcAddress(libGlesV2, "glBindFramebuffer");
    }
    if (!glBlitFramebuffer) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glBlitFramebuffer = (decltype(glBlitFramebuffer))GetProcAddress(libGlesV2, "glBlitFramebuffer");
    }
    if (!glBlitFramebufferANGLE) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glBlitFramebufferANGLE = (decltype(glBlitFramebuffer))GetProcAddress(libGlesV2, "glBlitFramebufferANGLE");
    }
    if (!glGenVertexArrays) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glGenVertexArrays = (decltype(glGenVertexArrays))GetProcAddress(libGlesV2, "glGenVertexArraysOES");
    }
    if (!glBindVertexArray) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glBindVertexArray = (decltype(glBindVertexArray))GetProcAddress(libGlesV2, "glBindVertexArrayOES");
    }
    if (!glCreateShader) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glCreateShader = (decltype(glCreateShader))GetProcAddress(libGlesV2, "glCreateShader");
    }
    if (!glShaderSource) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glShaderSource = (decltype(glShaderSource))GetProcAddress(libGlesV2, "glShaderSource");
    }
    if (!glCompileShader) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glCompileShader = (decltype(glCompileShader))GetProcAddress(libGlesV2, "glCompileShader");
    }
    if (!glGetShaderiv) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glGetShaderiv = (decltype(glGetShaderiv))GetProcAddress(libGlesV2, "glGetShaderiv");
    }
    if (!glGetShaderInfoLog) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glGetShaderInfoLog = (decltype(glGetShaderInfoLog))GetProcAddress(libGlesV2, "glGetShaderInfoLog");
    }
    if (!glCreateProgram) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glCreateProgram = (decltype(glCreateProgram))GetProcAddress(libGlesV2, "glCreateProgram");
    }
    if (!glAttachShader) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glAttachShader = (decltype(glAttachShader))GetProcAddress(libGlesV2, "glAttachShader");
    }
    if (!glLinkProgram) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glLinkProgram = (decltype(glLinkProgram))GetProcAddress(libGlesV2, "glLinkProgram");
    }
    if (!glGetProgramiv) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glGetProgramiv = (decltype(glGetProgramiv))GetProcAddress(libGlesV2, "glGetProgramiv");
    }
    if (!glGetAttribLocation) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glGetAttribLocation = (decltype(glGetAttribLocation))GetProcAddress(libGlesV2, "glGetAttribLocation");
    }
    if (!glGetUniformLocation) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glGetUniformLocation = (decltype(glGetUniformLocation))GetProcAddress(libGlesV2, "glGetUniformLocation");
    }
    if (!glDeleteShader) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glDeleteShader = (decltype(glDeleteShader))GetProcAddress(libGlesV2, "glDeleteShader");
    }
    if (!glUseProgram) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glUseProgram = (decltype(glUseProgram))GetProcAddress(libGlesV2, "glUseProgram");
    }
    if (!glGenBuffers) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glGenBuffers = (decltype(glGenBuffers))GetProcAddress(libGlesV2, "glGenBuffers");
    }
    if (!glBindBuffer) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glBindBuffer = (decltype(glBindBuffer))GetProcAddress(libGlesV2, "glBindBuffer");
    }
    if (!glBufferData) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glBufferData = (decltype(glBufferData))GetProcAddress(libGlesV2, "glBufferData");
    }
    if (!glEnableVertexAttribArray) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glEnableVertexAttribArray = (decltype(glEnableVertexAttribArray))GetProcAddress(libGlesV2, "glEnableVertexAttribArray");
    }
    if (!glVertexAttribPointer) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glVertexAttribPointer = (decltype(glVertexAttribPointer))GetProcAddress(libGlesV2, "glVertexAttribPointer");
    }
    if (!glUniform1i) {
      HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
      glUniform1i = (decltype(glUniform1i))GetProcAddress(libGlesV2, "glUniform1i");
    }
    
    GLint oldActiveTexture;
    RealGlGetIntegerv(GL_ACTIVE_TEXTURE, &oldActiveTexture);
    glActiveTexture(GL_TEXTURE0);
    GLint oldTexture2d;
    RealGlGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture2d);
    GLint oldFbo;
    RealGlGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFbo);
    GLint oldReadFbo;
    RealGlGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldReadFbo);
    GLint oldDrawFbo;
    RealGlGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldDrawFbo);
    GLint oldProgram;
    RealGlGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    GLint oldVao;
    RealGlGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVao);
    GLint oldArrayBuffer;
    RealGlGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldArrayBuffer);
    GLint oldViewport[4];
    RealGlGetIntegerv(GL_VIEWPORT, oldViewport);

    // getOut() << "get old X " << oldTex << " " << oldReadFbo << " " << oldDrawFbo << " " << oldProgram << " " << oldVao << " " << oldArrayBuffer << std::endl;

    if (!depthTexId) {
      // Hijacker::ensureClientDevice();

      EGLDisplay display = EGL_GetCurrentDisplay();
      if (display == EGL_NO_DISPLAY) {
        getOut() << "failed to get EGL display" << std::endl;
        abort();
      }

      HRESULT hr;
      {
        intptr_t egl_device_ptr = 0;
        intptr_t device_ptr = 0;
        // getOut() << "get device 1 " << (void *)EGL_QueryDisplayAttribEXT << " " << EGL_QueryDeviceAttribEXT << std::endl;
        EGLBoolean ok = (*((decltype(eglQueryDisplayAttribEXT) *)EGL_QueryDisplayAttribEXT))(display, EGL_DEVICE_EXT, &egl_device_ptr);
        // getOut() << "get device 2 " << (void *)egl_device_ptr << " " << (void *)ok << std::endl;
        ok = EGL_QueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(egl_device_ptr), EGL_D3D11_DEVICE_ANGLE, &device_ptr);
        // getOut() << "get device 3 " << (void *)device_ptr << " " << (void *)ok << " " << GetLastError() << std::endl;
        ID3D11Device *d3d11_device = reinterpret_cast<ID3D11Device *>(device_ptr);

        hr = d3d11_device->lpVtbl->QueryInterface(d3d11_device, IID_ID3D11Device5, (void **)&hijackerDevice);
        if (SUCCEEDED(hr)) {
          // nothing
        } else {
          getOut() << "device query failed" << std::endl;
          abort();
        }
        
        /* wglDXOpenDeviceNV = (decltype(wglDXOpenDeviceNV))wglGetProcAddress("wglDXOpenDeviceNV");
        getOut() << "ensure client device 4 " << (void *)wglDXOpenDeviceNV << std::endl;

        HANDLE hInteropDevice = wglDXOpenDeviceNV(hijackerDevice);

        getOut() << "ensure client device 5 " << (void *)hInteropDevice << " " << GetLastError() << std::endl; */

        ID3D11DeviceContext3 *context3;
        hijackerDevice->lpVtbl->GetImmediateContext3(hijackerDevice, &context3);
        hr = context3->lpVtbl->QueryInterface(context3, IID_ID3D11DeviceContext4, (void **)&hijackerContext);
        getOut() << "ensure client device 6" << std::endl;
        if (SUCCEEDED(hr)) {
          // nothing
        } else {
          getOut() << "context query failed" << std::endl;
          abort();
        }
        context3->lpVtbl->Release(context3);
      }

      {
        for (size_t i = 0; i < 2; i++) {
          ID3D11Fence *fence;
          HRESULT hr = hijackerDevice->lpVtbl->CreateFence(
            hijackerDevice,
            0, // value
            // D3D11_FENCE_FLAG_SHARED|D3D11_FENCE_FLAG_SHARED_CROSS_ADAPTER, // flags
            // D3D11_FENCE_FLAG_SHARED, // flags
            D3D11_FENCE_FLAG_NONE, // flags
            IID_ID3D11Fence, // interface
            (void **)&fence // out
          );
          if (SUCCEEDED(hr)) {
            // getOut() << "created fence " << (void *)fence << std::endl;
            // nothing
          } else {
            getOut() << "failed to create depth fence" << std::endl;
            abort();
          }
          fenceCache.push_back(fence);
        }
      }
      
      RealGlBindFramebuffer(GL_FRAMEBUFFER, oldFbo);
    }
    
    // getOut() << "generating depth 6 1 " << (void *)RealGlGetError() << std::endl;

    RealGlBindFramebuffer(GL_DRAW_FRAMEBUFFER, depthResolveFbo);
    // getOut() << "generating depth 6 2 " << (void *)RealGlGetError() << std::endl;
    glBlitFramebufferANGLE(
      0, 0,
      depthWidth, depthHeight,
      0, 0,
      depthWidth, depthHeight,
      GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
      GL_NEAREST
    );
    
    // getOut() << "generating depth 7 " << (void *)RealGlGetError() << std::endl;

    RealGlBindFramebuffer(GL_DRAW_FRAMEBUFFER, depthShFbo);
    glBindVertexArray(depthVao);
    glUseProgram(depthProgram);
    glActiveTexture(GL_TEXTURE0);
    RealGlBindTexture(GL_TEXTURE_2D, depthResolveTexId);
    RealGlViewport(0, 0, depthWidth, depthHeight);
    /* RealGlClearColor(0, 0, 0, 0);
    RealGlColorMask(true, true, true, true);
    RealGlClear(GL_COLOR_BUFFER_BIT); */
    RealGlDrawElements(
      GL_TRIANGLES,
      6,
      GL_UNSIGNED_SHORT,
      (void *)0
    );

    // getOut() << "generating depth 8 " << (void *)RealGlGetError() << std::endl;
    
    /* RealGlBindFramebuffer(GL_READ_FRAMEBUFFER, depthShFbo);
    std::vector<unsigned char> data(depthWidth * depthHeight * 4);
    // std::fill(data.begin(), data.end(), 2);
    RealGlReadPixels(0, 0, depthWidth, depthHeight, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    size_t count = 0;
    for (size_t i = 0; i < data.size(); i += 4) {
      if (data[i] == 234) {
        count += data[i];
      }
    }
    getOut() << "get real read pixels " << count << std::endl; */
    
    // RealGlFlush();

    RealGlBindTexture(GL_TEXTURE_2D, oldTexture2d);
    glActiveTexture(oldActiveTexture);
    RealGlBindFramebuffer(GL_FRAMEBUFFER, oldFbo);
    RealGlBindFramebuffer(GL_READ_FRAMEBUFFER, oldReadFbo);
    RealGlBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldDrawFbo);
    glUseProgram(oldProgram);
    glBindVertexArray(oldVao);
    glBindBuffer(GL_ARRAY_BUFFER, oldArrayBuffer);
    RealGlViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
    
    // getOut() << "generating depth 10 " << (void *)RealGlGetError() << std::endl;

    /* GLint type;
    RealGlGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
    GLint rbo;
    RealGlGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &rbo);
    getOut() << "blit rbo " << type << " " << rbo << " " << oldDrawFbo << std::endl; */

    /* for (int i = 0; i < 2; i++) {
      ID3D11Fence *&fence = fenceCache[i];
      // HANDLE &depthEvent = eventCache[i];
      
      ++fenceValue;
      hijackerContext->lpVtbl->Signal(hijackerContext, fence, fenceValue);
      // fence->lpVtbl->SetEventOnCompletion(fence, fenceValue, depthEvent);

      float zBufferParams[2];
      getZBufferParams(nearValue, farValue, reversed, scale, zBufferParams);

      g_hijacker->fnp.call<
        kHijacker_QueueDepthTex,
        int,
        HANDLE,
        std::tuple<float, float>,
        int,
        bool
      >(frontSharedDepthHandle, std::tuple<float, float>(zBufferParams[0], zBufferParams[1]), i, false);
    } */

    glPhase = 0;
  } else {
    glPhase = 0;
  }
}
void (STDMETHODCALLTYPE *RealGlClear)(
  GLbitfield mask
) = nullptr;
void STDMETHODCALLTYPE MineGlClear(
  GLbitfield mask
) {
  // handleGlClearHack();

  // getOut() << "RealGlClear " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl;
  RealGlClear(mask);
}
EGLBoolean (STDMETHODCALLTYPE *RealEGL_MakeCurrent)(
  EGLDisplay display,
 	EGLSurface draw,
 	EGLSurface read,
 	EGLContext context
) = nullptr;
EGLBoolean STDMETHODCALLTYPE MineEGL_MakeCurrent(
  EGLDisplay display,
 	EGLSurface draw,
 	EGLSurface read,
 	EGLContext context
) {
  glPhase = 0;
  // getOut() << "RealEGL_MakeCurrent " << (void *)display << " " << (void *)draw << " " << (void *)read << " " << (void *)context << " " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl;
  return RealEGL_MakeCurrent(display, draw, read, context);
}
/* BOOL (STDMETHODCALLTYPE *RealWglMakeCurrent)(
  HDC arg1,
  HGLRC arg2
) = nullptr;
BOOL STDMETHODCALLTYPE MineWglMakeCurrent(
  HDC arg1,
  HGLRC arg2
) {
  getOut() << "RealWglMakeCurrent" << (void *)arg1 << " " << (void *)arg2 << " " << GetCurrentProcessId() << ":" << GetCurrentThreadId() << std::endl;
  return RealWglMakeCurrent(arg1, arg2);
} */

}

using namespace hijacker;

Hijacker::Hijacker(FnProxy &fnp) : fnp(fnp) {}
/* void Hijacker::ensureClientDevice() {
  if (!hijackerDevice) {
    getOut() << "ensure client device 1" << std::endl;
    int32_t adapterIndex;
    ProxyGetDXGIOutputInfo(&adapterIndex);
    if (adapterIndex == -1) {
      adapterIndex = 0;
    }
    
    getOut() << "ensure client device 2" << std::endl;

    IDXGIFactory1 *dxgi_factory;
    IDXGIAdapter *adapter;
    HRESULT hr = CreateDXGIFactory1(IID_IDXGIFactory1, (void **)&dxgi_factory);
    dxgi_factory->lpVtbl->EnumAdapters(dxgi_factory, adapterIndex, &adapter);

    getOut() << "ensure client device 3" << std::endl;

    ID3D11Device *deviceBasic;
    ID3D11DeviceContext *contextBasic;
    D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_1
    };
    hr = D3D11CreateDevice(
      // adapter, // pAdapter
      NULL, // pAdapter
      D3D_DRIVER_TYPE_HARDWARE, // DriverType
      NULL, // Software
      0, // Flags
      featureLevels, // pFeatureLevels
      ARRAYSIZE(featureLevels), // FeatureLevels
      D3D11_SDK_VERSION, // SDKVersion
      &deviceBasic, // ppDevice
      NULL, // pFeatureLevel
      &contextBasic // ppImmediateContext
    );
    getOut() << "ensure client device 4" << std::endl;
    if (SUCCEEDED(hr)) {
      // nothing
    } else {
      getOut() << "hijacker client device creation failed " << (void *)hr << std::endl;
      abort();
    }

    hr = deviceBasic->lpVtbl->QueryInterface(deviceBasic, IID_ID3D11Device5, (void **)&hijackerDevice);
    if (SUCCEEDED(hr)) {
      // nothing
    } else {
      getOut() << "device query failed" << std::endl;
      abort();
    }

    getOut() << "ensure client device 5" << std::endl;

    ID3D11DeviceContext3 *context3;
    hijackerDevice->lpVtbl->GetImmediateContext3(hijackerDevice, &context3);
    hr = context3->lpVtbl->QueryInterface(context3, IID_ID3D11DeviceContext4, (void **)&hijackerContext);
    getOut() << "ensure client device 6" << std::endl;
    if (SUCCEEDED(hr)) {
      // nothing
    } else {
      getOut() << "context query failed" << std::endl;
      abort();
    }
    
    getOut() << "ensure client device 7" << std::endl;
    
    dxgi_factory->lpVtbl->Release(dxgi_factory);
    deviceBasic->lpVtbl->Release(deviceBasic);
    contextBasic->lpVtbl->Release(contextBasic);
    context3->lpVtbl->Release(context3);
    
    getOut() << "ensure client device 8" << std::endl;
  }
} */
void Hijacker::hijackPre() {
  if (!hijackedDx) {
    /* getOut() << "hijack pre 1" << std::endl;
    
    Microsoft::WRL::ComPtr<IDXGIFactory1> dxgi_factory;
    IDXGIAdapter *adapter;
    hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), &dxgi_factory);
    dxgi_factory->EnumAdapters(adapterIndex, &adapter);
    
    adapter->lpVtbl->Release(adapter);
    
    getOut() << "hijack pre 2" << std::endl; */
    
    /* HMODULE dll = GetModuleHandleA("Blocks.exe");
    decltype(D3D11CreateDevice) *lol = (decltype(D3D11CreateDevice) *)GetProcAddress(
      dll,
      "D3D11CreateDevice"
    ); */
    getOut() << "hijack pre 1" << std::endl;

    ID3D11Device *deviceBasic;
    ID3D11DeviceContext *contextBasic;
    D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_1
    };
    HRESULT hr = D3D11CreateDevice(
      NULL, // pAdapter
      D3D_DRIVER_TYPE_HARDWARE, // DriverType
      NULL, // Software
      0, // Flags
      featureLevels, // pFeatureLevels
      ARRAYSIZE(featureLevels), // FeatureLevels
      D3D11_SDK_VERSION, // SDKVersion
      &deviceBasic, // ppDevice
      NULL, // pFeatureLevel
      &contextBasic // ppImmediateContext
    );
    getOut() << "hijack pre 2" << std::endl;
    if (SUCCEEDED(hr)) {
      // nothing
    } else {
      getOut() << "hijack dx device creation failed " << (void *)hr << std::endl;
      abort();
    }
    hijackDx(contextBasic);
    getOut() << "hijack pre 3" << std::endl;
    
    /* HMODULE dll = GetModuleHandleA("d3d11.dll");
    FARPROC lol = GetProcAddress(
      dll,
      "D3D11CreateDevice"
    );
    
    getOut() << "hijack pre 1 " << (void *)lol << " " << (void *)D3D11CreateDevice << std::endl;
    LONG error = DetourTransactionBegin();
    checkDetourError("DetourTransactionBegin", error);

    error = DetourUpdateThread(GetCurrentThread());
    checkDetourError("DetourUpdateThread", error);
    
    RealD3D11CreateDeviceAndSwapChain = D3D11CreateDeviceAndSwapChain;
    getOut() << "hijack pre 2 " << (void *)RealD3D11CreateDeviceAndSwapChain << std::endl;
    error = DetourAttach(&(PVOID&)RealD3D11CreateDeviceAndSwapChain, MineD3D11CreateDeviceAndSwapChain);
    checkDetourError("RealD3D11CreateDeviceAndSwapChain", error);
    
    RealD3D11CreateDevice = (decltype(RealD3D11CreateDevice))lol;
    getOut() << "hijack pre 3 " << (void *)RealD3D11CreateDevice << std::endl;
    error = DetourAttach(&(PVOID&)RealD3D11CreateDevice, MineD3D11CreateDevice);
    checkDetourError("RealD3D11CreateDevice", error);
  
    error = DetourTransactionCommit();
    checkDetourError("DetourTransactionCommit", error);

    getOut() << "hijack pre 4" << std::endl; */
  }
}
/* LRESULT WINAPI DLLWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    getOut() << "dll window proc " << msg << std::endl;
    switch (msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        // handle other messages.

        default: // anything we dont handle.
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0; // just in case
} */

inline void *get_offset_addr(HMODULE module, uintptr_t offset) {
	return (void *)((uintptr_t)module + offset);
}

void Hijacker::hijackDxgi(HINSTANCE hinstDLL) {
  if (!hijackedDxgi) {
    // Semaphore s;
    
    /* getOut() << "threading 1" << std::endl;
    // std::thread([&]() -> void {
      getOut() << "threading 2" << std::endl;
      WNDCLASSEX wc{};
      wc.cbSize = sizeof(WNDCLASSEX);
      wc.style = CS_HREDRAW | CS_VREDRAW;
      wc.lpfnWndProc = DLLWindowProc;
      wc.hInstance = hinstDLL;
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
      wc.lpszClassName = "RT2";
      auto result = RegisterClassExA(&wc);
      if (!result) {
        getOut() << "failed to register class: " <<  (void *)GetLastError() << std::endl;
      }

      GetLastError();
      
      getOut() << "threading 3" << std::endl;

      HWND fakeHWnd = CreateWindowExA(
        NULL,
        wc.lpszClassName,    // name of the window class
        "d3d10 get-offset window",   // title of the window
        WS_OVERLAPPEDWINDOW,    // window style
        10,    // x-position of the window
        10,    // y-position of the window
        512,    // width of the window
        512,    // height of the window
        NULL,    // we have no parent window, NULL
        NULL,    // we aren't using menus, NULL
        hinstDLL,    // application handle
        NULL
      );
      // 0 RT2 Reality Tab 2 2147483648 0  0 2 2 0 0 00007FF7B9F80000 0
      getOut() << "call with: " <<
        NULL << " " <<
        "RT2" << " " <<    // name of the window class
        "d3d10 get-offset window" << " " <<   // title of the window
        WS_POPUP << " " <<    // window style
        0 << " " << " " <<    // x-position of the window
        0 << " " <<    // y-position of the window
        2 << " " <<    // width of the window
        2 << " " <<    // height of the window
        NULL << " " <<    // we have no parent window, NULL
        NULL << " " <<    // we aren't using menus, NULL
        hinstDLL << " " <<
        fakeHWnd << " " <<
        NULL <<
        std::endl;
      if (!fakeHWnd) {
        getOut() << "failed to create fake dxgi window: " << (void *)GetLastError() << std::endl;
        abort();
      }

      ID3D11Device *deviceBasic;
      ID3D11DeviceContext *contextBasic;
      IDXGISwapChain *swapChain;

      D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1
      };
      DXGI_SWAP_CHAIN_DESC swapChainDesc{};
      swapChainDesc.BufferCount = 2;
      swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      swapChainDesc.BufferDesc.Width = 2;
      swapChainDesc.BufferDesc.Height = 2;
      swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      swapChainDesc.OutputWindow = fakeHWnd;
      swapChainDesc.SampleDesc.Count = 1;
      swapChainDesc.Windowed = true;

      HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL, // pAdapter
        D3D_DRIVER_TYPE_HARDWARE, // DriverType
        NULL, // Software
        D3D11_CREATE_DEVICE_DEBUG, // Flags
        featureLevels, // pFeatureLevels
        ARRAYSIZE(featureLevels), // FeatureLevels
        D3D11_SDK_VERSION, // SDKVersion
        &swapChainDesc, // pSwapChainDesc,
        &swapChain, // ppSwapChain
        &deviceBasic, // ppDevice
        NULL, // pFeatureLevel
        &contextBasic // ppImmediateContext
      );
      if (FAILED(hr)) {
        getOut() << "failed to create fake dxgi window: " << (void *)hr << " " << (void *)GetLastError() << std::endl;
        abort();
      }

      getOut() << "threading 4" << std::endl;
      getOut() << "created swap chain: " << (void *)swapChain << std::endl; */
      
      HMODULE dxgiModule = LoadLibraryA("dxgi.dll");
      if (!dxgiModule) {
        getOut() << "failed to load dxgi module" << std::endl;
        abort();
      }
      
      HMODULE dcompModule = LoadLibraryA("dcomp.dll");
      if (!dcompModule) {
        getOut() << "failed to load dcomp module" << std::endl;
        abort();
      }
    
      LONG error = DetourTransactionBegin();
      checkDetourError("DetourTransactionBegin", error);

      error = DetourUpdateThread(GetCurrentThread());
      checkDetourError("DetourUpdateThread", error);
      
      RealPresent = (decltype(RealPresent))get_offset_addr(dxgiModule, g_offsets->Present);
      // getOut() << "got real present0 " << g_offsets->Present << " " << RealPresent << std::endl;
      error = DetourAttach(&(PVOID&)RealPresent, MinePresent);
      checkDetourError("RealPresent", error);
      
      RealPresent1 = (decltype(RealPresent1))get_offset_addr(dxgiModule, g_offsets->Present1);
      // getOut() << "got real present1 " << g_offsets->Present1 << " " << RealPresent1 << std::endl;
      error = DetourAttach(&(PVOID&)RealPresent1, MinePresent1);
      checkDetourError("RealPresent1", error);
      
      /* RealCreateTargetForHwnd = (decltype(RealCreateTargetForHwnd))get_offset_addr(dcompModule, g_offsets->CreateTargetForHwnd);
      error = DetourAttach(&(PVOID&)RealCreateTargetForHwnd, MineCreateTargetForHwnd);
      checkDetourError("RealCreateTargetForHwnd", error); */
      
      /* RealCreateWindowExA = CreateWindowExA;
      error = DetourAttach(&(PVOID&)RealCreateWindowExA, MineCreateWindowExA);
      checkDetourError("RealCreateWindowExA", error);
      
      RealCreateWindowExW = CreateWindowExW;
      error = DetourAttach(&(PVOID&)RealCreateWindowExW, MineCreateWindowExW);
      checkDetourError("RealCreateWindowExW", error); */

      error = DetourTransactionCommit();
      checkDetourError("DetourTransactionCommit", error);
      
      // getOut() << "threading 5" << std::endl;
      
      // s.unlock();
    // }).detach();
    
    // getOut() << "threading 6" << std::endl;
    
    // s.lock();
    
    // getOut() << "threading 7" << std::endl;
    
    hijackedDxgi = true;
  }
}
void Hijacker::unhijackDxgi() {
  if (hijackedDxgi) {
    LONG error = DetourTransactionBegin();
    checkDetourError("DetourTransactionBegin", error);

    error = DetourUpdateThread(GetCurrentThread());
    checkDetourError("DetourUpdateThread", error);
    
    error = DetourDetach(&(PVOID&)RealPresent, MinePresent);
    checkDetourError("RealPresent", error);
    
    error = DetourDetach(&(PVOID&)RealPresent1, MinePresent1);
    checkDetourError("RealPresent1", error);

    /* error = DetourDetach(&(PVOID&)RealCreateTargetForHwnd, MineCreateTargetForHwnd);
    checkDetourError("RealCreateTargetForHwnd", error); */

    /* error = DetourDetach(&(PVOID&)RealCreateWindowExA, MineCreateWindowExA);
    checkDetourError("RealCreateWindowExA", error);
    
    error = DetourDetach(&(PVOID&)RealCreateWindowExW, MineCreateWindowExW);
    checkDetourError("RealCreateWindowExW", error); */
    
    error = DetourTransactionCommit();
    checkDetourError("DetourTransactionCommit", error);
    
    hijackedDxgi = false;
  }
}
void Hijacker::hijackDx(ID3D11DeviceContext *context) {
  if (!hijackedDx) {
    ID3D11DeviceContext1 *context1;
    HRESULT hr = context->lpVtbl->QueryInterface(context, IID_ID3D11DeviceContext1, (void **)&context1);
    if (SUCCEEDED(hr)) {
      // nothing
    } else {
      getOut() << "hijack failed to get context1: " << (void *)hr << std::endl;
    }
    
    ID3D11Device *device;
    context->lpVtbl->GetDevice(context, &device);

    LONG error = DetourTransactionBegin();
    checkDetourError("DetourTransactionBegin", error);

    error = DetourUpdateThread(GetCurrentThread());
    checkDetourError("DetourUpdateThread", error);
    
    RealOMGetRenderTargets = context->lpVtbl->OMGetRenderTargets;
    error = DetourAttach(&(PVOID&)RealOMGetRenderTargets, MineOMGetRenderTargets);
    checkDetourError("RealOMGetRenderTargets", error);
    
    RealOMSetRenderTargets = context->lpVtbl->OMSetRenderTargets;
    error = DetourAttach(&(PVOID&)RealOMSetRenderTargets, MineOMSetRenderTargets);
    checkDetourError("RealOMSetRenderTargets", error);

    RealOMSetDepthStencilState = context->lpVtbl->OMSetDepthStencilState;
    error = DetourAttach(&(PVOID&)RealOMSetDepthStencilState, MineOMSetDepthStencilState);
    checkDetourError("RealOMSetDepthStencilState", error);

    RealDraw = context->lpVtbl->Draw;
    error = DetourAttach(&(PVOID&)RealDraw, MineDraw);
    checkDetourError("RealDraw", error);
    
    RealDrawAuto = context->lpVtbl->DrawAuto;
    error = DetourAttach(&(PVOID&)RealDrawAuto, MineDrawAuto);
    checkDetourError("RealDrawAuto", error);
    
    RealDrawIndexed = context->lpVtbl->DrawIndexed;
    error = DetourAttach(&(PVOID&)RealDrawIndexed, MineDrawIndexed);
    checkDetourError("RealDrawIndexed", error);
    
    RealDrawIndexedInstanced = context->lpVtbl->DrawIndexedInstanced;
    error = DetourAttach(&(PVOID&)RealDrawIndexedInstanced, MineDrawIndexedInstanced);
    checkDetourError("RealDrawIndexedInstanced", error);
    
    RealDrawIndexedInstancedIndirect = context->lpVtbl->DrawIndexedInstancedIndirect;
    error = DetourAttach(&(PVOID&)RealDrawIndexedInstancedIndirect, MineDrawIndexedInstancedIndirect);
    checkDetourError("RealDrawIndexedInstancedIndirect", error);
    
    RealDrawInstanced = context->lpVtbl->DrawInstanced;
    error = DetourAttach(&(PVOID&)RealDrawInstanced, MineDrawInstanced);
    checkDetourError("RealDrawInstanced", error);
    
    RealDrawInstancedIndirect = context->lpVtbl->DrawInstancedIndirect;
    error = DetourAttach(&(PVOID&)RealDrawInstancedIndirect, MineDrawInstancedIndirect);
    checkDetourError("RealDrawInstancedIndirect", error);
    
    RealCreateShaderResourceView  = device->lpVtbl->CreateShaderResourceView;
    error = DetourAttach(&(PVOID&)RealCreateShaderResourceView, MineCreateShaderResourceView);
    checkDetourError("RealCreateShaderResourceView", error);
    
    RealCreateDepthStencilView  = device->lpVtbl->CreateDepthStencilView;
    error = DetourAttach(&(PVOID&)RealCreateDepthStencilView, MineCreateDepthStencilView);
    checkDetourError("RealCreateDepthStencilView", error);
    
    RealCreateRenderTargetView  = device->lpVtbl->CreateRenderTargetView;
    error = DetourAttach(&(PVOID&)RealCreateRenderTargetView, MineCreateRenderTargetView);
    checkDetourError("RealCreateRenderTargetView", error);
    
    RealPSSetShaderResources  = context->lpVtbl->PSSetShaderResources;
    error = DetourAttach(&(PVOID&)RealPSSetShaderResources, MinePSSetShaderResources);
    checkDetourError("RealPSSetShaderResources", error);
    
    RealClearRenderTargetView = context->lpVtbl->ClearRenderTargetView;
    error = DetourAttach(&(PVOID&)RealClearRenderTargetView, MineClearRenderTargetView);
    checkDetourError("RealClearRenderTargetView", error);
    
    RealClearDepthStencilView = context->lpVtbl->ClearDepthStencilView;
    error = DetourAttach(&(PVOID&)RealClearDepthStencilView, MineClearDepthStencilView);
    checkDetourError("RealClearDepthStencilView", error);

    RealClearState = context->lpVtbl->ClearState;
    error = DetourAttach(&(PVOID&)RealClearState, MineClearState);
    checkDetourError("RealClearState", error);
    
    RealClearView = context1->lpVtbl->ClearView;
    error = DetourAttach(&(PVOID&)RealClearView, MineClearView);
    checkDetourError("RealClearView", error);

    RealResolveSubresource = context1->lpVtbl->ResolveSubresource;
    error = DetourAttach(&(PVOID&)RealResolveSubresource, MineResolveSubresource);
    checkDetourError("RealResolveSubresource", error);
    
    RealUpdateSubresource = context1->lpVtbl->UpdateSubresource;
    error = DetourAttach(&(PVOID&)RealUpdateSubresource, MineUpdateSubresource);
    checkDetourError("RealUpdateSubresource", error);
    
    RealCreateBuffer = device->lpVtbl->CreateBuffer;
    error = DetourAttach(&(PVOID&)RealCreateBuffer, MineCreateBuffer);
    checkDetourError("RealCreateBuffer", error);
    
    RealMap = context1->lpVtbl->Map;
    error = DetourAttach(&(PVOID&)RealMap, MineMap);
    checkDetourError("RealMap", error);
    
    RealUnmap = context1->lpVtbl->Unmap;
    error = DetourAttach(&(PVOID&)RealUnmap, MineUnmap);
    checkDetourError("RealUnmap", error);
    
    RealCopyResource = context1->lpVtbl->CopyResource;
    error = DetourAttach(&(PVOID&)RealCopyResource, MineCopyResource);
    checkDetourError("RealCopyResource", error);

    RealCopySubresourceRegion = context1->lpVtbl->CopySubresourceRegion;
    error = DetourAttach(&(PVOID&)RealCopySubresourceRegion, MineCopySubresourceRegion);
    checkDetourError("RealCopySubresourceRegion", error);
    
    RealCreateTexture2D = device->lpVtbl->CreateTexture2D;
    error = DetourAttach(&(PVOID&)RealCreateTexture2D, MineCreateTexture2D);
    checkDetourError("RealCreateTexture2D", error);
    
    RealCreateRasterizerState  = device->lpVtbl->CreateRasterizerState;
    error = DetourAttach(&(PVOID&)RealCreateRasterizerState , MineCreateRasterizerState);
    checkDetourError("RealCreateRasterizerState", error);

    RealRSSetState = context1->lpVtbl->RSSetState;
    error = DetourAttach(&(PVOID&)RealRSSetState, MineRSSetState);
    checkDetourError("RealRSSetState", error);

    error = DetourTransactionCommit();
    checkDetourError("DetourTransactionCommit", error);

    context1->lpVtbl->Release(context1);
    device->lpVtbl->Release(device);

    hijackedDx = true;
  }
}
void Hijacker::unhijackDx() {
  if (hijackedDx) {
    LONG error = DetourTransactionBegin();
    checkDetourError("DetourTransactionBegin", error);

    error = DetourUpdateThread(GetCurrentThread());
    checkDetourError("DetourUpdateThread", error);
    
    error = DetourDetach(&(PVOID&)RealOMGetRenderTargets, MineOMGetRenderTargets);
    checkDetourError("RealOMGetRenderTargets", error);
    
    error = DetourDetach(&(PVOID&)RealOMSetRenderTargets, MineOMSetRenderTargets);
    checkDetourError("RealOMSetRenderTargets", error);

    error = DetourDetach(&(PVOID&)RealOMSetDepthStencilState, MineOMSetDepthStencilState);
    checkDetourError("RealOMSetDepthStencilState", error);

    error = DetourDetach(&(PVOID&)RealDraw, MineDraw);
    checkDetourError("RealDraw", error);
    
    error = DetourDetach(&(PVOID&)RealDrawAuto, MineDrawAuto);
    checkDetourError("RealDrawAuto", error);
    
    error = DetourDetach(&(PVOID&)RealDrawIndexed, MineDrawIndexed);
    checkDetourError("RealDrawIndexed", error);
    
    error = DetourDetach(&(PVOID&)RealDrawIndexedInstanced, MineDrawIndexedInstanced);
    checkDetourError("RealDrawIndexedInstanced", error);
    
    error = DetourDetach(&(PVOID&)RealDrawIndexedInstancedIndirect, MineDrawIndexedInstancedIndirect);
    checkDetourError("RealDrawIndexedInstancedIndirect", error);
    
    error = DetourDetach(&(PVOID&)RealDrawInstanced, MineDrawInstanced);
    checkDetourError("RealDrawInstanced", error);
    
    error = DetourDetach(&(PVOID&)RealDrawInstancedIndirect, MineDrawInstancedIndirect);
    checkDetourError("RealDrawInstancedIndirect", error);
    
    error = DetourDetach(&(PVOID&)RealCreateShaderResourceView, MineCreateShaderResourceView);
    checkDetourError("RealCreateShaderResourceView", error);
    
    error = DetourDetach(&(PVOID&)RealCreateDepthStencilView, MineCreateDepthStencilView);
    checkDetourError("RealCreateDepthStencilView", error);
    
    error = DetourDetach(&(PVOID&)RealCreateRenderTargetView, MineCreateRenderTargetView);
    checkDetourError("RealCreateRenderTargetView", error);
    
    error = DetourDetach(&(PVOID&)RealPSSetShaderResources, MinePSSetShaderResources);
    checkDetourError("RealPSSetShaderResources", error);
    
    error = DetourDetach(&(PVOID&)RealClearRenderTargetView, MineClearRenderTargetView);
    checkDetourError("RealClearRenderTargetView", error);
    
    error = DetourDetach(&(PVOID&)RealClearDepthStencilView, MineClearDepthStencilView);
    checkDetourError("RealClearDepthStencilView", error);

    error = DetourDetach(&(PVOID&)RealClearState, MineClearState);
    checkDetourError("RealClearState", error);
    
    error = DetourDetach(&(PVOID&)RealClearView, MineClearView);
    checkDetourError("RealClearView", error);

    error = DetourDetach(&(PVOID&)RealResolveSubresource, MineResolveSubresource);
    checkDetourError("RealResolveSubresource", error);
    
    error = DetourDetach(&(PVOID&)RealUpdateSubresource, MineUpdateSubresource);
    checkDetourError("RealUpdateSubresource", error);
    
    error = DetourDetach(&(PVOID&)RealCreateBuffer, MineCreateBuffer);
    checkDetourError("RealCreateBuffer", error);
    
    error = DetourDetach(&(PVOID&)RealMap, MineMap);
    checkDetourError("RealMap", error);
    
    error = DetourDetach(&(PVOID&)RealUnmap, MineUnmap);
    checkDetourError("RealUnmap", error);
    
    error = DetourDetach(&(PVOID&)RealCopyResource, MineCopyResource);
    checkDetourError("RealCopyResource", error);

    error = DetourDetach(&(PVOID&)RealCopySubresourceRegion, MineCopySubresourceRegion);
    checkDetourError("RealCopySubresourceRegion", error);
    
    error = DetourDetach(&(PVOID&)RealCreateTexture2D, MineCreateTexture2D);
    checkDetourError("RealCreateTexture2D", error);
    
    error = DetourDetach(&(PVOID&)RealCreateRasterizerState , MineCreateRasterizerState);
    checkDetourError("RealCreateRasterizerState", error);

    error = DetourDetach(&(PVOID&)RealRSSetState, MineRSSetState);
    checkDetourError("RealRSSetState", error);

    error = DetourTransactionCommit();
    checkDetourError("DetourTransactionCommit", error);

    hijackedDx = false;
  }
}
void Hijacker::hijackGl() {
  return;
  if (!hijackedGl) {
    HMODULE libGlesV2 = LoadLibraryA("libglesv2.dll");
    HMODULE libOpenGl32 = LoadLibraryA("opengl32.dll");
    HMODULE libEgl = LoadLibraryA("libegl.dll");

    if (libGlesV2 != NULL && libOpenGl32 != NULL && libEgl != NULL) {
      decltype(RealEGL_MakeCurrent) EGL_MakeCurrent = (decltype(RealEGL_MakeCurrent))GetProcAddress(libEgl, "eglMakeCurrent");
      EGL_GetCurrentDisplay = (decltype(EGL_GetCurrentDisplay))GetProcAddress(libEgl, "eglGetCurrentDisplay");
      EGL_ChooseConfig = (decltype(EGL_ChooseConfig))GetProcAddress(libEgl, "eglChooseConfig");
      EGL_CreatePbufferFromClientBuffer = (decltype(EGL_CreatePbufferFromClientBuffer))GetProcAddress(libEgl, "eglCreatePbufferFromClientBuffer");
      EGL_BindTexImage = (decltype(EGL_BindTexImage))GetProcAddress(libEgl, "eglBindTexImage");
      EGL_QueryString = (decltype(EGL_QueryString))GetProcAddress(libEgl, "eglQueryString");
      EGL_QueryDisplayAttribEXT = (decltype(EGL_QueryDisplayAttribEXT))GetProcAddress(libEgl, "eglQueryDisplayAttribEXT");
      EGL_QueryDeviceAttribEXT = (decltype(EGL_QueryDeviceAttribEXT))GetProcAddress(libEgl, "eglQueryDeviceAttribEXT");
      EGL_GetError = (decltype(EGL_GetError))GetProcAddress(libEgl, "eglGetError");

      decltype(RealGlViewport) glViewport = (decltype(RealGlViewport))GetProcAddress(libGlesV2, "glViewport");
      decltype(RealGlGenFramebuffers) glGenFramebuffers = (decltype(RealGlGenFramebuffers))GetProcAddress(libGlesV2, "glGenFramebuffers");
      decltype(RealGlBindFramebuffer) glBindFramebuffer = (decltype(RealGlBindFramebuffer))GetProcAddress(libGlesV2, "glBindFramebuffer");
      decltype(RealGlGenRenderbuffers) glGenRenderbuffers = (decltype(RealGlGenRenderbuffers))GetProcAddress(libGlesV2, "glGenRenderbuffers");
      decltype(RealGlBindRenderbuffer) glBindRenderbuffer = (decltype(RealGlBindRenderbuffer))GetProcAddress(libGlesV2, "glBindRenderbuffer");
      decltype(RealGlFramebufferTexture2D) glFramebufferTexture2D = (decltype(RealGlFramebufferTexture2D))GetProcAddress(libGlesV2, "glFramebufferTexture2D");
      decltype(RealGlFramebufferTexture2DMultisampleEXT) glFramebufferTexture2DMultisampleEXT = (decltype(RealGlFramebufferTexture2DMultisampleEXT))GetProcAddress(libGlesV2, "glFramebufferTexture2DMultisampleEXT");
      decltype(RealGlFramebufferRenderbuffer) glFramebufferRenderbuffer = (decltype(RealGlFramebufferRenderbuffer))GetProcAddress(libGlesV2, "glFramebufferRenderbuffer");
      decltype(RealGlRenderbufferStorageMultisampleEXT) glRenderbufferStorageMultisampleEXT = (decltype(RealGlRenderbufferStorageMultisampleEXT))GetProcAddress(libGlesV2, "glRenderbufferStorageMultisampleEXT");
      decltype(RealGlRenderbufferStorage) glRenderbufferStorage = (decltype(RealGlRenderbufferStorage))GetProcAddress(libGlesV2, "glRenderbufferStorage");
      decltype(RealGlDiscardFramebufferEXT) glDiscardFramebufferEXT = (decltype(RealGlDiscardFramebufferEXT))GetProcAddress(libGlesV2, "glDiscardFramebufferEXT");
      decltype(RealGlDiscardFramebufferEXTContextANGLE) glDiscardFramebufferEXTContextANGLE = (decltype(RealGlDiscardFramebufferEXTContextANGLE))GetProcAddress(libGlesV2, "glDiscardFramebufferEXTContextANGLE");
      decltype(RealGlInvalidateFramebuffer) glInvalidateFramebuffer = (decltype(RealGlInvalidateFramebuffer))GetProcAddress(libGlesV2, "glInvalidateFramebuffer");
      decltype(RealDiscardFramebufferEXT) DiscardFramebufferEXT = (decltype(RealDiscardFramebufferEXT))GetProcAddress(libGlesV2, "?DiscardFramebufferEXT@gl@@YAXIHPEBI@Z");
      decltype(RealGlGenTextures) glGenTextures = (decltype(RealGlGenTextures))GetProcAddress(libGlesV2, "glGenTextures");
      decltype(RealGlBindTexture) glBindTexture = (decltype(RealGlBindTexture))GetProcAddress(libGlesV2, "glBindTexture");
      decltype(RealGlTexImage2D) glTexImage2D = (decltype(RealGlTexImage2D))GetProcAddress(libGlesV2, "glTexImage2D");
      decltype(RealGlTexParameteri) glTexParameteri = (decltype(RealGlTexParameteri))GetProcAddress(libGlesV2, "glTexParameteri");
      decltype(RealGlReadPixels) glReadPixels = (decltype(RealGlReadPixels))GetProcAddress(libGlesV2, "glReadPixels");
      decltype(RealGlTexStorage2DMultisample) glTexStorage2DMultisample = (decltype(RealGlTexStorage2DMultisample))GetProcAddress(libGlesV2, "glTexStorage2DMultisample");
      decltype(RealGlRequestExtensionANGLE) glRequestExtensionANGLE = (decltype(RealGlRequestExtensionANGLE))GetProcAddress(libGlesV2, "glRequestExtensionANGLE");
      decltype(RealGlDeleteTextures) glDeleteTextures = (decltype(RealGlDeleteTextures))GetProcAddress(libGlesV2, "glDeleteTextures");
      decltype(RealGlFenceSync) glFenceSync = (decltype(RealGlFenceSync))GetProcAddress(libGlesV2, "glFenceSync");
      decltype(RealGlDeleteSync) glDeleteSync = (decltype(RealGlDeleteSync))GetProcAddress(libGlesV2, "glDeleteSync");
      decltype(RealGlWaitSync) glWaitSync = (decltype(RealGlWaitSync))GetProcAddress(libGlesV2, "glWaitSync");
      decltype(RealGlClientWaitSync) glClientWaitSync = (decltype(RealGlClientWaitSync))GetProcAddress(libGlesV2, "glClientWaitSync");
      decltype(RealGlDrawElements) glDrawElements = (decltype(RealGlDrawElements))GetProcAddress(libGlesV2, "glDrawElements");
      decltype(RealGlClear) glClear = (decltype(RealGlClear))GetProcAddress(libGlesV2, "glClear");
      decltype(RealGlClearColor) glClearColor = (decltype(RealGlClearColor))GetProcAddress(libGlesV2, "glClearColor");
      decltype(RealGlColorMask) glColorMask = (decltype(RealGlColorMask))GetProcAddress(libGlesV2, "glColorMask");
      decltype(RealGlFlush) glFlush = (decltype(RealGlFlush))GetProcAddress(libGlesV2, "glFlush");
      decltype(RealGlGetError) glGetError = (decltype(RealGlGetError))GetProcAddress(libGlesV2, "glGetError");
  
      LONG error = DetourTransactionBegin();
      checkDetourError("DetourTransactionBegin", error);

      error = DetourUpdateThread(GetCurrentThread());
      checkDetourError("DetourUpdateThread", error);

      RealGlGenFramebuffers = glGenFramebuffers;
      error = DetourAttach(&(PVOID&)RealGlGenFramebuffers, MineGlGenFramebuffers);
      checkDetourError("RealGlGenFramebuffers", error);

      RealGlBindFramebuffer = glBindFramebuffer;
      error = DetourAttach(&(PVOID&)RealGlBindFramebuffer, MineGlBindFramebuffer);
      checkDetourError("RealGlBindFramebuffer", error);

      RealGlGenRenderbuffers = glGenRenderbuffers;
      error = DetourAttach(&(PVOID&)RealGlGenRenderbuffers, MineGlGenRenderbuffers);
      checkDetourError("RealGlGenRenderbuffers", error);
      
      RealGlBindRenderbuffer = glBindRenderbuffer;
      error = DetourAttach(&(PVOID&)RealGlBindRenderbuffer, MineGlBindRenderbuffer);
      checkDetourError("RealGlBindRenderbuffer", error);

      RealGlFramebufferTexture2D = glFramebufferTexture2D;
      error = DetourAttach(&(PVOID&)RealGlFramebufferTexture2D, MineGlFramebufferTexture2D);
      checkDetourError("RealGlFramebufferTexture2D", error);
      
      RealGlFramebufferTexture2DMultisampleEXT = glFramebufferTexture2DMultisampleEXT;
      error = DetourAttach(&(PVOID&)RealGlFramebufferTexture2DMultisampleEXT, MineGlFramebufferTexture2DMultisampleEXT);
      checkDetourError("RealGlFramebufferTexture2DMultisampleEXT", error);
      
      RealGlFramebufferRenderbuffer = glFramebufferRenderbuffer;
      error = DetourAttach(&(PVOID&)RealGlFramebufferRenderbuffer, MineGlFramebufferRenderbuffer);
      checkDetourError("RealGlFramebufferRenderbuffer", error);

      RealGlRenderbufferStorageMultisampleEXT = glRenderbufferStorageMultisampleEXT;
      error = DetourAttach(&(PVOID&)RealGlRenderbufferStorageMultisampleEXT, MineGlRenderbufferStorageMultisampleEXT);
      checkDetourError("RealGlRenderbufferStorageMultisampleEXT", error);
      
      RealGlDiscardFramebufferEXT = glDiscardFramebufferEXT;
      error = DetourAttach(&(PVOID&)RealGlDiscardFramebufferEXT, MineGlDiscardFramebufferEXT);
      checkDetourError("RealGlDiscardFramebufferEXT", error);
      
      RealGlDiscardFramebufferEXTContextANGLE = glDiscardFramebufferEXTContextANGLE;
      error = DetourAttach(&(PVOID&)RealGlDiscardFramebufferEXTContextANGLE, MineGlDiscardFramebufferEXTContextANGLE);
      checkDetourError("RealGlDiscardFramebufferEXTContextANGLE", error);

      RealGlInvalidateFramebuffer = glInvalidateFramebuffer;
      error = DetourAttach(&(PVOID&)RealGlInvalidateFramebuffer, MineGlInvalidateFramebuffer);
      checkDetourError("RealGlInvalidateFramebuffer", error);
      
      RealDiscardFramebufferEXT = DiscardFramebufferEXT;
      error = DetourAttach(&(PVOID&)RealDiscardFramebufferEXT, MineDiscardFramebufferEXT);
      checkDetourError("RealDiscardFramebufferEXT", error);

      RealGlDeleteTextures = glDeleteTextures;
      error = DetourAttach(&(PVOID&)RealGlDeleteTextures, MineGlDeleteTextures);
      checkDetourError("RealGlDeleteTextures", error);
      
      RealGlFenceSync = glFenceSync;
      error = DetourAttach(&(PVOID&)RealGlFenceSync, MineGlFenceSync);
      checkDetourError("RealGlFenceSync", error);
      
      RealGlDeleteSync = glDeleteSync;
      error = DetourAttach(&(PVOID&)RealGlDeleteSync, MineGlDeleteSync);
      checkDetourError("RealGlDeleteSync", error);
      
      RealGlWaitSync = glWaitSync;
      error = DetourAttach(&(PVOID&)RealGlWaitSync, MineGlWaitSync);
      checkDetourError("RealGlWaitSync", error);
      
      RealGlClientWaitSync = glClientWaitSync;
      error = DetourAttach(&(PVOID&)RealGlClientWaitSync, MineGlClientWaitSync);
      checkDetourError("RealGlClientWaitSync", error);

      RealGlClear = glClear;
      error = DetourAttach(&(PVOID&)RealGlClear, MineGlClear);
      checkDetourError("RealGlClear", error);
      
      RealGlClearColor = glClearColor;
      error = DetourAttach(&(PVOID&)RealGlClearColor, MineGlClearColor);
      checkDetourError("RealGlClearColor", error);
      
      RealGlColorMask = glColorMask;
      error = DetourAttach(&(PVOID&)RealGlColorMask, MineGlColorMask);
      checkDetourError("RealGlColorMask", error);
      
      RealEGL_MakeCurrent = EGL_MakeCurrent;
      error = DetourAttach(&(PVOID&)RealEGL_MakeCurrent, MineEGL_MakeCurrent);
      checkDetourError("RealEGL_MakeCurrent", error);

      RealGlGetIntegerv = (decltype(RealGlGetIntegerv))GetProcAddress(libGlesV2, "glGetIntegerv");
      RealGlGetFramebufferAttachmentParameteriv = (decltype(RealGlGetFramebufferAttachmentParameteriv))GetProcAddress(libGlesV2, "glGetFramebufferAttachmentParameteriv");
      RealGlGenTextures = glGenTextures;
      RealGlBindTexture = glBindTexture;
      RealGlTexImage2D = glTexImage2D;
      RealGlTexParameteri = glTexParameteri;
      RealGlReadPixels = glReadPixels;
      RealGlTexStorage2DMultisample = glTexStorage2DMultisample;
      RealGlRenderbufferStorage = glRenderbufferStorage;
      RealGlViewport = glViewport;
      RealGlDrawElements = glDrawElements;
      RealGlFlush = glFlush;
      RealGlGetError = glGetError;

      error = DetourTransactionCommit();
      checkDetourError("DetourTransactionCommit", error);
      
      hijackedGl = true;
    } else {
      getOut() << "failed to load gl hijack libs: " << (void *)libGlesV2 << " " << (void *)libOpenGl32 << " " << GetLastError() << std::endl;
    }
  }
}
void Hijacker::unhijackGl() {
  return;
  if (hijackedGl) {
    LONG error = DetourTransactionBegin();
    checkDetourError("DetourTransactionBegin", error);

    error = DetourUpdateThread(GetCurrentThread());
    checkDetourError("DetourUpdateThread", error);

    error = DetourDetach(&(PVOID&)RealGlGenFramebuffers, MineGlGenFramebuffers);
    checkDetourError("RealGlGenFramebuffers", error);

    error = DetourDetach(&(PVOID&)RealGlBindFramebuffer, MineGlBindFramebuffer);
    checkDetourError("RealGlBindFramebuffer", error);

    error = DetourDetach(&(PVOID&)RealGlGenRenderbuffers, MineGlGenRenderbuffers);
    checkDetourError("RealGlGenRenderbuffers", error);
    
    error = DetourDetach(&(PVOID&)RealGlBindRenderbuffer, MineGlBindRenderbuffer);
    checkDetourError("RealGlBindRenderbuffer", error);

    error = DetourDetach(&(PVOID&)RealGlFramebufferTexture2D, MineGlFramebufferTexture2D);
    checkDetourError("RealGlFramebufferTexture2D", error);
    
    error = DetourDetach(&(PVOID&)RealGlFramebufferTexture2DMultisampleEXT, MineGlFramebufferTexture2DMultisampleEXT);
    checkDetourError("RealGlFramebufferTexture2DMultisampleEXT", error);
    
    error = DetourDetach(&(PVOID&)RealGlFramebufferRenderbuffer, MineGlFramebufferRenderbuffer);
    checkDetourError("RealGlFramebufferRenderbuffer", error);

    error = DetourDetach(&(PVOID&)RealGlRenderbufferStorageMultisampleEXT, MineGlRenderbufferStorageMultisampleEXT);
    checkDetourError("RealGlRenderbufferStorageMultisampleEXT", error);
    
    error = DetourDetach(&(PVOID&)RealGlDiscardFramebufferEXT, MineGlDiscardFramebufferEXT);
    checkDetourError("RealGlDiscardFramebufferEXT", error);
    
    error = DetourDetach(&(PVOID&)RealGlDiscardFramebufferEXTContextANGLE, MineGlDiscardFramebufferEXTContextANGLE);
    checkDetourError("RealGlDiscardFramebufferEXTContextANGLE", error);

    error = DetourDetach(&(PVOID&)RealGlInvalidateFramebuffer, MineGlInvalidateFramebuffer);
    checkDetourError("RealGlInvalidateFramebuffer", error);
    
    error = DetourDetach(&(PVOID&)RealDiscardFramebufferEXT, MineDiscardFramebufferEXT);
    checkDetourError("RealDiscardFramebufferEXT", error);

    error = DetourDetach(&(PVOID&)RealGlDeleteTextures, MineGlDeleteTextures);
    checkDetourError("RealGlDeleteTextures", error);
    
    error = DetourDetach(&(PVOID&)RealGlFenceSync, MineGlFenceSync);
    checkDetourError("RealGlFenceSync", error);
    
    RealGlDeleteSync = glDeleteSync;
    error = DetourAttach(&(PVOID&)RealGlDeleteSync, MineGlDeleteSync);
    checkDetourError("RealGlDeleteSync", error);
    
    error = DetourDetach(&(PVOID&)RealGlWaitSync, MineGlWaitSync);
    checkDetourError("RealGlWaitSync", error);
    
    error = DetourDetach(&(PVOID&)RealGlClientWaitSync, MineGlClientWaitSync);
    checkDetourError("RealGlClientWaitSync", error);

    error = DetourDetach(&(PVOID&)RealGlClear, MineGlClear);
    checkDetourError("RealGlClear", error);
    
    error = DetourDetach(&(PVOID&)RealGlClearColor, MineGlClearColor);
    checkDetourError("RealGlClearColor", error);
    
    error = DetourDetach(&(PVOID&)RealGlColorMask, MineGlColorMask);
    checkDetourError("RealGlColorMask", error);
    
    error = DetourDetach(&(PVOID&)RealEGL_MakeCurrent, MineEGL_MakeCurrent);
    checkDetourError("RealEGL_MakeCurrent", error);
    
    hijackedGl = false;
  }
}