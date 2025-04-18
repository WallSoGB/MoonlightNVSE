#include "NVSEGlobalManager.hpp"

NVSEGlobalManager::NVSEGlobalManager() {
}

NVSEGlobalManager::~NVSEGlobalManager() {
}

void NVSEGlobalManager::Initialize(NVSEInterface* apNVSEInterface) {
	pNVSEInterface		= apNVSEInterface;
	pMsgInterface		= static_cast<NVSEMessagingInterface*>(apNVSEInterface->QueryInterface(kInterface_Messaging));
	uiPluginHandle		= apNVSEInterface->GetPluginHandle();
}

NVSEGlobalManager& NVSEGlobalManager::GetSingleton() {
	static NVSEGlobalManager kSingleton;
	return kSingleton;
}

bool NVSEGlobalManager::RegisterPluginEventListener(const char* apSender, NVSEMessagingInterface::EventCallback apCallback) const {
	return GetMsgInterface()->RegisterListener(uiPluginHandle, apSender, apCallback);
}

bool NVSEGlobalManager::DispatchPluginEvent(uint32_t auiMessageType, void* apData, uint32_t auiDataLength, const char* apReceiver) const {
	return GetMsgInterface()->Dispatch(uiPluginHandle, auiMessageType, apData, auiDataLength, apReceiver);
}