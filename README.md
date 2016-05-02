#esp8266-pvoutput-logger

Esp8266-pvoutput-logger is a kWh meter s0 pulse log application for pvoutput.org. It's designed on a ESP8266 ESP-12E WIFI Development Board. It has a built-in time service, interrupt handler, led control service, pvoutput.org client, queueing mechanism and scheduler service. It's main purpose is to monitor the energy output from your solar panels if you don't have a method to connect your converter to pvoutput.org.

##Putting the software on your ESP chip

First create a build environment on your favorite Linux distro (or maybe Cygwin under Windows?). The following steps will show you how to create it on Fedora or Debian (tested on x86 and ARM):

###Install the required build packages

####Fedora 23
Add the rpmfusion repos:

    sudo dnf install --nogpgcheck \
    http://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm \
    http://download1.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-$(rpm -E %fedora).noarch.rpm

Install the required packages:

    sudo dnf install make unrar autoconf automake libtool gcc gcc-c++ gperf flex \
    bison texinfo gawk ncurses-devel expat-devel python sed git pyserial patch \
    wget which file unzip bzip2
    
####Debian 8 Jessie

Install the required packages:

    sudo apt-get install make unrar-free autoconf automake libtool gcc g++ gperf flex \
    bison texinfo gawk libncurses5-dev libexpat1-dev python sed git python-serial patch \
    wget file unzip bzip2 libtool-bin

###Create the ESP build environment

Create the build directory:

    mkdir ~/esp-devel

Clone the SDK:

    cd ~/esp-devel  
    git clone https://github.com/pfalcon/esp-open-sdk.git --recursive

Build the toolchain:

    cd esp-open-sdk
    make STANDALONE=y

Add the compiler path to the PATH in your "~/.bash_profile":
  
    PATH=~/esp-devel/esp-open-sdk/xtensa-lx106-elf/bin:$PATH

Load your bash profile:
    . ~/.bash_profile

Get esptool (firmware builder):

    cd ~/esp-devel
    mkdir other
    cd other
    wget http://filez.zoobab.com/esp8266/esptool-0.0.2.zip
    unzip esptool-0.0.2.zip
    cd esptool
    sed -i 's/WINDOWS/LINUX/g' Makefile
    make

Get esptool.py (firmware uploader):

    cd ~/esp-devel
    git clone https://github.com/themadinventor/esptool

Get the internet of things SDK:

    mkdir esp_iot_sdk
    cd esp_iot_sdk
    wget http://bbs.espressif.com/download/file.php?id=1079 -O esp_iot_sdk_V1.5.2_16_01_29.zip
    unzip esp_iot_sdk_V1.5.2_16_01_29.zip
    ln -s esp_iot_sdk_v1.5.2 esp_iot_sdk-latest

###Building the software

####First fetch the software from git:

    cd ~/esp-devel
    git clone https://github.com/joeyloman/esp8266-pvoutput-logger.git
    cd esp8266-pvoutput-logger

####Configure the parameters in user/config.h:

Wifi configuration:  
\#define WIFI\_SSID "<Put your wifi SSID here>"  
\#define WIFI\_PASS "<Put your wifi password here>"

The number of blinks per kWh of your meter (for example: 1000 blinks):  
\#define PULSE_FACTOR 1000

The maximum power in watts your solar system can generate:  
\#define MAX_WATT_POWER 3924

Configure your timezone (for example GMT+1):  
\#define TIMEZONE +1

If you want to enable the dutch daylight savings time uncomment the following line by removing the first two slashes:  
\#define ENABLE\_DUCTH\_DST\_TIME

Configure the post interval to pvoutput (for example: every 5 minutes):  
\#define queue_post_interval 5

Choose your output client which you want to log your data to, PVOUTPUT or THINGSPEAK:  
\#define OUTPUT_CLIENT   PVOUTPUT

If you want to use PVOutput, configure the pvoutput.org API key and systemid:  
\#define PVOUTPUT_APIKEY "<Put your pvoutput apikey here>"  
\#define PVOUTPUT_SYSTEMID "<Put your pvoutput systemid here>"

If you want to log your data to ThingSpeak, configure the API key, the power field and the energy field:  
\#define THINGSPEAK_APIKEY "<Put your thingspeak apikey here>"  
\#define THINGSPEAK_POWER_FIELD "field1"  
\#define THINGSPEAK_ENERGY_FIELD "field2"

####Build the firmware and flash your chip:
Execute the build.sh script:

    ./build.sh

Connect your chip to the USB port and flash it:

    sudo sh -c 'chown $SUDO_USER /dev/ttyUSB0'
    make flash
    
If everything went fine the chip is ready to use. Below are the schematics on how to connect the wiring.

## Wire schematics

The following wire mappings are based on the "NodeMCU Lua ESP8266 ESP-12E WiFi Development Board" which I build this project on:

kWh meter "s0+" (used for pulse): GPIO13 (D7)  
kWh meter "s0-" (used for pulse): Ground (pin above D5)  
Green LED "+" (device is powered on): GPIO5 (D1)  
Blue LED "+" (wifi is connected): GPIO4 (D2)  
Red LED "+" (wifi connection error): GPIO2 (D4)  
Yellow LED "+" (blinks on every pulse): GPIO12 (D6/HSPIQ)
