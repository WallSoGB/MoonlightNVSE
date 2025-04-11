#include "DirectXUtils.hpp"
namespace DirectX {

	XMVECTOR XMLoadNiPoint2(const NiPoint2& arSource) {
		return XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&arSource));
	}

	void XMStoreNiPoint2(NiPoint2& arDest, const XMVECTOR& arSource) {
		XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&arDest), arSource);
	}

	XMVECTOR XMLoadNiPoint3(const NiPoint3& arSource) {
		return XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&arSource));
	}

	void XMStoreNiPoint3(NiPoint3& arDest, const XMVECTOR& arSource) {
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&arDest), arSource);
	}

	XMVECTOR XMLoadNiPoint4(const NiPoint4& arSource) {
		return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&arSource));
	}

	void XMStoreNiPoint4(NiPoint4& arDest, const XMVECTOR& arSource) {
		XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&arDest), arSource);
	}

	XMVECTOR XMLoadNiBound(const NiBound& arSource) {
		return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&arSource));
	}

	void XMStoreNiBound(NiBound& arDest, const XMVECTOR& arSource) {
		XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&arDest), arSource);
	}

	XMMATRIX XMLoadNiMatrix3(const NiMatrix3& arSource) {
		return XMLoadFloat3x3(reinterpret_cast<const XMFLOAT3X3*>(&arSource));
	}

	void XMStoreNiMatrix3(NiMatrix3& arDest, const XMMATRIX& arSource) {
		XMStoreFloat3x3(reinterpret_cast<XMFLOAT3X3*>(&arDest), arSource);
	}

	XMVECTOR XMLoadNiColor(const NiColor& arSource) {
		return XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&arSource));
	}

	void XMStoreNiColor(NiColor& arDest, const XMVECTOR& arSource) {
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&arDest), arSource);
	}

	XMVECTOR XMLoadNiColorA(const NiColorA& arSource) {
		return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&arSource));
	}

	void XMStoreNiColorA(NiColorA& arDest, const XMVECTOR& arSource) {
		XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&arDest), arSource);
	}

}