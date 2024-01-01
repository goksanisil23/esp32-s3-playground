# Setup #
- Source the ESP environment:
source $HOME/esp/esp-idf/export.sh

- Go to the project dir and run
idf.py set-target esp32s3

- Then build
idf.py build

- If WIFI address configuration is needed, change the following in 'sdkconfig' file
This can also be done via idf.py menuconfig
CONFIG_EXAMPLE_WIFI_SSID="TN-CM5590"
CONFIG_EXAMPLE_WIFI_PASSWORD="DotsEt5ochac"

- Disable the additional Wifi features of the wifi driver due to excessive SPI memory usage that interferes with lcd.
In wifi_connect.c, in example_wifi_start(void)
    cfg.feature_caps = 0;

- Flash and monitor
idf.py -p /dev/ttyACM0 flash monitor