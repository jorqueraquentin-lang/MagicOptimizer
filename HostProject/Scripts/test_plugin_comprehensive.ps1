# Magic Optimizer Plugin Comprehensive Test Script
Write-Host "🔍 Magic Optimizer Plugin Comprehensive Test" -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan

Write-Host "`n📋 TESTING STEPS:" -ForegroundColor Yellow

Write-Host "`n1️⃣ RESTART UNREAL EDITOR COMPLETELY" -ForegroundColor Green
Write-Host "   • Close Unreal Editor entirely" -ForegroundColor White
Write-Host "   • Reopen your project" -ForegroundColor White

Write-Host "`n2️⃣ TEST CONSOLE COMMAND (IMMEDIATE TEST)" -ForegroundColor Green
Write-Host "   • Open Console: Window → Developer Tools → Console" -ForegroundColor White
Write-Host "   • Type: MagicOptimizer.Test" -ForegroundColor White
Write-Host "   • Press Enter" -ForegroundColor White
Write-Host "   • Expected: Green message on screen + log entries" -ForegroundColor White

Write-Host "`n3️⃣ CHECK OUTPUT LOG FOR DEBUG MESSAGES" -ForegroundColor Green
Write-Host "   • Open Output Log: Window → Developer Tools → Output Log" -ForegroundColor White
Write-Host "   • Look for these SUCCESS messages:" -ForegroundColor White
Write-Host "     ✅ 'MagicOptimizer: Module started successfully'" -ForegroundColor Green
Write-Host "     ✅ 'MagicOptimizer: Registering tab spawner'" -ForegroundColor Green
Write-Host "     ✅ 'MagicOptimizer: Tab spawner registered successfully'" -ForegroundColor Green
Write-Host "     ✅ 'MagicOptimizer: RegisterMenus called'" -ForegroundColor Green
Write-Host "     ✅ 'MagicOptimizer: LevelEditor module is loaded, proceeding with menu registration'" -ForegroundColor Green
Write-Host "     ✅ 'MagicOptimizer: Menu and toolbar extenders added to LevelEditor'" -ForegroundColor Green

Write-Host "`n4️⃣ LOOK FOR MENU ITEMS (AFTER 1-2 SECONDS)" -ForegroundColor Green
Write-Host "   • Check Windows menu → Look for 'Magic Optimizer'" -ForegroundColor White
Write-Host "   • Check Tools menu → Look for 'Magic Optimizer (Backup)'" -ForegroundColor White
Write-Host "   • Check Toolbar → Look for Magic Optimizer button (next to Settings)" -ForegroundColor White

Write-Host "`n5️⃣ TEST TAB OPENING" -ForegroundColor Green
Write-Host "   • Click on any Magic Optimizer menu item or button" -ForegroundColor White
Write-Host "   • Expected: Magic Optimizer window opens" -ForegroundColor White
Write-Host "   • Expected: Window shows '🎯 Magic Optimizer - Ultimate UE5.6 Asset Optimization Platform'" -ForegroundColor White

Write-Host "`n🚨 TROUBLESHOOTING:" -ForegroundColor Red

Write-Host "`n❌ If console command doesn't work:" -ForegroundColor Red
Write-Host "   • Plugin not loading - check Output Log for errors" -ForegroundColor White
Write-Host "   • Try rebuilding plugin again" -ForegroundColor White

Write-Host "`n❌ If console works but no menus appear:" -ForegroundColor Red
Write-Host "   • Look for 'LevelEditor module not loaded yet' warning in logs" -ForegroundColor White
Write-Host "   • Wait 2-3 seconds and check again (delayed registration)" -ForegroundColor White
Write-Host "   • Check if PluginBrowser is enabled in Edit → Plugins" -ForegroundColor White

Write-Host "`n❌ If tab fails to open:" -ForegroundColor Red
Write-Host "   • Look for 'MagicOptimizer tab attempted to spawn but failed' in logs" -ForegroundColor White
Write-Host "   • Try resetting editor layout: Window → Load Layout → Default Editor Layout" -ForegroundColor White

Write-Host "`n✅ SUCCESS INDICATORS:" -ForegroundColor Green
Write-Host "   • Console command shows green message" -ForegroundColor White
Write-Host "   • All debug messages appear in Output Log" -ForegroundColor White
Write-Host "   • Menu items appear in Windows/Tools menu" -ForegroundColor White
Write-Host "   • Toolbar button appears" -ForegroundColor White
Write-Host "   • Clicking menu/button opens Magic Optimizer window" -ForegroundColor White

Write-Host "`n🎯 NEXT: Restart UE Editor and run these tests!" -ForegroundColor Cyan
Write-Host "`n📝 REPORT BACK:" -ForegroundColor Yellow
Write-Host "   • Which steps worked?" -ForegroundColor White
Write-Host "   • What messages do you see in Output Log?" -ForegroundColor White
Write-Host "   • Do any menu items appear?" -ForegroundColor White
