# E-FRAMEBUFFER
Just a small program that outputs FrameBuffer from the linux powered sbc to the SPI e-Paper screen

## build instrution

~~~
git clone https://github.com/WiringPi/WiringPi
cd WiringPi
./build

git clone https://github.com/wolfer-uwu/E_FRAMEBUFFER.git --depth 1
cd E-FRAMEBUFFER/
make
~~~

## running 
~~~
# enable spi
sudo raspi-config 
# Interfacing Options -> SPI -> Yes

# reboot
sudo reboot
~~~
~~~
cd E-FRAMEBUFFER/
./bin/edp
~~~

## Display connection
~~~
# connect epaper module
# res pin  -> Pin 11
# busy pin -> Pin 18
# d/c pin  -> Pin 22
# cs pin   -> Pin 24
# sck/scl pin  -> Pin 23
# mosi/sda pin -> Pin 19
~~~
