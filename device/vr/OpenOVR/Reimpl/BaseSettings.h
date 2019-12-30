#pragma once
#include "BaseCommon.h"

class BaseSettings {
private:
public:
	typedef vr::EVRSettingsError EVRSettingsError;

	virtual const char *GetSettingsErrorNameFromEnum(EVRSettingsError eError);

	// Returns true if file sync occurred (force or settings dirty)
	virtual bool Sync(bool bForce = false, EVRSettingsError *peError = nullptr);

	virtual void SetBool(const char *pchSection, const char *pchSettingsKey, bool bValue, EVRSettingsError *peError = nullptr);
	virtual void SetInt32(const char *pchSection, const char *pchSettingsKey, int32_t nValue, EVRSettingsError *peError = nullptr);
	virtual void SetFloat(const char *pchSection, const char *pchSettingsKey, float flValue, EVRSettingsError *peError = nullptr);
	virtual void SetString(const char *pchSection, const char *pchSettingsKey, const char *pchValue, EVRSettingsError *peError = nullptr);

	// Users of the system need to provide a proper default in default.vrsettings in the resources/settings/ directory
	// of either the runtime or the driver_xxx directory. Otherwise the default will be false, 0, 0.0 or ""
	virtual bool GetBool(const char *pchSection, const char *pchSettingsKey, EVRSettingsError *peError = nullptr);
	virtual int32_t GetInt32(const char *pchSection, const char *pchSettingsKey, EVRSettingsError *peError = nullptr);
	virtual float GetFloat(const char *pchSection, const char *pchSettingsKey, EVRSettingsError *peError = nullptr);
	virtual void GetString(const char *pchSection, const char *pchSettingsKey, VR_OUT_STRING() char *pchValue, uint32_t unValueLen, EVRSettingsError *peError = nullptr);

	virtual void RemoveSection(const char *pchSection, EVRSettingsError *peError = nullptr);
	virtual void RemoveKeyInSection(const char *pchSection, const char *pchSettingsKey, EVRSettingsError *peError = nullptr);
};
