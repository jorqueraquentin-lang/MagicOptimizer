#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMagicOptimizerStartupTest, "MagicOptimizer.Startup", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMagicOptimizerStartupTest::RunTest(const FString& Parameters)
{
    if (!FModuleManager::Get().IsModuleLoaded("MagicOptimizer"))
    {
        FModuleManager::Get().LoadModule("MagicOptimizer");
    }
    TestTrue(TEXT("MagicOptimizer module loaded"), FModuleManager::Get().IsModuleLoaded("MagicOptimizer"));
    return true;
}


