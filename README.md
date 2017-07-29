Get the arduino ide from https://www.arduino.cc/en/Main/Software

Follow the installation instructions in the getting started guide https://www.arduino.cc/en/Guide/HomePage

You could try a hello world sketch on an arduino at this point to be sure everything works.

Get the esp8266 board support package installed and do a hello world with the esp: http://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/

I like this ftdi cable from adafruit for flashing the chips: https://www.adafruit.com/product/70

note that you have to cast some magic before the esp is ready to
flash: press and hold gpio0 button, press and release reset buttin,
then release gpio button.  a red light will stay on on the chip.  then
you can upload to the board, blue light will flicker while the upload
is happening.
