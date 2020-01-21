#include "device/vr/openvr/test/overlayproxy.h"
#include "device/vr/openvr/test/fake_openvr_impl_api.h"

namespace vr {
char kIVROverlay_FindOverlay[] = "Input::FindOverlay";
char kIVROverlay_CreateOverlay[] = "Input::CreateOverlay";
char kIVROverlay_DestroyOverlay[] = "Input::DestroyOverlay";
char kIVROverlay_SetHighQualityOverlay[] = "Input::SetHighQualityOverlay";
char kIVROverlay_GetHighQualityOverlay[] = "Input::GetHighQualityOverlay";
char kIVROverlay_GetOverlayKey[] = "Input::GetOverlayKey";
char kIVROverlay_GetOverlayName[] = "Input::GetOverlayName";
char kIVROverlay_SetOverlayName[] = "Input::SetOverlayName";
char kIVROverlay_GetOverlayImageData[] = "Input::GetOverlayImageData";
char kIVROverlay_GetOverlayErrorNameFromEnum[] = "Input::GetOverlayErrorNameFromEnum";
char kIVROverlay_SetOverlayRenderingPid[] = "Input::SetOverlayRenderingPid";
char kIVROverlay_GetOverlayRenderingPid[] = "Input::GetOverlayRenderingPid";
char kIVROverlay_SetOverlayFlag[] = "Input::SetOverlayFlag";
char kIVROverlay_GetOverlayFlag[] = "Input::GetOverlayFlag";
char kIVROverlay_SetOverlayColor[] = "Input::SetOverlayColor";
char kIVROverlay_GetOverlayColor[] = "Input::GetOverlayColor";
char kIVROverlay_SetOverlayAlpha[] = "Input::SetOverlayAlpha";
char kIVROverlay_GetOverlayAlpha[] = "Input::GetOverlayAlpha";
char kIVROverlay_SetOverlayTexelAspect[] = "Input::SetOverlayTexelAspect";
char kIVROverlay_GetOverlayTexelAspect[] = "Input::SetOverlayTexelAspect";
char kIVROverlay_SetOverlaySortOrder[] = "Input::SetOverlaySortOrder";
char kIVROverlay_GetOverlaySortOrder[] = "Input::GetOverlaySortOrder";
char kIVROverlay_SetOverlayWidthInMeters[] = "Input::SetOverlayWidthInMeters";
char kIVROverlay_GetOverlayWidthInMeters[] = "Input::GetOverlayWidthInMeters";
char kIVROverlay_SetOverlayCurvature[] = "Input::SetOverlayCurvature";
char kIVROverlay_GetOverlayCurvature[] = "Input::GetOverlayCurvature";
char kIVROverlay_SetOverlayAutoCurveDistanceRangeInMeters[] = "Input::SetOverlayAutoCurveDistanceRangeInMeters";
char kIVROverlay_GetOverlayAutoCurveDistanceRangeInMeters[] = "Input::GetOverlayAutoCurveDistanceRangeInMeters";
char kIVROverlay_SetOverlayTextureColorSpace[] = "Input::SetOverlayTextureColorSpace";
char kIVROverlay_GetOverlayTextureColorSpace[] = "Input::GetOverlayTextureColorSpace";
char kIVROverlay_SetOverlayTextureBounds[] = "Input::SetOverlayTextureBounds";
char kIVROverlay_GetOverlayTextureBounds[] = "Input::GetOverlayTextureBounds";
char kIVROverlay_GetOverlayRenderModel[] = "Input::GetOverlayRenderModel";
char kIVROverlay_SetOverlayRenderModel[] = "Input::SetOverlayRenderModel";
char kIVROverlay_GetOverlayTransformType[] = "Input::GetOverlayTransformType";
char kIVROverlay_SetOverlayTransformAbsolute[] = "Input::SetOverlayTransformAbsolute";
char kIVROverlay_GetOverlayTransformAbsolute[] = "Input::GetOverlayTransformAbsolute";
char kIVROverlay_SetOverlayTransformTrackedDeviceRelative[] = "Input::SetOverlayTransformTrackedDeviceRelative";
char kIVROverlay_GetOverlayTransformTrackedDeviceRelative[] = "Input::GetOverlayTransformTrackedDeviceRelative";
char kIVROverlay_SetOverlayTransformTrackedDeviceComponent[] = "Input::SetOverlayTransformTrackedDeviceComponent";
char kIVROverlay_GetOverlayTransformTrackedDeviceComponent[] = "Input::GetOverlayTransformTrackedDeviceComponent";
char kIVROverlay_GetOverlayTransformOverlayRelative[] = "Input::GetOverlayTransformOverlayRelative";
char kIVROverlay_SetOverlayTransformOverlayRelative[] = "Input::SetOverlayTransformOverlayRelative";
char kIVROverlay_ShowOverlay[] = "Input::ShowOverlay";
char kIVROverlay_HideOverlay[] = "Input::HideOverlay";
char kIVROverlay_IsOverlayVisible[] = "Input::IsOverlayVisible";
char kIVROverlay_GetTransformForOverlayCoordinates[] = "Input::GetTransformForOverlayCoordinates";
char kIVROverlay_PollNextOverlayEvent[] = "Input::PollNextOverlayEvent";
char kIVROverlay_GetOverlayInputMethod[] = "Input::GetOverlayInputMethod";
char kIVROverlay_SetOverlayInputMethod[] = "Input::SetOverlayInputMethod";
char kIVROverlay_GetOverlayMouseScale[] = "Input::GetOverlayMouseScale";
char kIVROverlay_SetOverlayMouseScale[] = "Input::SetOverlayMouseScale";
char kIVROverlay_ComputeOverlayIntersection[] = "Input::ComputeOverlayIntersection";
char kIVROverlay_HandleControllerOverlayInteractionAsMouse[] = "Input::HandleControllerOverlayInteractionAsMouse";
char kIVROverlay_IsHoverTargetOverlay[] = "Input::IsHoverTargetOverlay";
// char kIVROverlay_GetGamepadFocusOverlay[] = "Input::GetGamepadFocusOverlay";
// char kIVROverlay_SetGamepadFocusOverlay[] = "Input::SetGamepadFocusOverlay";
// char kIVROverlay_SetOverlayNeighbor[] = "Input::SetOverlayNeighbor";
// char kIVROverlay_MoveGamepadFocusToNeighbor[] = "Input::MoveGamepadFocusToNeighbor";
char kIVROverlay_SetOverlayDualAnalogTransform[] = "Input::SetOverlayDualAnalogTransform";
char kIVROverlay_GetOverlayDualAnalogTransform[] = "Input::GetOverlayDualAnalogTransform";
char kIVROverlay_SetOverlayTexture[] = "Input::SetOverlayTexture";
char kIVROverlay_ClearOverlayTexture[] = "Input::ClearOverlayTexture";
char kIVROverlay_SetOverlayRaw[] = "Input::SetOverlayRaw";
char kIVROverlay_SetOverlayFromFile[] = "Input::SetOverlayFromFile";
char kIVROverlay_GetOverlayTexture[] = "Input::GetOverlayTexture";
char kIVROverlay_ReleaseNativeOverlayHandle[] = "Input::ReleaseNativeOverlayHandle";
char kIVROverlay_GetOverlayTextureSize[] = "Input::GetOverlayTextureSize";
char kIVROverlay_CreateDashboardOverlay[] = "Input::CreateDashboardOverlay";
char kIVROverlay_IsDashboardVisible[] = "Input::IsDashboardVisible";
char kIVROverlay_IsActiveDashboardOverlay[] = "Input::IsActiveDashboardOverlay";
char kIVROverlay_SetDashboardOverlaySceneProcess[] = "Input::SetDashboardOverlaySceneProcess";
char kIVROverlay_GetDashboardOverlaySceneProcess[] = "Input::GetDashboardOverlaySceneProcess";
char kIVROverlay_ShowDashboard[] = "Input::ShowDashboard";
char kIVROverlay_GetPrimaryDashboardDevice[] = "Input::GetPrimaryDashboardDevice";
char kIVROverlay_ShowKeyboard[] = "Input::ShowKeyboard";
char kIVROverlay_ShowKeyboardForOverlay[] = "Input::ShowKeyboardForOverlay";
char kIVROverlay_GetKeyboardText[] = "Input::GetKeyboardText";
char kIVROverlay_HideKeyboard[] = "Input::HideKeyboard";
char kIVROverlay_SetKeyboardTransformAbsolute[] = "Input::SetKeyboardTransformAbsolute";
char kIVROverlay_SetKeyboardPositionForOverlay[] = "Input::SetKeyboardPositionForOverlay";
char kIVROverlay_SetOverlayIntersectionMask[] = "Input::SetOverlayIntersectionMask";
char kIVROverlay_GetOverlayFlags[] = "Input::GetOverlayFlags";
char kIVROverlay_ShowMessageOverlay[] = "Input::ShowMessageOverlay";
char kIVROverlay_CloseMessageOverlay[] = "Input::CloseMessageOverlay";

PVROverlay::PVROverlay(IVROverlay *vroverlay, FnProxy &fnp) : vroverlay(vroverlay), fnp(fnp) {
  fnp.reg<
    kIVROverlay_FindOverlay,
    std::tuple<EVROverlayError, VROverlayHandle_t>,
    managed_binary<char>
  >([=](managed_binary<char> overlayKey) {
    VROverlayHandle_t overlayHandle;
    auto error = vroverlay->FindOverlay(overlayKey.data(), &overlayHandle);

    return std::tuple<EVROverlayError, VROverlayHandle_t>(
      error,
      overlayHandle
    );
  });
  fnp.reg<
    kIVROverlay_CreateOverlay,
    std::tuple<EVROverlayError, VROverlayHandle_t>,
    managed_binary<char>,
    managed_binary<char>
  >([=](managed_binary<char> overlayKey, managed_binary<char> overlayName) {
    VROverlayHandle_t overlayHandle;
    auto error = vroverlay->CreateOverlay(overlayKey.data(), overlayName.data(), &overlayHandle);

    return std::tuple<EVROverlayError, VROverlayHandle_t>(
      error,
      overlayHandle
    );
  });
  fnp.reg<
    kIVROverlay_DestroyOverlay,
    EVROverlayError,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    return vroverlay->DestroyOverlay(ulOverlayHandle);
  });
  fnp.reg<
    kIVROverlay_SetHighQualityOverlay,
    int
  >([=]() {
    getOut() << "SetHighQualityOverlay abort" << std::endl;
    abort();
    return 0;
  });
  fnp.reg<
    kIVROverlay_GetHighQualityOverlay,
    int
  >([=]() {
    getOut() << "GetHighQualityOverlay abort" << std::endl;
    abort();
    return 0;
  });
  fnp.reg<
    kIVROverlay_GetOverlayKey,
    std::tuple<uint32_t, managed_binary<char>, EVROverlayError>,
    VROverlayHandle_t,
    uint32_t
  >([=](VROverlayHandle_t ulOverlayHandle, uint32_t unBufferSize) {
    managed_binary<char> buffer(unBufferSize);
    EVROverlayError error;
    auto result = vroverlay->GetOverlayKey(ulOverlayHandle, buffer.data(), unBufferSize, &error);
    return std::tuple<uint32_t, managed_binary<char>, EVROverlayError>(
      result,
      std::move(buffer),
      error
    );
  });
  fnp.reg<
    kIVROverlay_GetOverlayName,
    std::tuple<uint32_t, managed_binary<char>, EVROverlayError>,
    VROverlayHandle_t,
    uint32_t
  >([=](VROverlayHandle_t ulOverlayHandle, uint32_t unBufferSize) {
    managed_binary<char> buffer(unBufferSize);
    EVROverlayError error;
    auto result = vroverlay->GetOverlayName(ulOverlayHandle, buffer.data(), unBufferSize, &error);
    return std::tuple<uint32_t, managed_binary<char>, EVROverlayError>(
      result,
      std::move(buffer),
      error
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayName,
    EVROverlayError,
    VROverlayHandle_t,
    managed_binary<char>
  >([=](VROverlayHandle_t ulOverlayHandle, managed_binary<char> name) {
    return vroverlay->SetOverlayName(ulOverlayHandle, name.data());
  });
  fnp.reg<
    kIVROverlay_GetOverlayImageData,
    std::tuple<EVROverlayError, managed_binary<char>, uint32_t, uint32_t>,
    VROverlayHandle_t,
    uint32_t
  >([=](VROverlayHandle_t ulOverlayHandle, uint32_t unBufferSize) {
    managed_binary<char> buffer(unBufferSize);
    uint32_t width;
    uint32_t height;
    auto error = vroverlay->GetOverlayImageData(ulOverlayHandle, buffer.data(), unBufferSize, &width, &height);
    return std::tuple<EVROverlayError, managed_binary<char>, uint32_t, uint32_t>(
      error,
      std::move(buffer),
      width,
      height
    );
  });
  fnp.reg<
    kIVROverlay_GetOverlayErrorNameFromEnum,
    int
  >([=]() {
    getOut() << "GetOverlayErrorNameFromEnum abort" << std::endl;
    abort();
    return 0;
  });
  fnp.reg<
    kIVROverlay_SetOverlayRenderingPid,
    EVROverlayError,
    VROverlayHandle_t,
    uint32_t
  >([=](VROverlayHandle_t ulOverlayHandle, uint32_t unPID) {
    return vroverlay->SetOverlayRenderingPid(ulOverlayHandle, unPID);
  });
  fnp.reg<
    kIVROverlay_GetOverlayRenderingPid,
    uint32_t,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    return vroverlay->GetOverlayRenderingPid(ulOverlayHandle);
  });
  fnp.reg<
    kIVROverlay_SetOverlayFlag,
    EVROverlayError,
    VROverlayHandle_t,
    VROverlayFlags,
    bool
  >([=](VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool bEnabled) {
    return vroverlay->SetOverlayFlag(ulOverlayHandle, eOverlayFlag, bEnabled);
  });
  fnp.reg<
    kIVROverlay_GetOverlayFlag,
    std::tuple<EVROverlayError, bool>,
    VROverlayHandle_t,
    VROverlayFlags
  >([=](VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag) {
    bool enabled;
    auto error = vroverlay->GetOverlayFlag(ulOverlayHandle, eOverlayFlag, &enabled);
    return std::tuple<EVROverlayError, bool>(
      error,
      enabled
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayColor,
    EVROverlayError,
    VROverlayHandle_t,
    float,
    float,
    float
  >([=](VROverlayHandle_t ulOverlayHandle, float fRed, float fGreen, float fBlue) {
    return vroverlay->SetOverlayColor(ulOverlayHandle, fRed, fGreen, fBlue);
  });
  fnp.reg<
    kIVROverlay_GetOverlayColor,
    std::tuple<EVROverlayError, float, float, float>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    float red;
    float green;
    float blue;
    auto error = vroverlay->GetOverlayColor(ulOverlayHandle, &red, &green, &blue);
    return std::tuple<EVROverlayError, float, float, float>(
      error,
      red,
      green,
      blue
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayAlpha,
    EVROverlayError,
    VROverlayHandle_t,
    float
  >([=](VROverlayHandle_t ulOverlayHandle, float fAlpha) {
    return vroverlay->SetOverlayAlpha(ulOverlayHandle, fAlpha);
  });
  fnp.reg<
    kIVROverlay_GetOverlayAlpha,
    std::tuple<EVROverlayError, float>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    float alpha;
    auto error = vroverlay->GetOverlayAlpha(ulOverlayHandle, &alpha);
    return std::tuple<EVROverlayError, float>(
      error,
      alpha
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayTexelAspect,
    EVROverlayError,
    VROverlayHandle_t,
    float
  >([=](VROverlayHandle_t ulOverlayHandle, float fTexelAspect) {
    return vroverlay->SetOverlayTexelAspect(ulOverlayHandle, fTexelAspect);
  });
  fnp.reg<
    kIVROverlay_GetOverlayTexelAspect,
    std::tuple<EVROverlayError, float>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    float texelAspect;
    auto error = vroverlay->GetOverlayTexelAspect(ulOverlayHandle, &texelAspect);
    return std::tuple<EVROverlayError, float>(
      error,
      texelAspect
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlaySortOrder,
    EVROverlayError,
    VROverlayHandle_t,
    uint32_t
  >([=](VROverlayHandle_t ulOverlayHandle, uint32_t unSortOrder) {
    return vroverlay->SetOverlaySortOrder(ulOverlayHandle, unSortOrder);
  });
  fnp.reg<
    kIVROverlay_GetOverlaySortOrder,
    std::tuple<EVROverlayError, uint32_t>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    uint32_t sortOrder;
    auto error = vroverlay->GetOverlaySortOrder(ulOverlayHandle, &sortOrder);
    return std::tuple<EVROverlayError, uint32_t>(
      error,
      sortOrder
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayWidthInMeters,
    EVROverlayError,
    VROverlayHandle_t,
    float
  >([=](VROverlayHandle_t ulOverlayHandle, float fWidthInMeters) {
    return vroverlay->SetOverlayWidthInMeters(ulOverlayHandle, fWidthInMeters);
  });
  fnp.reg<
    kIVROverlay_GetOverlayWidthInMeters,
    std::tuple<EVROverlayError, float>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    float widthInMeters;
    auto error = vroverlay->GetOverlayWidthInMeters(ulOverlayHandle, &widthInMeters);
    return std::tuple<EVROverlayError, float>(
      error,
      widthInMeters
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayCurvature,
    EVROverlayError,
    VROverlayHandle_t,
    float
  >([=](VROverlayHandle_t ulOverlayHandle, float fCurvature) {
    return vroverlay->SetOverlayCurvature(ulOverlayHandle, fCurvature);
  });
  fnp.reg<
    kIVROverlay_GetOverlayCurvature,
    std::tuple<EVROverlayError, float>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    float fCurvature;
    auto error = vroverlay->GetOverlayCurvature(ulOverlayHandle, &fCurvature);
    return std::tuple<EVROverlayError, float>(
      error,
      fCurvature
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayAutoCurveDistanceRangeInMeters,
    int
  >([=]() {
    getOut() << "SetOverlayAutoCurveDistanceRangeInMeters abort" << std::endl;
    abort();
    return 0;
  });
  fnp.reg<
    kIVROverlay_GetOverlayAutoCurveDistanceRangeInMeters,
    int
  >([=]() {
    getOut() << "SetOverlayAutoCurveDistanceRangeInMeters abort" << std::endl;
    abort();
    return 0;
  });
  fnp.reg<
    kIVROverlay_SetOverlayTextureColorSpace,
    EVROverlayError,
    VROverlayHandle_t,
    EColorSpace
  >([=](VROverlayHandle_t ulOverlayHandle, EColorSpace eTextureColorSpace) {
    return vroverlay->SetOverlayTextureColorSpace(ulOverlayHandle, eTextureColorSpace);
  });
  fnp.reg<
    kIVROverlay_GetOverlayTextureColorSpace,
    std::tuple<EVROverlayError, EColorSpace>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    EColorSpace textureColorSpace;
    auto error = vroverlay->GetOverlayTextureColorSpace(ulOverlayHandle, &textureColorSpace);
    return std::tuple<EVROverlayError, EColorSpace>(
      error,
      textureColorSpace
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayTextureBounds,
    EVROverlayError,
    VROverlayHandle_t,
    VRTextureBounds_t
  >([=](VROverlayHandle_t ulOverlayHandle, VRTextureBounds_t overlayTextureBounds) {
    return vroverlay->SetOverlayTextureBounds(ulOverlayHandle, &overlayTextureBounds);
  });
  fnp.reg<
    kIVROverlay_GetOverlayTextureBounds,
    std::tuple<EVROverlayError, VRTextureBounds_t>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    VRTextureBounds_t overlayTextureBounds;
    auto error = vroverlay->GetOverlayTextureBounds(ulOverlayHandle, &overlayTextureBounds);
    return std::tuple<EVROverlayError, VRTextureBounds_t>(
      error,
      overlayTextureBounds
    );
  });
  fnp.reg<
    kIVROverlay_GetOverlayRenderModel,
    std::tuple<uint32_t, managed_binary<char>, HmdColor_t, EVROverlayError>,
    VROverlayHandle_t,
    uint32_t
  >([=](VROverlayHandle_t ulOverlayHandle, uint32_t unBufferSize) {
    managed_binary<char> buffer(unBufferSize);
    HmdColor_t color;
    EVROverlayError error;
    auto result = vroverlay->GetOverlayRenderModel(ulOverlayHandle, buffer.data(), unBufferSize, &color, &error);
    return std::tuple<uint32_t, managed_binary<char>, HmdColor_t, EVROverlayError>(
      result,
      std::move(buffer),
      color,
      error
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayRenderModel,
    EVROverlayError,
    VROverlayHandle_t,
    managed_binary<char>,
    HmdColor_t
  >([=](VROverlayHandle_t ulOverlayHandle, managed_binary<char> buffer, HmdColor_t color) {
    return vroverlay->SetOverlayRenderModel(ulOverlayHandle, buffer.data(), &color);
  });
  fnp.reg<
    kIVROverlay_GetOverlayTransformType,
    std::tuple<EVROverlayError, VROverlayTransformType>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    VROverlayTransformType overlayTransformType;
    auto error = vroverlay->GetOverlayTransformType(ulOverlayHandle, &overlayTransformType);
    return std::tuple<EVROverlayError, VROverlayTransformType>(
      error,
      overlayTransformType
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayTransformAbsolute,
    EVROverlayError,
    VROverlayHandle_t,
    ETrackingUniverseOrigin,
    HmdMatrix34_t
  >([=](VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, HmdMatrix34_t matTrackingOriginToOverlayTransform) {
    return vroverlay->SetOverlayTransformAbsolute(ulOverlayHandle, eTrackingOrigin, &matTrackingOriginToOverlayTransform);
  });
  fnp.reg<
    kIVROverlay_GetOverlayTransformAbsolute,
    std::tuple<EVROverlayError, ETrackingUniverseOrigin, HmdMatrix34_t>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    ETrackingUniverseOrigin trackingOrigin;
    HmdMatrix34_t matTrackingOriginToOverlayTransform;
    auto error = vroverlay->GetOverlayTransformAbsolute(ulOverlayHandle, &trackingOrigin, &matTrackingOriginToOverlayTransform);
    return std::tuple<EVROverlayError, ETrackingUniverseOrigin, HmdMatrix34_t>(
      error,
      trackingOrigin,
      matTrackingOriginToOverlayTransform
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayTransformTrackedDeviceRelative,
    EVROverlayError,
    VROverlayHandle_t,
    TrackedDeviceIndex_t,
    HmdMatrix34_t
  >([=](VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unTrackedDevice, HmdMatrix34_t matTrackingOriginToOverlayTransform) {
    return vroverlay->SetOverlayTransformTrackedDeviceRelative(ulOverlayHandle, unTrackedDevice, &matTrackingOriginToOverlayTransform);
  });
  fnp.reg<
    kIVROverlay_GetOverlayTransformTrackedDeviceRelative,
    std::tuple<EVROverlayError, TrackedDeviceIndex_t, HmdMatrix34_t>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    TrackedDeviceIndex_t trackedDevice;
    HmdMatrix34_t matTrackingOriginToOverlayTransform;
    auto error = vroverlay->GetOverlayTransformTrackedDeviceRelative(ulOverlayHandle, &trackedDevice, &matTrackingOriginToOverlayTransform);
    return std::tuple<EVROverlayError, TrackedDeviceIndex_t, HmdMatrix34_t>(
      error,
      trackedDevice,
      matTrackingOriginToOverlayTransform
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayTransformTrackedDeviceComponent,
    EVROverlayError,
    VROverlayHandle_t,
    TrackedDeviceIndex_t,
    managed_binary<char>
  >([=](VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unDeviceIndex, managed_binary<char> name) {
    return vroverlay->SetOverlayTransformTrackedDeviceComponent(ulOverlayHandle, unDeviceIndex, name.data());
  });
  fnp.reg<
    kIVROverlay_GetOverlayTransformTrackedDeviceComponent,
    std::tuple<EVROverlayError, TrackedDeviceIndex_t, managed_binary<char>>,
    VROverlayHandle_t,
    uint32_t
  >([=](VROverlayHandle_t ulOverlayHandle, uint32_t unComponentNameSize) {
    TrackedDeviceIndex_t deviceIndex;
    managed_binary<char> componentName(unComponentNameSize);
    auto error = vroverlay->GetOverlayTransformTrackedDeviceComponent(ulOverlayHandle, &deviceIndex, componentName.data(), unComponentNameSize);
    return std::tuple<EVROverlayError, TrackedDeviceIndex_t, managed_binary<char>>(
      error,
      deviceIndex,
      std::move(componentName)
    );
  });
  fnp.reg<
    kIVROverlay_GetOverlayTransformOverlayRelative,
    std::tuple<EVROverlayError, VROverlayHandle_t, HmdMatrix34_t>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    VROverlayHandle_t overlayHandleParent;
    HmdMatrix34_t matParentOverlayToOverlayTransform;
    auto error = vroverlay->GetOverlayTransformOverlayRelative(ulOverlayHandle, &overlayHandleParent, &matParentOverlayToOverlayTransform);
    return std::tuple<EVROverlayError, VROverlayHandle_t, HmdMatrix34_t>(
      error,
      overlayHandleParent,
      matParentOverlayToOverlayTransform
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayTransformOverlayRelative,
    EVROverlayError,
    VROverlayHandle_t,
    VROverlayHandle_t,
    HmdMatrix34_t
  >([=](VROverlayHandle_t ulOverlayHandle, VROverlayHandle_t ulOverlayHandleParent, HmdMatrix34_t matParentOverlayToOverlayTransform) {
    return vroverlay->SetOverlayTransformOverlayRelative(ulOverlayHandle, ulOverlayHandleParent, &matParentOverlayToOverlayTransform);
  });
  fnp.reg<
    kIVROverlay_ShowOverlay,
    EVROverlayError,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    return vroverlay->ShowOverlay(ulOverlayHandle);
  });
  fnp.reg<
    kIVROverlay_HideOverlay,
    EVROverlayError,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    return vroverlay->HideOverlay(ulOverlayHandle);
  });
  fnp.reg<
    kIVROverlay_IsOverlayVisible,
    bool,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    return vroverlay->IsOverlayVisible(ulOverlayHandle);
  });
  fnp.reg<
    kIVROverlay_GetTransformForOverlayCoordinates,
    std::tuple<EVROverlayError, HmdMatrix34_t>,
    VROverlayHandle_t,
    ETrackingUniverseOrigin,
    HmdVector2_t
  >([=](VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, HmdVector2_t coordinatesInOverlay) {
    HmdMatrix34_t matTransform;
    auto error = vroverlay->GetTransformForOverlayCoordinates(ulOverlayHandle, eTrackingOrigin, coordinatesInOverlay, &matTransform);
    return std::tuple<EVROverlayError, HmdMatrix34_t>(
      error,
      matTransform
    );
  });
  fnp.reg<
    kIVROverlay_PollNextOverlayEvent,
    std::tuple<bool, VREvent_t>,
    VROverlayHandle_t,
    uint32_t
  >([=](VROverlayHandle_t ulOverlayHandle, uint32_t uncbVREvent) {
    VREvent_t event;
    auto result = vroverlay->PollNextOverlayEvent(ulOverlayHandle, &event, uncbVREvent);
    return std::tuple<bool, VREvent_t>(
      result,
      event
    );
  });
  fnp.reg<
    kIVROverlay_GetOverlayInputMethod,
    std::tuple<EVROverlayError, VROverlayInputMethod>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    VROverlayInputMethod inputMethod;
    auto error = vroverlay->GetOverlayInputMethod(ulOverlayHandle, &inputMethod);
    return std::tuple<EVROverlayError, VROverlayInputMethod>(
      error,
      inputMethod
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayInputMethod,
    EVROverlayError,
    VROverlayHandle_t,
    VROverlayInputMethod
  >([=](VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod eInputMethod) {
    return vroverlay->SetOverlayInputMethod(ulOverlayHandle, eInputMethod);
  });
  fnp.reg<
    kIVROverlay_GetOverlayMouseScale,
    std::tuple<EVROverlayError, HmdVector2_t>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    HmdVector2_t mouseScale;
    auto error = vroverlay->GetOverlayMouseScale(ulOverlayHandle, &mouseScale);
    return std::tuple<EVROverlayError, HmdVector2_t>(
      error,
      mouseScale
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayMouseScale,
    EVROverlayError,
    VROverlayHandle_t,
    HmdVector2_t
  >([=](VROverlayHandle_t ulOverlayHandle, HmdVector2_t mouseScale) {
    return vroverlay->GetOverlayMouseScale(ulOverlayHandle, &mouseScale);
  });
  fnp.reg<
    kIVROverlay_ComputeOverlayIntersection,
    std::tuple<bool, vr::VROverlayIntersectionResults_t>,
    VROverlayHandle_t,
    vr::VROverlayIntersectionParams_t
  >([=](VROverlayHandle_t ulOverlayHandle, VROverlayIntersectionParams_t params) {
    vr::VROverlayIntersectionResults_t results;
    auto result = vroverlay->ComputeOverlayIntersection(ulOverlayHandle, &params, &results);
    return std::tuple<bool, vr::VROverlayIntersectionResults_t>(
      result,
      results
    );
  });
  fnp.reg<
    kIVROverlay_HandleControllerOverlayInteractionAsMouse,
    int
  >([=]() {
    getOut() << "HandleControllerOverlayInteractionAsMouse abort" << std::endl;
    abort();
    return 0;
  });
  fnp.reg<
    kIVROverlay_IsHoverTargetOverlay,
    bool,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    return vroverlay->IsHoverTargetOverlay(ulOverlayHandle);
  });
  /* fnp.reg<
    kIVROverlay_GetGamepadFocusOverlay,
    int
  >([=]() {
    getOut() << "GetGamepadFocusOverlay abort" << std::endl;
    abort();
    return 0;
  });
  fnp.reg<
    kIVROverlay_SetGamepadFocusOverlay,
    int
  >([=]() {
    getOut() << "SetGamepadFocusOverlay abort" << std::endl;
    abort();
    return 0;
  });
  fnp.reg<
    kIVROverlay_SetOverlayNeighbor,
    EVROverlayError,
    EOverlayDirection,
    VROverlayHandle_t,
    VROverlayHandle_t
  >([=](EOverlayDirection eDirection, VROverlayHandle_t ulFrom, VROverlayHandle_t ulTo) {
    return vroverlay->SetOverlayNeighbor(eDirection, ulFrom, ulTo);
  });
  fnp.reg<
    kIVROverlay_MoveGamepadFocusToNeighbor,
    EVROverlayError,
    EOverlayDirection,
    VROverlayHandle_t
  >([=](EOverlayDirection eDirection, VROverlayHandle_t ulFrom) {
    return vroverlay->MoveGamepadFocusToNeighbor(eDirection, ulFrom);
  }); */
  fnp.reg<
    kIVROverlay_SetOverlayDualAnalogTransform,
    EVROverlayError,
    VROverlayHandle_t,
    EDualAnalogWhich,
    HmdVector2_t,
    float
  >([=](VROverlayHandle_t ulOverlay, EDualAnalogWhich eWhich, HmdVector2_t vCenter, float fRadius) {
    return vroverlay->SetOverlayDualAnalogTransform(ulOverlay, eWhich, &vCenter, fRadius);
  });
  fnp.reg<
    kIVROverlay_GetOverlayDualAnalogTransform,
    std::tuple<EVROverlayError, HmdVector2_t, float>,
    VROverlayHandle_t,
    EDualAnalogWhich
  >([=](VROverlayHandle_t ulOverlay, EDualAnalogWhich eWhich) {
    HmdVector2_t center;
    float radius;
    auto error = vroverlay->GetOverlayDualAnalogTransform(ulOverlay, eWhich, &center, &radius);
    return std::tuple<EVROverlayError, HmdVector2_t, float>(
      error,
      center,
      radius
    );
  });
  fnp.reg<
    kIVROverlay_SetOverlayTexture,
    int
  >([=]() {
    getOut() << "SetOverlayTexture abort" << std::endl;
    abort();
    return 0;
  });
  fnp.reg<
    kIVROverlay_ClearOverlayTexture,
    EVROverlayError,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlay) {
    return vroverlay->ClearOverlayTexture(ulOverlay);
  });
  fnp.reg<
    kIVROverlay_SetOverlayRaw,
    int
  >([=]() {
    getOut() << "SetOverlayRaw abort" << std::endl;
    abort();
    return 0;
  });
  fnp.reg<
    kIVROverlay_SetOverlayFromFile,
    int
  >([=]() {
    getOut() << "SetOverlayFromFile abort" << std::endl;
    abort();
    return 0;
  });
  fnp.reg<
    kIVROverlay_GetOverlayTexture,
    int
  >([=]() {
    getOut() << "GetOverlayTexture abort" << std::endl;
    abort();
    return 0;
  });
  fnp.reg<
    kIVROverlay_ReleaseNativeOverlayHandle,
    int
  >([=]() {
    getOut() << "ReleaseNativeOverlayHandle abort" << std::endl;
    abort();
    return 0;
  });
  fnp.reg<
    kIVROverlay_GetOverlayTextureSize,
    std::tuple<EVROverlayError, uint32_t, uint32_t>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    uint32_t width;
    uint32_t height;
    auto error = vroverlay->GetOverlayTextureSize(ulOverlayHandle, &width, &height);
    return std::tuple<EVROverlayError, uint32_t, uint32_t>(
      error,
      width,
      height
    );
  });
  fnp.reg<
    kIVROverlay_CreateDashboardOverlay,
    std::tuple<EVROverlayError, VROverlayHandle_t, VROverlayHandle_t>,
    managed_binary<char>,
    managed_binary<char>
  >([=](managed_binary<char> overlayKey, managed_binary<char> overlayFriendlyName) {
    VROverlayHandle_t mainHandle;
    VROverlayHandle_t thumbnailHandle;
    auto error = vroverlay->CreateDashboardOverlay(overlayKey.data(), overlayFriendlyName.data(), &mainHandle, &thumbnailHandle);
    return std::tuple<EVROverlayError, VROverlayHandle_t, VROverlayHandle_t>(
      error,
      mainHandle,
      thumbnailHandle
    );
  });
  fnp.reg<
    kIVROverlay_IsDashboardVisible,
    bool
  >([=]() {
    return vroverlay->IsDashboardVisible();
  });
  fnp.reg<
    kIVROverlay_IsActiveDashboardOverlay,
    bool,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    return vroverlay->IsActiveDashboardOverlay(ulOverlayHandle);
  });
  fnp.reg<
    kIVROverlay_SetDashboardOverlaySceneProcess,
    EVROverlayError,
    VROverlayHandle_t,
    uint32_t
  >([=](VROverlayHandle_t ulOverlayHandle, uint32_t unProcessId) {
    return vroverlay->SetDashboardOverlaySceneProcess(ulOverlayHandle, unProcessId);
  });
  fnp.reg<
    kIVROverlay_GetDashboardOverlaySceneProcess,
    std::tuple<EVROverlayError, uint32_t>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    uint32_t processId;
    auto error = vroverlay->GetDashboardOverlaySceneProcess(ulOverlayHandle, &processId);
    return std::tuple<EVROverlayError, uint32_t>(
      error,
      processId
    );
  });
  fnp.reg<
    kIVROverlay_ShowDashboard,
    int,
    managed_binary<char>
  >([=](managed_binary<char> overlayToShow) {
    vroverlay->ShowDashboard(overlayToShow.data());
    return 0;
  });
  fnp.reg<
    kIVROverlay_GetPrimaryDashboardDevice,
    TrackedDeviceIndex_t
  >([=]() {
    return vroverlay->GetPrimaryDashboardDevice();
  });
  fnp.reg<
    kIVROverlay_ShowKeyboard,
    EVROverlayError,
    EGamepadTextInputMode,
    EGamepadTextInputLineMode,
    managed_binary<char>,
    managed_binary<char>,
    std::tuple<uint32_t, bool, uint64_t>
  >([=](
    EGamepadTextInputMode eInputMode,
    EGamepadTextInputLineMode eLineInputMode,
    managed_binary<char> description,
    managed_binary<char> existingText,
    std::tuple<uint32_t, bool, uint64_t> options
  ) {
    uint32_t unCharMax = std::get<0>(options);
    bool bUseMinimalMode = std::get<1>(options);
    uint64_t uUserValue = std::get<2>(options);
    return vroverlay->ShowKeyboard(eInputMode, eLineInputMode, description.data(), unCharMax, existingText.data(), bUseMinimalMode, uUserValue);
  });
  fnp.reg<
    kIVROverlay_ShowKeyboardForOverlay,
    EVROverlayError,
    VROverlayHandle_t,
    EGamepadTextInputMode,
    EGamepadTextInputLineMode,
    managed_binary<char>,
    managed_binary<char>,
    std::tuple<uint32_t, bool, uint64_t>
  >([=](
    VROverlayHandle_t ulOverlayHandle,
    EGamepadTextInputMode eInputMode,
    EGamepadTextInputLineMode eLineInputMode,
    managed_binary<char> description,
    managed_binary<char> existingText,
    std::tuple<uint32_t, bool, uint64_t> options
   ) {
    uint32_t unCharMax = std::get<0>(options);
    bool bUseMinimalMode = std::get<1>(options);
    uint64_t uUserValue = std::get<2>(options);
    return vroverlay->ShowKeyboardForOverlay(ulOverlayHandle, eInputMode, eLineInputMode, description.data(), unCharMax, existingText.data(), bUseMinimalMode, uUserValue);
  });
  fnp.reg<
    kIVROverlay_GetKeyboardText,
    std::tuple<uint32_t, managed_binary<char>>,
    uint32_t
  >([=](uint32_t cchText) {
    managed_binary<char> text(cchText);
    auto result = vroverlay->GetKeyboardText(text.data(), cchText);
    return std::tuple<uint32_t, managed_binary<char>>(
      cchText,
      std::move(text)
    );
  });
  fnp.reg<
    kIVROverlay_HideKeyboard,
    int
  >([=]() {
    vroverlay->HideKeyboard();
    return 0;
  });
  fnp.reg<
    kIVROverlay_SetKeyboardTransformAbsolute,
    int,
    ETrackingUniverseOrigin,
    HmdMatrix34_t
  >([=](ETrackingUniverseOrigin eTrackingOrigin, HmdMatrix34_t matTrackingOriginToKeyboardTransform) {
    vroverlay->SetKeyboardTransformAbsolute(eTrackingOrigin, &matTrackingOriginToKeyboardTransform);
    return 0;
  });
  fnp.reg<
    kIVROverlay_SetKeyboardPositionForOverlay,
    int,
    VROverlayHandle_t,
    HmdRect2_t
  >([=](VROverlayHandle_t ulOverlayHandle, HmdRect2_t avoidRect) {
    vroverlay->SetKeyboardPositionForOverlay(ulOverlayHandle, avoidRect);
    return 0;
  });
  fnp.reg<
    kIVROverlay_SetOverlayIntersectionMask,
    EVROverlayError,
    VROverlayHandle_t,
    managed_binary<vr::VROverlayIntersectionMaskPrimitive_t>,
    uint32_t,
    uint32_t
  >([=](VROverlayHandle_t ulOverlayHandle, managed_binary<vr::VROverlayIntersectionMaskPrimitive_t> maskPrimitives, uint32_t unNumMaskPrimitives, uint32_t unPrimitiveSize) {
    return vroverlay->SetOverlayIntersectionMask(ulOverlayHandle, maskPrimitives.data(), unNumMaskPrimitives, unPrimitiveSize);
  });
  fnp.reg<
    kIVROverlay_GetOverlayFlags,
    std::tuple<EVROverlayError, uint32_t>,
    VROverlayHandle_t
  >([=](VROverlayHandle_t ulOverlayHandle) {
    uint32_t flags;
    auto error = vroverlay->GetOverlayFlags(ulOverlayHandle, &flags);
    return std::tuple<EVROverlayError, uint32_t>(
      error,
      flags
    );
  });
  fnp.reg<
    kIVROverlay_ShowMessageOverlay,
    VRMessageOverlayResponse,
    managed_binary<char>,
    managed_binary<char>,
    managed_binary<char>,
    managed_binary<char>,
    managed_binary<char>,
    managed_binary<char>
  >([=](managed_binary<char> text, managed_binary<char> caption, managed_binary<char> button0Text, managed_binary<char> button1Text, managed_binary<char> button2Text, managed_binary<char> button3Text) {
    return vroverlay->ShowMessageOverlay(
      text.data(),
      caption.data(),
      button0Text.data(),
      button1Text.size() > 0 ? button1Text.data() : nullptr,
      button2Text.size() > 0 ? button2Text.data() : nullptr,
      button3Text.size() > 0 ? button3Text.data() : nullptr
    );
  });
  fnp.reg<
    kIVROverlay_CloseMessageOverlay,
    int
  >([=]() {
    vroverlay->CloseMessageOverlay();
    return 0;
  });
}
EVROverlayError PVROverlay::FindOverlay(const char *pchOverlayKey, VROverlayHandle_t *pOverlayHandle) {
  managed_binary<char> overlayKey(strlen(pchOverlayKey)+1);
  memcpy(overlayKey.data(), pchOverlayKey, overlayKey.size());

  auto result = fnp.call<
    kIVROverlay_FindOverlay,
    std::tuple<EVROverlayError, VROverlayHandle_t>,
    managed_binary<char>
  >(std::move(overlayKey));

  *pOverlayHandle = std::get<1>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::CreateOverlay(const char *pchOverlayKey, const char *pchOverlayName, VROverlayHandle_t *pOverlayHandle) {
  managed_binary<char> overlayKey(strlen(pchOverlayKey)+1);
  memcpy(overlayKey.data(), pchOverlayKey, overlayKey.size());
  managed_binary<char> overlayName(strlen(pchOverlayName)+1);
  memcpy(overlayName.data(), pchOverlayName, overlayName.size());

  auto result = fnp.call<
    kIVROverlay_CreateOverlay,
    std::tuple<EVROverlayError, VROverlayHandle_t>,
    managed_binary<char>,
    managed_binary<char>
  >(std::move(overlayKey), std::move(overlayName));

  *pOverlayHandle = std::get<1>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::DestroyOverlay(VROverlayHandle_t ulOverlayHandle) {
  return fnp.call<
    kIVROverlay_DestroyOverlay,
    EVROverlayError,
    VROverlayHandle_t
  >(ulOverlayHandle);
}
EVROverlayError PVROverlay::SetHighQualityOverlay(VROverlayHandle_t ulOverlayHandle) {
  getOut() << "SetHighQualityOverlay abort" << std::endl;
  abort();
  return VROverlayError_None;
}
VROverlayHandle_t PVROverlay::GetHighQualityOverlay() {
  return fnp.call<
    kIVROverlay_GetHighQualityOverlay,
    VROverlayHandle_t
  >();
}
uint32_t PVROverlay::GetOverlayKey(VROverlayHandle_t ulOverlayHandle, char *pchValue, uint32_t unBufferSize, EVROverlayError *pError) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayKey,
    std::tuple<uint32_t, managed_binary<char>, EVROverlayError>,
    VROverlayHandle_t,
    uint32_t
  >(ulOverlayHandle, unBufferSize);
  memcpy(pchValue, std::get<1>(result).data(), std::get<1>(result).size());
  if (pError) {
    *pError = std::get<2>(result);
  }
  return std::get<0>(result);
}
uint32_t PVROverlay::GetOverlayName(VROverlayHandle_t ulOverlayHandle, char *pchValue, uint32_t unBufferSize, EVROverlayError *pError) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayName,
    std::tuple<uint32_t, managed_binary<char>, EVROverlayError>,
    VROverlayHandle_t,
    uint32_t
  >(ulOverlayHandle, unBufferSize);
  memcpy(pchValue, std::get<1>(result).data(), std::get<1>(result).size());
  if (pError) {
    *pError = std::get<2>(result);
  }
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayName(VROverlayHandle_t ulOverlayHandle, const char *pchName) {
  managed_binary<char> name(strlen(pchName)+1);
  memcpy(name.data(), pchName, name.size());

  return fnp.call<
    kIVROverlay_SetOverlayName,
    EVROverlayError,
    VROverlayHandle_t,
    managed_binary<char>
  >(ulOverlayHandle, std::move(name));
}
EVROverlayError PVROverlay::GetOverlayImageData(VROverlayHandle_t ulOverlayHandle, void *pvBuffer, uint32_t unBufferSize, uint32_t *punWidth, uint32_t *punHeight) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayImageData,
    std::tuple<EVROverlayError, managed_binary<char>, uint32_t, uint32_t>,
    VROverlayHandle_t,
    uint32_t
  >(ulOverlayHandle, unBufferSize);
  memcpy(pvBuffer, std::get<1>(result).data(), std::get<1>(result).size());
  *punWidth = std::get<2>(result);
  *punHeight = std::get<3>(result);
  return std::get<0>(result);
}
const char *PVROverlay::GetOverlayErrorNameFromEnum(EVROverlayError error) {
  getOut() << "GetOverlayErrorNameFromEnum abort" << std::endl;
  abort();
  return "";
}
EVROverlayError PVROverlay::SetOverlayRenderingPid(VROverlayHandle_t ulOverlayHandle, uint32_t unPID) {
  return fnp.call<
    kIVROverlay_SetOverlayRenderingPid,
    EVROverlayError,
    VROverlayHandle_t,
    uint32_t
  >(ulOverlayHandle, unPID);
}
uint32_t PVROverlay::GetOverlayRenderingPid(VROverlayHandle_t ulOverlayHandle) {
  return fnp.call<
    kIVROverlay_GetOverlayRenderingPid,
    uint32_t,
    VROverlayHandle_t
  >(ulOverlayHandle);
}
EVROverlayError PVROverlay::SetOverlayFlag(VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool bEnabled) {
  return fnp.call<
    kIVROverlay_SetOverlayFlag,
    EVROverlayError,
    VROverlayHandle_t,
    VROverlayFlags,
    bool
  >(ulOverlayHandle, eOverlayFlag, bEnabled);
}
EVROverlayError PVROverlay::GetOverlayFlag(VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool *pbEnabled) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayFlag,
    std::tuple<EVROverlayError, bool>,
    VROverlayHandle_t,
    VROverlayFlags
  >(ulOverlayHandle, eOverlayFlag);
  *pbEnabled = std::get<1>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayColor(VROverlayHandle_t ulOverlayHandle, float fRed, float fGreen, float fBlue) {
  return fnp.call<
    kIVROverlay_SetOverlayColor,
    EVROverlayError,
    VROverlayHandle_t,
    float,
    float,
    float
  >(ulOverlayHandle, fRed, fGreen, fBlue);
}
EVROverlayError PVROverlay::GetOverlayColor(VROverlayHandle_t ulOverlayHandle, float *pfRed, float *pfGreen, float *pfBlue) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayColor,
    std::tuple<EVROverlayError, float, float, float>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *pfRed = std::get<1>(result);
  *pfGreen = std::get<2>(result);
  *pfBlue = std::get<3>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayAlpha(VROverlayHandle_t ulOverlayHandle, float fAlpha) {
  return fnp.call<
    kIVROverlay_SetOverlayAlpha,
    EVROverlayError,
    VROverlayHandle_t,
    float
  >(ulOverlayHandle, fAlpha);
}
EVROverlayError PVROverlay::GetOverlayAlpha(VROverlayHandle_t ulOverlayHandle, float *pfAlpha) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayAlpha,
    std::tuple<EVROverlayError, float>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *pfAlpha = std::get<1>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayTexelAspect(VROverlayHandle_t ulOverlayHandle, float fTexelAspect) {
  return fnp.call<
    kIVROverlay_SetOverlayTexelAspect,
    EVROverlayError,
    VROverlayHandle_t,
    float
  >(ulOverlayHandle, fTexelAspect);
}
EVROverlayError PVROverlay::GetOverlayTexelAspect(VROverlayHandle_t ulOverlayHandle, float *pfTexelAspect) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayTexelAspect,
    std::tuple<EVROverlayError, float>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *pfTexelAspect = std::get<1>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlaySortOrder(VROverlayHandle_t ulOverlayHandle, uint32_t unSortOrder) {
  return fnp.call<
    kIVROverlay_SetOverlaySortOrder,
    EVROverlayError,
    VROverlayHandle_t,
    uint32_t
  >(ulOverlayHandle, unSortOrder);
}
EVROverlayError PVROverlay::GetOverlaySortOrder(VROverlayHandle_t ulOverlayHandle, uint32_t *punSortOrder) {
  auto result = fnp.call<
    kIVROverlay_GetOverlaySortOrder,
    std::tuple<EVROverlayError, uint32_t>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *punSortOrder = std::get<1>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayWidthInMeters(VROverlayHandle_t ulOverlayHandle, float fWidthInMeters) {
  return fnp.call<
    kIVROverlay_SetOverlayWidthInMeters,
    EVROverlayError,
    VROverlayHandle_t,
    float
  >(ulOverlayHandle, fWidthInMeters);
}
EVROverlayError PVROverlay::GetOverlayWidthInMeters(VROverlayHandle_t ulOverlayHandle, float *pfWidthInMeters) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayWidthInMeters,
    std::tuple<EVROverlayError, float>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *pfWidthInMeters = std::get<1>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayCurvature( VROverlayHandle_t ulOverlayHandle, float fCurvature ) {
  return fnp.call<
    kIVROverlay_SetOverlayCurvature,
    EVROverlayError,
    VROverlayHandle_t,
    float
  >(ulOverlayHandle, fCurvature);
}
EVROverlayError PVROverlay::GetOverlayCurvature( VROverlayHandle_t ulOverlayHandle, float *pfCurvature ) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayCurvature,
    std::tuple<EVROverlayError, float>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *pfCurvature = std::get<1>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayAutoCurveDistanceRangeInMeters(VROverlayHandle_t ulOverlayHandle, float fMinDistanceInMeters, float fMaxDistanceInMeters) {
  getOut() << "SetOverlayAutoCurveDistanceRangeInMeters abort" << std::endl;
  abort();
  return VROverlayError_None;
}
EVROverlayError PVROverlay::GetOverlayAutoCurveDistanceRangeInMeters(VROverlayHandle_t ulOverlayHandle, float *pfMinDistanceInMeters, float *pfMaxDistanceInMeters) {
  getOut() << "GetOverlayAutoCurveDistanceRangeInMeters abort" << std::endl;
  abort();
  return VROverlayError_None;
}
EVROverlayError PVROverlay::SetOverlayTextureColorSpace(VROverlayHandle_t ulOverlayHandle, EColorSpace eTextureColorSpace) {
  return fnp.call<
    kIVROverlay_SetOverlayTextureColorSpace,
    EVROverlayError,
    VROverlayHandle_t,
    EColorSpace
  >(ulOverlayHandle, eTextureColorSpace);
}
EVROverlayError PVROverlay::GetOverlayTextureColorSpace(VROverlayHandle_t ulOverlayHandle, EColorSpace *peTextureColorSpace) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayTextureColorSpace,
    std::tuple<EVROverlayError, EColorSpace>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *peTextureColorSpace = std::get<1>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayTextureBounds(VROverlayHandle_t ulOverlayHandle, const VRTextureBounds_t *pOverlayTextureBounds) {
  return fnp.call<
    kIVROverlay_SetOverlayTextureBounds,
    EVROverlayError,
    VROverlayHandle_t,
    VRTextureBounds_t
  >(ulOverlayHandle, *pOverlayTextureBounds);
}
EVROverlayError PVROverlay::GetOverlayTextureBounds(VROverlayHandle_t ulOverlayHandle, VRTextureBounds_t *pOverlayTextureBounds) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayTextureBounds,
    std::tuple<EVROverlayError, VRTextureBounds_t>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *pOverlayTextureBounds = std::get<1>(result);
  return std::get<0>(result);
}
uint32_t PVROverlay::GetOverlayRenderModel(VROverlayHandle_t ulOverlayHandle, char *pchValue, uint32_t unBufferSize, HmdColor_t *pColor, EVROverlayError *pError) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayRenderModel,
    std::tuple<uint32_t, managed_binary<char>, HmdColor_t, EVROverlayError>,
    VROverlayHandle_t,
    uint32_t
  >(ulOverlayHandle, unBufferSize);
  memcpy(pchValue, std::get<1>(result).data(), std::get<1>(result).size());
  *pColor = std::get<2>(result);
  if (pError) {
    *pError = std::get<3>(result);
  }
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayRenderModel(VROverlayHandle_t ulOverlayHandle, const char *pchRenderModel, const HmdColor_t *pColor) {
  managed_binary<char> renderModel(strlen(pchRenderModel)+1);
  memcpy(renderModel.data(), pchRenderModel, renderModel.size());
  return fnp.call<
    kIVROverlay_SetOverlayRenderModel,
    EVROverlayError,
    VROverlayHandle_t,
    managed_binary<char>,
    HmdColor_t
  >(ulOverlayHandle, std::move(renderModel), *pColor);
}
EVROverlayError PVROverlay::GetOverlayTransformType(VROverlayHandle_t ulOverlayHandle, VROverlayTransformType *peTransformType) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayTransformType,
    std::tuple<EVROverlayError, VROverlayTransformType>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *peTransformType = std::get<1>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayTransformAbsolute(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, const HmdMatrix34_t *pmatTrackingOriginToOverlayTransform) {
  return fnp.call<
    kIVROverlay_SetOverlayTransformAbsolute,
    EVROverlayError,
    VROverlayHandle_t,
    ETrackingUniverseOrigin,
    HmdMatrix34_t
  >(ulOverlayHandle, eTrackingOrigin, *pmatTrackingOriginToOverlayTransform);
}
EVROverlayError PVROverlay::GetOverlayTransformAbsolute(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin *peTrackingOrigin, HmdMatrix34_t *pmatTrackingOriginToOverlayTransform) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayTransformAbsolute,
    std::tuple<EVROverlayError, ETrackingUniverseOrigin, HmdMatrix34_t>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *peTrackingOrigin = std::get<1>(result);
  *pmatTrackingOriginToOverlayTransform = std::get<2>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayTransformTrackedDeviceRelative(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unTrackedDevice, const HmdMatrix34_t *pmatTrackedDeviceToOverlayTransform) {
  return fnp.call<
    kIVROverlay_SetOverlayTransformTrackedDeviceRelative,
    EVROverlayError,
    VROverlayHandle_t,
    TrackedDeviceIndex_t,
    HmdMatrix34_t
  >(ulOverlayHandle, unTrackedDevice, *pmatTrackedDeviceToOverlayTransform);
}
EVROverlayError PVROverlay::GetOverlayTransformTrackedDeviceRelative(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t *punTrackedDevice, HmdMatrix34_t *pmatTrackedDeviceToOverlayTransform) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayTransformTrackedDeviceRelative,
    std::tuple<EVROverlayError, TrackedDeviceIndex_t, HmdMatrix34_t>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *punTrackedDevice = std::get<1>(result);
  *pmatTrackedDeviceToOverlayTransform = std::get<2>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayTransformTrackedDeviceComponent(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unDeviceIndex, const char *pchComponentName) {
  managed_binary<char> name(strlen(pchComponentName)+1);
  memcpy(name.data(), pchComponentName, name.size());
  return fnp.call<
    kIVROverlay_SetOverlayTransformTrackedDeviceComponent,
    EVROverlayError,
    VROverlayHandle_t,
    TrackedDeviceIndex_t,
    managed_binary<char>
  >(ulOverlayHandle, unDeviceIndex, std::move(name));
}
EVROverlayError PVROverlay::GetOverlayTransformTrackedDeviceComponent(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t *punDeviceIndex, char *pchComponentName, uint32_t unComponentNameSize) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayTransformTrackedDeviceComponent,
    std::tuple<EVROverlayError, TrackedDeviceIndex_t, managed_binary<char>>,
    VROverlayHandle_t,
    uint32_t
  >(ulOverlayHandle, unComponentNameSize);
  *punDeviceIndex = std::get<1>(result);
  memcpy(pchComponentName, std::get<2>(result).data(), std::get<2>(result).size());
  return std::get<0>(result);
}
EVROverlayError PVROverlay::GetOverlayTransformOverlayRelative(VROverlayHandle_t ulOverlayHandle, VROverlayHandle_t *ulOverlayHandleParent, HmdMatrix34_t *pmatParentOverlayToOverlayTransform) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayTransformOverlayRelative,
    std::tuple<EVROverlayError, VROverlayHandle_t, HmdMatrix34_t>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *ulOverlayHandleParent = std::get<1>(result);
  *pmatParentOverlayToOverlayTransform = std::get<2>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayTransformOverlayRelative(VROverlayHandle_t ulOverlayHandle, VROverlayHandle_t ulOverlayHandleParent, const HmdMatrix34_t *pmatParentOverlayToOverlayTransform) {
  return fnp.call<
    kIVROverlay_SetOverlayTransformOverlayRelative,
    EVROverlayError,
    VROverlayHandle_t,
    VROverlayHandle_t,
    HmdMatrix34_t
  >(ulOverlayHandle, ulOverlayHandleParent, *pmatParentOverlayToOverlayTransform);
}
EVROverlayError PVROverlay::ShowOverlay(VROverlayHandle_t ulOverlayHandle) {
  return fnp.call<
    kIVROverlay_ShowOverlay,
    EVROverlayError,
    VROverlayHandle_t
  >(ulOverlayHandle);
}
EVROverlayError PVROverlay::HideOverlay(VROverlayHandle_t ulOverlayHandle) {
  return fnp.call<
    kIVROverlay_HideOverlay,
    EVROverlayError,
    VROverlayHandle_t
  >(ulOverlayHandle);
}
bool PVROverlay::IsOverlayVisible(VROverlayHandle_t ulOverlayHandle) {
  return fnp.call<
    kIVROverlay_IsOverlayVisible,
    bool,
    VROverlayHandle_t
  >(ulOverlayHandle);
}
EVROverlayError PVROverlay::GetTransformForOverlayCoordinates(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, HmdVector2_t coordinatesInOverlay, HmdMatrix34_t *pmatTransform) {
  auto result = fnp.call<
    kIVROverlay_GetTransformForOverlayCoordinates,
    std::tuple<EVROverlayError, HmdMatrix34_t>,
    VROverlayHandle_t,
    ETrackingUniverseOrigin,
    HmdVector2_t
  >(ulOverlayHandle, eTrackingOrigin, coordinatesInOverlay);
  *pmatTransform = std::get<1>(result);
  return std::get<0>(result);
}
bool PVROverlay::PollNextOverlayEvent(VROverlayHandle_t ulOverlayHandle, VREvent_t *pEvent, uint32_t uncbVREvent) {
  auto result = fnp.call<
    kIVROverlay_PollNextOverlayEvent,
    std::tuple<bool, VREvent_t>,
    VROverlayHandle_t,
    uint32_t
  >(ulOverlayHandle, uncbVREvent);
  *pEvent = std::get<1>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::GetOverlayInputMethod(VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod *peInputMethod) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayInputMethod,
    std::tuple<EVROverlayError, VROverlayInputMethod>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *peInputMethod = std::get<1>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayInputMethod(VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod eInputMethod) {
  return fnp.call<
    kIVROverlay_SetOverlayInputMethod,
    EVROverlayError,
    VROverlayHandle_t,
    VROverlayInputMethod
  >(ulOverlayHandle, eInputMethod);
}
EVROverlayError PVROverlay::GetOverlayMouseScale(VROverlayHandle_t ulOverlayHandle, HmdVector2_t *pvecMouseScale) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayMouseScale,
    std::tuple<EVROverlayError, HmdVector2_t>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *pvecMouseScale = std::get<1>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayMouseScale(VROverlayHandle_t ulOverlayHandle, const HmdVector2_t *pvecMouseScale) {
  return fnp.call<
    kIVROverlay_SetOverlayMouseScale,
    EVROverlayError,
    VROverlayHandle_t,
    HmdVector2_t
  >(ulOverlayHandle, *pvecMouseScale);
}
bool PVROverlay::ComputeOverlayIntersection(VROverlayHandle_t ulOverlayHandle, const vr::VROverlayIntersectionParams_t *pParams, vr::VROverlayIntersectionResults_t *pResults) {
  auto result = fnp.call<
    kIVROverlay_ComputeOverlayIntersection,
    std::tuple<bool, vr::VROverlayIntersectionResults_t>,
    VROverlayHandle_t,
    vr::VROverlayIntersectionParams_t
  >(ulOverlayHandle, *pParams);
  *pResults = std::get<1>(result);
  return std::get<0>(result);
}
bool PVROverlay::HandleControllerOverlayInteractionAsMouse(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unControllerDeviceIndex) {
  getOut() << "HandleControllerOverlayInteractionAsMouse abort" << std::endl;
  abort();
  return false;
}
bool PVROverlay::IsHoverTargetOverlay(VROverlayHandle_t ulOverlayHandle) {
  return fnp.call<
    kIVROverlay_IsHoverTargetOverlay,
    bool,
    VROverlayHandle_t
  >(ulOverlayHandle);
}
/* VROverlayHandle_t PVROverlay::GetGamepadFocusOverlay() {
  getOut() << "GetGamepadFocusOverlay abort" << std::endl;
  abort();
  return 0;
}
EVROverlayError PVROverlay::SetGamepadFocusOverlay(VROverlayHandle_t ulNewFocusOverlay) {
  getOut() << "SetGamepadFocusOverlay abort" << std::endl;
  abort();
  return 0;
}
EVROverlayError PVROverlay::SetOverlayNeighbor(EOverlayDirection eDirection, VROverlayHandle_t ulFrom, VROverlayHandle_t ulTo) {
  return fnp.call<
    kIVROverlay_SetOverlayNeighbor,
    EVROverlayError,
    EOverlayDirection,
    VROverlayHandle_t,
    VROverlayHandle_t
  >(eDirection, ulFrom, ulTo);
}
EVROverlayError PVROverlay::MoveGamepadFocusToNeighbor(EOverlayDirection eDirection, VROverlayHandle_t ulFrom) {
  return fnp.call<
    kIVROverlay_MoveGamepadFocusToNeighbor,
    EVROverlayError,
    EOverlayDirection,
    VROverlayHandle_t
  >(eDirection, ulFrom);
} */
EVROverlayError PVROverlay::SetOverlayDualAnalogTransform(VROverlayHandle_t ulOverlay, EDualAnalogWhich eWhich, const HmdVector2_t * vCenter, float fRadius) {
  return fnp.call<
    kIVROverlay_SetOverlayDualAnalogTransform,
    EVROverlayError,
    VROverlayHandle_t,
    EDualAnalogWhich,
    HmdVector2_t,
    float
  >(ulOverlay, eWhich, *vCenter, fRadius);
}
EVROverlayError PVROverlay::GetOverlayDualAnalogTransform(VROverlayHandle_t ulOverlay, EDualAnalogWhich eWhich, HmdVector2_t *pvCenter, float *pfRadius) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayDualAnalogTransform,
    std::tuple<EVROverlayError, HmdVector2_t, float>,
    VROverlayHandle_t,
    EDualAnalogWhich
  >(ulOverlay, eWhich);
  *pvCenter = std::get<1>(result);
  *pfRadius = std::get<2>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::SetOverlayTexture(VROverlayHandle_t ulOverlayHandle, const Texture_t *pTexture) {
  getOut() << "SetOverlayTexture not implemented" << std::endl;
  return VROverlayError_None;
}
EVROverlayError PVROverlay::ClearOverlayTexture(VROverlayHandle_t ulOverlayHandle) {
  return fnp.call<
    kIVROverlay_ClearOverlayTexture,
    EVROverlayError,
    VROverlayHandle_t
  >(ulOverlayHandle);
}
EVROverlayError PVROverlay::SetOverlayRaw(VROverlayHandle_t ulOverlayHandle, void *pvBuffer, uint32_t unWidth, uint32_t unHeight, uint32_t unDepth) {
  getOut() << "SetOverlayRaw abort" << std::endl;
  abort();
  return VROverlayError_None;
}
EVROverlayError PVROverlay::SetOverlayFromFile(VROverlayHandle_t ulOverlayHandle, const char *pchFilePath) {
  getOut() << "SetOverlayFromFile abort" << std::endl;
  abort();
  return VROverlayError_None;
}
EVROverlayError PVROverlay::GetOverlayTexture(VROverlayHandle_t ulOverlayHandle, void **pNativeTextureHandle, void *pNativeTextureRef, uint32_t *pWidth, uint32_t *pHeight, uint32_t *pNativeFormat, ETextureType *pAPIType, EColorSpace *pColorSpace, VRTextureBounds_t *pTextureBounds) {
  getOut() << "GetOverlayTexture abort" << std::endl;
  abort();
  return VROverlayError_None;
}
EVROverlayError PVROverlay::ReleaseNativeOverlayHandle(VROverlayHandle_t ulOverlayHandle, void *pNativeTextureHandle) {
  getOut() << "ReleaseNativeOverlayHandle abort" << std::endl;
  abort();
  return VROverlayError_None;
}
EVROverlayError PVROverlay::GetOverlayTextureSize(VROverlayHandle_t ulOverlayHandle, uint32_t *pWidth, uint32_t *pHeight) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayTextureSize,
    std::tuple<EVROverlayError, uint32_t, uint32_t>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *pWidth = std::get<1>(result);
  *pHeight = std::get<2>(result);
  return std::get<0>(result);
}
EVROverlayError PVROverlay::CreateDashboardOverlay(const char *pchOverlayKey, const char *pchOverlayFriendlyName, VROverlayHandle_t * pMainHandle, VROverlayHandle_t *pThumbnailHandle) {
  managed_binary<char> overlayKey(strlen(pchOverlayKey)+1);
  memcpy(overlayKey.data(), pchOverlayKey, overlayKey.size());
  managed_binary<char> overlayFriendlyName(strlen(pchOverlayFriendlyName)+1);
  memcpy(overlayFriendlyName.data(), pchOverlayFriendlyName, overlayFriendlyName.size());

  auto result = fnp.call<
    kIVROverlay_CreateDashboardOverlay,
    std::tuple<EVROverlayError, VROverlayHandle_t, VROverlayHandle_t>,
    managed_binary<char>,
    managed_binary<char>
  >(std::move(overlayKey), std::move(overlayFriendlyName));
  *pMainHandle = std::get<1>(result);
  *pThumbnailHandle = std::get<2>(result);
  return std::get<0>(result);
}
bool PVROverlay::IsDashboardVisible() {
  return fnp.call<
    kIVROverlay_IsDashboardVisible,
    bool
  >();
}
bool PVROverlay::IsActiveDashboardOverlay(VROverlayHandle_t ulOverlayHandle) {
  return fnp.call<
    kIVROverlay_IsActiveDashboardOverlay,
    bool,
    VROverlayHandle_t
  >(ulOverlayHandle);
}
EVROverlayError PVROverlay::SetDashboardOverlaySceneProcess(VROverlayHandle_t ulOverlayHandle, uint32_t unProcessId) {
  return fnp.call<
    kIVROverlay_SetDashboardOverlaySceneProcess,
    EVROverlayError,
    VROverlayHandle_t,
    uint32_t
  >(ulOverlayHandle, unProcessId);
}
EVROverlayError PVROverlay::GetDashboardOverlaySceneProcess(VROverlayHandle_t ulOverlayHandle, uint32_t *punProcessId) {
  auto result = fnp.call<
    kIVROverlay_GetDashboardOverlaySceneProcess,
    std::tuple<EVROverlayError, uint32_t>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *punProcessId = std::get<1>(result);
  return std::get<0>(result);
}
void PVROverlay::ShowDashboard(const char *pchOverlayToShow) {
  managed_binary<char> overlayToShow(strlen(pchOverlayToShow)+1);
  memcpy(overlayToShow.data(), pchOverlayToShow, overlayToShow.size());

  fnp.call<
    kIVROverlay_ShowDashboard,
    int,
    managed_binary<char>
  >(std::move(overlayToShow));
}
TrackedDeviceIndex_t PVROverlay::GetPrimaryDashboardDevice() {
  return fnp.call<
    kIVROverlay_GetPrimaryDashboardDevice,
    TrackedDeviceIndex_t
  >();
}
EVROverlayError PVROverlay::ShowKeyboard(EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char *pchDescription, uint32_t unCharMax, const char *pchExistingText, bool bUseMinimalMode, uint64_t uUserValue) {
  managed_binary<char> description(strlen(pchDescription)+1);
  memcpy(description.data(), pchDescription, description.size());
  managed_binary<char> existingText(strlen(pchExistingText)+1);
  memcpy(existingText.data(), pchExistingText, existingText.size());

  return fnp.call<
    kIVROverlay_ShowKeyboard,
    EVROverlayError,
    EGamepadTextInputMode,
    EGamepadTextInputLineMode,
    managed_binary<char>,
    managed_binary<char>,
    std::tuple<uint32_t, bool, uint64_t>
  >(eInputMode, eLineInputMode, std::move(description), std::move(existingText), std::tuple<uint32_t, bool, uint64_t>(unCharMax, bUseMinimalMode, uUserValue));
}
EVROverlayError PVROverlay::ShowKeyboardForOverlay(VROverlayHandle_t ulOverlayHandle, EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char *pchDescription, uint32_t unCharMax, const char *pchExistingText, bool bUseMinimalMode, uint64_t uUserValue) {
  managed_binary<char> description(strlen(pchDescription)+1);
  memcpy(description.data(), pchDescription, description.size());
  managed_binary<char> existingText(strlen(pchExistingText)+1);
  memcpy(existingText.data(), pchExistingText, existingText.size());

  return fnp.call<
    kIVROverlay_ShowKeyboardForOverlay,
    EVROverlayError,
    VROverlayHandle_t,
    EGamepadTextInputMode,
    EGamepadTextInputLineMode,
    managed_binary<char>,
    managed_binary<char>,
    std::tuple<uint32_t, bool, uint64_t>
  >(ulOverlayHandle, eInputMode, eLineInputMode, std::move(description), std::move(existingText), std::tuple<uint32_t, bool, uint64_t>(unCharMax, bUseMinimalMode, uUserValue));
}
uint32_t PVROverlay::GetKeyboardText(char *pchText, uint32_t cchText) {
  auto result = fnp.call<
    kIVROverlay_GetKeyboardText,
    std::tuple<uint32_t, managed_binary<char>>,
    uint32_t
  >(cchText);
  memcpy(pchText, std::get<1>(result).data(), std::get<1>(result).size());
  return std::get<0>(result);
}
void PVROverlay::HideKeyboard() {
  fnp.call<
    kIVROverlay_HideKeyboard,
    int
  >();
}
void PVROverlay::SetKeyboardTransformAbsolute(ETrackingUniverseOrigin eTrackingOrigin, const HmdMatrix34_t *pmatTrackingOriginToKeyboardTransform) {
  fnp.call<
    kIVROverlay_SetKeyboardTransformAbsolute,
    int,
    ETrackingUniverseOrigin,
    HmdMatrix34_t
  >(eTrackingOrigin, *pmatTrackingOriginToKeyboardTransform);
}
void PVROverlay::SetKeyboardPositionForOverlay(VROverlayHandle_t ulOverlayHandle, HmdRect2_t avoidRect) {
  fnp.call<
    kIVROverlay_SetKeyboardPositionForOverlay,
    int,
    VROverlayHandle_t,
    HmdRect2_t
  >(ulOverlayHandle, avoidRect);
}
EVROverlayError PVROverlay::SetOverlayIntersectionMask(VROverlayHandle_t ulOverlayHandle, vr::VROverlayIntersectionMaskPrimitive_t *pMaskPrimitives, uint32_t unNumMaskPrimitives, uint32_t unPrimitiveSize) {
  managed_binary<vr::VROverlayIntersectionMaskPrimitive_t> maskPrimitives(unNumMaskPrimitives);
  memcpy(maskPrimitives.data(), pMaskPrimitives, maskPrimitives.size() * sizeof(vr::VROverlayIntersectionMaskPrimitive_t));

  return fnp.call<
    kIVROverlay_SetOverlayIntersectionMask,
    EVROverlayError,
    VROverlayHandle_t,
    managed_binary<vr::VROverlayIntersectionMaskPrimitive_t>,
    uint32_t,
    uint32_t
  >(ulOverlayHandle, std::move(maskPrimitives), unNumMaskPrimitives, unPrimitiveSize);
}
EVROverlayError PVROverlay::GetOverlayFlags(VROverlayHandle_t ulOverlayHandle, uint32_t *pFlags) {
  auto result = fnp.call<
    kIVROverlay_GetOverlayFlags,
    std::tuple<EVROverlayError, uint32_t>,
    VROverlayHandle_t
  >(ulOverlayHandle);
  *pFlags = std::get<1>(result);
  return std::get<0>(result);
}
VRMessageOverlayResponse PVROverlay::ShowMessageOverlay(const char* pchText, const char* pchCaption, const char* pchButton0Text, const char* pchButton1Text, const char* pchButton2Text, const char* pchButton3Text) {
  managed_binary<char> text(strlen(pchText)+1);
  memcpy(text.data(), pchText, text.size());
  managed_binary<char> caption(strlen(pchCaption)+1);
  memcpy(caption.data(), pchCaption, caption.size());
  managed_binary<char> button0Text(strlen(pchButton0Text)+1);
  memcpy(button0Text.data(), pchButton0Text, button0Text.size());
  managed_binary<char> button1Text(pchButton1Text ? strlen(pchButton1Text)+1 : 0);
  if (pchButton1Text) {
    memcpy(button1Text.data(), pchButton1Text, button1Text.size());
  }
  managed_binary<char> button2Text(pchButton2Text ? strlen(pchButton2Text)+1 : 0);
  if (pchButton2Text) {
    memcpy(button2Text.data(), pchButton2Text, button2Text.size());
  }
  managed_binary<char> button3Text(pchButton3Text ? strlen(pchButton3Text)+1 : 0);
  if (pchButton3Text) {
    memcpy(button3Text.data(), pchButton3Text, button3Text.size());
  }

  return fnp.call<
    kIVROverlay_ShowMessageOverlay,
    VRMessageOverlayResponse,
    managed_binary<char>,
    managed_binary<char>,
    managed_binary<char>,
    managed_binary<char>,
    managed_binary<char>,
    managed_binary<char>
  >(std::move(text), std::move(caption), std::move(button0Text), std::move(button1Text), std::move(button2Text), std::move(button3Text));
}
void PVROverlay::CloseMessageOverlay() {
  fnp.call<
    kIVROverlay_CloseMessageOverlay,
    int
  >();
}
}