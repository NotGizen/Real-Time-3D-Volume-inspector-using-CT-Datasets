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
#include "CompiledShaders/RaymarchingCS.h"

using namespace GameCore;
using namespace Graphics;
namespace
{
    RootSignature mainRS;
    ComputePSO raymarchingCS(L"Raymarching CS");
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
    mainRS.Reset(1, 0);
 
    mainRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);

    mainRS.Finalize(L"Raymarching RS");

    CreatePSO(raymarchingCS, g_pRaymarchingCS);
}

void VolumeInspector::Cleanup( void )
{
    // Free up resources in an orderly fashion
}

void VolumeInspector::Update( float /*deltaT*/ )
{
    ScopedTimer _prof(L"Update State");

    // Update something
}

void VolumeInspector::RenderScene( void )
{
    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

    gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    gfxContext.ClearColor(g_SceneColorBuffer);
    gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
    gfxContext.SetViewportAndScissor(0, 0, g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());

    // Rendering something
    ComputeContext& Context = gfxContext.GetComputeContext();

    Context.SetRootSignature(mainRS);
    Context.SetPipelineState(raymarchingCS);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.SetDynamicDescriptors(0, 0, 1, &g_SceneColorBuffer.GetUAV());

    Context.Dispatch2D(g_SSAOFullScreen.GetWidth(), g_SSAOFullScreen.GetHeight());

    gfxContext.Finish();
}
