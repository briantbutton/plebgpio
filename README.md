# plebgpio

**plebgpio** provides a high level API for GPIO operation of LEDs and buttons.&nbsp; 
It is configured using a simple config file using *"standard"* conventions.&nbsp; 
Control and telemetry passes through another handful of files.&nbsp; 
The config file and other files live in the "pleb" directory (`/etc/pleb/gpio`).&nbsp; 

### Highlights

&#9679; Easy to set up &mdash; even easier to operate&nbsp;  
&#9679; Negligible usage of system resources (memory and cpu)&nbsp;  
&#9679; Accessible by any language running on Linux&nbsp;  
&#9679; Uses *(in 2024)* the very latest GPIO kernel API&nbsp;  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*(see fine print on these highlights below)*&nbsp; 

## Raison d'être &nbsp; 

### Easy

Some folks wanna do edgy things with GPIO; we plebeians just wanted an easy way to manage LEDs and buttons.&nbsp; 
Our searches did not reveal much, and most of that was discontinued.&nbsp; 
**plebgpio** is intended to fill this void.&nbsp; 

### Current

**plebgpio** is based on the V2 ioctl interface ( aka GPIO Character Device Userspace API &mdash; https://www.kernel.org/doc/html/latest/userspace-api/gpio/chardev.html ).&nbsp; 
This is hot off the presses and, God willing, will be with us for a very long time.&nbsp; 

### Versatile

Reading and writing to files with names like `btn0` and `led1` can be done from Bash, Python, JavaScript, C and other languages.&nbsp; 
Here is what it looks like using Bash script.

	$ echo 0 > /etc/pleb-gpio/led1              # turn off LED1
	$ cat /etc/pleb-gpio/btn0                   # see whether BTN0 is depressed
	                                            # 'has been pressed' not 'is sad'

## Basic Usage

### Basic Configuration

The configuration file (`/etc/pleb/gpio/config.txt`) allows one specify some LEDs and buttons.&nbsp; 
Once specified, they can be controlled/monitored by using their filename, as you see above.&nbsp; 
Since ground pins are not specified, the entry for a button is gonna be a one line configuration.&nbsp; 

	[btn0]
	hot=17

The entry above defines a button `btn0`, using the non-ground lead at GPIO #17.&nbsp; 
*All* pin numbers are GPIO references.&nbsp; 
(https://pinout.xyz/)&nbsp; 

A single color LED will likewise be one configuration line.&nbsp; 

	[led1]
	pwr=25

Whoops!&nbsp; 
That was easy.&nbsp; 
Those lines configure a button, labeled `btn0`, using a ground pin and GPIO #17, and also an LED, labeled `led1`, using a ground pin and GPIO #25.&nbsp; 

### Basic Operation

Turning on the LED is now a matter of writing into its file.&nbsp; 
A '1' for on and a '0' for off.&nbsp; 
Done like this:&nbsp; 

	$ echo 1 > /etc/pleb-gpio/led1              # turn on LED1

Buttons work the same, but you read the file, instead of writing.&nbsp; 
The operation below will show a '1' or a '0'.&nbsp; 

	$ cat /etc/pleb-gpio/btn0                   # see whether BTN0 is depressed

If you want your software to monitor `btn0`, you will want to poll this file.&nbsp;
Our poll rate is 250ms.&nbsp; 

## Advanced Operation &mdash; Colors

### Color Configuration

**plebgpio** supports multi-color _Common Cathode_ LEDs.&nbsp; 
To set multiple colors, we add a `colors=` line to the config file.&nbsp;
Also, we no longer specify `pwr=` but specify the color of each pin.&nbsp; 
See this example.&nbsp; 

	[led2]
	colors=3
	red=11
	green=9
	blue=10

### Color Operation

While monochrome LEDs work with a binary code (0 or 1), multi-color LEDs take an octal code (0 through 7).&nbsp; 
The coding is rgb; red == 4, green == 2 and blue == 1.&nbsp;
Now, we do it this way:&nbsp; 

	$ echo 5 > /etc/pleb-gpio/led2              # magenta in LED2

## Advanced Operation &mdash; Programs

### Program Configuration

We frequently signal information with time-related behavior &mdash; like a fast blink or a slow blink.&nbsp; 
This can be done, using **programs**.&nbsp; 
A program is a time-series of display, probably running in a loop.&nbsp; 
Let's look at one.

	[prog6]
	next=-----------------------<
	led1=ggooggoooooooooooooooooo

Program 6 blinks green twice.&nbsp;
Forever.&nbsp; 

Each period of the program is one eighth of a second, 125ms.&nbsp; 
The top row, `next=` describes what happens next &mdash; `-` means "continue" `<` means "loop from the start", and `x` means stop.&nbsp; 
The second row, `led1=` assigns an octal value to LED1 for that period.&nbsp; 

In a program, we use letters for the octal codes:&nbsp;  
&nbsp;&nbsp;~&nbsp;o: 0&nbsp; _(off)_&nbsp;  
&nbsp;&nbsp;~&nbsp;g: 1&nbsp; _(green)_&nbsp;  
&nbsp;&nbsp;~&nbsp;b: 2&nbsp; _(blue)_&nbsp;  
&nbsp;&nbsp;~&nbsp;c: 3&nbsp; _(cyan)_&nbsp;  



### Program Operation

While monochrome LEDs work with a binary code (0 or 1), multi-color LEDs take an octal code .&nbsp; 
The coding is rgb; red == 4, green == 2 and blue == 1.&nbsp;
Now, we do it this way:&nbsp; 

	$ echo 5 > /etc/pleb-gpio/led5              # magenta in LED2






# plebgpio

**plebgpio** provides a high level API for GPIO operation of LEDs and buttons, using the v2 ioctl interface ( aka GPIO Character Device Userspace API &mdash; https://www.kernel.org/doc/html/latest/userspace-api/gpio/chardev.html ).&nbsp; 
It is designed to run in background; communication is done with a simple file interface.&nbsp; 

**plebgpio** is dedicated to supporting buttons and leds, nothing more.&nbsp; 
However, in that space, you might find it is easy to use and powerful.&nbsp; 
It supports single-color, two-color or three-color LEDs, and user-pressable buttons.&nbsp; 
Up to four LEDs and three buttons are supported.&nbsp; 



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




