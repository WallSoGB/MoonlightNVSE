#include "external/simpleini/SimpleIni.h"
#include "GameData.hpp"
#include "nvse/PluginAPI.h"
#include <shared/NVSEManager/NVSEGlobalManager.hpp>

struct alignas(16) TimeOfDayData {
	TimeOfDayData() : fStart(0.f), fMid(0.f), fEnd(0.f), fTransTime(0.f) {}

	TimeOfDayData(float afStart, float afEnd) : fStart(afStart), fEnd(afEnd) {
		fMid = (fStart + fEnd) * 0.5f;
		fTransTime = (*(float*)0x11CCEA8) * 0.5f;
	}

	TimeOfDayData(float afStart, float afEnd, float afTransTime) : fStart(afStart), fEnd(afEnd), fTransTime(afTransTime) {
		fMid = (fStart + fEnd) * 0.5f;
	}

	float fStart;
	float fMid;
	float fEnd;
	float fTransTime;

	float GetTimeAt(float afPercent) const {
		return (fStart + fEnd) * afPercent;
	}

	float GetTransStart() const {
		return fMid - fTransTime;
	}

	float GetTransEnd() const {
		return fMid + fTransTime;
	}
};

static GameSetting bMoonPhaseBrightness;
static GameSetting fMoonlightMultipliers[5];
static NiMatrix3 kMoonRotation = NiMatrix3( 0.f, 0.f, 0.f,
										   -1.f, 1.f, 0.f,
										    0.f, 0.f, 1.f);

TimeOfDayData kSunrise;
TimeOfDayData kSunset;

enum MoonlightEvents : uint32_t {
	ML_COLOR_START = 0,
	ML_COLOR_END,
};


static __forceinline float Clamp(const float afValue, const float afMin, const float afMax) {
	return afValue < afMin ? afMin : afValue > afMax ? afMax : afValue;
}

static float GetMoonPhaseMult(const float afPhase) {
	float fPhaseMult = 1.f;

	if (!bMoonPhaseBrightness.uValue.b) [[unlikely]]
		return fPhaseMult;

	if (afPhase > 4.25f && afPhase < 5.25f)
		fPhaseMult = fMoonlightMultipliers[0].uValue.f;
	else if (afPhase > 3.25f && afPhase < 4.25f || afPhase > 5.25f && afPhase < 6.25f)
		fPhaseMult = fMoonlightMultipliers[1].uValue.f;
	else if (afPhase > 2.25f && afPhase < 3.25f || afPhase > 6.25f && afPhase < 7.25f)
		fPhaseMult = fMoonlightMultipliers[2].uValue.f;
	else if (afPhase > 1.25f && afPhase < 2.25f || afPhase > 7.25f && afPhase < 8.25f || afPhase < 0.25f)
		fPhaseMult = fMoonlightMultipliers[3].uValue.f;
	else if (afPhase > 0.25f && afPhase < 1.25f)
		fPhaseMult = fMoonlightMultipliers[4].uValue.f;

	return fPhaseMult;
}

static float CalculateNightFade(const float afGameHour, const TimeOfDayData& arSunrise, const TimeOfDayData& arSunset) {
	float fSunsetTransEnd = arSunset.GetTransEnd();
	return afGameHour > fSunsetTransEnd ? (afGameHour - fSunsetTransEnd) : -(afGameHour - arSunrise.fStart);
}

static float CalculateDayFade(const float afGameHour, const TimeOfDayData& arSunrise, const TimeOfDayData& arSunset) {
	// Sunset
	if (afGameHour >= arSunset.fStart && afGameHour <= arSunset.fEnd)
		return -(afGameHour - arSunset.GetTransEnd());

	// Sunrise
	if (afGameHour >= arSunrise.fStart && afGameHour <= arSunrise.fEnd)
		return (afGameHour - arSunrise.fStart);

	return 1.f;
}

static float GetGeometryAlpha(NiAVObject* apObject) {
	const SkyShaderProperty* pSkyProp = reinterpret_cast<SkyShaderProperty*>(apObject->GetProperty(3));
	if (pSkyProp && pSkyProp->m_eShaderType == 13) [[likely]]
		return pSkyProp->kVertexColor.a;

	return 1.f;
}

#pragma optimize( "t", on )
class FNVHooks : public NiLight {
public:
	void SetMoonlightRot(NiMatrix3& arRotation) {
		const TES* pTES = TES::GetSingleton();
		Sky* pSky = Sky::GetSingleton();

		const Moon* pCurrentMoon = pSky->pMasser ? pSky->pMasser : pSky->pSecunda;

		// Some worldspaces don't have a moon (like Tranquility Lane)
		if (pCurrentMoon) [[likely]] {
			const float fGameHour = pSky->fCurrentGameHour;

			// Sunrise Values
			kSunrise = TimeOfDayData(pSky->GetSunriseBegin(), pSky->GetSunriseEnd());

			// Sunset Values
			kSunset = TimeOfDayData(pSky->GetSunsetBegin(), pSky->GetSunsetEnd());

			if (fGameHour >= kSunset.fEnd || fGameHour < kSunrise.fStart) {
				// Apply moon's rotation to the sunlight
				arRotation = pCurrentMoon->spMoonNode->GetWorldRotate() * kMoonRotation;
			}
		}

		SetLocalRotate(arRotation);
		
		// Fixes sky rotation in interiors marked as exterior. 
		// It doesn't respect the north angle offset in vanilla, making sunrise happen at south etc.
		if (pTES->pInteriorCell && pSky->eMode > Sky::SM_INTERIOR) [[unlikely]] {
			float fNorthAngle = -pTES->pInteriorCell->GetNorthRotation();
			pSky->spRoot->m_kLocal.m_Rotate.MakeZRotation(fNorthAngle);
			TESMain::GetWeatherRoot()->m_kLocal.m_Rotate = pSky->spRoot->m_kLocal.m_Rotate;
		}
	}

	void SetMoonlightColor(const NiColor& arColor) {
		NVSEGlobalManager::GetSingleton().DispatchPluginEvent(ML_COLOR_START, &const_cast<NiColor&>(arColor), sizeof(uintptr_t));
		const TES* pTES = TES::GetSingleton();
		Sky* pSky = Sky::GetSingleton();

		const Moon* pCurrentMoon = pSky->pMasser ? pSky->pMasser : pSky->pSecunda;

		NiColor kColor = arColor;

		// Some worldspaces don't have a moon (like Tranquility Lane)
		if (pCurrentMoon) [[likely]] {
			const float fGameHour = pSky->fCurrentGameHour;

			// Moon phase "amount"
			const float fPhase = pSky->CalculateMoonPhase();

			// Moon phase brightness multiplier
			float fMoonVisibility = 1.f;

			// Used to fade the sunlight during sunrise and sunset, in order to avoid a sudden change in light rotation
			float fMultiplier = 1.f;

			// Used to fade the moon/sun mesh during sunrise and sunset - game controlled
			float fMeshAlpha = 1.f;

			NiAVObject* pSkyObjectMesh = nullptr;

			if (fGameHour >= kSunset.GetTransEnd() || fGameHour < kSunrise.fStart) {
				pSkyObjectMesh = pCurrentMoon->spMoonMesh;

				fMultiplier = CalculateNightFade(fGameHour, kSunrise, kSunset);

				fMoonVisibility = GetMoonPhaseMult(fPhase);
			}
			else {
				pSkyObjectMesh = pSky->pSun->spSunBase;

				fMultiplier = CalculateDayFade(fGameHour, kSunrise, kSunset);
			}

			if (pSkyObjectMesh) [[likely]]
				fMeshAlpha = GetGeometryAlpha(pSkyObjectMesh);

				DirectX::XMVECTOR kMultiplier = DirectX::XMVectorClamp(DirectX::XMVectorReplicate(fMultiplier), DirectX::XMVectorZero(), DirectX::XMVectorSplatOne());
				kMultiplier = DirectX::XMVectorMultiply(kMultiplier, DirectX::XMVectorReplicate(fMoonVisibility));
				kMultiplier = DirectX::XMVectorMultiply(kMultiplier, DirectX::XMVectorReplicate(fMeshAlpha));

				// Convert sun light color to HSL
				DirectX::XMVECTOR kSunLightColorHSV = DirectX::XMColorRGBToHSV(DirectX::XMLoadNiColor(kColor));
				const DirectX::XMVECTOR kMultVector = DirectX::XMVectorSet(1.f, 1.f, DirectX::XMVectorGetX(kMultiplier), 1.f);
				DirectX::XMVECTOR kSunLightColorRGB = DirectX::XMColorHSVToRGB(DirectX::XMVectorMultiply(kSunLightColorHSV, kMultVector));
				DirectX::XMStoreNiColor(kColor, kSunLightColorRGB);
		}

		m_kDiff = kColor;
		NVSEGlobalManager::GetSingleton().DispatchPluginEvent(ML_COLOR_END, &m_kDiff, sizeof(uintptr_t));
	}
};
#pragma optimize( "", on )

class GECKHooks {
public:
	void SetMoonlightRot(NiPoint3& arRotation) {
		GECK::Sky* pSky = GECK::Sky::GetSingleton();
		float gameHour = pSky->fCurrentGameHour;
		// Not bothering with color fade
		arRotation.y = -arRotation.y;
		if (pSky->pMasser && gameHour >= pSky->GetSunsetEnd() || gameHour < pSky->GetSunriseBegin()) {
			NiMatrix3& rotMatrix = pSky->pMasser->spRoot->m_kLocal.m_Rotate;
			arRotation.x = -(rotMatrix.m_pEntry[0][0] * 0.5f);
			arRotation.y = rotMatrix.m_pEntry[1][0];
			arRotation.z = rotMatrix.m_pEntry[2][0];
		}
		arRotation.Unitize();
	}
};

static void InitializeGameSettings() {
	bMoonPhaseBrightness.Initialize("iMoonPhaseBrightness", true);
	char cMultName[16];

	auto funcComputeMoonlightMultiplier = [](int index) -> float {
		switch (index) {
		case 0: return 0.0f;
		case 1: return 0.33f;
		case 2: return 0.5f;
		case 3: return 0.9f;
		case 4: return 1.0f;
		default: return 0.0f; // Default case if index is out of range
		}
	};

	for (uint32_t i = 0; i < 5; i++) {
		sprintf_s(cMultName, "fMoonMult%d", i);
		fMoonlightMultipliers[i].Initialize(cMultName, funcComputeMoonlightMultiplier(i));
	}

	CSimpleIniA ini;
	ini.SetUnicode();
	SI_Error rc = ini.LoadFile("Data\\NVSE\\Plugins\\MoonlightNVSE.ini");
	if (rc >= 0) {
		bMoonPhaseBrightness.uValue.b = ini.GetBoolValue("Main", "bMoonPhaseBrightness", true);
		for (uint32_t i = 0; i < 5; i++) {
			sprintf_s(cMultName, "fMoonMult%d", i);
			fMoonlightMultipliers[i].uValue.f = ini.GetDoubleValue("Main", cMultName, funcComputeMoonlightMultiplier(i));
		}
	}
}

static void MessageHandler(NVSEMessagingInterface::Message* msg) {
	if (msg->type == NVSEMessagingInterface::kMessage_DeferredInit)
		InitializeGameSettings();
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info) {
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "MoonlightNVSE";
	info->version = 210;
	return true;
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Load(NVSEInterface* nvse) {
	if (!nvse->isEditor) {
		NVSEGlobalManager& rNVSE = NVSEGlobalManager::GetSingleton();
		rNVSE.Initialize(nvse);
		rNVSE.RegisterPluginEventListener("NVSE", MessageHandler);

		// FNV
		ReplaceCallEx(0x6422EE, &FNVHooks::SetMoonlightRot);
		ReplaceCallEx(0x64232A, &FNVHooks::SetMoonlightColor);
	}
	else {
		// GECK
		ReplaceCallEx(0x685940, &GECKHooks::SetMoonlightRot);
	}

	return true;
}

BOOL WINAPI DllMain(
	HANDLE  hDllHandle,
	DWORD   dwReason,
	LPVOID  lpreserved
)
{
	return TRUE;
}
