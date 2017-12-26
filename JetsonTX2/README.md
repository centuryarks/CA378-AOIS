# CA378-AOIS for Jetson TX2

This is Software Setup Guide.

## History

- **2017/12/15** - v1.0.0: Initial Release
- **2017/12/18** - v1.0.1: "How to build sample code" added
- **2017/12/26** - v1.0.2: Support 2 to 5 connected multiple cameras

## 1. Environment configuration

Before installing the CA 378-AOIS driver, please implement the following environment.

#### Prerequisites

- Installing Jetpack 3.0 (Linux for Tegra R27.1)
- Setting sudo permissions

#### Step 1. Install Jetpack 3.0

(1) Register as user at NVIDIA DEVELOPER site.

https://developer.nvidia.com

(2) Download Jetpack 3.0

https://developer.nvidia.com/embedded/jetpack-3_0

(3) Install according to the procedure on page 10 below.

http://developer2.download.nvidia.com/embedded/L4T/r27_Release_v1.0/Docs/Jetson_X2_Developer_Kit_User_Guide.pdf?WZACmxm6jRHQtvToEuvEm4kspfaRaZJk8iX8mMEtn-YgwECKmEIn-GFEW5UPf9HIdkALsnxIZX4qZxO43CE3crogni16PuviWZO4bLF23I12fMhJ1jxyn4qq5OZHaMImZrchFQAhDNjQln9rSK6fw0lGAfzB12fu5WXOU717PVLQW6slDJIKTg

#### Step 2. Setting sudopermissions

(1) Execute the following command.

```
$ sudo visudo
```

(2) Add the following 2 lines of "nvidia".

```
# User privilege specification
root    ALL=(ALL:ALL) ALL
nvidia  ALL=(ALL:ALL) ALL

# Members of the admin group may gain root privileges
%admin ALL=(ALL) ALL

# Allow members of group sudo to execute any command
%sudo   ALL=(ALL:ALL) ALL
%nvidia ALL=(ALL:ALL) NOPASSWD: ALL
```

(3) Reboot Jetson TX2.

```
$ sudo reboot
```

## 2. Driver install

#### Installation procedure

(1) Download "CA378_2L_v1.0.2_L4T27.1.tar.gz" from the following site.

https://github.com/centuryarks/CA378-AOIS/releases

```
$ wget --no-check-certificate https://github.com/centuryarks/CA378-AOIS/releases/download/v1.0.2/CA378_2L_v1.0.2_L4T27.1.tar.gz
```

(2) Extract "CA378_2L_v1.0.2_L4T27.1.tar.gz" file.

```
$ tar zxvf CA378_2L_v1.0.2_L4T27.1.tar.gz
```

(3) Edit "/boot/extlinux/extlinux.conf" and add the following the line of "FDT".

```
$ sudo vim /boot/extlinux/extlinux.conf
```

```
TIMEOUT 30
DEFAULT primary

MENU TITLE p2771-0000 eMMC boot options

LABEL primary
      MENU LABEL primary kernel
      LINUX /boot/Image
      FDT /boot/tegra186-quill-p3310-1000-c03-00-imx378.dtb
      APPEND fbcon=map:0 net.ifnames=0 console=tty0 OS=l4t console=ttyS0,115200n8 memtype=0 video=tegrafb no_console_suspend=1 earlycon=uart8250,mmio32,0x03100000 gpt tegraid=18.1.2.0.0 tegra_keep_boot_clocks maxcpus=6 android.kerneltype=normal androidboot.serialno=0335115020673 vpr_resize root=/dev/mmcblk0p1 rw rootwait
```

(4) Configure the number of camera connections.

Execute the following command and enter the number of camera connections.

```
$ cd CA378_2L_v1.0.2_L4T27.1/
$ ./Install.sh
What is the number of camera connections? : 5
CN2 : CH1 Camera module installed
CN3 : CH2 Camera module installed
CN4 : CH3 Camera module installed
CN5 : CH4 Camera module installed
CN6 : CH5 Camera module installed
Cameras will be available after reboot.
```

(5) Shutdown Jetson TX2 and turn off the power.

(6) Connect the Jetson TX2 conversion board as shown in the picture below.

![JetsonTX2Board](https://github.com/centuryarks/Images/blob/master/JetsonTX2BoardConnection6X.JPG)

***Please connect so that it becomes serial number from CN2 as follows.***

```
1 unit connection:  CN2
2 units connection: CN2 to CN3
3 units connection: CN2 to CN4
4 units connection: CN2 to CN5
5 units connection: CN2 to CN6
6 units connection: CN2 to CN7
```

(7) Turn on the power and start up Jetson TX2.

***If you want to change the number of camera connections, repeat steps 4 to 7.***

## 3. Software install

Please install by the following procedure.

#### Installation procedure

(1) Download "demo_v1.0.0_tx2.tar.gz" from the following site.

https://github.com/centuryarks/Sample/releases

```
$ wget --no-check-certificate https://github.com/centuryarks/Sample/releases/download/v1.0/demo_v1.0.0_tx2.tar.gz
```

(2) Extract "demo_v1.0.0_tx2.tar.gz" file.

```
$ tar zxvf demo_v1.0.0_tx2.tar.gz
```

(3) Execute "Install.sh" in the extracted folder.

```
$ cd demo
$ ./Install.sh
```

(4) A shortcut is created on the desktop.

DEMO

## 4. Demonstration functions

### 4.1. Focus & OIS

Procedure of starting Focus & OIS:

(1) Click "DEMO" on the desktop.

(2) After a while the GUI screen will be displayed.

(3) Please change the distance of the object, or move the camera, confirm the function.

![image4](https://github.com/centuryarks/Images/blob/master/doc/image4.jpg)

#### Procedure of finishing Focus & OIS:

(1) Click the [x]

![image5](https://github.com/centuryarks/Images/blob/master/doc/image5.jpg)


The following section describes each function of Focus & OIS.

![image6](https://github.com/centuryarks/Images/blob/master/doc/image6.png)

##### LSC

- Check to enable shading correction.

(*) Theoretical values have been set.

#### Focus Mode

- Direct: Directly specify the focus position. 
- Infinity: Set the focus position to infinity.
- Macro: Set the focus position to the short distance.
- Focus Position: Focus position.
- Apply: Apply the settings.
- Auto Focus ON: Enable auto focus. 
- Auto Focus OFF: Disable auto focus.

(*) Current debug control is for demo.

#### OIS Mode

- OISOFF: Disable OIS.
	- It corresponds to each OIS mode.
		- Zero Shutter
		- Exposure / Shake eval.
		- Movie
		- High SR Movie
		- View Finder
- Apply: Apply the settings.

#### Still Capture

- 12M Normal: Capture 12Mpixel normal still image.


### 4.1. Focus & OIS


#### Description of the script file:

It describes about the "script/demo.sh".

```
#!/bin/sh
cd /home/nvidia/demo
sudo ./bin/DemoGUI -id 0 -w 4056 -h 3040 -fps 30
```

Function Description
 - -id Specify 0 to 5 when 6 camera modules are connected.
 - -w Specify the width.
 - -h Specify the height.
 - -fps Specify the frame rate.

The combination of resolution and frame rate is as follows.
 - 4056 x 3040 @ 30 fps
 - 3840 x 2160 @ 30 fps
 - 1920 x 1080 @ 30 fps
 - 1920 x 1080 @ 60 fps
 - 1920 x 1080 @ 120 fps

(*) To add a mode, edit "script / preview.sh".


### 4.2. 12Mpixel still image capturing

#### Procedure of capturing 12Mpixel still image:

(1) Adjust the focus.

(It is useful to turn on Auto Focus and turn Auto Focus OFF when focus is on)

(2) Click the [12M Normal] button

![image6a](https://github.com/centuryarks/Images/blob/master/doc/image6a.png)

(3) Images can be captured in JPEG and YUV format. 

***(RAW format will be available soon.)***

![image7](https://github.com/centuryarks/Images/blob/master/doc/image7.jpg)

### 4.3. Multi cameras control

Please start one at a time the following command although it will take some time.

```
$ cd demo
$ ./script/demo.sh 0 &
$ ./script/demo.sh 1 &
$ ./script/demo.sh 2 &
$ ./script/demo.sh 3 &
$ ./script/demo.sh 4 &
$ ./script/demo.sh 5 &
```

![image8](https://github.com/centuryarks/Images/blob/master/doc/image8.jpg)

***For it exceeds the processing of Jetson TX2 (1.4G pixel/sec),
Depending on Jetson's environment and conditions, it may not be possible to display 6 cameras.***

## 5. Sample Script

### 5.1. Camera display

The camera can be displayed on the screen with the following command.

```
$ gst-launch-1.0 nvcamerasrc ! 'video/x-raw(memory:NVMM),width=4056,height=3040,framerate=(fraction)30/1' ! nvvidconv ! nvoverlaysink -e
$ gst-launch-1.0 nvcamerasrc ! 'video/x-raw(memory:NVMM),width=3840,height=2160,framerate=(fraction)30/1' ! nvvidconv ! nvoverlaysink -e
$ gst-launch-1.0 nvcamerasrc ! 'video/x-raw(memory:NVMM),width=1920,height=1080,framerate=(fraction)120/1' ! nvvidconv ! nvoverlaysink -e
```

### 5.2. JPEG still image capturing

JPEG still image can be captured with the following command.

```
$ gst-launch-1.0 nvcamerasrc num-buffers=1 ! 'video/x-raw(memory:NVMM),width=3840,height=2160,format=I420,framerate=30/1' \
    ! nvvidconv ! nvjpegenc ! filesink location=test.jpg -e
```

### 5.3. H.264 video recording

H.264 video can be recorded with the following command. (Maximum 60 fps)

```
$ gst-launch-1.0 nvcamerasrc num-buffers=100 ! 'video/x-raw(memory:NVMM),width=1920,height=1080,format=I420,framerate=60/1' \
    ! nvvidconv ! omxh264enc ! qtmux ! filesink location=test.mp4 -e
```

### 5.4. YUV video recording

YUV video can be recorded with the following command.

```
$ gst-launch-1.0 nvcamerasrc num-buffers=200 ! 'video/x-raw(memory:NVMM),width=1920,height=1080,format=I420,framerate=120/1' \
    ! nvvidconv ! 'video/x-raw,width=1920,height=1080,format=I420,framerate=120/1' ! filesink location=test.yuv -e
```

### 5.5. Multiple cameras display

Multiple cameras can be displayed on the screen with the following command.

```
$ gst-launch-1.0 nvcamerasrc sensor-id=0 ! 'video/x-raw(memory:NVMM), width=(int)3840, height=(int)2160, framerate=(fraction)30/1' !\
    nvvidconv flip-method=2 ! 'video/x-raw, width=(int)640, height=(int)360, framerate=(fraction)30/1' ! queue ! xvimagesink \
    nvcamerasrc sensor-id=1 ! 'video/x-raw(memory:NVMM), width=(int)3840, height=(int)2160, framerate=(fraction)30/1' ! \
    nvvidconv flip-method=2 ! 'video/x-raw, width=(int)640, height=(int)360, framerate=(fraction)30/1' ! queue ! xvimagesink \
    nvcamerasrc sensor-id=2 ! 'video/x-raw(memory:NVMM), width=(int)3840, height=(int)2160, framerate=(fraction)30/1' ! \
    nvvidconv flip-method=2 ! 'video/x-raw, width=(int)640, height=(int)360, framerate=(fraction)30/1' ! queue ! xvimagesink \
    nvcamerasrc sensor-id=3 ! 'video/x-raw(memory:NVMM), width=(int)3840, height=(int)2160, framerate=(fraction)30/1' ! \
    nvvidconv flip-method=2 ! 'video/x-raw, width=(int)640, height=(int)360, framerate=(fraction)30/1' ! queue ! xvimagesink \
    nvcamerasrc sensor-id=4 ! 'video/x-raw(memory:NVMM), width=(int)3840, height=(int)2160, framerate=(fraction)30/1' ! \
    nvvidconv flip-method=2 ! 'video/x-raw, width=(int)640, height=(int)360, framerate=(fraction)30/1' ! queue ! xvimagesink \
    nvcamerasrc sensor-id=5 ! 'video/x-raw(memory:NVMM), width=(int)3840, height=(int)2160, framerate=(fraction)30/1' ! \
    nvvidconv flip-method=2 ! 'video/x-raw, width=(int)640, height=(int)360, framerate=(fraction)30/1' ! queue ! xvimagesink
```

***For it exceeds the processing of Jetson TX2 (1.4G pixel/sec),
Depending on Jetson's environment and conditions, it may not be possible to display 6 cameras.***

## 6. How to build sample code

### 6.1. Install build environment

```
$ sudo apt-get install qt4-dev-tools
```

Install the build environment with the following command.

### 6.2. GUI design

(1) Search "qt" and select "Qt 4 Designer"

![image11](https://github.com/centuryarks/Images/blob/master/doc/image11.png)

(2) Click "Open...", select and open "demo/src/mainwindow.ui"

![image12](https://github.com/centuryarks/Images/blob/master/doc/image12.png)

(3) Change design and save

![image13](https://github.com/centuryarks/Images/blob/master/doc/image13.png)

### 6. 3. Build

Build with the following command.

```
$ qmake
$ make
```
