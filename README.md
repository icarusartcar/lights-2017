

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

## Teh Codes

Go to https://github.com/icarusartcar/lights-2017 (where you may be
reading this) and learn to use git in order to get the code.
Hopefully you know git already, otherwise... block out some time.

In the toplevel directory of the checkout there are several
directories.  Each is an arduino "sketch" that you can open with the
idea, buiild and upload to the chip.

### hello_world

this one just blinks a light, hello-arduino-style.   Open the directory, compile, upload, blue light blinks.

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

### blifnars

This is the whole-enchilada version with webserver and opc mode.  Get
it built and flashed... by default you'll see the unicorn puke mode.
