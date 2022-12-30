#pragma once
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

class NiBSBoneLODController;
class NiBSplineCompTransformInterpolator;
struct CombatTarget;
struct BGSSaveLoadFileEntry;
class Sky;
class BSTempNodeManager;
class CombatProcedure;
class CombatAction;
class CombatGoal;
class PathingLocation;
class PathingCoverLocation;
class BSAudioManagerThread;
class ImageSpaceModifierInstanceRB;
struct NavMeshClosedDoorInfo;


// 08
class SkyObject
{
public:
	virtual SkyObject* Destroy(bool doFree);
	virtual NiNode* GetNiNode(void);
	virtual void		InitNiNode(NiNode* skyNode04);
	virtual void		Update(Sky* sky, float value);

	NiNode* rootNode;	// 04
};

// 1C
class Atmosphere : public SkyObject
{
public:
	virtual void		Init(NiNode* niNode, void* _fogProp);

	NiNode* node08;	// 08
	void* fogProp;	// 0C	Same as *0x11DEB00
	NiRefObject* object10;	// 10
	NiRefObject* object14;	// 14
	UInt8				byte18;		// 18
	UInt8				pad19[3];	// 19
};

// 10
class Stars : public SkyObject
{
public:
	NiNode* node08;	// 08
	float			flt0C;		// 0C
};

// 2C
class Sun : public SkyObject
{
public:
	NiBillboardNode* node08;
	NiBillboardNode* sunGlareNode;
	NiTriShape* spSunBase;
	NiTriShape* shape14;
	UInt32 unk18;
	void* sunLight;
	float fGlareScale;
	UInt8 bDoOcclusionTests;
	UInt8 byte25;
	UInt8 byte26;
	UInt8 byte27;
	void* spSunAccumulator;
};

// 5C
class Clouds : public SkyObject
{
public:
	void* layers[4];		// 08
	NiSourceTexture* textures[4];	// 18
	NiVector3			layerPos[4];	// 28
	UInt16				numLayers;		// 58
	UInt8				byte5A;			// 5A
	UInt8				byte5B;			// 5B
};

// 7C
class Moon : public SkyObject
{
public:
	virtual void	Refresh(NiNode* niNode, const char* moonStr);

	NiNode* spMoonNode;			// 08
	NiNode* spShadowNode;			// 0C
	NiTriShape* spMoonMesh;			// 10
	NiTriShape* spShadowMesh;			// 14
	String			moonTexture[8];		// 18
	//	0	Full Moon
	//	1	Three Wan
	//	2	Half Wan
	//	3	One Wan
	//	4	No Moon
	//	5	One Wax
	//	6	Half Wax
	//	7	Three Wax
	float			fadeStart;				// 58
	float			fadeEnd;				// 5C
	float			shadowEarlyFade;				// 60
	float			speed;				// 64
	float			zOffset;				// 68
	UInt32			size;				// 6C
	UInt32			eUpdateMoonTexture;				// 70
	float			fAngleFadeStart;				// 74
	float			lastUpdateHour;				// 78
};

// 18
class Precipitation
{
public:
	virtual Precipitation* Destroy(bool doFree);

	NiNode* node04;	// 04
	NiNode* node08;	// 08
	UInt32		unk0C;		// 0C
	float		unk10;		// 10
	UInt32		unk14;		// 14
};

// 1C
class ImageSpaceModifierInstance : public NiObject
{
public:
	virtual void	Unk_23(void);
	virtual void	Unk_24(void);
	virtual void* GetInstanceForm();
	virtual void	Unk_26(char* buffer);

	UInt8					hidden;			// 08
	UInt8					pad09[3];		// 09
	float					percent;		// 0C
	NiObject* obj10;			// 10
	float					flt14;			// 14
	UInt32					unk18;			// 18
};

// 44
class ImageSpaceModifierInstanceDRB : public ImageSpaceModifierInstance
{
public:
	float					flt1C;			// 1C
	float					flt20;			// 20
	float					flt24;			// 24
	float					flt28;			// 28
	float					flt2C;			// 2C
	UInt32					unk30;			// 30
	UInt32					unk34;			// 34
	float					flt38;			// 38
	float					flt3C;			// 3C
	UInt32					unk40;			// 40
};

// 30
class ImageSpaceModifierInstanceForm : public ImageSpaceModifierInstance
{
public:
	TESImageSpaceModifier* imageSpaceMod;		// 1C
	TESImageSpaceModifier* lastImageSpaceMod;	// 20
	float					lastStrength;		// 24
	NiObject* lastTarget;		// 28
	float					transitionTime;		// 2C
};


class Sky
{
public:
	virtual Sky* Destructor(bool doFree);

	struct SkySound
	{
		UInt32		unk00;		// 00
		UInt32		unk04;		// 04
		UInt32		unk08;		// 08
		TESWeather* weather;	// 0C
		UInt32		type;		// 10
		UInt32		soundID;	// 14
	};

	enum SKY_MODE
	{
		SM_NONE = 0x0,
		SM_INTERIOR = 0x1,
		SM_SKYDOME_ONLY = 0x2,
		SM_FULL = 0x3,
	};


	NiNode* niNode004;			// 004
	NiNode* niNode008;			// 008
	TESClimate* currClimate;		// 00C
	TESWeather* currWeather;		// 010
	TESWeather* transWeather;		// 014	Previous weather, gradually fading, on weather transition
	TESWeather* defaultWeather;	// 018	Picked from currClimate weathers list. currClimate is set to this unless there's a regional weather
	TESWeather* overrideWeather;	// 01C
	Atmosphere* atmosphere;		// 020
	Stars* stars;				// 024
	Sun* sun;				// 028
	Clouds* clouds;			// 02C
	Moon* masserMoon;		// 030
	Moon* secundaMoon;		// 034
	Precipitation* precipitation;		// 038
	NiVector3						vector03C;			// 03C
	NiColor							waterReflection;	// 048
	NiVector3						vector054;			// 054
	NiColor							sunAmbient;			// 060
	NiColor							sunDirectional;		// 06C
	NiVector3						vector078;			// 078
	NiVector3						vector084;			// 084
	NiVector3						vector090;			// 090
	NiVector3						vector09C;			// 09C
	NiVector3						vector0A8;			// 0A8
	NiVector3						vector0B4;			// 0B4
	NiColor							sunFog;				// 0C0
	float							windSpeed;			// 0CC
	float							windDirection;		// 0D0
	UInt32							unk0D4[5];			// 0D4
	float							fogPower;			// 0E8
	float							gameHour;			// 0EC
	float							lastUpdateHour;		// 0F0
	float							weatherPercent;		// 0F4
	SKY_MODE						mode;				// 0F8
	tList<SkySound>* skySounds;			// 0FC
	float							lightningFxPerc;	// 100
	UInt32							unk104;				// 104
	float							flt108;				// 108
	float							flt10C;				// 10C
	float							flt110;				// 110
	UInt32							unk114;				// 114
	UInt32							flags;				// 118
	ImageSpaceModifierInstanceForm* currFadeInIMOD;	// 11C
	ImageSpaceModifierInstanceForm* currFadeOutIMOD;	// 120
	ImageSpaceModifierInstanceForm* transFadeInIMOD;	// 124	On weather transition, set to the previuos weather fadeIn/OutIMODs
	ImageSpaceModifierInstanceForm* transFadeOutIMOD;	// 128		"			"
	float							flt12C;				// 12C	Always 12.0
	float							flt130;				// 130	Always 23.99
	float							flt134;				// 134	Always 0

	__forceinline static Sky* Get() { return *(Sky**)0x11DEA20; }

	void RefreshMoon();

	bool GetIsRaining();
};
static_assert(sizeof(Sky) == 0x138);

class Sky_GECK {
public:
	virtual Sky_GECK* Destructor(bool doFree);
	// Most of this is incorrect. I don't care. I only need the moon.
	NiNode* niNode004;
	NiNode* niNode008;
	TESWeather* pLastWeather;
	TESWeather* pDefaultWeather;
	TESWeather* pOverrideWeather;
	TESWeather* pCurrentWeather;
	float flt134;
	Stars* stars;
	Clouds* clouds;
	Atmosphere* atmosphere;
	float f12_0;
	Sun* sun;
	Precipitation* precipitation;
	Moon* masserMoon;
	Moon* secundaMoon;
	NiColor colorSkyUpper;
	NiColor colorFog;
	NiColor colorUnused050;
	NiColor colorAmbient;
	NiColor colorSunlight;
	NiColor colorSun;
	NiColor colorStars;
	NiColor colorSkyLower;
	NiColor colorHorizon;
	NiColor colorUnused0A8;
	TESClimate* pCurrentClimate;
	NiVector3 vector0B4;
	NiColor sunFog;
	float windSpeed;
	float windDirection;
	float fogNearPlane;
	float fogFarPlane;
	UInt32 unk0DC;
	UInt32 unk0E0;
	UInt32 unk0E4;
	float cellFogPower;
	float fCurrentGameHour;
	float fLastWeatherUpdate;
	float fCurrentWeatherPct;
	UInt32 mode;
	UInt32 unk0F8;
	void* pSkySoundList;
	UInt32 uiFlashTime;
	UInt32 uiLastMoonPhaseUpdate;
	float windowUpdateTimer;
	float fAccelBeginPct;
	UInt32 unk114;
	UInt32 uiFlags;
	ImageSpaceModifierInstanceForm* fadeInIMODCurrent;
	ImageSpaceModifierInstanceForm* fadeOutIMODCurrent;
	ImageSpaceModifierInstanceForm* fadeInIMODLast;
	ImageSpaceModifierInstanceForm* fadeOutIMODLast;
	float f23_99;
	__forceinline static Sky_GECK* Get() { return StdCall<Sky_GECK*>(0x42DAA0); }
};

class GameTimeGlobals
{
public:
	TESGlobal* year;
	TESGlobal* month;
	TESGlobal* day;
	TESGlobal* hour;
	TESGlobal* daysPassed;
	TESGlobal* timeScale;
	UInt32 count18;
	bool gameLoaded;
	UInt8 pad1D[3];
	UInt32 unk20;
	UInt32 unk24;
	UInt32 unk28;
	float lastUpdHour;
	UInt32 initialized;
};


// C4
class TES {
public:
	TES();
	~TES();

	virtual void		Fn_00(UInt32 arg1, UInt32 arg2, UInt32 arg3, UInt32 arg4, UInt32 arg5);

	UInt32								unk04;				// 04
	void* gridCellArray;		// 08
	NiNode* niNode0C;			// 0C
	NiNode* niNode10;			// 10
	NiNode* niNode14;			// 14
	BSTempNodeManager* tempNodeMgr;		// 18
	UInt32* directionalLight;	// 1C
	void* ptr20;				// 20
	SInt32								extGridX;			// 24
	SInt32								extGridY;			// 28
	SInt32								extCoordX;			// 2C
	SInt32								extCoordY;			// 30
	TESObjectCELL* currentInterior;	// 34
	TESObjectCELL** interiorsBuffer;	// 38
	TESObjectCELL** exteriorsBuffer;	// 3C
	UInt32								unk40[9];			// 40
	void* waterManager;		// 64
	Sky* sky;				// 68
	tList<ImageSpaceModifierInstance>	activeIMODs;		// 6C
	UInt32								unk74[3];			// 74
	float								flt80;				// 80	Abs X distance from centre of grid.
	float								flt84;				// 84	Abs Y distance from centre of grid.
	TESWorldSpace* currentWrldspc;	// 88
	tList<UInt32>						list8C;				// 8C
	tList<UInt32>						list94;				// 94
	tList<UInt32>						list9C;				// 9C
	QueuedFile* unkA4;				// A4
	NiSourceTexture* unkA8;				// A8
	QueuedFile* unkAC;				// AC
	void* ptrB0;				// B0
	UInt32								unkB4[2];			// B4
	void* navMeshInfoMap;	// BC
	void* areaBound;			// C0

	bool GetTerrainHeight(float* posXY, float* result);
	__forceinline static TES* Get() { return *(TES**)0x11DEA10; }
};
STATIC_ASSERT(sizeof(TES) == 0xC4);