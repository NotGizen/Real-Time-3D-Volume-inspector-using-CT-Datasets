#include "pch.h"
#include "GameCore.h"
#include "GraphicsCore.h"
#include "SystemTime.h"
#include "TextRenderer.h"
#include "GameInput.h"
#include "CommandContext.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "BufferManager.h"
#include "Camera.h"
#include "CameraController.h"
#include "CompiledShaders/RaymarchingCS.h"
#include <memory>

using namespace GameCore;
using namespace Graphics;
using namespace  Math;
namespace
{
    RootSignature mainRS;

    ComputePSO raymarchingCS(L"Raymarching CS");
    Texture volumeTex;

    Camera camera;
    std::unique_ptr<CameraController> cameraController;
	float camFOV = 0.78f;
    float speed = 0.1f;
    struct RayParams
    {
        float camPos[3];   float fovTan;
        float camFwd[3];   float aspect;
        float camRight[3]; float pad0;
        float camUp[3];    float pad1;
        uint32_t outDim[2]; uint32_t pad2[2];
    };
}
class VolumeInspector : public GameCore::IGameApp
{
public:

    VolumeInspector()
    {
    }
    
    virtual void Startup( void ) override;
    virtual void Cleanup( void ) override;

    virtual void Update( float deltaT ) override;
    virtual void RenderScene( void ) override;

private:
};

CREATE_APPLICATION( VolumeInspector )

#define CreatePSO( ObjName, ShaderByteCode ) \
    ObjName.SetRootSignature(mainRS); \
    ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
    ObjName.Finalize();


void VolumeInspector::Startup( void )
{
    // Setup your data
    //Camera
    
    camera.SetEyeAtUp(Vector3(0, 0, 2.5f), Vector3(kZero), Vector3(kYUnitVector));
    camera.SetPerspectiveMatrix(camFOV,
        (float)g_SceneColorBuffer.GetHeight() / g_SceneColorBuffer.GetWidth(), 0.1f, 100.0f);
    camera.Update();
    auto* fps = new FlyingFPSCamera(camera, Vector3(kYUnitVector));
    fps->SetSpeed(100.f);
    cameraController.reset(fps);
    //Textures
    volumeTex.Create3DTex(0, 128, 128, 128, DXGI_FORMAT_R8G8B8A8_UNORM, nullptr);

	//Root signature
    mainRS.Reset(2, 0);
 
    mainRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
    mainRS[1].InitAsConstants(1, sizeof(RayParams) / 4);
    //mainRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

    mainRS.Finalize(L"Raymarching RS");

    //PSO's
    CreatePSO(raymarchingCS, g_pRaymarchingCS);
}

void VolumeInspector::Cleanup( void )
{
    // Free up resources in an orderly fashion
	volumeTex.Destroy();
}

void VolumeInspector::Update( float deltaT )
{
    ScopedTimer _prof(L"Update State");
    cameraController->Update(deltaT);
    // Update something
}

void VolumeInspector::RenderScene( void )
{
    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");
    ComputeContext& Context = gfxContext.GetComputeContext();

    RayParams p = {};
    XMStoreFloat3((XMFLOAT3*)p.camPos, camera.GetPosition());
    XMStoreFloat3((XMFLOAT3*)p.camFwd, camera.GetForwardVec());
    XMStoreFloat3((XMFLOAT3*)p.camRight, camera.GetRightVec());
    XMStoreFloat3((XMFLOAT3*)p.camUp, camera.GetUpVec());
    p.fovTan = tanf(camFOV * 0.5f);
    p.aspect = (float)g_SceneColorBuffer.GetWidth() / g_SceneColorBuffer.GetHeight();
    p.outDim[0] = g_SceneColorBuffer.GetWidth();
    p.outDim[1] = g_SceneColorBuffer.GetHeight();
    // Rendering something

    Context.SetRootSignature(mainRS);
    Context.SetConstantArray(1, sizeof(p) / 4, &p);
    Context.SetPipelineState(raymarchingCS);
    //gfxContext.ClearColor(g_SceneColorBuffer);
    gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
    //Context.TransitionResource(volumeTex, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.SetDynamicDescriptors(0, 0, 1, &g_SceneColorBuffer.GetUAV());

    Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight(), 8, 8);


    //gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    //gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
    //gfxContext.SetViewportAndScissor(0, 0, g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());


    gfxContext.Finish();
}
