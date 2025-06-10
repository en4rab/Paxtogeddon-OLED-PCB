### Overview

The Paxtogeddon reader is source code to create a weaponised reader to target Paxton Net2 and Switch2 tokens.

Features:

* CLK/DATA decoding straight off the back of the reader
* Paxton auth token decoding
  * Also determines if its was a Net2 or Switch2 token, and even the colour of the fob!
* Persistent log saving on SPIFFS
* Wifi AP ect for in the field shinanigans

### Quote from the codes main developer

```
"Anyone can share it mate, do whatever you want with it. 
Stick it on your own GitHub, I don’t mind at all. 👍"
```

Note I was not the main writer of this code, but Dan couldn't be bothered to put it on GitHub because he not a hacker.

### Quote from En4rab

```
“I would have used my own code but it was shit and Daniel's is much better”
```

### Bill of materials

* ESP32 dev board
* 0.22uF Electrolytic capacitor
* 1uF Electrolytic capacitor
* 100nF Ceramic capacitor
* 2x 1kOhm resistors
* 7805 Voltage Regulator
* 2x 1A Diodes
* desire to get root

### Janky Circuit Design

If anyone wants to draw this a bit nicer be my guest

![Janky Circuit Drawing](https://github.com/00Waz/Paxtogeddon-Reader/blob/main/circuit.png?raw=true)