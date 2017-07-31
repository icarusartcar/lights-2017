# Preliminaries

## Arduino

Get the arduino ide from https://www.arduino.cc/en/Main/Software

Follow the installation instructions in the getting started guide
https://www.arduino.cc/en/Guide/HomePage

You could try a hello world sketch on a regular arduino at this point
to be sure everything works.

## ESP8266

Get the esp8266 board support package installed:
http://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/

I recommend this USB cable with embedded FTDI chip for flashing the
ESP: https://www.adafruit.com/product/70, but if you've got some other
way, great.

*NOTE* that you have to cast some magic before the esp is ready to
flash, it won't automatically realize that you're trying to flash it and accept what your're sending:  you'll see

```
  warning: espcomm_sync failed
  error: espcomm_open failed
  error: espcomm_upload_mem failed
  error: espcomm_upload_mem failed
```

Now the magic: press and hold the `GPIO0` button, press and release
`RESET` button, then release `GPIO0` button.  a red light will
stay on on the chip.  then you can upload to the board, blue light
will flicker while the upload is happening.

## Teh Hardwares

Where we talk to APA102 LEDs, We use an esp8266 chip on a board with a
SN74ATH125 level-shifter chip.  This is because the logic output of
the esp is at 3.3V and the LEDs need to see 5V to work correctly.  Be
sure you have one of these little boardlets that we mount inside
weatherproof monoprice boxes.


## Teh Codes

Go to https://github.com/icarusartcar/lights-2017 (where you may be
reading this) and learn to use git in order to get the code.
Hopefully you know git already, otherwise... block out some time.

In the toplevel directory of the checkout there are several
directories.  Each is an arduino "sketch" that you can open with the
idea, buiild and upload to the chip.




### hello_world

This one just blinks a light, hello-arduino-style.  Open the
directory, compile, upload, blue light blinks.  Should run the same on
an actual arduino... this verifies that the toolchain is working
correctly.

### unicorn_puke

This is one of the patterns from the FastLED demo loop, modified to
run on our rig with APA102 and level shifter.  To build this you have
to...

#### Add the FastLED library

Click `Sketch -> Include Library -> Manage Libraries` search for
*FastLED*, add the latest version which right now looks like 3.1.3.

#### Compile and Upload Sketch

When you compile you'll see

```
note: #pragma message: FastLED version 3.001.003
 #    pragma message "FastLED version 3.001.003"
                     ^
```

Which is the world's most useless use of a #pragma message as far as I can tell. You'll also see

```
 #      pragma message "No hardware SPI pins defined.  All SPI access will default to bitbanged output"
                       ^
```

which isn't much better, and also safe to ignore.

Upload the sketch and you'll see unicorn puke over 200 leds.

# teh blifnars

This is the whole-enchilada version with webserver and opc mode.  Get
it built and flashed... by default you'll see the unicorn puke mode.

By default the chip will try to connect to ssid outerspace/outerspace.
If the chip is new to the router, you won't know what IP address it
got.  Go to the router's DHCP client table and have a look.  The rouer
should be configured icarus/outerspace for administration.

Note the mac address of the chip, (it is not printed on the chip),
give it a descriptive name, add the mac-to-ip-and-hostname mapping to
the static DHCP table.  This way you can refer to the chip by its
hostname when you work with it.  Here we'll use "dancebox".

When you've done this, reset the chip.  It should very quickly connect
to the router and get an IP, and be "ping" able and so forth.

You should be able to hit it with a browser, but `curl` gives me fewer
problems as it doesn't cache pages or DNS information, etc.:

```
http://dancebox
```

this should show you a little status page.   If you see it good.

You can set variables with an HTTP GET:

```
http://dancebox/set?relaya_state=1
```

note that your shell might see the question mark as a wildcard and
require you to quote the whole url or escape the character.  So if you
get an error figure out if it is curl or it is your shell that is
complaining.

The other important command is the led-mode switch, for instance for
'dots' mode:

```
http://192.168.1.177/setmode?mode=dots
```

try `unicorn_puke`, `cops`, `dots`, and search the code for these
strings to see how these modes are registered and switched to.

If you set `opc_mode`,

```
http://192.168.1.177/setmode?mode=opc_mode
```

the strip will go black and the chip will start listening on port
`7890` (standard for opc), and set led colors based on the messages it
receives.  At this point you can send it some stuff.

# Open Pixel Control

To be written:

* sample opc python script that does the blinky
* esp8266 TCP stack limitations
* how to break a megastrip up in to opc channels so as not to
  encounter those limitations

# WIRING

The color scheme for the four-wire (APA102) lights is:

- *BLUE* or *BLACK* Negative
- *RED*  +5V
- *GREEN* Data (esp pin 13)
- *YELLOW* Clock (esp pin 14)
