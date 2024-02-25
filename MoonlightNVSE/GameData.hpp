#pragma once

#include "SafeWrite.h"
#include "Utilities.h"

#define ASSERT_SIZE(a, b) static_assert(sizeof(a) == b, "Wrong structure size!");
#define ASSERT_OFFSET(a, b, c) static_assert(offsetof(a, b) == c, "Wrong member offset!");
#define CREATE_OBJECT(CLASS, ADDRESS) static CLASS* CreateObject() { return StdCall<CLASS*>(ADDRESS); };

class NiNode;
class NiGeometry;
class NiTriBasedGeom;
class NiTriStrips;
class NiTriShape;
class NiStream;
class NiParticles;
class NiLines;
class NiCloningProcess;
class NiViewerStringsArray;
class NiFixedString;
class NiObjectGroup;
class NiControllerManager;
class NiCullingProcess;
class NiDX9Renderer;
class NiDX9Renderer;
class NiProperty;
class NiColor;

class BSResizableTriShape;
class BSSegmentedTriShape;
class BSFadeNode;
class BSMultiBoundNode;

class TESObjectCELL;
class TESWeather;
class Atmosphere;
class Stars;
class Sun;
class Clouds;
class Moon;
class Moon;
class Precipitation;
class ImageSpaceModifierInstanceForm;

class bhkNiCollisionObject;
class bhkCollisionObject;
class bhkBlendCollisionObject;
class bhkRigidBody;
class bhkLimitedHingeConstraint;


class NiUpdateData {
public:
	NiUpdateData(float afTime = 0, bool abUpdateControllers = 0, bool abIsMultiThreaded = 0, bool abyte6 = 0, bool abUpdateGeomorphs = 0, bool abUpdateShadowSceneNode = 0)
		: fTime(afTime), bUpdateControllers(abUpdateControllers), bIsMultiThreaded(abIsMultiThreaded), byte6(abyte6), bUpdateGeomorphs(abUpdateGeomorphs), bUpdateShadowSceneNode(abUpdateShadowSceneNode)
	{}
	~NiUpdateData() {};

	float fTime;
	bool bUpdateControllers;
	bool bIsMultiThreaded;
	bool byte6;
	bool bUpdateGeomorphs;
	bool bUpdateShadowSceneNode;

	static NiUpdateData kDefaultUpdateData;
};

class NiMatrix3 {
public:
	NiMatrix3() {};
	NiMatrix3(float m00, float m10, float m20, float m01, float m11, float m21, float m02, float m12, float m22) {
		m_pEntry[0][0] = m00;
		m_pEntry[0][1] = m10;
		m_pEntry[0][2] = m20;
		m_pEntry[1][0] = m01;
		m_pEntry[1][1] = m11;
		m_pEntry[1][2] = m21;
		m_pEntry[2][0] = m02;
		m_pEntry[2][1] = m12;
		m_pEntry[2][2] = m22;
	}

	float m_pEntry[3][3];

	void MakeZRotation(float afAngle) {
		float fCos = std::cos(afAngle);
		float fSin = std::sin(afAngle);

		m_pEntry[0][0] = fCos;
		m_pEntry[0][1] = fSin;
		m_pEntry[0][2] = 0.f;

		m_pEntry[1][0] = -fSin;
		m_pEntry[1][1] = fCos;
		m_pEntry[1][2] = 0.f;

		m_pEntry[2][0] = 0.f;
		m_pEntry[2][1] = 0.f;
		m_pEntry[2][2] = 1.f;
	}

	NiMatrix3 operator* (const NiMatrix3& mat) const {
		NiMatrix3 result;
		result.m_pEntry[0][0] =
			m_pEntry[0][0] * mat.m_pEntry[0][0] +
			m_pEntry[0][1] * mat.m_pEntry[1][0] +
			m_pEntry[0][2] * mat.m_pEntry[2][0];
		result.m_pEntry[1][0] =
			m_pEntry[1][0] * mat.m_pEntry[0][0] +
			m_pEntry[1][1] * mat.m_pEntry[1][0] +
			m_pEntry[1][2] * mat.m_pEntry[2][0];
		result.m_pEntry[2][0] =
			m_pEntry[2][0] * mat.m_pEntry[0][0] +
			m_pEntry[2][1] * mat.m_pEntry[1][0] +
			m_pEntry[2][2] * mat.m_pEntry[2][0];
		result.m_pEntry[0][1] =
			m_pEntry[0][0] * mat.m_pEntry[0][1] +
			m_pEntry[0][1] * mat.m_pEntry[1][1] +
			m_pEntry[0][2] * mat.m_pEntry[2][1];
		result.m_pEntry[1][1] =
			m_pEntry[1][0] * mat.m_pEntry[0][1] +
			m_pEntry[1][1] * mat.m_pEntry[1][1] +
			m_pEntry[1][2] * mat.m_pEntry[2][1];
		result.m_pEntry[2][1] =
			m_pEntry[2][0] * mat.m_pEntry[0][1] +
			m_pEntry[2][1] * mat.m_pEntry[1][1] +
			m_pEntry[2][2] * mat.m_pEntry[2][1];
		result.m_pEntry[0][2] =
			m_pEntry[0][0] * mat.m_pEntry[0][2] +
			m_pEntry[0][1] * mat.m_pEntry[1][2] +
			m_pEntry[0][2] * mat.m_pEntry[2][2];
		result.m_pEntry[1][2] =
			m_pEntry[1][0] * mat.m_pEntry[0][2] +
			m_pEntry[1][1] * mat.m_pEntry[1][2] +
			m_pEntry[1][2] * mat.m_pEntry[2][2];
		result.m_pEntry[2][2] =
			m_pEntry[2][0] * mat.m_pEntry[0][2] +
			m_pEntry[2][1] * mat.m_pEntry[1][2] +
			m_pEntry[2][2] * mat.m_pEntry[2][2];
		return result;
	}
};

class NiPoint3 {
public:
	float x, y, z;

	__forceinline float Length() const {
		return std::sqrt(x * x + y * y + z * z);
	}

	__forceinline float Unitize() {
		float fLength = Length();

		if (fLength > 1e-06f) {
			float fRecip = 1.f / fLength;
			x *= fRecip;
			y *= fRecip;
			z *= fRecip;
			return fLength;
		}

		x = 0.f;
		y = 0.f;
		z = 0.f;
		fLength = 0.f;
		return fLength;
	}
};

class NiColor {
public:
	float r, g, b;
};

class NiBound {
public:
	NiPoint3	m_kCenter;
	float		m_fRadius;
};

class NiTransform {
public:
	NiMatrix3	m_Rotate;
	NiPoint3	m_Translate;
	float		m_fScale;
};

class NiRTTI {
public:
	const char*		m_pcName;
	const NiRTTI*	m_pkBaseRTTI;
};

template <typename T_Data>
class NiTArray {
public:
	virtual ~NiTArray();

	T_Data* m_pBase;
	UInt16 m_usMaxSize;
	UInt16 m_usSize;
	UInt16 m_usESize;
	UInt16 m_usGrowBy;
};

ASSERT_SIZE(NiTArray<void*>, 0x10);

typedef void* NiTListIterator;

template <typename T_Data>
class NiTListItem {
public:
	NiTListItem*	m_pkNext;
	NiTListItem*	m_pkPrev;
	T_Data			m_element;
};

template <typename T_Data>
class NiTListBase {
public:
	NiTListItem<T_Data>*	m_pkHead;
	NiTListItem<T_Data>*	m_pkTail;
	UInt32					m_uiCount;

	inline UInt32 GetSize() const { return m_uiCount; };
	bool IsEmpty() const { return m_uiCount == 0; };

	NiTListIterator GetHeadPos() const { return m_pkHead; };
	NiTListIterator GetTailPos() const { return m_pkTail; };
};

template <class T_Data>
class BSSimpleList {
public:
	T_Data					m_item;
	BSSimpleList<T_Data>*	m_pkNext;
};

template <class T_Data>
class NiPointer {
public:
	NiPointer(T_Data* apObject = (T_Data*)0) {
		m_pObject = apObject;
		if (m_pObject)
			m_pObject->IncRefCount();
	}
	NiPointer(const NiPointer& arPointer) {
		m_pObject = arPointer.m_pObject;
		if (m_pObject)
			m_pObject->IncRefCount();
	}
	~NiPointer() {
		if (m_pObject)
			m_pObject->DecRefCount();
	}

	T_Data* m_pObject;

	__forceinline NiPointer<T_Data>& operator =(const NiPointer& arPointer) {
		if (m_pObject != arPointer.m_pObject) {
			if (m_pObject)
				m_pObject->DecRefCount();
			m_pObject = arPointer.m_pObject;
			if (m_pObject)
				m_pObject->IncRefCount();
		}
		return *this;
	}

	__forceinline NiPointer<T_Data>& operator =(T_Data* apObject) {
		if (m_pObject != apObject) {
			if (m_pObject)
				m_pObject->DecRefCount();
			m_pObject = apObject;
			if (m_pObject)
				m_pObject->IncRefCount();
		}
		return *this;
	}

	__forceinline bool operator==(T_Data* apObject) const { return (m_pObject == apObject); }
	__forceinline bool operator==(const NiPointer& arPointer) const { return (m_pObject == arPointer.m_pObject); }
	__forceinline operator bool() const { return m_pObject != nullptr; }
	__forceinline operator T_Data* () const { return m_pObject; }
	__forceinline T_Data& operator*() const { return *m_pObject; }
	__forceinline T_Data* operator->() const { return m_pObject; }
};

class NiRefObject {
public:
    virtual			~NiRefObject();
    virtual void	DeleteThis();

    UInt32 m_uiRefCount;

    // 0x40F6E0
    inline void IncRefCount() {
        InterlockedIncrement(&m_uiRefCount);
    }

    // 0x401970
    inline void DecRefCount() {
        if (!InterlockedDecrement(&m_uiRefCount))
            DeleteThis();
    }
};

class NiObject : public NiRefObject {
public:
    virtual const NiRTTI*				GetRTTI() const;												// 02 | Returns NiRTTI of the object
	virtual NiNode*						IsNiNode() const;												// 03 | Returns this if it's a NiNode, otherwise nullptr
	virtual BSFadeNode*					IsFadeNode() const;												// 04 | Returns this if it's a BSFadeNode, otherwise nullptr
	virtual BSMultiBoundNode*			IsMultiBoundNode() const;										// 05 | Returns this if it's a BSMultiBoundNode, otherwise nullptr
	virtual NiGeometry*					IsGeometry() const;												// 06 | Returns this if it's a NiGeometry, otherwise nullptr
	virtual NiTriBasedGeom*				IsTriBasedGeometry() const;										// 07 | Returns this if it's a NiTriBasedGeom, otherwise nullptr
	virtual NiTriStrips*				IsTriStrips() const;											// 08 | Returns this if it's a NiTriStrips, otherwise nullptr
	virtual NiTriShape*					IsTriShape() const;												// 09 | Returns this if it's a NiTriShape, otherwise nullptr
	virtual BSSegmentedTriShape*		IsSegmentedTriShape() const;									// 10 | Returns this if it's a BSSegmentedTriShape, otherwise nullptr
	virtual BSResizableTriShape*		IsResizableTriShape() const;									// 11 | Returns this if it's a BSResizableTriShape, otherwise nullptr
	virtual NiParticles*				IsParticlesGeom() const;										// 12 | Returns this if it's a NiParticles, otherwise nullptr
	virtual NiLines*					IsLinesGeom() const;											// 13 | Returns this if it's a NiLines, otherwise nullptr
	virtual bhkCollisionObject*			IsBhkNiCollisionObject() const;									// 14 | Returns this if it's a bhkCollisionObject, otherwise nullptr
	virtual bhkBlendCollisionObject*	IsBhkBlendCollisionObject() const;								// 15 | Returns this if it's a bhkBlendCollisionObject, otherwise nullptr
	virtual bhkRigidBody*				IsBhkRigidBody() const;											// 16 | Returns this if it's a bhkRigidBody, otherwise nullptr
	virtual bhkLimitedHingeConstraint*	IsBhkLimitedHingeConstraint() const;							// 17 | Returns this if it's a bhkLimitedHingeConstraint, otherwise nullptr
	virtual NiObject*					CreateClone(NiCloningProcess* apCloning);						// 18 | Creates a clone of this object
	virtual void						LoadBinary(NiStream* apStream);									// 19 | Loads objects from disk
	virtual void						LinkObject(NiStream* apStream);									// 20 | Called by the streaming system to resolve links to other objects once it can be guaranteed that all objects have been loaded
	virtual void						RegisterStreamables(NiStream* apStream);						// 21 | When an object is inserted into a stream, it calls register streamables to make sure that any contained objects or objects linked in a scene graph are streamed as well
	virtual void						SaveBinary(NiStream* apStream);									// 22 | Saves objects to disk
	virtual bool						IsEqual(NiObject* apObject) const;								// 23 | Compares this object with another
	virtual void						GetViewerStrings(NiViewerStringsArray* apStrings);				// 24 | Gets strings containing information about the object
	virtual void						AddViewerStrings(NiViewerStringsArray* apStrings);				// 25 | Adds additional strings containing information about the object
	virtual void						ProcessClone(NiCloningProcess* apCloning);						// 26 | Post process for CreateClone
	virtual void						PostLinkObject(NiStream* apStream);								// 27 | Called by the streaming system to resolve any tasks that require other objects to be correctly linked. It is called by the streaming system after LinkObject has been called on all streamed objects
	virtual bool						StreamCanSkip();												// 28
	virtual const NiRTTI*				GetStreamableRTTI();											// 29
	virtual void						SetBound(NiBound* apNewBound);									// 30 | Replaces the bound of the object
	virtual void						GetBlockAllocationSize();										// 31 | Used by geometry data
	virtual NiObjectGroup*				GetGroup();														// 32 | Used by geometry data
	virtual void						SetGroup(NiObjectGroup* apGroup);								// 33 | Used by geometry data
	virtual NiControllerManager*		IsControllerManager() const;									// 34 | Returns this if it's a NiControllerManager, otherwise nullptr
};

class NiObjectNET : public NiObject {
public:
	const char*	m_kName;
	void*		m_spControllers;
	void**		m_ppkExtra;
	UInt16		m_usExtraDataSize;
	UInt16		m_usMaxSize;
};

class NiAVObject : public NiObjectNET {
public:
	virtual void			UpdateControllers(NiUpdateData* arData);
	virtual void			ApplyTransform(NiMatrix3& akMat, NiPoint3& akTrn, bool abOnLeft);
	virtual void			Unk_39();
	virtual NiAVObject*		GetObject_(const NiFixedString& kName);
	virtual NiAVObject*		GetObjectByName(const NiFixedString& kName);
	virtual void			SetSelectiveUpdateFlags(bool* bSelectiveUpdate, bool bSelectiveUpdateTransforms, bool* bRigid);
	virtual void			UpdateDownwardPass(const NiUpdateData& arData, UInt32 uFlags);
	virtual void			UpdateSelectedDownwardPass(const NiUpdateData& arData, UInt32 uFlags);
	virtual void			UpdateRigidDownwardPass(const NiUpdateData& arData, UInt32 uFlags);
	virtual void			Unk_46(void* arg);
	virtual void			UpdateTransform();
	virtual void			UpdateWorldData(const NiUpdateData& arData);
	virtual void			UpdateWorldBound();
	virtual void			UpdateTransformAndBounds(const NiUpdateData& arData);
	virtual void			PreAttachUpdate(NiNode* pEventualParent, const NiUpdateData& arData);
	virtual void			PreAttachUpdateProperties(NiNode* pEventualParent);
	virtual void			DetachParent();
	virtual void			UpdateUpwardPassParent(void* arg);
	virtual void			OnVisible(NiCullingProcess* kCuller);
	virtual void			PurgeRendererData(NiDX9Renderer* apRenderer);

	NiNode*							m_pkParent;
	bhkNiCollisionObject*			m_spCollisionObject;
	NiBound*						m_kWorldBound;
	NiTListBase<NiProperty*>		m_kPropertyList;
	Bitfield32						m_uiFlags;
	NiTransform						m_kLocal;
	NiTransform						m_kWorld;

	void SetLocalTranslate(const NiPoint3& pos) {
		m_kLocal.m_Translate = pos;
	}

	void SetLocalTranslate(const NiPoint3* pos) {
		m_kLocal.m_Translate = *pos;
	}
	const NiPoint3& GetLocalTranslate() const {
		return m_kLocal.m_Translate;
	}

	void SetWorldTranslate(const NiPoint3& pos) {
		m_kWorld.m_Translate = pos;
	}

	void SetWorldTranslate(const NiPoint3* pos) {
		m_kWorld.m_Translate = *pos;
	}

	const NiPoint3& GetWorldTranslate() const {
		return m_kWorld.m_Translate;
	}

	const NiMatrix3& GetWorldRotate() const {
		return m_kWorld.m_Rotate;
	}

	void SetLocalRotate(const NiMatrix3& kRot) {
		m_kLocal.m_Rotate = kRot;
	}

	void SetLocalRotate(const NiMatrix3* rot) {
		m_kLocal.m_Rotate = *rot;
	}

	const NiMatrix3& GetLocalRotate() const {
		return m_kLocal.m_Rotate;
	}

	void Update(NiUpdateData& arData) {
		ThisStdCall(0xA59C60, this, &arData);
	}
};

class NiNode : public NiAVObject {
public:
	virtual void			AttachChild(NiAVObject* apChild, bool abFirstAvail);
	virtual void			InsertChildAt(UInt32 i, NiAVObject* apChild);
	virtual void			DetachChild(NiAVObject* apChild, NiAVObject*& aspAVObject);
	virtual void			DetachChildAlt(NiAVObject* apChild);
	virtual void			DetachChildAt(UInt32 i, NiAVObject*& aspAVObject);
	virtual NiAVObject*		DetachChildAtAlt(UInt32 i);
	virtual void			SetAt(UInt32 i, NiAVObject* apChild, NiAVObject*& aspAVObject);
	virtual void			SetAtAlt(UInt32 i, NiAVObject* apChild);
	virtual void			UpdateUpwardPass();

	NiTArray<NiAVObject*> m_kChildren;

    UInt32 GetChildCount() const {
		return m_kChildren.m_usESize;
    }

	UInt32 GetArrayCount() const {
		return m_kChildren.m_usSize;
	}

	NiAVObject* GetAt(UInt32 index) const {
		if (index >= GetArrayCount())
			return nullptr;

		return m_kChildren.m_pBase[index];
	}
};

class PlayerCharacter {
public:
	char		filler[1684];
	NiAVObject* spPlayerNode;

	static PlayerCharacter* GetSingleton() {
		return *(PlayerCharacter**)0x011DEA3C;
	}
};

class TESMain {
public:
	static __forceinline TESMain* GetSingleton()	{ return *(TESMain**)0x11DEA0C; };
	static __forceinline NiNode*  GetWeatherRoot()	{ return *(NiNode**)0x11DEDA4; };
};

class TES {
public:
	char			filler[0x34];
	TESObjectCELL*	pInteriorCell;

	static TES* GetSingleton() {
		return *(TES**)0x11DEA10;
	}
};

class TESGlobal {
public:
	char	filler[0x24];
	float	fData;
};

ASSERT_OFFSET(TESGlobal, fData, 0x24);

class Calendar {
public:
	TESGlobal*	pGameYear;
	TESGlobal*	pGameMonth;
	TESGlobal*	pGameDay;
	TESGlobal*	pGameHour;
	TESGlobal*	pGameDaysPassed;
	TESGlobal*	pTimeScale;
	UInt32		uiMidnightsPassed;
	bool		bGameLoaded;
	UInt32		unk20;
	UInt32		unk24;
	UInt32		unk28;
	float		fLastUpdHour;
	UInt32		initialized;

	static __forceinline Calendar* GetSingleton() { return (Calendar*)0x11DE7B8; };

	__forceinline float GetDaysPassed() const {
		if (pGameDaysPassed)
			return pGameDaysPassed->fData;
		return 0;
	}

	__forceinline float GetGameHour() const {
		if (pGameHour)
			return pGameHour->fData;
		return 0;
	}

	__forceinline float GetMinutes() const {
		float fHour = GetGameHour();
		return (fHour - std::floor(fHour)) * 60;
	}

	__forceinline float GetSeconds() const {
		float fMinutes = GetMinutes();
		return (fMinutes - std::floor(fMinutes)) * 60;
	}
};

class SkyObject {
public:
	virtual				~SkyObject();

	NiPointer<NiNode>	spRoot;
};

class Moon : public SkyObject {
public:
	NiPointer<NiNode>		spMoonNode;
	NiPointer<NiNode>		spShadowNode;
	NiPointer<NiAVObject>	spMoonMesh;
	NiPointer<NiAVObject>	spShadowMesh;
};

class TESClimate {
public:
	TESClimate();
	~TESClimate();

	enum {
		SUNRISE_BEGIN,
		SUNRISE_END,
		SUNSET_BEGIN,
		SUNSET_END,
		VOLATILITY,
		MOON_DATA
	};

	char			filler[0x50];
	UInt8			ucData[6];

	__forceinline UInt8 GetMoonPhaseDays() const {
		return ucData[MOON_DATA] & 0x3F;
	}

	__forceinline UInt8 GetTransTime(UInt32 auiTime) const {
		return auiTime > 3 ? 0 : ucData[auiTime];
	}
};

class Sun : public SkyObject {
public:
	NiPointer<NiAVObject>	spSunBaseNode;
	NiPointer<NiAVObject>	spSunGlareNode;
	NiPointer<NiAVObject>	spSunBase;
};

class Sky {
public:
	virtual ~Sky();

	enum SkyColors {
		SC_SKY_UPPER	= 0,
		SC_FOG			= 1,
		SC_CLOUDS_LOWER = 2,
		SC_AMBIENT		= 3,
		SC_SUNLIGHT		= 4,
		SC_SUN			= 5,
		SC_STARS		= 6,
		SC_SKY_LOWER	= 7,
		SC_HORIZON		= 8,
		SC_CLOUDS_UPPER = 9,
		SC_COUNT,
	};

	struct SkySound {
		UInt32		unk00;
		UInt32		unk04;
		UInt32		unk08;
		TESWeather* pWeather;
		UInt32		uiType;
		UInt32		uiSoundID;
	};

	enum SkyMode {
		SM_NONE			= 0,
		SM_INTERIOR		= 1,
		SM_SKYDOME_ONLY = 2,
		SM_FULL			= 3,
	};

	NiPointer<NiNode>				spRoot;
	NiPointer<NiNode>				spMoonsRoot;
	TESClimate*						pCurrentClimate;
	TESWeather*						pCurrentWeather;
	TESWeather*						pLastWeather;
	TESWeather*						pDefaultWeather;
	TESWeather*						pOverrideWeather;
	Atmosphere*						pAtmosphere;
	Stars*							pStars;
	Sun*							pSun;
	Clouds*							pClouds;
	Moon*							pMasser;
	Moon*							pSecunda;
	Precipitation*					pPrecipitation;
	NiColor							kColors[SC_COUNT];
	NiColor							kColorUnkown0B4;
	NiColor							kColorSunFog;
	float							fWindSpeed;
	float							fWindAngle;
	float							fFogNearPlane;
	float							fFogFarPlane;
	UInt32							unk0DC;
	UInt32							unk0E0;
	UInt32							unk0E4;
	float							fFogPower;
	float							fCurrentGameHour;
	float							fLastWeatherUpdate;
	float							fCurrentWeatherPct;
	SkyMode							eMode;
	BSSimpleList<SkySound>*			pSkySoundList;
	float							fFlash;
	UInt32							uiFlashTime;
	UInt32							uiLastMoonPhaseUpdate;
	float							fWindowReflectionTimer;
	float							fAccelBeginPct;
	UInt32							unk114;
	UInt32							uiFlags;
	ImageSpaceModifierInstanceForm* pFadeInIMODCurrent;
	ImageSpaceModifierInstanceForm* pFadeOutIMODCurrent;
	ImageSpaceModifierInstanceForm* pFadeInIMODLast;
	ImageSpaceModifierInstanceForm* pFadeOutIMODLast;
	float							f12_0;
	float							f23_99;
	float							f0_0;

	static __forceinline Sky* GetSingleton()	{ return *(Sky**)0x11CCB78; };
	__forceinline NiColor& GetSunLightColor()	{ return kColors[SC_SUNLIGHT]; }
	__forceinline NiColor& GetAmbientColor()	{ return kColors[SC_AMBIENT]; }

	__forceinline float GetSunriseBegin() {
		return ThisStdCall<float>(0x595EA0, this);
	}

	__forceinline float GetSunriseEnd() {
		return ThisStdCall<float>(0x595F50, this);
	}

	__forceinline float GetSunsetBegin() {
		return ThisStdCall<float>(0x595FC0, this);
	}

	__forceinline float GetSunsetEnd() {
		return ThisStdCall<float>(0x596030, this);
	}

	float CalculateMoonPhase() const {
		return (fmod(Calendar::GetSingleton()->GetDaysPassed(), pCurrentClimate->GetMoonPhaseDays() * 8)) / (pCurrentClimate->GetMoonPhaseDays());
	}
};

namespace GECK {
	class Sky {
	public:
		char	filler[0x38];
		Moon*	pMasser;
		char	filler2[0xB8];
		float	fCurrentGameHour;

		static __forceinline Sky* GetSingleton() { return *(Sky**)0xEDF1DC; };

		__forceinline float GetSunriseBegin() {
			return ThisStdCall<float>(0x6803A0, this);
		}

		__forceinline float GetSunriseEnd() {
			return ThisStdCall<float>(0x6803E0, this);
		}

		__forceinline float GetSunsetBegin() {
			return ThisStdCall<float>(0x595FC0, this);
		}

		__forceinline float GetSunsetEnd() {
			return ThisStdCall<float>(0x680420, this);
		}
	};
}

class TESObjectCELL {
public:
	float GetNorthRotation() const {
		return ThisStdCall<float>(0x555AD0, this);
	}
};

class GameSetting {
public:
	union Info {
		const char*		str;
		int				i;
		unsigned int	u;
		float			f;
		bool			b;
		char			c;
		char			h;
	};

	GameSetting() {
		memset(this, 0, sizeof(GameSetting));
	}

	~GameSetting() {
		ThisStdCall(0x404A00, this);
	}

	void*		__vtable;
	Info		uValue;
	const char* pKey;

	void Initialize(const char* apName, bool value) {
		ThisStdCall(0x40C150, this, apName, (int)value);
	}

	void Initialize(const char* apName, float value) {
		ThisStdCall(0x40C150, this, apName, (float)value);
	}
};