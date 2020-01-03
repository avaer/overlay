#include "stdafx.h"
#define BASE_IMPL
#include "BaseClientCore.h"
// #include "steamvr_abi.h"

// #include <fstream>

// #include <codecvt>

// #include "Misc/json/json.h"

// #include <ShlObj.h>

#include "device/vr/openvr/test/fake_openvr_impl_api.h"
#include "device/vr/OpenOVR/Reimpl/static_bases.gen.h"

using namespace std;
using namespace vr;

/* enum Runtime {
	Default = 0,
	OpenComposite = 1,
	SteamVR = 2,
}; */

EVRInitError BaseClientCore::Init(vr::EVRApplicationType eApplicationType, const char * pStartupInfo) {
  // getOut() << "core client init 1" << std::endl;
	// EVRInitError err;
	// VR_InitInternal2(&err, eApplicationType, pStartupInfo);
  // return VRInitError_None;
  EVRInitError result;
  // if (vr::g_pvrclientcore) {
    result = vr::g_pvrclientcore->Init(eApplicationType, pStartupInfo);
    // getOut() << "core client init 2" << std::endl;
  /* } else {
    result = vr::g_vrclientcore->Init(eApplicationType, pStartupInfo);
  } */
  return result;
}

void BaseClientCore::Cleanup() {
  getOut() << "core client cleanup" << std::endl;
	// Note that this object is not affected by the shutdown, as it is handled seperately
	//  from all the other interface objects and is only destroyed when the DLL is unloaded.
	//  VR_ShutdownInternal();
  // if (vr::g_pvrclientcore) {
    vr::g_pvrclientcore->Cleanup();
  /* } else {
    vr::g_vrclientcore->Cleanup();
  } */
}

EVRInitError BaseClientCore::IsInterfaceVersionValid(const char * pchInterfaceVersion) {
  getOut() << "check interface version valid" << std::endl;
  return VRInitError_None;
	// return VR_IsInterfaceVersionValid(pchInterfaceVersion) ? VRInitError_None : VRInitError_Init_InvalidInterface;
}

void * BaseClientCore::GetGenericInterface(const char * pchNameAndVersion, EVRInitError * peError) {
  getOut() << "get generic interface " << pchNameAndVersion << std::endl;
  // return nullptr;
  void *iface = CreateInterfaceByName(pchNameAndVersion);
  if (iface) {
    *peError = VRInitError_None;
  }  else {
    *peError = VRInitError_Init_InterfaceNotFound;
  }
  // getOut() << "get generic interface 1 " << pchNameAndVersion << " " << iface << std::endl;
  return iface;
	// return VR_GetGenericInterface(pchNameAndVersion, peError);
}

bool BaseClientCore::BIsHmdPresent() {
  return true;
	// return VR_IsHmdPresent();
}

const char * BaseClientCore::GetEnglishStringForHmdError(vr::EVRInitError eError) {
  // getOut() << "get error string " << (int)eError << std::endl;
  return "error";
	// return VR_GetVRInitErrorAsEnglishDescription(eError);
}

const char * BaseClientCore::GetIDForVRInitError(vr::EVRInitError eError) {
  // getOut() << "get id string " << (int)eError << std::endl;
  return "error";
	// return VR_GetVRInitErrorAsSymbol(eError);
}