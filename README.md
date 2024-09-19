# plebgpio

plebgpio provides a high level API for GPIO operation of LEDs and buttons.&nbsp; 
It can be run from Bash (Unix shell) or NodeJS.&nbsp; 

Key features:&nbsp;   
&bull;  The interface works with buttons and LEDs, not GPIO pins&nbsp;  
&bull;  Operating plebgpio does not require root privileges&nbsp;  
&bull;  The NodeJS module has a lighweight button interrupt capability

## WIP

This is not commplete.&nbsp;
&apos;plebgpio&apos; C code is fine but the Javascript code is not.

It was developed for Ubuntu Linux but *may* run on other distributions&nbsp;


## Concepts

### No sudo

While root privilege is required to install 'plebgpio', it is not required to invoke it.&nbsp; 

### Stateless binary program

Normal operation is done by invoking a compiled program ('plebgpio') from the shell or from NodeJS.&nbsp; 
Parameters tell 'plebgpio' what to do.&nbsp; 
The program iself holds no state.&nbsp; 
Instead, the calling utility hands in parameters representing a command, and the current state.&nbsp; 
The program executes the command and hands back a new state.&nbsp;

So a series of shell calls would look like this.

	export GPIO="";export GPIO=$(plebgpio initialize 1);
	export GPIO=$(plebgpio write led1 $GPIO red)

The shell variable GPIO holds the state information.

### Built-in structure

plebgpio is controlled with reference to LEDs and BTNs, not pins.&nbsp; 
The mapping from one to another is coded into the program iself.&nbsp; 
In fact, there are eight mappings available.&nbsp; 
The actual mapping is selected with the initialize command, e.g.&nbsp;

	plebgpio initialize 1

This means that plebgpio might not be right for someone casually experimenting with different configurations on a breadboard.&nbsp; 
However, if you can restrict yourself to two or five wiring plans, they are easy to specify.&nbsp; 

## Commands

### Initialize

The command sequence always starts with an initialize.  Without it, nothing runs.  The formula:

	plebgpio initialize [1-8]

How it looks in bash *(specifying wiring pattern number #3)*.

	export GPIO="";export GPIO=$(plebgpio initialize 3);

### Write LED

plebgpio supports configuration of up to four LEDs.  

	plebgpio initialize [1-8]

How it looks in bash *(specifying wiring pattern number #3)*.

	export GPIO="";export GPIO=$(plebgpio initialize 3);





	//        ~  ~  ~   LED-0   ~  ~  ~       ~  ~  ~   LED-1   ~  ~  ~       ~  ~  ~   LED-2   ~  ~  ~     BTN-0   BTN-1   BTN-2  empty
	//        R       G       B       W       R       G       B       W       R       G       B       W
	    {     17 ,    27 ,    22 ,    22 ,    25 ,     8 ,     7 ,     7 ,    11 ,     9 ,    10 ,    10 ,    17 ,     4 ,     0 ,     0  },
	    {      0 ,     0 ,     0 ,     0 ,     1 ,     1 ,     1 ,     0 ,     1 ,     1 ,     1 ,     0 ,     1 ,     0 ,     0 ,     0  },
	//        0       1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16      

The first row gives pin numbers and the second row are bits to say what pins are available to plebgpio.

