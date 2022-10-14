#include "nvse/PluginAPI.h"
#include "nvse/CommandTable.h"
#include "nvse/GameAPI.h"
#include "nvse/ParamInfos.h"
#include "nvse/GameObjects.h"
#include "nvse/SafeWrite.h"
#include "nvse/GameData.h"
#include "nvse/NiObjects.h"
#include "nvse/Utilities.h"
#include "nvse/utility.h"
#include <string>

IDebugLog		gLog("logs\\moonlightNVSE.log");
PluginHandle	g_pluginHandle = kPluginHandle_Invalid;

NVSEMessagingInterface* g_messagingInterface{};
NVSEInterface* g_nvseInterface{};
NVSECommandTableInterface* g_cmdTableInterface{};

NVSEScriptInterface* g_script{};
NVSEStringVarInterface* g_stringInterface{};
NVSEArrayVarInterface* g_arrayInterface{};
NVSEDataInterface* g_dataInterface{};
NVSESerializationInterface* g_serializationInterface{};
NVSEConsoleInterface* g_consoleInterface{};
NVSEEventManagerInterface* g_eventInterface{};
GameTimeGlobals* g_gameTimeGlobals = (GameTimeGlobals*)0x11DE7B8;
bool (*ExtractArgsEx)(COMMAND_ARGS_EX, ...);

typedef struct HSVColor
{
	float h;
	float s;
	float v;
} HSVColor;

NiColor HSVToRGB(HSVColor HSV)
{
	NiColor RGB;
	double      hh, p, q, t, ff;
	long        i;

	HSV.s = HSV.s / 100;
	HSV.v = (HSV.v * 255) / 100;

	if (HSV.s <= 0.0) {
		RGB.r = HSV.v;
		RGB.g = HSV.v;
		RGB.b = HSV.v;
	}
	else {
		hh = HSV.h;
		if (hh >= 360.0) hh = 0.0;
		hh /= 60.0;
		i = (long)hh;
		ff = hh - i;
		p = HSV.v * (1.0 - HSV.s);
		q = HSV.v * (1.0 - (HSV.s * ff));
		t = HSV.v * (1.0 - (HSV.s * (1.0 - ff)));

		switch (i) {
		case 0:
			RGB.r = HSV.v;
			RGB.g = t;
			RGB.b = p;
			break;
		case 1:
			RGB.r = q;
			RGB.g = HSV.v;
			RGB.b = p;
			break;
		case 2:
			RGB.r = p;
			RGB.g = HSV.v;
			RGB.b = t;
			break;
		case 3:
			RGB.r = p;
			RGB.g = q;
			RGB.b = HSV.v;
			break;
		case 4:
			RGB.r = t;
			RGB.g = p;
			RGB.b = HSV.v;
			break;
		default:
			RGB.r = HSV.v;
			RGB.g = p;
			RGB.b = q;
			break;
		}
	}
	RGB.r = RGB.r / 255;
	RGB.g = RGB.g / 255;
	RGB.b = RGB.b / 255;

	if (RGB.r > 1) RGB.r = 1;
	if (RGB.g > 1) RGB.g = 1;
	if (RGB.b > 1) RGB.b = 1;

#ifdef _DEBUG
	_MESSAGE("[HSVToRGB] " "R %f, G %f, B %f", RGB.r, RGB.g, RGB.b);
#endif
	return RGB;
}

HSVColor RGBToHSV(NiColor RGB)
{
	HSVColor HSV;

	float fCMax = max(max(RGB.r, RGB.g), RGB.b);
	float fCMin = min(min(RGB.r, RGB.g), RGB.b);
	float fDelta = fCMax - fCMin;

	if (fDelta > 0) {
		if (fCMax == RGB.r) {
			HSV.h = 60 * (fmod(((RGB.g - RGB.b) / fDelta), 6));
		}
		else if (fCMax == RGB.g) {
			HSV.h = 60 * (((RGB.b - RGB.r) / fDelta) + 2);
		}
		else if (fCMax == RGB.b) {
			HSV.h = 60 * (((RGB.r - RGB.g) / fDelta) + 4);
		}

		if (fCMax > 0) {
			HSV.s = (fDelta / fCMax) * 100;
		}
		else {
			HSV.s = 0;
		}

		HSV.v = fCMax;
	}
	else {
		HSV.h = 0;
		HSV.s = 0;
		HSV.v = fCMax;
	}

	if (HSV.h < 0) {
		HSV.h = 360 + HSV.h;
	}
	HSV.v = HSV.v * 100;
#ifdef _DEBUG
	_MESSAGE("[HextoHSV] " "H %f, S %f, V %f", HSV.h, HSV.s, HSV.v);
#endif
	return HSV;
}

HSVColor HexToHSV(UInt32 HexValue)
{
	NiColor RGB;

	RGB.b = ((HexValue >> 16) & 0xFF) / 255.0;
	RGB.g = ((HexValue >> 8) & 0xFF) / 255.0;
	RGB.r = ((HexValue) & 0xFF) / 255.0;

	return RGBToHSV(RGB);
}

float GetDaysPassed()
{
	if (g_gameTimeGlobals->daysPassed)
	{
		return g_gameTimeGlobals->daysPassed->data;
	}
	return 1.0F;
}

float multiplier = 1;
float sunriseStart, sunriseEnd, sunsetStart, sunsetEnd;
float daysPassed;
float moonVisibility = 1;

void __fastcall SetMoonLight(NiNode* object, void* dummy, NiMatrix33* position) {
	Sky* g_sky = Sky::Get();
	TESWeather* weather = g_sky->currWeather;
	NiMatrix33* rotMatrix = position;
	HSVColor currentColor = RGBToHSV(g_sky->sunDirectional);

	if (g_sky->masserMoon != nullptr) {
		UInt32* sunriseColor = &weather->colors[4][0];
		UInt32* sunsetColor = &weather->colors[4][2];
		UInt32* nightColor = &weather->colors[4][3];

		const float gameHour = ThisStdCall<double>(0x966A20, g_sky);

		// Sunrise Values
		sunriseStart = *(float*)0x11CCCFC;
		sunriseEnd = ThisStdCall<double>(0x595F50, g_sky);

		// Sunset Values
		sunsetStart = ThisStdCall<double>(0x595FC0, g_sky);
		sunsetEnd = *(float*)0x11CCD00;

		daysPassed = GetDaysPassed();
		float phase = (fmod(daysPassed, 24)) / 3;

		if ((gameHour >= sunsetEnd) || (gameHour < sunriseStart)) {
			rotMatrix = &g_sky->masserMoon->rootNode->m_transformLocal.rotate;
			rotMatrix->cr[0][0] = -(rotMatrix->cr[0][0] * 0.5);

#ifdef _DEBUG
			_MESSAGE("[Time] " "Night is in progress!");
#endif

			// Moon phase management
			if ((phase > 4.25) && (phase < 5.25)) {
				moonVisibility = 0;
			}
			else if ((phase > 3.25) && (phase < 4.25) || (phase > 5.25) && (phase < 6.25)) {
				moonVisibility = 0.3;
			}
			else if ((phase > 2.25) && (phase < 3.25) || (phase > 6.25) && (phase < 7.25)) {
				moonVisibility = 0.5;
			}
			else if ((phase > 1.25) && (phase < 2.25) || (phase > 7.25) && (phase < 8.25)) {
				moonVisibility = 0.9;
			}
			else if ((phase > 0.25) && (phase < 1.25)) {
				moonVisibility = 1;
			}

			// Night
			if (gameHour > sunsetEnd) {
				multiplier = (gameHour - sunsetEnd);
#ifdef _DEBUG
				_MESSAGE("[Time] " "Night is in progress! (First half)");
#endif
			}
			else {
				multiplier = -(gameHour - sunriseStart);
#ifdef _DEBUG
				_MESSAGE("[Time] " "Night is in progress! (Second half)");
#endif
			}
			currentColor.v *= min(max(multiplier, 0), 1) * moonVisibility;
		}
		else {
			//Sunset
			if ((gameHour >= sunsetStart) && (gameHour <= sunsetEnd)) {
				multiplier = -((gameHour - sunsetEnd));
				currentColor.v *= min(max(multiplier, 0), 1);
#ifdef _DEBUG
				_MESSAGE("[Time] " "Sunset is in progress!");
#endif
			}
			// Sunrise
			else if ((gameHour >= sunriseStart) && (gameHour <= sunriseEnd - 0.5)) {
				multiplier = (gameHour - sunriseStart);
				currentColor.v *= min(max(multiplier, 0), 1);
#ifdef _DEBUG
				_MESSAGE("[Time] " "Sunrise is in progress!");
#endif
			}
		}
		g_sky->sunDirectional = HSVToRGB(currentColor);
#ifdef _DEBUG
		NiColor currentColorRGB = HSVToRGB(currentColor);
		_MESSAGE("[Sunlight]" "Current RGB color is R: %f, G: %f, B: %f", currentColorRGB.r, currentColorRGB.g, currentColorRGB.b);
		_MESSAGE("[Sunlight]" "Current HSV color is H: %f, S: %f, V: %f", currentColor.h, currentColor.s, currentColor.v);
		_MESSAGE("[Weather] " "Weather %s", currentWeather.weather->GetEditorID());
		_MESSAGE("[Moon]    " "Current phase is %f", phase);
		_MESSAGE("[Moon]    " "Current visiblity is %f", moonVisibility);
		_MESSAGE("[Time]    " "Days passed %f", daysPassed);
		_MESSAGE("[Time]    " "Current hour is %f", gameHour);
#endif
	}
	ThisStdCall(0x0043FA80, object, rotMatrix);
			}

bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info)
{
	// fill out the info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "MoonlightNVSE";
	info->version = 130;

	// version checks
	if (nvse->nvseVersion < PACKED_NVSE_VERSION)
	{
		_ERROR("NVSE version too old (got %08X expected at least %08X)", nvse->nvseVersion, PACKED_NVSE_VERSION);
		return false;
	}

	if (!nvse->isEditor)
	{
		if (nvse->runtimeVersion < RUNTIME_VERSION_1_4_0_525)
		{
			_ERROR("incorrect runtime version (got %08X need at least %08X)", nvse->runtimeVersion, RUNTIME_VERSION_1_4_0_525);
			return false;
		}

		if (nvse->isNogore)
		{
			_ERROR("NoGore is not supported");
			return false;
		}
	}
	else
	{
		if (nvse->editorVersion < CS_VERSION_1_4_0_518)
		{
			_ERROR("incorrect editor version (got %08X need at least %08X)", nvse->editorVersion, CS_VERSION_1_4_0_518);
			return false;
		}
	}

	// version checks pass
	// any version compatibility checks should be done here
	return true;
}

bool NVSEPlugin_Load(NVSEInterface* nvse)
{
	_MESSAGE("MoonlightNVSE loaded!");

	g_pluginHandle = nvse->GetPluginHandle();

	g_nvseInterface = nvse;

	if (!nvse->isEditor) {
		WriteRelCall(0x6422EE, (UInt32)SetMoonLight);
	}

	return true;
}
