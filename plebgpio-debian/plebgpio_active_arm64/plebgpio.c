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
#define NO_PROBLEM                  0
#define MISSING_DIR                91
#define CONFIG_PARSE_ERROR         92
#define CHIP_OPEN_ERROR            93
#define BAD_OPCODE                 94
#define FAILED_GPIO_WRITE          95
#define FAILED_GPIO_READ           96
#define NO_LINES_CONFIGURED        97
#define MAX_PATH                   60
#define PROGRAM_LENGTH             96
#define LAST_PROGRAM               61
#define ZEROTXT                    48                  // '0'
#define ZERTXT                     48                  // '0'
#define ZERTXT_                   111                  // 'o' - off
#define ONETXT                     49                  // '1'
#define ONETXT_                    98                  // 'b' - blue
#define ONETXT__                  111                  // 'n' - on
#define TWOTXT                     50                  // '2'
#define TWOTXT_                   103                  // 'g' - green
#define TRETXT                     51                  // '3'
#define TRETXT_                    99                  // 'c' - cyan
#define QUATXT                     52                  // '4'
#define QUATXT_                   114                  // 'r' - red
#define QUITXT                     53                  // '5'
#define QUITXT_                   109                  // 'm' - magenta
#define SESTXT                     54                  // '6'
#define SESTXT_                   121                  // 'y' - yellow
#define SETTXT                     55                  // '7'
#define SETTXT_                   119                  // 'w' - white
#define OCTTXT                     56                  // '8'
#define NINETXT                    57                  // '9'
#define DASHTXT                    45                  // '-'
#define BACKTXT                    60                  // '<'
#define STOPTXT                   120                  // 'x'
#define zero                        0
#define one                         1
#define two                         2
#define four                        4
#define five                        5
#define six                         6
#define eight                       8
#define GPIOCHIP     "/dev/gpiochip0"
#define INFOCOLS                   3u
#define INPUTTIMEOUT              100                  // poll input timeout (ms)
#define CHAR_ERROR                126                  // '~'
#define VAL_ERROR                  73                  // Signals base62, base36 (or base16) conversion failed


#include "static.c"
#include "util.c"
#include "file.c"
#include "ini.c"
#include "drank.c"
#include "prog.c"
#include "parse.c"

/*
  COMPILE
  $ gcc -Wall -Wno-char-subscripts gpio.c -o gpio
  $ gcc -Wall -Wno-char-subscripts gpio.c -Wno-unused-but-set-variable -Wno-unused-variable -o gpio


  $ sudo cat /sys/kernel/debug/gpio

  /etc/pleb/gpio/
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


int main(int argc, char **argv) {
  char    delayms             = 125,
          new_program         = initialize_config(argc,argv),
          next_opcode         = LOOP,
          failed              = 0,
          bad_program         = 0,
          off                 = 0;
  int     i                   = -1,
          j                   = -1,
          cycles              = 1000000,
          gearing             = 12,read_mask,testlimit;
  struct  timespec ts         = { .tv_sec = 0 }, 
                   tr         = { .tv_sec = 0 };
  unsigned delayns            = delayms*1000000;

  if ( config.line_count==0 ) return NO_LINES_CONFIGURED;

  ts.tv_nsec                  = delayns / overspeed;
  testlimit                   = test * 60 * 1000 * overspeed / ( delayms * gearing );

  read_mask                   = config.read_mask;

  // *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
  //   SETUP GPIO                       SETUP GPIO                       SETUP GPIO
  if ( !problem )
    if ( ( chip_fd = gpio_dev_open (GPIOCHIP) ) < 0)
      problem                 = CHIP_OPEN_ERROR;

  if ( !problem ) {

    struct gpio_v2_line_attribute rd_attr            = { .id = 1 };
    rd_attr.id                = GPIO_V2_LINE_ATTR_ID_FLAGS;
    rd_attr.flags             = GPIO_V2_LINE_FLAG_INPUT;

    struct gpio_v2_line_config_attribute rd_cfg_attr = { .mask = read_mask };
    rd_cfg_attr.attr          = rd_attr;
    rd_cfg_attr.mask          = read_mask;

    struct gpio_v2_line_config linecfg               = { .flags =  GPIO_V2_LINE_FLAG_OUTPUT };
    struct gpio_v2_line_request linereq              = { .offsets = {0} };
    struct gpio_v2_line_values linevals              = { .bits = read_mask, .mask = read_mask };

    gpio_v2_t pins            = { .linecfg = &linecfg, .linereq = &linereq, .linevals= &linevals };
    pins.fd                   = chip_fd;

    while ( config.line_count-1>j++ )                  pins.linereq->offsets[j]= config.offsets[j]->pin;

#if VERBOSE == 3
    printf("\nread_mask == %d, weight == %d, line_limit == %d\n",read_mask,config.curr_weight,config.line_count-1);
#endif

    pins.linereq->num_lines   = j;

    pins.linevals->bits       = read_mask;
    pins.linevals->mask       = read_mask;

    pins.linecfg->num_attrs   = 1;
    pins.linecfg->attrs[0]    = rd_cfg_attr;

    if (prn_gpio_v2_ghip_info (pins.fd) == -1)         return 1;
    if (gpio_line_cfg_ioctl (&pins) == -1)             return 1;
  //   SETUP GPIO                       SETUP GPIO                       SETUP GPIO
  // *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*


#if VERBOSE == 3
    printf ("-------------- now writing and reading -------------------\ndelay (ns) == %u, step == %d\n\n", delayns/overspeed,overspeed);
    printf("\ncycles == %d, i == %d\n",cycles,i);
#endif


    // *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
    //  LOOPA LOOP                     LOOPA LOOP                      LOOPA LOOP
    gearing                   = gearing-1;
    next_opcode               = transition_program(new_program);
    while ( !failed && cycles>i++ ) {
      j                       = -1;
      while ( !failed && gearing>j++ ) {
        if ( !bad_program  && ( j%overspeed)==0 ) {
          if ( program==0 || off==1 ) {
            set_leds ( &pins , off ) ;
            off               = 0;
          } else {
            if ( next_opcode!=STOP || program_ix!=0 ) {
              next_opcode     = step_program ( &pins ) ;
              if ( problem!=NO_PROBLEM )               failed              = 1;
              if ( next_opcode==BAD_OPCODE )           bad_program         = 1;
            }
          }
        }
        if ( !failed ) {
          failed              = read_and_process_buttons ( &pins ) ;
          new_program         = retrieve_program();
          if ( i < testlimit )  off = retrieve_off();
          if( new_program!=VAL_ERROR && new_program!=program ){
            bad_program       = 0;
            next_opcode       = transition_program(new_program);
          }
          nanosleep (&ts, &tr);
        }
      }
    }
    //  LOOPA LOOP                     LOOPA LOOP                      LOOPA LOOP
    // *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*

    usleep (INPUTTIMEOUT * 1000);               // delay input timeout microsecs 
    gpio_dev_close (chip_fd);
  }
  return problem;
}
