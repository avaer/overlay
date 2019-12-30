#ifndef _openvr_test_proxy_h_
#define _openvr_test_proxy_h_

#include <D3D11_1.h>
#include <DXGI1_4.h>
#include <wrl.h>

#include "third_party/openvr/src/headers/openvr.h"
#include "device/vr/openvr/test/out.h"
#include "device/vr/openvr/test/glcontext.h"
#include "device/vr/openvr/test/fnproxy.h"

namespace vr {
class PVRCompositor : public IVRCompositor {
public:
  IVRSystem *vrsystem;
  IVRCompositor *vrcompositor;
  FnProxy &fnp;
  
  Microsoft::WRL::ComPtr<ID3D11Device> device;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
  /* ID3D11Device *device;
  ID3D11DeviceContext *context; */
  ID3D11Texture2D *shTexLeft = nullptr;
  ID3D11Texture2D *shTexRight = nullptr;
  ID3D11Texture2D *shTexLeftOld = nullptr;
  ID3D11Texture2D *shTexRightOld = nullptr;
  ID3D11Texture2D *shTexLeft2 = nullptr;
  ID3D11Texture2D *shTexRight2 = nullptr;
  HANDLE hDevice = NULL;
  HANDLE shTexInLeftObjectHandle = NULL;
  HANDLE shTexInRightObjectHandle = NULL;
  HANDLE shTexOutLeftObjectHandle = NULL;
  HANDLE shTexOutRightObjectHandle = NULL;
  GLuint shFboLeft = 0;
  GLuint shFboRight = 0;
  GLuint shTexInLeft = 0;
  GLuint shTexInRight = 0;
  GLuint shFboId = 0;
  GLuint shVaoId = 0;
  HANDLE shTexLeftHandle = 0;
  HANDLE shTexRightHandle = 0;
  
  HANDLE handleLeftLatched = nullptr;
  HANDLE handleRightLatched = nullptr;
  ID3D11Texture2D *texLeftLatched = nullptr;
  ID3D11Texture2D *texRightLatched = nullptr;
  
  GLFWwindow *subWindow = nullptr;
  
  PVRCompositor(IVRSystem *vrsystem, IVRCompositor *vrcompositor, FnProxy &fnp);
	virtual void SetTrackingSpace( ETrackingUniverseOrigin eOrigin );
	virtual ETrackingUniverseOrigin GetTrackingSpace();
	virtual EVRCompositorError WaitGetPoses( VR_ARRAY_COUNT( unRenderPoseArrayCount ) TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount,
		  VR_ARRAY_COUNT( unGamePoseArrayCount ) TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount );
	virtual EVRCompositorError GetLastPoses( VR_ARRAY_COUNT( unRenderPoseArrayCount ) TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount,
		  VR_ARRAY_COUNT( unGamePoseArrayCount ) TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount );
	virtual EVRCompositorError GetLastPoseForTrackedDeviceIndex( TrackedDeviceIndex_t unDeviceIndex, TrackedDevicePose_t *pOutputPose, TrackedDevicePose_t *pOutputGamePose );
	virtual EVRCompositorError Submit( EVREye eEye, const Texture_t *pTexture, const VRTextureBounds_t* pBounds = 0, EVRSubmitFlags nSubmitFlags = Submit_Default );
	virtual void ClearLastSubmittedFrame();
	virtual void PostPresentHandoff();
	virtual bool GetFrameTiming( Compositor_FrameTiming *pTiming, uint32_t unFramesAgo = 0 );
	virtual uint32_t GetFrameTimings( VR_ARRAY_COUNT( nFrames ) Compositor_FrameTiming *pTiming, uint32_t nFrames );
	virtual float GetFrameTimeRemaining();
	virtual void GetCumulativeStats( Compositor_CumulativeStats *pStats, uint32_t nStatsSizeInBytes );
	virtual void FadeToColor( float fSeconds, float fRed, float fGreen, float fBlue, float fAlpha, bool bBackground = false );
	virtual HmdColor_t GetCurrentFadeColor( bool bBackground = false );
	virtual void FadeGrid( float fSeconds, bool bFadeIn );
	virtual float GetCurrentGridAlpha();
	virtual EVRCompositorError SetSkyboxOverride( VR_ARRAY_COUNT( unTextureCount ) const Texture_t *pTextures, uint32_t unTextureCount );
	virtual void ClearSkyboxOverride();
	virtual void CompositorBringToFront();
	virtual void CompositorGoToBack();
	virtual void CompositorQuit();
	virtual bool IsFullscreen();
	virtual uint32_t GetCurrentSceneFocusProcess();
  virtual uint32_t GetLastFrameRenderer();
	virtual bool CanRenderScene();
	virtual void ShowMirrorWindow();
	virtual void HideMirrorWindow();
	virtual bool IsMirrorWindowVisible();
	virtual void CompositorDumpImages();
	virtual bool ShouldAppRenderWithLowResources();
	virtual void ForceInterleavedReprojectionOn( bool bOverride );
	virtual void ForceReconnectProcess();
	virtual void SuspendRendering( bool bSuspend );
	virtual vr::EVRCompositorError GetMirrorTextureD3D11( vr::EVREye eEye, void *pD3D11DeviceOrResource, void **ppD3D11ShaderResourceView );
	virtual void ReleaseMirrorTextureD3D11( void *pD3D11ShaderResourceView );
	virtual vr::EVRCompositorError GetMirrorTextureGL( vr::EVREye eEye, vr::glUInt_t *pglTextureId, vr::glSharedTextureHandle_t *pglSharedTextureHandle );
	virtual bool ReleaseSharedGLTexture( vr::glUInt_t glTextureId, vr::glSharedTextureHandle_t glSharedTextureHandle );
	virtual void LockGLSharedTextureForAccess( vr::glSharedTextureHandle_t glSharedTextureHandle );
	virtual void UnlockGLSharedTextureForAccess( vr::glSharedTextureHandle_t glSharedTextureHandle );
	virtual uint32_t GetVulkanInstanceExtensionsRequired( VR_OUT_STRING() char *pchValue, uint32_t unBufferSize );
	virtual uint32_t GetVulkanDeviceExtensionsRequired( VkPhysicalDevice_T *pPhysicalDevice, VR_OUT_STRING() char *pchValue, uint32_t unBufferSize );
	virtual void SetExplicitTimingMode( EVRCompositorTimingMode eTimingMode );
	virtual EVRCompositorError SubmitExplicitTimingData();
	virtual bool IsMotionSmoothingEnabled();
	virtual bool IsMotionSmoothingSupported();
	virtual bool IsCurrentSceneFocusAppLoading();
};
}

#endif