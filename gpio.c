#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h> 


#define VERBOSE                     1
#define WHYT                        7
#define YELW                        6
#define MGNT                        5
#define REDD                        4
#define CYAN                        3
#define GREN                        2
#define BLUE                        1
#define OFFF                        0
#define CNTNU                       2
#define LOOP                        3
#define STOP                        4
#define BAD_OPCODE                 12
#define FAILED_GPIO_WRITE          13
#define FAILED_GPIO_READ           14
#define MAX_PATH                   60
#define PROGRAM_LENGTH             80
#define ZEROTXT                    48
#define ZEROTXT_                  111
#define ONETXT                     49
#define ONETXT_                    98
#define TWOTXT                     50
#define TWOTXT_                   103
#define TRETXT                     51
#define TRETXT_                    99
#define QUATXT                     52
#define QUATXT_                   114
#define QUITXT                     53
#define QUITXT_                   109
#define SESTXT                     54
#define SESTXT_                   121
#define SETTXT                     55
#define SETTXT_                   119
#define OCTTXT                     56
#define NINETXT                    57
#define DASHTXT                    45
#define BACKTXT                    60
#define STOPTXT                   120
#define zero                        0
#define one                         1
#define two                         2
#define four                        4
#define five                        5
#define six                         6
#define eight                       8
#define GPIOCHIP     "/dev/gpiochip0"
#define INFOCOLS                   3u
#define INPUTTIMEOUT              100          // poll input timeout (ms)


#include "static.c"
#include "util.c"
#include "file.c"
#include "ini.c"
#include "drank.c"
#include "prog.c"

/*
  COMPILE
  $ gcc -Wall -Wno-char-subscripts gpio.c -o gpio
  $ gcc -Wall -Wno-char-subscripts gpio.c -Wno-unused-but-set-variable -Wno-unused-variable -o gpio

  EMPOWER
  $ sudo mv gpio /sbin
  $ sudo chown root:service /sbin/gpio
  $ sudo chmod 4755 /sbin/gpio

  $ sudo cat /sys/kernel/debug/gpio


                  $ sudo setcap cap_fowner+ep /sbin/gpio
                  $ sudo setcap cap_chown+ep /sbin/gpio
                  $ sudo setcap "cap_fowner,cap_chown+ep" /sbin/gpio


  /etc/bn/gpio/
            ~ led0/
            ~ led1/
            ~ led2/
            ~ led3/
            ~ btn0
            ~ btn1
            ~ btn2
            ~ prog
            ~ config.txt
*/


// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
//  INITIALIZE                       INITIALIZE                        INITIALIZE
int is_device_string(const char* string){
  char    str0,str1,str2,str3,str4;
  int     is                  = 0;
  if ( strlen(string)==4 || strlen(string)==5 ) {
    str0                      = string[0];
    str1                      = string[1];
    str2                      = string[2];
    str3                      = string[3];
    if ( strlen(string) == 4 ) {
      if ( ( ( str0==76 && str1==69 && str2==68 ) || ( str0==108 && str1==101 && str2==100 ) ) && ( str3==48 || str3==49 || str3==50 || str3==51 ) )
        is                    = 0 + 1 + (str3-ZEROTXT);
      if ( ( ( str0==66 && str1==84 && str2==78 ) || ( str0==98  && str1==116 && str2==110 ) ) && ( str3==48 || str3==49 || str3==50 ) )
        is                    = 0 - 1 - (str3-ZEROTXT);
    } else {
      str4                    = string[4];
      if ( ( str4>ONETXT-1 && str4<NINETXT+1 ) || ( str4>64 && str4<71 ) || ( str4>96 && str4<103 ) ) {
        if ( str4>64 && str4<71 )
          str4                = str4 - 7;
        if ( str4>96 && str4<103 )
          str4                = str4 - 39;
        if ( ( str0==80 && str1==82 && str2==79 && str3==71 ) || ( str0==112 && str1==114 && str2==111 && str3==103 ) ) 
          is                  = 10 + str4 - ZEROTXT;
      }
    }
  }
  return is;
}
int handler(void* conf, const char* sect, const char* name, const char* valu){
  // bn_gpio_config* config      = (bn_gpio_config*)conf;
  bn_gpio_led* led;
  bn_gpio_btn* btn;
  char    val0                = valu[0];
  char    nam0                = name[0];
  int     is                  = is_device_string(sect),prog_num;
#if VERBOSE == 3
  printf("handler: sect == '%s', name == '%s', valu == '%s'\n",sect,name,valu);
#endif
  if ( is==0 ) {
    return 0;
  } else {
    if ( is>0 && is<10 ) {
      led                     = leds[is-1];
      if( strcmp("colors",name)==0 || strcmp("pwr",name)==0 || strcmp("red",name)==0 || strcmp("green",name)==0 || strcmp("blue",name)==0 ) {
        if( nam0==99 ) {
          if ( strlen(valu)==1 && ( val0==ZEROTXT || val0==ONETXT || val0==TWOTXT || val0==TRETXT ) )
            led->colrs        = val0 - ZEROTXT;
        } else {
          if ( nam0==112 )             led->blu->pin   = pinOf(valu);                        // pwr
          if ( nam0==114 )             led->red->pin   = pinOf(valu);                        // red
          if ( nam0==103 )             led->grn->pin   = pinOf(valu);                        // green
          if ( nam0==98  )             led->blu->pin   = pinOf(valu);                        // blue
#if VERBOSE == 3
          if ( pinOf(valu)>0 )
            printf("handler: led%d %s set to '%s'\n",(is-1),name,valu);
#endif
        }
      }
    }
    if ( is < 0 ) {
      btn                     = btns[0-1-is];
      if(strcmp("hot",name)==0)     btn->hot->pin   = pinOf(valu);
      if(strcmp("step",name)==0)
        if( strcmp("1",valu)==0 || strcmp("2",valu)==0 ) 
          overspeed           = valu[0] - ZEROTXT;
#if VERBOSE == 3
      if(strcmp("hot",name)==0)
        printf("handler: btn%d hot set to '%s'\n",(0-1-is),valu);
#endif
    }
    if ( is>10 && strlen(valu)<PROGRAM_LENGTH+1 ) {
      prog_num                = is-10;
      if ( strcmp("next",name)==0 )    update_prog(prog_num,0,valu);
      if ( strcmp("led0",name)==0 )    update_prog(prog_num,1,valu);
      if ( strcmp("led1",name)==0 )    update_prog(prog_num,2,valu);
      if ( strcmp("led2",name)==0 )    update_prog(prog_num,3,valu);
      if ( strcmp("led3",name)==0 )    update_prog(prog_num,4,valu);
    }
    return 1;
  }
}
void process_btn_status(int bits){
}
int initialize_config(int argc, char** argv){
  int     parse_result;
  char    fullpath[MAX_PATH];

  bngpiodir_ok                = verify_dir_exists(bngpiodir);

  if( bngpiodir_ok==1 ){
    assemble_path(fullpath,"","","config.txt");
    parse_result              = ini_parse(fullpath,handler,&config);
#if VERBOSE == 3
    printf("result from ini_parse == %d\n",parse_result);
#endif
    if(parse_result==0){
      count_lines();
    }
  }
  print_config(config);
  return config.curr_weight;
}
void doze_briefly(){
  struct  timespec remaining, request = { 0, 62500000 }; 
  int     slept               = nanosleep(&request, &remaining);

#if VERBOSE == 1 || VERBOSE == 2|| VERBOSE == 3
  if( slept!=0 ){
    if(errno==EINTR){
      printf("interrupted by a signal handler\n");
    }else{
      printf("bad sleep\n");
    }
  }
#endif
}
//  INITIALIZE                       INITIALIZE                        INITIALIZE
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*


int main(int argc, char **argv) {
  unsigned delayns            = 125000000;
  char    new_program         = retrieve_program(),
          next_opcode         = LOOP,
          failed_read         = 0,
          failed_write        = 0,
          failed_program      = 0;
  int     i                   = -1,
          j                   = -1,
          cycles              = 10000,
          gearing             = 12,read_mask,write_mask;
  struct  timespec ts         = { .tv_sec = 0 }, 
                   tr         = { .tv_sec = 0 };

  setlinebuf(stdout);
  initialize_config(argc,argv);
  ts.tv_nsec                  = delayns / overspeed;

  read_mask                   = config.read_mask;
  write_mask                  = config.write_mask;

  /*     */
  // *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
  //   SETUP GPIO                       SETUP GPIO                       SETUP GPIO

  chip_fd                     = gpio_dev_open (GPIOCHIP);

  struct gpio_v2_line_attribute rd_attr            = { .id = 1 };
  rd_attr.id                  = GPIO_V2_LINE_ATTR_ID_FLAGS;
  rd_attr.flags               = GPIO_V2_LINE_FLAG_INPUT;

  struct gpio_v2_line_config_attribute rd_cfg_attr = { .mask = read_mask };
  rd_cfg_attr.attr            = rd_attr;
  rd_cfg_attr.mask            = read_mask;  

  struct gpio_v2_line_config linecfg               = { .flags =  GPIO_V2_LINE_FLAG_OUTPUT };
  struct gpio_v2_line_request linereq              = { .offsets = {0} };
  struct gpio_v2_line_values linevals              = { .bits = read_mask, .mask = read_mask };

  gpio_v2_t pins              = { .linecfg = &linecfg, .linereq = &linereq, .linevals= &linevals };
  pins.fd                     = chip_fd;

  while ( config.line_count-1>j++ )
    pins.linereq->offsets[j]= config.offsets[j]->pin;

#if VERBOSE == 3
  printf("\nread_mask == %d, write_mask == %d, weight == %d, line_limit == %d\n",read_mask,write_mask,config.curr_weight,config.line_count-1);
#endif

  pins.linereq->num_lines     = j;

  pins.linevals->bits         = read_mask;
  pins.linevals->mask         = read_mask;

  pins.linecfg->num_attrs     = 1;
  pins.linecfg->attrs[0]      = rd_cfg_attr;
  /*          */

  if (prn_gpio_v2_ghip_info (pins.fd) == -1)         return 1;
  if (gpio_line_cfg_ioctl (&pins) == -1)             return 1;
  //   SETUP GPIO                       SETUP GPIO                       SETUP GPIO
  // *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*



#if VERBOSE == 3
  printf ("-------------- now writing and reading -------------------\ndelay (ns) == %u, step == %d\n\n", delayns/overspeed,overspeed);
  printf("\ncycles == %d, i == %d\n",cycles,i);
#endif

  next_opcode                 = transition_program(new_program);
  while ( failed_read==0 && failed_write==0 && cycles>i++ ) {
    j                         = -1;
    while ( failed_read==0 && failed_write==0 && gearing>j++ ) {
      if ( failed_program!=1 && ( next_opcode!=STOP || program_ix!=0 ) && ( ( j % overspeed ) == 0 ) ) {
        next_opcode           = step_program ( &pins , write_mask ) ;
        if ( next_opcode==FAILED_GPIO_WRITE )      failed_write        = 1;
        if ( next_opcode==BAD_OPCODE )             failed_program      = 1;
      }
      failed_read             = read_and_process_buttons ( &pins , read_mask ) ;
      new_program             = retrieve_program();
      // printf("main:  new_program == %d, program == %d\n",new_program,program);
      if( new_program!=program && new_program>0 ){
        failed_program        = 0;
        next_opcode           = transition_program(new_program);
      }
      nanosleep (&ts, &tr);
    }
  }

  usleep (INPUTTIMEOUT * 1000);     /* delay input timeout microsecs */

  /* close gpiochipX file descriptor */
  gpio_dev_close (chip_fd);
}
