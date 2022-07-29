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
	_MESSAGE("[HSVToHEX]" "R %f, G %f, B %f", rgb.r, rgb.g, rgb.b);
#endif
	return ((static_cast<int>(rgb.b) & 0xff) << 16) + ((static_cast<int>(rgb.g) & 0xff) << 8) + (static_cast<int>(rgb.r) & 0xff);
}

HsvColor HEXToHSV(UInt32 hexValue)
{
	HsvColor hsv;
	RgbColor rgb;

	rgb.r = hexValue / 1000000;
	rgb.g = hexValue / 1000 % 1000;
	rgb.b = hexValue % 1000;

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
	hsv.v = (hsv.v / 255) * 100;
	#ifdef _DEBUG
	_MESSAGE("[HEXtoHSV]" "H %f, S %f, V %f", hsv.h, hsv.s, hsv.v);
	#endif
	return hsv;
}


UInt32 nightColorOrg, sunriseColorOrg, sunsetColorOrg, currentWeather = 0;
float nightV = 0;
float moonFadeOutHour = 4.7;

void __fastcall SetMoonLight(NiNode* object, void* dummy, NiMatrix33* position) {
	Sky* g_sky = Sky::Get();
	TESWeather* weather = g_sky->currWeather;
	NiMatrix33* rotMatrix = nullptr;

	UInt32* sunriseColor = &weather->colors[4][0];
	UInt32* sunsetColor = &weather->colors[4][2];
	UInt32* nightColor = &weather->colors[4][3];

	float const gameHour = ThisStdCall<double>(0x966A20, g_sky);
	float sunriseStart, sunriseEnd, sunsetStart, sunsetEnd;

	sunriseStart = ThisStdCall<double>(0x595EA0, g_sky);
	sunriseEnd = ThisStdCall<double>(0x595F50, g_sky);

	sunsetStart = ThisStdCall<double>(0x595FC0, g_sky);
	sunsetEnd = ThisStdCall<double>(0x596030, g_sky);

	float sunriseLength = sunriseEnd - sunriseStart;
	float sunsetLength = sunsetEnd - sunsetStart;
	float nightLength = (24 - sunsetEnd) + sunriseStart;

	if (!nightColorOrg || !sunriseColorOrg || !sunsetColorOrg || currentWeather != reinterpret_cast<UInt32>(weather)) {
		currentWeather = reinterpret_cast<UInt32>(weather);
		sunriseColorOrg = RGBHexToDec(*sunriseColor);
		sunsetColorOrg = RGBHexToDec(*sunsetColor);
		nightColorOrg = RGBHexToDec(*nightColor);
		#ifdef _DEBUG
		_MESSAGE("[Sunrise]" "The length of sunrise is %f, start: %f, end: %f", sunriseLength, sunriseStart, sunriseEnd);
		_MESSAGE("[Sunset]" "The length of sunset is %f, start: %f, end: %f", sunsetLength, sunsetStart, sunsetEnd);
		_MESSAGE("[Night]" "The length of night is %f, start: %f, end: %f", nightLength, sunsetEnd, sunriseStart);
		#endif
	}

	#ifdef _DEBUG
	_MESSAGE("[Sunrise]" "sunriseColorOrg is %i, current is %x", sunriseColorOrg,*sunriseColor);
	_MESSAGE("[Sunset]" "sunsetColorOrg is %i, current is %x", sunsetColorOrg, *sunsetColor);
	_MESSAGE("[Night]" "Nightcolor is %i, current is %i", nightColorOrg, RGBHexToDec(*nightColor));
	_MESSAGE("[Weather]" "Weather %i", weather);
	#endif

	HsvColor nightColorHSVOrg = HEXToHSV(nightColorOrg);
	HsvColor nightColorHSV = nightColorHSVOrg;

	HsvColor sunriseColorHSVOrg = HEXToHSV(sunriseColorOrg);
	HsvColor sunriseColorHSV = sunriseColorHSVOrg;

	HsvColor sunsetColorHSVOrg = HEXToHSV(sunsetColorOrg);
	HsvColor sunsetColorHSV = sunsetColorHSVOrg;

	int* moonPhase = reinterpret_cast<int*>(0x11CCA80);

	if ((gameHour >= sunsetEnd) || (gameHour < sunriseStart)) {
		rotMatrix = &g_sky->masserMoon->rootNode->m_transformLocal.rotate;
		rotMatrix->cr[0][0] = -(rotMatrix->cr[0][0] * 0.5);

		if (*moonPhase == 4) {
			weather->colors[4][3] = 0;
		}
		else {
			if (gameHour > sunsetEnd) {
				nightV = (gameHour - sunsetEnd) * 200;
				nightColorHSV.v = min(max(nightV, 0), nightColorHSVOrg.v);
				weather->colors[4][3] = HSVToHEX(nightColorHSV);
			}
			else {
				nightV = -(gameHour / (6 - moonFadeOutHour) - (moonFadeOutHour / (6 - moonFadeOutHour)) ) * 100;
				nightColorHSV.v = min(max(nightV, 0), nightColorHSVOrg.v);
				weather->colors[4][3] = HSVToHEX(nightColorHSV);
			}
		}

		weather->colors[4][0] = 0;
		#ifdef _DEBUG
		_MESSAGE("[Night]" "Nightcolor.V is %f, original is %f", nightColorHSV.v, nightColorHSVOrg.v);
		#endif
	}
	else {
		rotMatrix = position;
		if ( (gameHour >= sunsetStart) && (gameHour <= sunsetEnd) ) {
			sunsetColorHSV.v = min(max(-((gameHour / sunsetLength) - (sunsetEnd / sunsetLength)) * 100, 0), sunsetColorHSVOrg.v);
			weather->colors[4][2] = HSVToHEX(sunsetColorHSV);
			weather->colors[4][3] = 0;
			#ifdef _DEBUG
			_MESSAGE("[Sunset]" "sunsetColor is %i, current is %i", sunsetColorOrg, RGBHexToDec(*sunsetColor));
			_MESSAGE("[Sunset]" "sunsetColorHSV.V is %f, original is %f", sunsetColorHSV.v, sunsetColorHSVOrg.v);
			_MESSAGE("[Time]" "Current hour is %f", gameHour);
			#endif

		}
		else if ( (gameHour >= sunriseStart) && (gameHour <= sunriseEnd - 0.25) ) {
			sunriseColorHSV.v = min(max(((gameHour / sunriseLength) - (sunriseStart/sunriseLength)) * 100, 0), sunriseColorHSVOrg.v);
			#ifdef _DEBUG
			_MESSAGE("[Sunrise]" "sunriseColor is %i, current is %i", sunriseColorOrg, RGBHexToDec(*sunriseColor));
			_MESSAGE("[Sunrise]" "sunriseColorHSV.V is %f, original is %f", sunriseColorHSV.v, sunriseColorHSVOrg.v);
			#endif
			weather->colors[4][0] = HSVToHEX(sunriseColorHSV);
		}
	}

	#ifdef _DEBUG
	_MESSAGE("[Time]" "Current hour is %f", gameHour);
	#endif

	ThisStdCall(0x0043FA80, object, rotMatrix);
}

bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info)
{
	_MESSAGE("query");

	// fill out the info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "MoonlightNVSE";
	info->version = 110;

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
