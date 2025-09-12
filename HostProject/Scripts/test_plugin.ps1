# Magic Optimizer Plugin Test Script
Write-Host "🔍 Magic Optimizer Plugin Test Script" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan

Write-Host "`n📋 TESTING CHECKLIST:" -ForegroundColor Yellow
Write-Host "1. Restart Unreal Editor completely" -ForegroundColor White
Write-Host "2. Check for Magic Optimizer in multiple locations:" -ForegroundColor White
Write-Host "   • Windows menu → Look for 'Magic Optimizer'" -ForegroundColor Gray
Write-Host "   • Tools menu → Look for 'Magic Optimizer (Backup)'" -ForegroundColor Gray
Write-Host "   • Toolbar → Look for Magic Optimizer button" -ForegroundColor Gray
Write-Host "3. Test console command: MagicOptimizer.Test" -ForegroundColor White
Write-Host "4. Check Output Log for debug messages" -ForegroundColor White

Write-Host "`n🔧 DEBUGGING STEPS:" -ForegroundColor Yellow
Write-Host "1. Open Output Log (Window → Developer Tools → Output Log)" -ForegroundColor White
Write-Host "2. Look for these messages:" -ForegroundColor White
Write-Host "   • 'MagicOptimizer: Module started successfully'" -ForegroundColor Green
Write-Host "   • 'MagicOptimizer: RegisterMenus called'" -ForegroundColor Green
Write-Host "   • 'MagicOptimizer: Menu extender added to LevelEditor'" -ForegroundColor Green
Write-Host "3. Open Console (Window → Developer Tools → Console)" -ForegroundColor White
Write-Host "4. Type: MagicOptimizer.Test" -ForegroundColor White
Write-Host "5. You should see a green message on screen" -ForegroundColor White

Write-Host "`n🚨 IF STILL NOT WORKING:" -ForegroundColor Red
Write-Host "1. Check if plugin is enabled in Edit → Plugins" -ForegroundColor White
Write-Host "2. Look for any error messages in Output Log" -ForegroundColor White
Write-Host "3. Try running: MagicOptimizer.Test in console" -ForegroundColor White
Write-Host "4. Check if PluginBrowser is enabled in project" -ForegroundColor White

Write-Host "`n✅ SUCCESS INDICATORS:" -ForegroundColor Green
Write-Host "• Console command works and shows green message" -ForegroundColor White
Write-Host "• Debug logs show successful registration" -ForegroundColor White
Write-Host "• Menu items appear in Windows/Tools menu" -ForegroundColor White
Write-Host "• Toolbar button appears" -ForegroundColor White

Write-Host "`n🎯 NEXT: Restart UE Editor and test!" -ForegroundColor Cyan
