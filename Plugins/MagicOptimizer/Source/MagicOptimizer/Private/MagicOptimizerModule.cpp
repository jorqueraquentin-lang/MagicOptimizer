#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMagicOptimizerModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        UE_LOG(LogTemp, Log, TEXT("MagicOptimizer (Runtime) module started"));
    }

    virtual void ShutdownModule() override
    {
        UE_LOG(LogTemp, Log, TEXT("MagicOptimizer (Runtime) module shutdown"));
    }
};

IMPLEMENT_MODULE(FMagicOptimizerModule, MagicOptimizer)
