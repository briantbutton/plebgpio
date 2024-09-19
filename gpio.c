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


#define CMD_EMPTY                   0
#define VAL_NULL                   17
#define VAL_ON                      8
#define VAL_WHITE                   7
#define VAL_YELLOW                  6
#define VAL_MAGENTA                 5
#define VAL_RED                     4
#define VAL_CYAN                    3
#define VAL_GREEN                   2
#define VAL_BLUE                    1
#define VAL_OFF                     0
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
#define ZEROTXT                    48
#define ONETXT                     49
#define forty                      40
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
#define TWOO                        1


#include "static.c"
#include "util.c"
#include "file.c"
#include "ini.c"
#include "drank.c"
#include "prog.c"

/*
  COMPILE
  $ gcc -Wall -Wno-char-subscripts gpio.c -o gpio

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
                   ~ actv
            ~ led1/
            ~ led2/
            ~ led3/
            ~ btn0/
            ~ btn1/
            ~ btn2/
            ~ pattrn
            ~ prog

*/


// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
//  INITIALIZE                       INITIALIZE                        INITIALIZE
int is_device_string(const char* string){
  char    str0,str1,str2,str3;
  int     is                  = 0;
  if ( strlen(string) == 4 ) {
    str0                      = string[0];
    str1                      = string[1];
    str2                      = string[2];
    str3                      = string[3];
    if(str3==48||str3==49||str3==50||str3==51){
      if ( ( str0==76 && str1==69 && str2==68 ) || ( str0==108 && str1==101 && str2==100 ) )
        is                    = 0 + 1 + (str3-ZEROTXT);
      if ( ( ( str0==66 && str1==84 && str2==78 ) || ( str0==98 && str1==116 && str2==110 ) ) && str3!=51 )
        is                    = 0 - 1 - (str3-ZEROTXT);
    }
  }
  return is;
}
int handler(void* conf, const char* sect, const char* name, const char* valu){
  bn_gpio_config* config      = (bn_gpio_config*)conf;
  bn_gpio_led* led;
  bn_gpio_btn* btn;
  int     is                  = is_device_string(sect),num;
  printf("handler: sect == '%s', name == '%s', valu == '%s'\n",sect,name,valu);
  if ( is!=0 ) {
    if ( is>0 ) {
      if ( is==4 )                  led  = config->led3;
      if ( is==3 )                  led  = config->led2;
      if ( is==2 )                  led  = config->led1;
      if ( is==1 )                  led  = config->led0;
      if(strcmp("colors",name)==0) {
        if(strcmp("3",valu)==0)     led->colrs = 3;
        if(strcmp("2",valu)==0)     led->colrs = 2;
        if(strcmp("1",valu)==0)     led->colrs = 1;
        if(strcmp("0",valu)==0)     led->colrs = 0;
      }
      if(strcmp("pwr",name)==0)     led->blu->pin   = pinOf(valu);
      if(strcmp("red",name)==0)     led->red->pin   = pinOf(valu);
      if(strcmp("green",name)==0)   led->grn->pin   = pinOf(valu);
      if(strcmp("blue",name)==0)    led->blu->pin   = pinOf(valu);
    } else {
      if ( is==-3 )                 btn  = config->btn2;
      if ( is==-2 )                 btn  = config->btn1;
      if ( is==-1 )                 btn  = config->btn0;
      if(strcmp("hot",name)==0)     btn->hot->pin   = pinOf(valu);
      if(strcmp("step",name)==0)
        if( strcmp("1",valu)==0 || strcmp("2",valu)==0 ) 
          overspeed           = valu[0] - ZEROTXT;
      if(strcmp("hot",name)==0) {
        if(is==-1)  printf("handler: btn0 hot set to '%s'\n",valu);
        if(is==-2)  printf("handler: btn1 hot set to '%s'\n",valu);
        if(is==-3)  printf("handler: btn2 hot set to '%s'\n",valu);
      }
    }
    return 1;
  }else{
    return 0;
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
    printf("result from ini_parse == %d\n",parse_result);
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

  if( slept!=0 ){
    if(errno==EINTR){
      printf("interrupted by a signal handler\n");
    }else{
      printf("bad sleep\n");
    }
  }
}
//  INITIALIZE                       INITIALIZE                        INITIALIZE
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*


int main(int argc, char **argv) {
  unsigned delayns            = 125000000;
  char    new_program         = retrieve_program(),
          prog                = program*5,
          next_opcode         = LOOP,
          failed_read         = 0,
          failed_write        = 0,
          failed_program      = 0;
  int     i                   = -1,
          j                   = -1,
          cycles              = 1000,
          gearing             = 12,read_mask,write_mask,read_values,line_limit;
  struct  timespec ts         = { .tv_sec = 0 }, 
                   tr         = { .tv_sec = 0 };

  initialize_config(argc,argv);
  ts.tv_nsec                  = delayns / overspeed;

  read_mask                   = config.read_mask;
  write_mask                  = config.write_mask;
  line_limit                  = config.line_count-1;

  /*     */
  // *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
  //   SETUP GPIO                       SETUP GPIO                       SETUP GPIO

  // if ( TWOO==1 ) {
    chip_fd                   = gpio_dev_open (GPIOCHIP);

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

    while ( config.line_count-1>j++ )
      pins.linereq->offsets[j]= config.offsets[j]->pin;


    // This is for the button pin #17  --  hack
    // pins.linereq->offsets[j]  = 17;
    // j                         = j+1;
    // This is for the button pin #17  --  hack

    printf("\nread_mask == %d, write_mask == %d, weight == %d, line_limit == %d\n",read_mask,write_mask,config.curr_weight,line_limit);

    pins.linereq->num_lines   = j;

    pins.linevals->bits       = read_mask;
    pins.linevals->mask       = read_mask;

    pins.linecfg->num_attrs   = 1;
    pins.linecfg->attrs[0]    = rd_cfg_attr;
  // }
  /*          */

  if (prn_gpio_v2_ghip_info (pins.fd) == -1)         return 1;
  if (gpio_line_cfg_ioctl (&pins) == -1)             return 1;
  //   SETUP GPIO                       SETUP GPIO                       SETUP GPIO
  // *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*



  printf ("-------------- now writing and reading -------------------\ndelay (ns) == %u, step == %d\n\n", delayns/overspeed,overspeed);
  printf("\ncycles == %d, i == %d\n",cycles,i);

  while ( failed_read==0 && failed_write==0 && cycles>i++ ) {
    j                         = -1;
    while ( failed_read==0 && failed_write==0 && gearing>j++ ) {
      if ( failed_program!=1 && ( next_opcode!=STOP || program_ix!=0 ) && ( ( j % overspeed ) == 0 ) ) {
        next_opcode           = step_program ( &pins , write_mask ) ;
        if ( next_opcode==FAILED_GPIO_WRITE )      failed_write        = 1;
        if ( next_opcode==BAD_OPCODE )             failed_program      = 1;
      }
      read_values             = gpio_line_get_values ( &pins , read_mask , read_mask ) ;
      if ( read_values<0 ) {
        failed_read           = 1;
      } else {
        process_btn_status(read_values);
      }
      new_program             = retrieve_program();
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
