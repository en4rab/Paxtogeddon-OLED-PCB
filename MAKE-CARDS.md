To configure the Paxtogeddon reader you will need to make a couple of config cards.

These are best made by rewriting some Paxton ISO cards. It is assumed you will have some spare as the making full use of the reader requires being able to clone cards. It is entirely possible to clone to a blank hitag2 cards by writing the appropriate password and configuration to it but writing those carries with it a risk of a bad write bricking the card. It is much easier to buy a pack of Paxton ISO cards from eBay and rewrite those.



#### Paxton function cards

Paxton readers made after 2015 will respond to config cards for the first 2 minutes after power on. These cards allow you to configure options mostly relating to the switch2 system but of interest to this application are "Silent Operation" and "Backlight Dimmer"

These two cards will allow you to turn off the beep when a card is scanned and adjust the brightness of the led's on the front of the reader. You can either buy these cards or rewrite a Paxton card.  

To convert a normal Paxton card into a Silent operation card use the following commands on your proxmark3:  

```
lf hitag wrbl --ht2 -k BDF5E846 -p 4 -d 84030810  
lf hitag wrbl --ht2 -k BDF5E846 -p 5 -d AE441204  
lf hitag wrbl --ht2 -k BDF5E846 -p 6 -d ADAB0840  
lf hitag wrbl --ht2 -k BDF5E846 -p 7 -d 08610084
```  

Present this card to the reader within 2 mins of power on and it will toggle beeping when reading a card. The reader will still beep once at power on.  

To change a Paxton card to a backlight dimmer card:  
```
lf hitag wrbl --ht2 -k BDF5E846 -p 4 -d 840309C0  
lf hitag wrbl --ht2 -k BDF5E846 -p 5 -d B06A7A88  
lf hitag wrbl --ht2 -k BDF5E846 -p 6 -d 3CF27840  
lf hitag wrbl --ht2 -k BDF5E846 -p 7 -d 09216084
```  

Each time you present the card it changes the LED brightness very roughly like 100% > 75% > 50% > 25% > off > 100%

If you don't want the LED's active they can always be left disconnected but this gives the option of them to be working but dimmed so they wont show through a bag.



#### Paxtogeddon function cards

The paxtogeddon code looks for Paxton cards with specific numbers in order to trigger specific functions.

The proxmark commands below are assuming you are changing the number of an already valid Net2 card if you are writing it to a blank hitag2 you would need to program the card as a Paxton card and write blocks 6 and 7 too. 

| Number   | Function                                                     |
| -------- | ------------------------------------------------------------ |
| 99999999 | Test card (will be logged as "Test")                         |
| 99999998 | Defaults the settings and Reboots the ESP32 whilst retaining the log |
| 99999997 | Reboots the ESP32 and connects to an access point if SSID and PW are set |
| 99999996 | Shows AP info on the OLED if the OLED is set as connected    |

##### 99999999 Test Card

```
lf hitag wrbl --ht2 -k BDF5E846 -p 4 -d CE739CE4  
lf hitag wrbl --ht2 -k BDF5E846 -p 5 -d CE7F0006
```

##### 99999998 Reset to Defaults

```
lf hitag wrbl --ht2 -k BDF5E846 -p 4 -d CE739CE4  
lf hitag wrbl --ht2 -k BDF5E846 -p 5 -d CA3F0006
```

##### 99999997 Connect to AP

```
lf hitag wrbl --ht2 -k BDF5E846 -p 4 -d CE739CE4  
lf hitag wrbl --ht2 -k BDF5E846 -p 5 -d C9FF0006
```

##### 99999996 Show AP info

```
lf hitag wrbl --ht2 -k BDF5E846 -p 4 -d CE739CE4  
lf hitag wrbl --ht2 -k BDF5E846 -p 5 -d CDBF0006
```
