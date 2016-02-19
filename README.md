#esp8266-pvoutput-logger

Esp8266-pvoutput-logger is a kWh meter s0 pulse log application for pvoutput.org. It's designed on a ESP8266 ESP-12E WIFI Development Board. It has a built-in time service, interrupt handler, led control service, pvoutput.org client, queueing mechanism and scheduler service. It's main purpose is to monitor the energy output from your solar panels if you don't have a method to connect your converter to pvoutput.org.

##Putting the software on your ESP chip

First create a build environment on your favorite Linux distro (or maybe Cygwin under Windows?). The following steps will show you how to create it on Fedora:

###Create the build environment on Fedora 23 (x86/ARM)

Create the build directory:

    mkdir ~/esp-devel

Add the rpmfusion repos:

    sudo dnf install --nogpgcheck \
    http://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm \
    http://download1.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-$(rpm -E %fedora).noarch.rpm

Install the required packages:

    sudo dnf install make unrar autoconf automake libtool gcc gcc-c++ gperf flex \
    bison texinfo gawk ncurses-devel expat-devel python sed git pyserial patch \
    wget which file unzip bzip2

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
    wget http://bbs.espressif.com/download/file.php?id=838 -O esp_iot_sdk_v1.4.0_15_09_18.zip
    unzip esp_iot_sdk_v1.4.0_15_09_18.zip

Add the following line to header of file "~/esp-devel/esp_iot_sdk/esp_iot_sdk_v1.4.0/include/osapi.h":

    #include <c_types.h>

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

Configure your timezone (for example GMT+1):  
\#define TIMEZONE +1

If you want to enable the dutch daylight savings time uncomment the following line by removing the first two slashes:  
\#define ENABLE\_DUCTH\_DST\_TIME

Configure the post interval to pvoutput (for example: every 5 minutes):  
\#define queue_post_interval 5

Configure the pvoutput.org API key and systemid:  
\#define PVOUTPUT_APIKEY "<Put your pvoutput apikey here>"  
\#define PVOUTPUT_SYSTEMID "<Put your pvoutput systemid here>"

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
