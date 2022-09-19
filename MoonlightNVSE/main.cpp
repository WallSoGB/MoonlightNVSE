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

typedef struct RgbColor
{
	float r;
	float g;
	float b;
} RgbColor;

typedef struct HsvColor
{
	float h;
	float s;
	float v;
} HsvColor;

unsigned long HSVToHEX(HsvColor hsv)
{
	RgbColor rgb;
	double      hh, p, q, t, ff;
	long        i;

	hsv.s = hsv.s / 100;
	hsv.v = (hsv.v * 255) / 100;

	if (hsv.s <= 0.0) {       // < is bogus, just shuts up warnings
		rgb.r = hsv.v;
		rgb.g = hsv.v;
		rgb.b = hsv.v;
	}
	else {
		hh = hsv.h;
		if (hh >= 360.0) hh = 0.0;
		hh /= 60.0;
		i = (long)hh;
		ff = hh - i;
		p = hsv.v * (1.0 - hsv.s);
		q = hsv.v * (1.0 - (hsv.s * ff));
		t = hsv.v * (1.0 - (hsv.s * (1.0 - ff)));

		switch (i) {
		case 0:
			rgb.r = hsv.v;
			rgb.g = t;
			rgb.b = p;
			break;
		case 1:
			rgb.r = q;
			rgb.g = hsv.v;
			rgb.b = p;
			break;
		case 2:
			rgb.r = p;
			rgb.g = hsv.v;
			rgb.b = t;
			break;
		case 3:
			rgb.r = p;
			rgb.g = q;
			rgb.b = hsv.v;
			break;
		case 4:
			rgb.r = t;
			rgb.g = p;
			rgb.b = hsv.v;
			break;
		default:
			rgb.r = hsv.v;
			rgb.g = p;
			rgb.b = q;
			break;
		}
	}
	rgb.r = round(rgb.r);
	rgb.g = round(rgb.g);
	rgb.b = round(rgb.b);
#ifdef _DEBUG
	_MESSAGE("[HSVToHEX] " "R %f, G %f, B %f", rgb.r, rgb.g, rgb.b);
#endif
	return ((static_cast<int>(rgb.b) & 0xff) << 16) + ((static_cast<int>(rgb.g) & 0xff) << 8) + (static_cast<int>(rgb.r) & 0xff);
}

HsvColor HEXToHSV(UInt32 hexValue)
{
	HsvColor hsv;
	RgbColor rgb;

	rgb.b = ((hexValue >> 16) & 0xFF) / 255.0;
	rgb.g = ((hexValue >> 8) & 0xFF) / 255.0;
	rgb.r = ((hexValue) & 0xFF) / 255.0;

	float fCMax = max(max(rgb.r, rgb.g), rgb.b);
	float fCMin = min(min(rgb.r, rgb.g), rgb.b);
	float fDelta = fCMax - fCMin;

	if (fDelta > 0) {
		if (fCMax == rgb.r) {
			hsv.h = 60 * (fmod(((rgb.g - rgb.b) / fDelta), 6));
		}
		else if (fCMax == rgb.g) {
			hsv.h = 60 * (((rgb.b - rgb.r) / fDelta) + 2);
		}
		else if (fCMax == rgb.b) {
			hsv.h = 60 * (((rgb.r - rgb.g) / fDelta) + 4);
		}

		if (fCMax > 0) {
			hsv.s = (fDelta / fCMax) * 100;
		}
		else {
			hsv.s = 0;
		}

		hsv.v = fCMax;
	}
	else {
		hsv.h = 0;
		hsv.s = 0;
		hsv.v = fCMax;
	}

	if (hsv.h < 0) {
		hsv.h = 360 + hsv.h;
	}
	hsv.v = hsv.v * 100;
#ifdef _DEBUG
	_MESSAGE("[HEXtoHSV] " "H %f, S %f, V %f", hsv.h, hsv.s, hsv.v);
#endif
	return hsv;
}


float GetDaysPassed()
{
	if (g_gameTimeGlobals->daysPassed)
	{
		return g_gameTimeGlobals->daysPassed->data;
	}
	return 1.0F;
}

UInt32 nightColorOrg, sunriseColorOrg, sunsetColorOrg;
TESWeather* currentWeather = nullptr;
float nightV = 0;
float moonFadeOutHour = 4.7; // should not change... right?
float sunriseStart, sunriseEnd, sunsetStart, sunsetEnd;
float daysPassed;

void __fastcall SetMoonLight(NiNode* object, void* dummy, NiMatrix33* position) {
	Sky* g_sky = Sky::Get();
	TESWeather* weather = g_sky->currWeather;
	NiMatrix33* rotMatrix = position;
	UInt32* nightColor = &weather->colors[4][3];

	if (g_sky->masserMoon != nullptr) {
		UInt32* sunriseColor = &weather->colors[4][0];
		UInt32* sunsetColor = &weather->colors[4][2];

		const float gameHour = ThisStdCall<double>(0x966A20, g_sky);

		sunriseStart = ThisStdCall<double>(0x595EA0, g_sky);
		sunriseEnd = ThisStdCall<double>(0x595F50, g_sky);

		sunsetStart = ThisStdCall<double>(0x595FC0, g_sky);
		sunsetEnd = ThisStdCall<double>(0x596030, g_sky);

		float sunriseLength = sunriseEnd - sunriseStart;
		float sunsetLength = sunsetEnd - sunsetStart;
		float nightLength = (24 - sunsetEnd) + sunriseStart;

		if (currentWeather == nullptr || (currentWeather->GetEditorID() != weather->GetEditorID())) {
			if (currentWeather != nullptr) {
#ifdef _DEBUG
				_MESSAGE("[Weather] " "Weather change occurred! Was: %s, is %s", currentWeather->GetEditorID(), weather->GetEditorID());
#endif
				currentWeather->colors[4][0] = sunriseColorOrg;
				currentWeather->colors[4][2] = sunsetColorOrg;
				currentWeather->colors[4][3] = nightColorOrg;
			}

#ifdef _DEBUG
			_MESSAGE("[Sunrise] " "The length of sunrise is %f, start: %f, end: %f", sunriseLength, sunriseStart, sunriseEnd);
			_MESSAGE("[Sunset]  " "The length of sunset  is %f, start: %f, end: %f", sunsetLength, sunsetStart, sunsetEnd);
			_MESSAGE("[Night]   " "The length of night   is %f, start: %f, end: %f", nightLength, sunsetEnd, sunriseStart);
#endif

			currentWeather = weather;
			sunriseColorOrg = *sunriseColor;
			sunsetColorOrg = *sunsetColor;
			nightColorOrg = *nightColor;

		}

		HsvColor nightColorHSVOrg = HEXToHSV(nightColorOrg);
		HsvColor nightColorHSV = nightColorHSVOrg;

		HsvColor sunriseColorHSVOrg = HEXToHSV(sunriseColorOrg);
		HsvColor sunriseColorHSV = sunriseColorHSVOrg;

		HsvColor sunsetColorHSVOrg = HEXToHSV(sunsetColorOrg);
		HsvColor sunsetColorHSV = sunsetColorHSVOrg;

		//int* moonPhase = reinterpret_cast<int*>(0x11CCA80);

		daysPassed = GetDaysPassed();
		float phase = (fmod(daysPassed, 24)) / 3;

#ifdef _DEBUG
		_MESSAGE("[Sunrise] " "sunriseColorOrg is %i, current is %i", RGBHexToDec(sunriseColorOrg), RGBHexToDec(*sunriseColor));
		_MESSAGE("[Sunset]  " "sunsetColorOrg  is %i, current is %i", RGBHexToDec(sunsetColorOrg), RGBHexToDec(*sunsetColor));
		_MESSAGE("[Night]   " "nightColorOrg   is %i, current is %i", RGBHexToDec(nightColorOrg), RGBHexToDec(*nightColor));
		_MESSAGE("[Weather] " "Weather %s", currentWeather->GetEditorID());
		_MESSAGE("[Moon]    " "Current phase is %f", phase);
		_MESSAGE("[Time]    " "Days passed %f", daysPassed);
		_MESSAGE("[Time]    " "Current hour is %f", gameHour);
#endif


		if ((gameHour >= sunsetEnd) || (gameHour < sunriseStart)) {
			rotMatrix = &g_sky->masserMoon->rootNode->m_transformLocal.rotate;
			rotMatrix->cr[0][0] = -(rotMatrix->cr[0][0] * 0.5);

			if ((phase > 4.25) && (phase < 5.25)) {
				weather->colors[4][3] = 0;
			}
			else {
				if (gameHour > sunsetEnd) {
					nightV = (gameHour - sunsetEnd) * 200;
					nightColorHSV.v = min(max(nightV, 0), nightColorHSVOrg.v);
					weather->colors[4][3] = HSVToHEX(nightColorHSV);
				}
				else {
					nightV = -(gameHour / (6 - moonFadeOutHour) - (moonFadeOutHour / (6 - moonFadeOutHour))) * 100;
					nightColorHSV.v = min(max(nightV, 0), nightColorHSVOrg.v);
					weather->colors[4][3] = HSVToHEX(nightColorHSV);
				}
			}

			weather->colors[4][0] = 0;
#ifdef _DEBUG
			_MESSAGE("[Night]   " "Nightcolor.V is %f, original is %f", nightColorHSV.v, nightColorHSVOrg.v);
#endif
		}
		else {
			if ((gameHour >= sunsetStart) && (gameHour <= sunsetEnd)) {
				sunsetColorHSV.v = min(max(-((gameHour / sunsetLength) - (sunsetEnd / sunsetLength)) * 100, 0), sunsetColorHSVOrg.v);
				weather->colors[4][2] = HSVToHEX(sunsetColorHSV);
				weather->colors[4][3] = 0;
#ifdef _DEBUG
				_MESSAGE("[Sunset]  " "sunsetColorHSV.V is %f, original is %f", sunsetColorHSV.v, sunsetColorHSVOrg.v);
#endif

			}
			else if ((gameHour >= sunriseStart) && (gameHour <= sunriseEnd - 0.25)) {
				sunriseColorHSV.v = min(max(((gameHour / sunriseLength) - (sunriseStart / sunriseLength)) * 100, 0), sunriseColorHSVOrg.v);
#ifdef _DEBUG
				_MESSAGE("[Sunrise] " "sunriseColorHSV.V is %f, original is %f", sunriseColorHSV.v, sunriseColorHSVOrg.v);
#endif
				weather->colors[4][0] = HSVToHEX(sunriseColorHSV);
			}
		}
	}
	else {
		weather->colors[4][3] = 0;
	}
	ThisStdCall(0x0043FA80, object, rotMatrix);
}

bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info)
{
	_MESSAGE("query");

	// fill out the info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "MoonlightNVSE";
	info->version = 122;

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
	_MESSAGE("load");

	g_pluginHandle = nvse->GetPluginHandle();

	g_nvseInterface = nvse;

	if (!nvse->isEditor) {
		WriteRelCall(0x6422EE, (UInt32)SetMoonLight);
	}

	return true;
}
