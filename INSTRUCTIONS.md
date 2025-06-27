To configure the Paxtogeddon reader you will need to make a couple of config cards.

These are best made by rewriting some Paxton ISO cards. It is assumed you will have some spare as the making full use of the reader requires being able to clone cards. It is entirely possible to clone to a blank hitag2 cards by writing the appropriate password and configuration to it but writing those carries with it a risk of a bad write bricking the card. It is much easier to buy a pack of paxton ISO cards from ebay and rewrite those.



#### Paxton function cards

Paxton readers made after 2015 will respond to config cards for the first 2 minutes after power on. These cards allow you to configure options mostly relating to the switch2 system but of interest to this application are "Silent Operation" and "Backlight Dimmer"

These two cards will allow you to turn off the beep when a card is scanned and adjust the brightness of the leds on the front of the reader. You can either buy these cards or rewrite a Paxton card.  

To convert a normal Paxton card into a Silent operation card use the following commands on your proxmark3:  

lf hitag wrbl --ht2 -k BDF5E846 -p 4 -d 84030810  
lf hitag wrbl --ht2 -k BDF5E846 -p 5 -d AE441204  
lf hitag wrbl --ht2 -k BDF5E846 -p 6 -d ADAB0840  
lf hitag wrbl --ht2 -k BDF5E846 -p 7 -d 08610084  

Present this card to the reader within 2 mins of power on and it will toggle beeping when reading a card. The reader will still beep once at power on.  

To change a paxton card to a backlight dimmer card:  
lf hitag wrbl --ht2 -k BDF5E846 -p 4 -d 840309C0  
lf hitag wrbl --ht2 -k BDF5E846 -p 5 -d B06A7A88  
lf hitag wrbl --ht2 -k BDF5E846 -p 6 -d 3CF27840  
lf hitag wrbl --ht2 -k BDF5E846 -p 7 -d 09216084  

Each time you present the card it changes the LED brightness very roughly like 100% > 75% > 50% > 25% > off > 100%

If you don't want the LED's active they can always be left disconnected but this gives the option of them to be working but dimmed so they wont show through a bag.