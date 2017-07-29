

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
