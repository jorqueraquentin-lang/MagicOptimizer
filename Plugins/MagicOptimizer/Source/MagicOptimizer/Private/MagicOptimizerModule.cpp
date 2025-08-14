#include "MagicOptimizer.h"
#include "Modules/ModuleManager.h"

void FMagicOptimizerModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("MagicOptimizer (Runtime) module started"));
}

void FMagicOptimizerModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("MagicOptimizer (Runtime) module shutdown"));
}

IMPLEMENT_MODULE(FMagicOptimizerModule, MagicOptimizer)
