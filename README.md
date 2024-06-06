# plebgpio

plebgpio provides a high level API for GPIO operation of LEDs and buttons.&nbsp; 
It can be run from Bash (Unix shell) or NodeJS.&nbsp; 

Key features:&nbsp;   
&bull;  The interface is higher level; for example one may write a color to an LED.&nbsp; 
&bull;  Operation does not require root privileges

## WIP

This is not commplete.&nbsp;
&apos;gpio.c&apos is fine but the NodeJS code is not.

It was developed for Ubuntu Linux but *may* run on other flavors&nbsp;


## Concepts

### No sudo

While root privilege is required to install 'plebgpio', it is not required to invoke it.&nbsp; 

### Stateless binary program

Normal operation is done by invoking a compiled program ('gpio') from the shell or from NodeJS.&nbsp; 
Parameters tell 'plebgpio' what to do.&nbsp;
The program iself holds no state.&nbsp; 
Instead, the calling utility hands in parameters representing a command, and the current state.&nbsp; 
The program executes the command and hands back a new state.&nbsp;

So a series of shell calls would look like this.

	export GPIO="";export GPIO=$(plebgpio initialize 1);
	export GPIO=$(plebgpio write led1 $GPIO red)


### Built-in structure

plebgpio is built around LEDs and BTNs, not pins.&nbsp; 
It contains up to 8 patterns, coded into the source &mdash; see the example below&nbsp;

	//  PATTERN0: LED1, LED2 and BTN0     (requires 'gpio=17=pu' in config.txt)
	//        ~  ~  ~   LED-0   ~  ~  ~       ~  ~  ~   LED-1   ~  ~  ~       ~  ~  ~   LED-2   ~  ~  ~     BTN-0   BTN-1   BTN-2  empty
	//        R       G       B       W       R       G       B       W       R       G       B       W
	    {     17 ,    27 ,    22 ,    22 ,    25 ,     8 ,     7 ,     7 ,    11 ,     9 ,    10 ,    10 ,    17 ,     4 ,     0 ,     0  },
	    {      0 ,     0 ,     0 ,     0 ,     1 ,     1 ,     1 ,     0 ,     1 ,     1 ,     1 ,     0 ,     1 ,     0 ,     0 ,     0  },
	//        0       1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16      

The first row gives pin numbers and the second row are bits to say what pins are available to plebgpio.

