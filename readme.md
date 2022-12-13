# About

This project is a digital light meter used for metering photos. 
There were two goals in mind when building this:
1. Let the user track the camera settings for each shot they've taken on a film camera
2. Correctly meter for a scene based on a desired shutter speed or aperture

# Parts
- [Adafruit Feather m0](https://www.adafruit.com/product/2772)
    - built in microSD reader
    - built in battery support 
    - built in serial over microUSB
- [BH1750 Light Sensor](https://learn.adafruit.com/adafruit-bh1750-ambient-light-sensor)
- [SSD1306 OLED Display](https://www.amazon.com/Songhe-0-96-inch-I2C-Raspberry/dp/B085WCRS7C/ref=sr_1_3?keywords=SSD1306&qid=1670950235&sr=8-3)
- [ArduCAM OV2640 2MP Plus](https://www.arducam.com/product/arducam-2mp-spi-camera-b0067-arduino/)

![diagram](/demo%20images/diagram.png)
> Some connections aren't shown for simplicity

# Results

All the code powering the following screens is contained in `/main`.

## Screens
### Bootup
![boot screen](/demo%20images/startup.gif)
On startup, the following values are displayed
- ISO value
- Lens focal length
- Shot number
- Battery percentage
- Battery charge bar
### Main Screen
![main screen](/demo%20images/ss%20prio%20marked%20up.png)
### Settings
![settings screen](/demo%20images/settings/settings.png)
The overall settings screen allows you to set the following values:
- ISO value
- Lens focal length
- Illuminance constant
- Shot number
- List files on SD card
![settings screen ISO](/demo%20images/settings/set%20iso.gif)
> Adjusting the ISO from settings
![settings screen ls all](/demo%20images/settings/ls%20all.gif)
> Listing the contents of the file system from settings

### History
The history screen allows the user to view the shots they've taken and see the contents of each shot
![history overview](/demo%20images/history/history.png)
![history detail](/demo%20images/history/history%20detail.gif)

## Build

Here are images of the light meter device

![front](/demo%20images/glamour%20shots/DSC02798.JPG)
![back](/demo%20images/glamour%20shots/DSC02759.JPG)
![side](/demo%20images/glamour%20shots/DSC02769.JPG)

## Film
To test this device, I shot a roll of Porta 400 film, pushed 1 stop to ISO 800, and metered with this device. Here are some results:

### Example Shot 1
![photo under bridge reference](/film%20example/sd%20backup/35.JPG)
> reference photo from light meter

![photo under bridge](/film%20example/film%20export/Scan0-2.png)
```
ISO 800
CVAL 250
F_LEN 28
PRIO SS
APT 3.50
SS 1/250.00
LUX 1050
EV_ACTUAL 11.71
EV_CALC 11.58
EV_DELTA 0.13
```
> shot settings from light meter

### Example Shot 2
![bridge photo reference](/film%20example/sd%20backup/34.JPG)
> reference photo from light meter

![bridge photo](/film%20example/film%20export/Scan0-3.png)
```
ISO 800
CVAL 250
F_LEN 28
PRIO SS
APT 8.00
SS 1/250.00
LUX 6585
EV_ACTUAL 14.36
EV_CALC 13.97
EV_DELTA 0.40
```
> shot settings from light meter

# Demo Video
[Youtube Link](https://youtu.be/QNY8x09aPGU)

# References
There were a couple of projects online that used the BH1750 light meter to assess lighting conditions but I only found one that calculated camera settings based on the input value, [linked here](https://create.arduino.cc/projecthub/alankrantas/ardumeter-arduino-incident-light-meter-606f63).

The following wikipedia articles gave me the formulas used for camera setting calculations
[Light meter](https://en.wikipedia.org/wiki/Light_meter)
[Illuminance](https://en.wikipedia.org/wiki/Illuminance)