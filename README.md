# plebgpio

**plebgpio** provides a high level API for GPIO operation of LEDs and buttons.&nbsp; 
It is configured using a simple config file using *"standard"* conventions.&nbsp; 
Control and telemetry passes through another handful of files.&nbsp; 
The config file and other files live in the "pleb" directory (`/etc/pleb/gpio/`).&nbsp; 

### Highlights

&#9679; Easy to set up &mdash; even easier to operate&nbsp;  
&#9679; Negligible usage of system resources (memory and cpu)&nbsp;  
&#9679; Accessible by any language running on Linux&nbsp;  
&#9679; Uses *(in 2024)* the very latest GPIO kernel API&nbsp;  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*(see fine print on these highlights below)*&nbsp; 

## Raison d'être &nbsp; 

### Limited scope

Some folks wanna do edgy things with GPIO; we plebeians just wanted an easy way to manage LEDs and buttons.&nbsp; 
Our searches did not reveal much, and most of that was discontinued.&nbsp; 
**plebgpio** is intended to fill this void.&nbsp; 

### Current

**plebgpio** is based on the V2 ioctl interface ( aka GPIO Character Device Userspace API &mdash; https://www.kernel.org/doc/html/latest/userspace-api/gpio/chardev.html ).&nbsp; 
This is hot off the presses and, God willing, will be with us for a very long time.&nbsp; 

### Easy and versatile

The concept is simple; writing to a file writes a value to the LED that file maps to.&nbsp; 
Reading a button&apos;s value is done by reading from the file representing that button.&nbsp; 

	$ echo 0 > /etc/pleb-gpio/led1            # turn off LED1
	                                          # 
	$ cat /etc/pleb-gpio/btn0                 # see whether BTN0 is depressed
	                                          # ('has been pressed' not 'is sad')

It&apos;s universally accessibile.&nbsp; 
Reading and writing to files with names like `btn0` and `led1` can be done from Bash, Python, JavaScript, C and other languages.&nbsp; 
Here is what it looks like using Bash script.

## Basic Usage

### Basic Configuration

The configuration file (`/etc/pleb/gpio/config.txt`) allows one to specify some LEDs and buttons.&nbsp; 
Once specified, they can be controlled/monitored by using their filename, as you see above.&nbsp; 
Since ground pins are not specified, the entry for a button is a two line configuration &mdash; label the button and name the non-ground pin.&nbsp; 

	[btn0]
	hot=17

The entry above defines a button `btn0`, using the non-ground lead at GPIO #17.&nbsp; 
*All* pin numbers are GPIO references.&nbsp; 
(https://pinout.xyz/)&nbsp; 

A single color LED will likewise be two configuration lines.&nbsp; 

	[led1]
	pwr=25

That was easy!&nbsp; 
The lines above configure a button, labeled `btn0`, using a ground pin and GPIO #17, and also an LED, labeled `led1`, using a ground pin and GPIO #25.&nbsp; 
Now we can put them to use.&nbsp; 

### Basic Operation

Turning on the LED1 is now a matter of writing into its file.&nbsp; 
A '1' for on and a '0' for off.&nbsp; 
Done like this:&nbsp; 

	$ echo 1 > /etc/pleb-gpio/led1            # turn on LED1

Buttons work the same, but you read the file, instead of writing.&nbsp; 
The operation below will return a '1' or a '0'.&nbsp; 

	$ cat /etc/pleb-gpio/btn0                 # see whether BTN0 is depressed

If you want your software to monitor `btn0`, you will want to poll this file.&nbsp;
Our poll rate is 250ms.&nbsp; 

## Advanced Operation &mdash; Colors

### Color Configuration

**plebgpio** supports multi-color LEDs.&nbsp; 
To set multiple colors, we add a `colors=` line to the config file.&nbsp;
Also, we replace the `pwr=` line with a separate line for each color.&nbsp; 
See this example.&nbsp; 

	[led2]
	colors=3
	red=11
	green=9
	blue=10

### Color Operation

Putting a multi-color LED to work is similar to a monochrome LED.&nbsp; 
While monochrome LEDs accept a binary value (0 or 1), 3-color LEDs take an octal code (0 through 7).&nbsp; 
The coding is rgb; red == 4, green == 2 and blue == 1.&nbsp;
Now, we do it this way:&nbsp; 

	$ echo 5 > /etc/pleb-gpio/led2              # magenta in LED2

## Advanced Operation &mdash; Programs

### Program Configuration

We frequently signal information with time-related behavior &mdash; like a fast blink or a slow blink.&nbsp; 
This can be done by the user program but **plebgpio** supports it to offload housekeeping, using "**programs**".&nbsp; 
A program is a time-series of display, normally running in a loop.&nbsp; 
Let's look at one.&nbsp; 

	[prog6]
	next=-----------------------<
	led1=ggooggoooooooooooooooooo

Program 6 blinks green twice.&nbsp;
Later, it does it again.&nbsp; 
Forever.&nbsp; 

Each period of the program is one eighth of a second, 125ms.&nbsp; 
The top row, `next=` describes what happens next &mdash; `-` means "continue" `<` means "loop from the start", and `x` means stop.&nbsp; 
The second row, `led1=` assigns an octal value to LED1 for that period.&nbsp; 
Program 6 above illuminates LED1 green for 250ms, then off for 250ms then green again.&nbsp; 

In a program, we use letters for the octal codes:&nbsp;  
&nbsp;&nbsp;&bull;&nbsp;o: 0&nbsp; _(off)_&nbsp;  
&nbsp;&nbsp;&bull;&nbsp;g: 1&nbsp; _(green)_&nbsp;  
&nbsp;&nbsp;&bull;&nbsp;b: 2&nbsp; _(blue)_&nbsp;  
&nbsp;&nbsp;&bull;&nbsp;c: 3&nbsp; _(cyan)_&nbsp;  
&nbsp;&nbsp;&bull;&nbsp;r: 4&nbsp; _(red)_&nbsp;  
&nbsp;&nbsp;&bull;&nbsp;m: 5&nbsp; _(magenta)_&nbsp;  

A program can be up to 80 segments _(10 seconds)_ long before it repeats or stops.&nbsp; 
It may specify multiple LEDs which will display in lock step.&nbsp; 
This program is very pretty.&nbsp; 

	[prog2]
	next=-----------------------------------------------------------------------<
	led1=rrrrggggbbbbrrrrggggbbbbrrrrggggbbbbrrrrggggbbbbrrrrggggbbbbrrrrggggbbbb
	led2=ccccmmmmyyyyccccmmmmyyyyccccmmmmyyyyccccmmmmyyyyccccmmmmyyyyccccmmmmyyyy 

### Program Operation

Up to 8 programs (1 through 9) may be specified in config.txt.&nbsp; 
To run a program, put its decimal digit into `/etc/pleb/gpio/prog`.&nbsp; 
When a program is running, all values in the individual LED files (e.g. `led1`) are ignored.&nbsp; 
Programs do not affect button operation &mdash; that continues per normal.

Program Zero is special; it means no program, and **plebgpio** looks to the individual LED files to set the LEDs.&nbsp; 
When the system starts, it will be running program 0, unless otherwise configured.&nbsp; 

## How it works

**plebgpio** runs in the background.&nbsp; 
It loops, monitoring and filling the files described above.&nbsp; 
The normal interval is 125ms but it can be set for 62.5ms.&nbsp; 

Ideally **plebgpio** is launched by SystemD and is a quite tool to provide a simplified interface for GPIO.&nbsp; 



If you are reading this, **STOP**.&nbsp; 
The text below is detritus from editing efforts.




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




