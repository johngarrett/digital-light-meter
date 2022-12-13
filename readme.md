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

# Libraries Used
[BH1750](https://github.com/claws/BH1750)
[Adafruit_SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
[ArduCAM/Arduino](https://github.com/ArduCAM/Arduino)

# Results

The final product allows the user to set information about their camera's conditions (like ISO or lens focal length) which persists on the SD card across boots.

When in use, the user can set a desired shutter speed or aperture and the corresponding value will be calculated and displayed. 

They then can hit record and the shot metadata along with a reference photo captured by the OV2640 will be saved to the SD card under the corresponding shot number.

## Screens
### Startup
![boot screen](/demo%20images/startup.gif)

On startup, the following values are read from storage and displayed
- ISO value
- Lens focal length
- Shot number
- Battery percentage
- Battery charge bar
### Main Screen
![main screen ss prio](/demo%20images/ss%20prio%20marked%20up.png)
> shutter speed priority mode

![main screen apt prio](/demo%20images/apt%20prio%20marked%20up.png)
> aperture priority mode
### Settings
![settings screen](/demo%20images/settings/settings.png)
The settings screen allows the user to set the ISO value, lens focal length, illuminance constant, shot number, and list files on SD card.
![settings screen ISO](/demo%20images/settings/set%20iso.gif)
> Adjusting the ISO from settings

![settings screen ls all](/demo%20images/settings/ls%20all.gif)
> Listing the contents of the microSD card from settings

### History
The history screen allows the user to view the shots they've taken and see the contents of each shot
![history overview](/demo%20images/history/history.png)
![history detail](/demo%20images/history/history%20detail.gif)

## Build

Here are images of the light meter device. 

![front](/demo%20images/glamour%20shots/DSC02798.JPG)
![back](/demo%20images/glamour%20shots/DSC02759.JPG)
![side](/demo%20images/glamour%20shots/DSC02769.JPG)

The front and back panels were 3d printed and the remaining sides were laser cut from the following design
![case design](/demo%20images/case%20design.png)

## Film
To test this device, I shot a roll of Porta 400 film, pushed 1 stop to ISO 800, and metered with this device. Below are two examples from that roll.

### Example Shot 1
![photo under bridge reference](/film%20example/sd%20backup/35.JPG)
> reference photo from light meter

![photo under bridge](/film%20example/film%20export/Scan0-2.png)
> resultant photo

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
> resultant photo

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

# Retrospective
## Iterative process
### Phases
There were three major phases of this project.

1. The breadboard phase
![breadboard image](/demo%20images/breadboard.jpeg)
In this phase, every electrical connection was extremely fragile. The device stayed like this for a long time as the software was built out.

2. The cardboard case
![cardboard case image](/demo%20images/cardboard%20case.png)
This was a good way to estimate the dimensions for the final case and assess the overall feel of the device. In this phase, I was also able to calibrate the calculations as I was able to test a wider variety of conditions

3. The final case

This is the current state of the project and the images can be seen under [build](#Build).


### Things that didn't work
1. Using CircuitPython
[CircuitPython](https://circuitpython.org/) looked to be an easy to use alternative to developing with the Arduino tools but the display library for the SSD1306 immediatly used all the available memory on the feather m0.
2. Displaying images on screen
I really wanted to display the reference images on screen when capturing the image or viewing image history. However, that would require converting the stored JPG to a BMP image, converting each RGB pixel of the BMP image into a single grayscale value, and finally scaling the image down to fit on the 128x32 display. Due to time constraints and minimal ROI, this was scrapped.
3. 3D printing the correct dimensions
The resultant case for this device is partially hand carved because, no matter how many times I measured, the parts didn't fit well into the case. This mixed with the 4 hour printing prevented me from iterating as much as I wished.

## Skills learned

- How to 3D print
- How to laser cut
- The math behind photography exposure
- Designing software on a constrained device
- Making necessary compromises to get to the final product


# References
There were a couple of projects online that used the BH1750 light meter to assess lighting conditions but I only found one that calculated camera settings based on the input value, [linked here](https://create.arduino.cc/projecthub/alankrantas/ardumeter-arduino-incident-light-meter-606f63).

The following wikipedia articles gave me the formulas used for camera setting calculations
[Light meter](https://en.wikipedia.org/wiki/Light_meter)
[Illuminance](https://en.wikipedia.org/wiki/Illuminance)
