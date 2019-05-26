#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IMessageContext.h"

DECLARE_LOG_CATEGORY_EXTERN(LogParty, Log, All);


class FLanPartyModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
