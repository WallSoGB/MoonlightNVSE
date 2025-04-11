#pragma once

#include "DirectXMath.h"

class NiPoint2;
class NiPoint3;
class NiPoint4;
class NiColor;
class NiColorA;
class NiMatrix3;
class NiBound;

namespace DirectX {
	XMVECTOR XMLoadNiPoint2(const NiPoint2& arSource);

	void XMStoreNiPoint2(NiPoint2& arDest, const XMVECTOR& arSource);

	XMVECTOR XMLoadNiPoint3(const NiPoint3& arSource);

	void XMStoreNiPoint3(NiPoint3& arDest, const XMVECTOR& arSource);

	XMVECTOR XMLoadNiPoint4(const NiPoint4& arSource);

	void XMStoreNiPoint4(NiPoint4& arDest, const XMVECTOR& arSource);

	XMVECTOR XMLoadNiBound(const NiBound& arSource);

	void XMStoreNiBound(NiBound& arDest, const XMVECTOR& arSource);

	XMMATRIX XMLoadNiMatrix3(const NiMatrix3& arSource);

	void XMStoreNiMatrix3(NiMatrix3& arDest, const XMMATRIX& arSource);

	XMVECTOR XMLoadNiColor(const NiColor& arSource);

	void XMStoreNiColor(NiColor& arDest, const XMVECTOR& arSource);

	XMVECTOR XMLoadNiColorA(const NiColorA& arSource);

	void XMStoreNiColorA(NiColorA& arDest, const XMVECTOR& arSource);
}