# plebgpio

**plebgpio** provides a high level API for GPIO operation of LEDs and buttons, using the v2 ioctl interface ( aka GPIO Character Device Userspace API &mdash; https://www.kernel.org/doc/html/latest/userspace-api/gpio/chardev.html ).&nbsp; 
It is designed to run in background; communication is done with a simple file interface.&nbsp; 

**plebgpio** is dedicated to supporting buttons and leds, nothing more.&nbsp; 
However, in that space, you might find it is easy to use and powerful.&nbsp; 
It supports single-color, two-color or three-color LEDs, and user-pressable buttons.&nbsp; 
Up to four LEDs and three buttons are supported.&nbsp; 

The LEDs may be set by poking octal values into intuitively-named files (e.g. `led0`).&nbsp; 
Likewise, the buttons may be sampled by reading a binary value from a file.&nbsp; 
Mapping between GPIO pins and LEDs or buttons are done with a config file.&nbsp; 
All of these files are in the same **pleb** directory (`/etc/bn/gpio`).

This means that the user code has a very simple UI, reading and writing a couple named files to observe buttons and control LEDs.&nbsp; 

**plebgpio** is launched with systemd.&nbsp; 
After launch, it periodically reads and writes values to and from the LED and button files, also updating the values in the GPIO pins accordingly.&nbsp; 
Normally, the period is 125ms, plenty fast enough for humans, although it can be configured to 62.5ms.&nbsp; 


## In overview

### Configure

The first step is to configure **plebgpio**, by editing `config.txt` in the **pleb** directory.&nbsp; 
Configuration permits you to define LEDs (up to 4) and buttons (up to 3).&nbsp; 
For LEDs, one must specify the number of colors, and name the active GPIO pin(s).&nbsp; 
If your first LED (led0) is simple red LED with the power pin at GPIO #25, the config entries will look like this:&nbsp; 

	[led0]
	colors=1
	pwr=25

Whoops!&nbsp; 
That was easy.&nbsp; 

Specifying a button is similar.&nbsp; 

	[btn0]
	hot=17

_(Note that ground pins are never specified.)_&nbsp; 

When **plebgpio** starts up, it will read `config.txt`, and behave accordingly.&nbsp; 

### Operate

Operation is done through reading and writing single characters in named files in the **pleb** directory.&nbsp; 

Readable files include:&nbsp;  
&#9679; **btn0** &nbsp;  
&#9679; **btn1** &nbsp;  
&#9679; **btn2** &nbsp;  

Writable files include:&nbsp;  
&#9679; **led0** &nbsp;  
&#9679; **led1** &nbsp;  
&#9679; **led2** &nbsp;  
&#9679; **led3** &nbsp;  
&#9679; **prog** &nbsp;  

_(Programs are an advanced way to display LED behavior, see below.)_&nbsp; 

## Installation

(coming back here)


## Acknowlegements




