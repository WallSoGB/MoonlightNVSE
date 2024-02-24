#include <DirectXMath.h>
#include "external/simpleini/SimpleIni.h"
#include "GameData.hpp"
#include "nvse/PluginAPI.h"
#include "SafeWrite.h"

NVSEInterface* g_nvseInterface{};
IDebugLog	   gLog("logs\\MoonlightNVSE.log");

static GameSetting bMoonPhaseBrightness;
static GameSetting fMoonlightMultipliers[5];

static __forceinline float Clamp(const float afValue, const float afMin, const float afMax) {
	return afValue < afMin ? afMin : afValue > afMax ? afMax : afValue;
}

static float CalculateMoonVisibility(const float afPhase) {
	float fMoonVisibility = 1.f;

	if (!bMoonPhaseBrightness.uValue.b)
		return fMoonVisibility;

	if (afPhase > 4.25f && afPhase < 5.25f)
		fMoonVisibility = fMoonlightMultipliers[0].uValue.f;
	else if (afPhase > 3.25f && afPhase < 4.25f || afPhase > 5.25f && afPhase < 6.25f)
		fMoonVisibility = fMoonlightMultipliers[1].uValue.f;
	else if (afPhase > 2.25f && afPhase < 3.25f || afPhase > 6.25f && afPhase < 7.25f)
		fMoonVisibility = fMoonlightMultipliers[2].uValue.f;
	else if (afPhase > 1.25f && afPhase < 2.25f || afPhase > 7.25f && afPhase < 8.25f || afPhase < 0.25f)
		fMoonVisibility = fMoonlightMultipliers[3].uValue.f;
	else if (afPhase > 0.25f && afPhase < 1.25f)
		fMoonVisibility = fMoonlightMultipliers[4].uValue.f;

	return fMoonVisibility;
}

static float CalculateNightFade(const float afGameHour, const float afSunsetEnd, const float afSunriseStart) {
	return afGameHour > afSunsetEnd ? (afGameHour - afSunsetEnd) : -(afGameHour - afSunriseStart);
}

static float CalculateDayFade(const float afGameHour, const float afSunriseStart, const float afSunriseEnd, const float afSunsetStart, const float afSunsetEnd) {
	//Sunset
	if (afGameHour >= afSunsetStart && afGameHour <= afSunsetEnd)
		return -(afGameHour - afSunsetEnd);

	// Sunrise
	if (afGameHour >= afSunriseStart && afGameHour <= afSunriseEnd)
		return (afGameHour - afSunriseStart);

	return 1.f;
}

static void __fastcall SetMoonlightFNV(NiNode* apSunLight, void*, NiMatrix3& arRotation) {
	Sky* pSky = Sky::GetSingleton();
	TES* pTES = TES::GetSingleton();
	PlayerCharacter* pPlayer = PlayerCharacter::GetSingleton();

	// Some worldspaces don't have a moon (like Tranquility Lane)
	if (pSky->pMasser != nullptr) {
		const float fGameHour = pSky->fCurrentGameHour;

		// Sunrise Values
		const float fSunriseStart = pSky->GetSunriseBegin();
		const float fSunriseEnd = pSky->GetSunriseEnd();

		// Sunset Values
		const float fSunsetStart = pSky->GetSunsetBegin();
		const float fSunsetEnd = pSky->GetSunsetEnd();

		// Moon phase "amount"
		const float fPhase = pSky->CalculateMoonPhase();

		// Moon phase brightness multiplier
		float fMoonVisibility = 1.f;

		// Used to fade the sunlight during sunrise and sunset, in order to avoid a sudden change in light arRotation
		float fMultiplier = 1.f;

		if (fGameHour >= fSunsetEnd || fGameHour < fSunriseStart) {
			// Apply moon's rotation to the sun
			arRotation = pSky->pMasser->spRoot->m_kLocal.m_Rotate;
			arRotation.m_pEntry[0][0] = -(arRotation.m_pEntry[0][0] * 0.5f);
			
			fMoonVisibility = CalculateMoonVisibility(fPhase);

			fMultiplier = CalculateNightFade(fGameHour, fSunsetEnd, fSunriseStart);
		}
		else {
			fMultiplier = CalculateDayFade(fGameHour, fSunriseStart, fSunriseEnd, fSunsetStart, fSunsetEnd);
		}

		// Convert sun light color to HSL
		NiColor kSunLightColor = pSky->GetSunLightColor();
		DirectX::XMVECTOR kSunLightColorHSV = DirectX::XMColorRGBToHSV(DirectX::XMVectorSet(kSunLightColor.r, kSunLightColor.g, kSunLightColor.b, 1));
		kSunLightColorHSV.m128_f32[2] *= Clamp(fMultiplier, 0.f, 1.f) * fMoonVisibility;
		DirectX::XMVECTOR kSunLightColorRGB = DirectX::XMColorHSVToRGB(kSunLightColorHSV);

		pSky->kColors[Sky::SC_SUNLIGHT].r = kSunLightColorRGB.m128_f32[0];
		pSky->kColors[Sky::SC_SUNLIGHT].g = kSunLightColorRGB.m128_f32[1];
		pSky->kColors[Sky::SC_SUNLIGHT].b = kSunLightColorRGB.m128_f32[2];
	}

	apSunLight->SetLocalRotate(arRotation);

	// Fixes sky arRotation in interiors marked as exterior. 
	// It doesn't respect the north angle offset in vanilla, making sunrise happen at south etc.
	if (pTES->pInteriorCell) {
		float fNorthAngle = -pTES->pInteriorCell->GetNorthRotation();
		pSky->spRoot->m_kLocal.m_Rotate.MakeZRotation(fNorthAngle);
		TESMain::GetWeatherRoot()->m_kLocal.m_Rotate.MakeZRotation(fNorthAngle);
	}
}

static void __fastcall SetMoonlightGECK(NiPoint3& arRotation) {
	GECK::Sky* pSky = GECK::Sky::GetSingleton();
	float gameHour = pSky->fCurrentGameHour;
	// Not bothering with color fade
	arRotation.y = -arRotation.y;
	if (pSky->pMasser != nullptr && gameHour >= pSky->GetSunsetEnd() || gameHour < pSky->GetSunriseBegin()) {
		NiMatrix3& rotMatrix = pSky->pMasser->spRoot->m_kLocal.m_Rotate;
		arRotation.x = -(rotMatrix.m_pEntry[0][0] * 0.5f);
		arRotation.y = rotMatrix.m_pEntry[1][0];
		arRotation.z = rotMatrix.m_pEntry[2][0];
	}
	arRotation.Unitize();
}

static void InitializeGameSettings() {
	bMoonPhaseBrightness.Initialize("iMoonPhaseBrightness", true);
	fMoonlightMultipliers[0].Initialize("fMoonMult0", 0.f);
	fMoonlightMultipliers[1].Initialize("fMoonMult1", 0.33f);
	fMoonlightMultipliers[2].Initialize("fMoonMult2", 0.5f);
	fMoonlightMultipliers[3].Initialize("fMoonMult3", 0.9f);
	fMoonlightMultipliers[4].Initialize("fMoonMult4", 1.f);

	CSimpleIniA ini;
	ini.SetUnicode();
	SI_Error rc = ini.LoadFile("Data\\NVSE\\Plugins\\MoonlightNVSE.ini");
	if (rc < 0) {
		_MESSAGE("MoonlightNVSE.ini not found, using default values");
	}
	else {
		bMoonPhaseBrightness.uValue.b = ini.GetBoolValue("Main", "bMoonPhaseBrightness", true);
		fMoonlightMultipliers[0].uValue.f = ini.GetDoubleValue("Main", "fMoonMult0", 0.f);
		fMoonlightMultipliers[1].uValue.f = ini.GetDoubleValue("Main", "fMoonMult1", 0.33f);
		fMoonlightMultipliers[2].uValue.f = ini.GetDoubleValue("Main", "fMoonMult2", 0.5f);
		fMoonlightMultipliers[3].uValue.f = ini.GetDoubleValue("Main", "fMoonMult3", 0.9f);
		fMoonlightMultipliers[4].uValue.f = ini.GetDoubleValue("Main", "fMoonMult4", 1.f);
	};
}

bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info) {
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "MoonlightNVSE";
	info->version = 200;
	return true;
}

static void MessageHandler(NVSEMessagingInterface::Message* msg) {
	switch (msg->type) {
	case NVSEMessagingInterface::kMessage_DeferredInit:
		InitializeGameSettings();
		break;
	default:
		break;
	}
}


bool NVSEPlugin_Load(NVSEInterface* nvse) {
	if (!nvse->isEditor) {
		((NVSEMessagingInterface*)nvse->QueryInterface(kInterface_Messaging))->RegisterListener(nvse->GetPluginHandle(), "NVSE", MessageHandler);

		// FNV
		WriteRelCall(0x6422EE, (UInt32)SetMoonlightFNV);
	}
	else {
		// GECK
		WriteRelCall(0x685940, (UInt32)SetMoonlightGECK);
	}

	return true;
}