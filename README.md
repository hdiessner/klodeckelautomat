# klodeckelautomat
This project serves the purpose to provide remote control of telescope covers that prevent dust from covering the front lense of telescopes.

Hardware requirements:
- Arduino
- Servo motors (one for each moving cover)
- Cover material for covering the front opening of the telescope
- Mounting material to attach the servo to the dew shield of the telescope
- Housing for Arduino
- push buttons for each cover if manual operation is wanted

Software:
- platformio project with code for the Arduino
- .py Python script to control open / close operations via command line
- .html front-end to use a web browser to open / close the cover

# Serial commands

| ID | Data                   | Description          |
|----|------------------------|----------------------|
|  0 |                        | Data Acknowledge     |
|  1 |                        | Error                |
|  2 | (0..7), (0..255)       | Set Servo x to y     |
|  3 | (0..7), 0-OPEN 1-CLOSE | Store Servo x as pos |
|  4 | (0..2)                 | Button Pressed       |
|  5 | (0..7), 0-OPEN 1-CLOSE | Servo x to pos       |

# Pinout

| Pin | Description |
|-----|-------------|
| 2   | Button 1    |
| 3   | Button 2    |
| 4   | Button 3    |
| 5   | OE          |
| A4  | SDA         |
| A5  | SCL         |

Addition in October 2017:
- 3D sketch for 3D printing of a telescope clamp that attaches to the dew shield of an AstroPhysics Starfire 155EDF and that provides the hosuing of a servo.

https://github.com/thijse/Arduino-CmdMessenger/issues/27
