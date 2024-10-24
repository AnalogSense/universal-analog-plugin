REM Build universal-analog-plugin

sun abiv0
del abiv0.exp abiv0.lib

sun abiv1
del abiv1.exp abiv1.lib

@mkdir universal-analog-plugin
mv abiv0.dll universal-analog-plugin/abiv0.dll
mv abiv1.dll universal-analog-plugin/abiv1.dll

REM Build universal-analog-plugin-with-wooting-device-support

sun abiv0-pluswooting
del abiv0-pluswooting.exp abiv0-pluswooting.lib

sun abiv1-pluswooting
del abiv1-pluswooting.exp abiv1-pluswooting.lib

@mkdir universal-analog-plugin-with-wooting-device-support
mv abiv0-pluswooting.dll universal-analog-plugin-with-wooting-device-support/abiv0.dll
mv abiv1-pluswooting.dll universal-analog-plugin-with-wooting-device-support/abiv1.dll
