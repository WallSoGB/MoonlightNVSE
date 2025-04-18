#pragma once

#include "nvse/PluginAPI.h"

class NVSEGlobalManager {
private:
	NVSEInterface*				pNVSEInterface		= nullptr;
	NVSEMessagingInterface*		pMsgInterface		= nullptr;
	uint32_t					uiPluginHandle		= 0;
public:
	NVSEGlobalManager();
	~NVSEGlobalManager();

	void Initialize(NVSEInterface* apNVSEInterface);

	static NVSEGlobalManager& GetSingleton();

	NVSEInterface*				GetNVSEInterface() const	{ return pNVSEInterface;	}
	NVSEMessagingInterface*		GetMsgInterface() const		{ return pMsgInterface;		}
	uint32_t					GetPluginHandle() const		{ return uiPluginHandle;	}

	// Messaging

	bool RegisterPluginEventListener(const char* apSender, NVSEMessagingInterface::EventCallback apCallback) const;
	bool DispatchPluginEvent(uint32_t auiMessageType, void* apData, uint32_t auiDataLength, const char* apReceiver = nullptr) const;
};