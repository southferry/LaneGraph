

#pragma once

#include "Modules/ModuleManager.h"

class FLaneGraphEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	void RegisterMenus();
	void SystemBuildGraphCommand();
};
