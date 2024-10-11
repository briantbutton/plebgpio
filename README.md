# plebgpio

**plebgpio** provides a high level API for GPIO operation of LEDs and buttons.&nbsp; 
It is configured with a simple config file using *"standard"* conventions.&nbsp; 
Control and telemetry passes through another handful of files.&nbsp; 
The config file and other files live in the "pleb" directory (`/etc/pleb/gpio/`).&nbsp; 

### Highlights

&#9679; Easy to set up &mdash; even easier to operate&nbsp;  
&#9679; Negligible usage of system resources _(memory and cpu)_&nbsp;  
&#9679; Accessible by any language running on Linux&nbsp;  
&#9679; Uses the very latest GPIO kernel API&nbsp;  

## Raison d'être &nbsp; 

### Limited scope

Some folks wanna do edgy things with GPIO; we plebeians just wanted an easy way to manage LEDs and buttons.&nbsp; 
Our searches did not reveal much, and most of that was discontinued.&nbsp; 
**plebgpio** is intended to fill this void.&nbsp; 

### Current

**plebgpio** is based on the V2 ioctl interface (aka [GPIO Character Device Userspace API](https://www.kernel.org/doc/html/latest/userspace-api/gpio/chardev.html)).&nbsp; 
This is hot off the presses and, God willing, will be with us for a very long time.&nbsp; 

### Easy and versatile

The concept is simple:&nbsp; writing to a file sets a value for the LED that file represents.&nbsp; 
Reading a button&apos;s value is done by reading from the file representing that button.&nbsp; 

	$ echo 0 > /etc/pleb-gpio/led1            # turn off LED1
	                                          # 
	$ cat /etc/pleb-gpio/btn0                 # see whether BTN0 is depressed
	                                          # ('has been pressed' not 'is sad')

It&apos;s universally accessibile.&nbsp; 
Reading and writing to files with names like `btn0` and `led1` can be done from Bash, Python, JavaScript, C and other languages.&nbsp; 

## Basic Usage

### Basic Configuration

The configuration file (`/etc/pleb/gpio/config.txt`) allows one to specify some LEDs and buttons.&nbsp; 
Once specified, they can be controlled/monitored by using their filename.&nbsp; 
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

	$ echo 1 > /etc/pleb/gpio/led1            # turn on LED1

Buttons work the same, but you read the file, instead of writing.&nbsp; 
The operation below will return a '1' or a '0'.&nbsp; 

	$ cat /etc/pleb/gpio/btn0                 # see whether BTN0 is depressed

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

In a program, _(and ONLY in a program)_ we use letters for the octal (or binary) codes:&nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;`o`&nbsp;&nbsp; 0&nbsp;&nbsp; _(off)_&nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;`p`&nbsp;&nbsp; 1&nbsp;&nbsp; _(pwr)_&nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;`b`&nbsp;&nbsp; 1&nbsp;&nbsp; _(blue)_&nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;`g`&nbsp;&nbsp; 2&nbsp;&nbsp; _(green)_&nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;`c`&nbsp;&nbsp; 3&nbsp;&nbsp; _(cyan)_&nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;`r`&nbsp;&nbsp; 4&nbsp;&nbsp; _(red)_&nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;`m`&nbsp;&nbsp; 5&nbsp;&nbsp; _(magenta)_&nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;`y`&nbsp;&nbsp; 6&nbsp;&nbsp; _(yellow)_&nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;`w`&nbsp;&nbsp; 7&nbsp;&nbsp; _(white)_&nbsp;  

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

Ideally **plebgpio** is launched by SystemD and is a quietly provides a simplified interface for GPIO.&nbsp; 

### Installation

There is no installation program for **plebgpio** yet but the steps should be easy for people comfortable with the craft.&nbsp; 
The installing user must have root privilege.&nbsp; 
The `Makefile` describes how to compile and install the **plebgpio** program.&nbsp; 
There is a sample systemd service file that should get you going.&nbsp; 
Short of making it a systemd process, one can invoke **plebgpio** in background by calling.&nbsp;  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;$&nbsp;plebgpio &amp;&nbsp; 

**WAIT!**&nbsp;&nbsp; 
Stop.&nbsp; 
Before invoking it, you should create the **pleb** directory.&nbsp; 
_(As noted, there is no installation program.)_&nbsp; 
Soooo . . .&nbsp; 

	  $ sudo mkdir /etc/pleb
	  $ sudo mkdir /etc/pleb/gpio
	  $ sudo chown root:dialout /etc/pleb/gpio
	  $ sudo chmod 2770 /etc/pleb/gpio

And you probably want to put an item or two in config.txt.&nbsp; 
Here is a comprehensive list of the available sections:&nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;pleb &nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;led0 &nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;led1 &nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;led2 &nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;led3 &nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;btn0 &nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;btn1 &nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;**btn2** &nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;**prog1** &nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;**prog2** &nbsp;   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&bull;&nbsp;  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&bull;&nbsp;  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&bull;&nbsp;  
&nbsp;&nbsp;&nbsp;&bull;&nbsp;**prog9** &nbsp;  

 :warning: Be careful.&nbsp; 
 An improper `config.txt` will stop everything and error reporting is negligible right now.&nbsp; 

## Acknowlegements

Someone going by **drankinatty** in the [Raspberry Pi Forums](https://forums.raspberrypi.com/viewtopic.php?p=2218500) wrote the V2 functions that are being used here.&nbsp; 
Thanks **drankinatty** wherever you are.&nbsp; 

The config file parser uses the simple but effective code we found [here](https://github.com/benhoyt/inih).&nbsp; 
We just popped it in, it compiled and worked!&nbsp; 




