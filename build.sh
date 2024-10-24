# Build universal-analog-plugin

sun abiv0
sun abiv1
mkdir universal-analog-plugin > /dev/null 2>&1
mv libabiv0.so universal-analog-plugin/abiv0.so
mv libabiv1.so universal-analog-plugin/abiv1.so

# Build universal-analog-plugin-with-wooting-device-support

sun abiv0-pluswooting
sun abiv1-pluswooting
mkdir universal-analog-plugin-with-wooting-device-support > /dev/null 2>&1
mv libabiv0-pluswooting.so universal-analog-plugin-with-wooting-device-support/abiv0.so
mv libabiv1-pluswooting.so universal-analog-plugin-with-wooting-device-support/abiv1.so
