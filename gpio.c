#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CMD_EMPTY               0
#define CMD_INITIALIZE          1
#define CMD_ACTIVATE            2
#define CMD_DEACTIVATE          3
#define CMD_WRITE               4
#define CMD_READ                5
#define CMD_READALL             6
#define CMD_ERROR_SYNTAX        7
#define CMD_NOT_INITIALIZED     8
#define OBJ_INVALID             0
#define OBJ_LED                 1
#define OBJ_BTN                 2
#define VAL_NULL               17
#define VAL_ON                  8
#define VAL_WHITE               7
#define VAL_YELLOW              6
#define VAL_MAGENTA             5
#define VAL_RED                 4
#define VAL_CYAN                3
#define VAL_GREEN               2
#define VAL_BLUE                1
#define VAL_OFF                 0
#define THIRTY                 30
#define FORTY                  40
#define ZERO                    0
#define ONE                     1
#define FOUR                    4
#define FIVE                    5
#define SIX                     6
#define COMMANDCOUNT            6
#define VALUECOUNT              9
#define OK                      0
#define NOTHINGTODO             3

static char initialized       = 0;
static char objNum            = 0;
static char verbose           = 0;
static int  argc_             = 1;
                              // 
                              // Unactivated state == 400000000000
                              //    RGB LEDs [0,1,2,3] |~~~~~~|^^ 
                              //                               ||
                              //                            Buttons [0,1,2]
                              // 
                              //      ~  ~  ~   LED-0   ~  ~  ~       ~  ~  ~   LED-1   ~  ~  ~       ~  ~  ~   LED-2   ~  ~  ~       ~  ~  ~   LED-3   ~  ~  ~     BTN-0   BTN-1   BTN-2
                              //      R       G       B       W       R       G       B       W       R       G       B       W       R       G       B       W
                              // 
static char pins[FORTY]       = {     17 ,    27 ,    22 ,    17 ,    15 ,    23 ,    24 ,    15 ,    10 ,     9 ,    11 ,    10 ,     0 ,     0 ,     0 ,    14 ,     0 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 };
static char bits[FORTY]       = {      0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 };
static char available[FORTY]  = {      1 ,     1 ,     1 ,     0 ,     1 ,     1 ,     1 ,     0 ,     1 ,     1 ,     1 ,     0 ,     0 ,     0 ,     0 ,     1 ,     1 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 };
static char activated[FORTY]  = {      0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 };
// static char count[FORTY]      = {      0 ,     1 ,     2 ,     3 ,     4 ,     5 ,     6 ,     7 ,     8 ,     9 ,    10 ,    11 ,    12 ,    13 ,    14 ,    15 ,    16 ,    17 ,    18 ,    19 ,    20 ,    21 ,    22 ,    23 ,    24 ,    25 ,    26 ,    27 ,    28 ,    29 ,    30 ,    31 ,    32 ,    33 ,    34 ,    35 ,    36 ,    37 ,    38 ,    39 };
static char state[13]         = {     52 ,    48 ,    48 ,    48 ,    48 ,    48 ,    48 ,    48 ,    48 ,    48 ,    48 ,    48 ,     0 };
static int  indices[6]        = {      0 ,     1 ,     2 ,     3 ,     4 ,     5 };
static char nouns[3][4]       = {
                                    {      0 ,     0 ,     0 ,     0 },
                                    {    108 ,   101 ,   100 ,     0 },
                                    {     98 ,   116 ,   110 ,     0 }
                                };
static char commands[6][12]   = {
                                    { CMD_INITIALIZE     ,    10 ,   105 ,   110 ,   105 ,   116 ,   105 ,    97 ,   108 ,   105 ,   122 ,   101 } , 
                                    { CMD_ACTIVATE       ,     8 ,    97 ,    99 ,   116 ,   105 ,   118 ,    97 ,   116 ,   101 ,     0 ,     0 } , 
                                    { CMD_DEACTIVATE     ,    10 ,   100 ,   101 ,    97 ,    99 ,   116 ,   105 ,   118 ,    97 ,   116 ,   101 } , 
                                    { CMD_WRITE          ,     5 ,   119 ,   114 ,   105 ,   116 ,   101 ,     0 ,     0 ,     0 ,     0 ,     0 } , 
                                    { CMD_READALL        ,     7 ,   114 ,   101 ,    97 ,   100 ,    97 ,   108 ,   108 ,     0 ,     0 ,     0 } , 
                                    { CMD_READ           ,     4 ,   114 ,   101 ,    97 ,   100 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } 
                                };
static char values[9][12]     = {
                                    { VAL_ON             ,     2 ,   111 ,   110 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_OFF            ,     3 ,   111 ,   102 ,   102 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_WHITE          ,     5 ,   119 ,   104 ,   105 ,   116 ,   101 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_BLUE           ,     4 ,    98 ,   108 ,   117 ,   101 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_RED            ,     3 ,   114 ,   101 ,   100 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_GREEN          ,     5 ,   103 ,   114 ,   101 ,   101 ,   110 ,     0 ,     0 ,     0 ,     0 ,     0 } , 
                                    { VAL_YELLOW         ,     6 ,   121 ,   101 ,   108 ,   108 ,   111 ,   119 ,     0 ,     0 ,     0 ,     0 } , 
                                    { VAL_CYAN           ,     4 ,    99 ,   121 ,    97 ,   110 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } , 
                                    { VAL_MAGENTA        ,     7 ,   109 ,    97 ,   103 ,   101 ,   110 ,   116 ,    97 ,     0 ,     0 ,     0 } 
                                };
static char test1[5]          = {   VAL_WHITE ,      VAL_ON ,     VAL_RED ,  VAL_YELLOW , VAL_MAGENTA };
static char test2[5]          = {   VAL_WHITE ,   VAL_GREEN ,   VAL_GREEN ,  VAL_YELLOW ,    VAL_CYAN };
static char test3[5]          = {   VAL_WHITE ,    VAL_BLUE ,    VAL_BLUE , VAL_MAGENTA ,    VAL_CYAN };
/*
  COMPILE
  $ gcc -Wall gpio.c -o gpio

  EMPOWER
  $ sudo mv gpio /sbin
  $ sudo setcap cap_fowner+ep /sbin/gpio
  $ sudo setcap cap_chown+ep /sbin/gpio
  $ sudo setcap "cap_fowner,cap_chown+ep" /sbin/gpio

  INVOKE
  Initialize
  $ export GPIO="";export GPIO=$(sudo /home/service/gpio/gpio initialize);echo $GPIO
  Activate
  $ export GPIO=$(sudo /home/service/gpio/gpio activate led0 $GPIO);echo $GPIO
  Set Value
  $ export GPIO=$(sudo /home/service/gpio/gpio write led0 $GPIO red);echo $GPIO
  Read Value
  $ export GPIO=$(sudo /home/service/gpio/gpio read btn0 $GPIO);echo $GPIO
  Read Values
  $ export GPIO=$(sudo /home/service/gpio/gpio readall $GPIO);echo $GPIO
*/
int command (char *string, char *result, char max)
{
  FILE *fp;
  char buf[128];
  int  i                      = 0;
  int  l                      = 10;
  while(i<l)
    result[i++]               = 0;
  result[0]                   = 97;
  result[1]                   = 98;
  result[2]                   = 99;
  fp                          = popen (string, "r");
  if (fp == NULL){
    return 1;
  }else{
    if (fgets (buf, sizeof(buf) - 1, fp) == NULL){
      // Might be empty result, that is ok
      pclose (fp);
      return 1;
    }else{
      pclose (fp);
      result[0]               = 0;
      result[1]               = 0;
      result[2]               = 0;
      i                       = 0;
      while(i<l){
        if(sizeof(buf)>i&&max>i)
          result[i]           = buf[i];
        i                     = i+1;
      }
      return 0;
    }
  }
}
void err_print(char* string){
  if(verbose==1)  fprintf(stderr,"%s\n",string);
}
void command_print (char *buf){
  char    result[10];
  err_print(buf);
  command(buf,result,ZERO);
}
char led_offset(char ledNum){
  return 0+ledNum*4;
}
char btn_offset(char btnNum){
  return 16+btnNum*1;
}
char octal_value(char* array, int offset){
  char    octal               = 0;
  if ( array[offset+0] == 1 || array[offset+3] == 1 )
    octal                     = octal+4;
  if ( array[offset+1] == 1 )
    octal                     = octal+2;
  if ( array[offset+2] == 1 )
    octal                     = octal+1;
  return octal+48;
}
char match_string(char* a, char* b, char bl){
  int     i                   = 0;
  char    matches             = 0;
  if(strlen(a)==bl){
    matches                   = 1;
    while(i<bl&&matches==1){
      if(a[i]!=b[i])
        matches               = 0;
      i                       = i+1;
    }
  }
  return matches;
}
char select_from_list(char* c, char (*list)[12], char defalt, int count){
  char    res                 = defalt;
  int     i                   = 0;
  while ( i < count && res ==  defalt ) {
    if ( ( match_string ( c , &list[i][2] , list[i][1] ) == 1 ) ) 
      res                     = list[i][0];
    i                         = i+1;
  }
  return res;
}
// Returns 1 for led and 2 for btn; 0 for invalid
char valid_obj_and_state(char cmd, char *arg2, char * arg3){
  char    valid               = OBJ_INVALID;
  char    isBtn               = 0;
  char    isLed               = 0;
  char    arg20,arg21,arg22,arg23,arg30;
  if(cmd==CMD_READALL){
    valid                     = OBJ_BTN;
  }else{
    arg20                     = arg2[0];
    arg21                     = arg2[1];
    arg22                     = arg2[2];
    arg23                     = arg2[3];
    arg30                     = arg3[0];
    if(((arg23==48||arg23==49||arg23==50||arg23==51))&&((arg30==52||arg30==53||arg30==54||arg30==55))){
      if((arg20==76&&arg21==69&&arg22==68)||(arg20==108&&arg21==101&&arg22==100)){
        isLed                 = 1;
        objNum                = arg23-48;
      }
      if(((arg20==66&&arg21==84&&arg22==78)||(arg20==98&&arg21==116&&arg22==110))&&arg23!=51){
        isBtn                 = 1;
        objNum                = arg23-48;
      }
      if(isLed==1&&cmd!=CMD_READ)
        valid                 = OBJ_LED;
      if(isBtn==1&&cmd!=CMD_WRITE)
        valid                 = OBJ_BTN;
    }
  }
  return valid;
}
void initialize_system(){
  command_print("chown root:www-data /sys/class/gpio/export");
  command_print("chmod 222 /sys/class/gpio/export");
  command_print("chown root:www-data /sys/class/gpio/unexport");
  command_print("chmod 222 /sys/class/gpio/unexport");
  err_print("\n");
  initialized                 = 1;
}
void set_obj_state(char* array, char c, char o){
  if(c==52||c==53||c==54||c==55){
    if(available[o+0]==1){
      array[o+0]          = 1;
    }else{
      array[o+3]          = 1;      
    }
  }
  if(c==50||c==51||c==54||c==55)
    array[o+1]            = 1;
  if(c==49||c==51||c==53||c==55)
    array[o+2]            = 1;
}
void set_led_activ(char c, char o){
  set_obj_state(activated,c,o);
}
void set_led_value(char c, char o){
  set_obj_state(bits,c,o);
}
void set_btn_activ(char c, char o){
  set_obj_state(activated,c,o);
}
void set_btn_value(char c, char o){
  set_obj_state(bits,c,o);
}
char parse_state(char *arg3){
  char    isValid             = 1;
  int     i                   = -1,
          l                   = 4;
  while(l>i++){
    set_led_activ(arg3[i+i+1],i*4);
    set_led_value(arg3[i+i+2],i*4);
  }
  return isValid;
}


// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
//  ACTIVATE                           ACTIVATE                          ACTIVATE
void deactivate_obj(char count, char offset){
  char    i                   = 0,pin;
  int     index;
  char    buf[64];
  while ( i < count ) {
    index                     = i+offset;
    if ( available[index] == 1 ) 
      if ( activated[index] == 1 ) {
        pin                   = pins[index];
        sprintf(buf,"echo \"%d\" > /sys/class/gpio/unexport",pin);
        command_print(buf);
        err_print("\n");
        activated[index]      = 0;
      }
    i                         = i+1;
  }
}
void activate_obj(char count, char offset, char *dir){
  char    i                   = 0,pin;
  int     index;
  char    buf[64];
  while ( i < count ) {
    index                     = i+offset;
    if ( available[index] == 1 ) 
      if ( activated[index] == 0 ) {
        pin                   = pins[index];
        sprintf(buf,"echo \"%d\" > /sys/class/gpio/export",pin);
        command_print(buf);
        sprintf(buf,"chown root:www-data /sys/class/gpio/gpio%d/value",pin);
        command_print(buf);
        sprintf(buf,"chmod 666 /sys/class/gpio/gpio%d/value",pin);
        command_print(buf);
        sprintf(buf,"echo \"%s\" > /sys/class/gpio/gpio%d/direction",dir,pin);
        command_print(buf);
        err_print("\n");
        activated[index]      = 1;
      }
    i                         = i+1;
  }
}
void activate_led(char ledNum){
  char    dir[4]              = {   111 ,  117 ,  116 ,    0 } ; 
  activate_obj(FOUR,led_offset(ledNum),dir);
}
void deactivate_led(char ledNum){
  deactivate_obj(FOUR,led_offset(ledNum));
}
void activate_btn(char btnNum){
  char    dir[4]              = {   105 ,  110 ,    0 ,    0 } ; 
  activate_obj(ONE,btn_offset(btnNum),dir);
}
void deactivate_btn(char btnNum){
  deactivate_obj(ONE,btn_offset(btnNum));
}
void activate(int obj, char num){
  char    buf[64];
  if(obj==OBJ_LED||obj==OBJ_BTN){
    if(verbose==1){
      sprintf(buf,"activate, %s == %d",nouns[obj],num);
      err_print(buf);
    }
    if(obj==OBJ_LED){
      activate_led(num);
    }else{
      activate_btn(num);
    }
  }
}
void deactivate(int obj, char num){
  char    buf[64];
  if(obj==OBJ_LED||obj==OBJ_BTN){
    if(verbose==1){
      sprintf(buf,"deactivate, %s == %d",nouns[obj],num);
      err_print(buf);
    }
    if(obj==OBJ_LED){
      deactivate_led(num);
    }else{
      deactivate_btn(num);
    }
  }
}
//  ACTIVATE                           ACTIVATE                          ACTIVATE
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*

// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
//  LED                                  LED                                  LED
void set_led_color(int index, char val, char* choices){
  char    pin                 = pins[index];
  char    n                   = 0;
  char    buf[64];
  if ( available[index] == 1 ) 
    if ( activated[index] == 1 ) {
      if ( val == choices[0] ||  val == choices[1] || val == choices[2] || val == choices[3] || val == choices[4] )
        n                     = 1;
      if(bits[index]!=n){
        sprintf(buf,"echo \"%d\" > /sys/class/gpio/gpio%d/value",n,pin);
        command_print(buf);
        err_print("\n");
        bits[index]           = n;
      }
    }
}
void set_led(char ledNum, char val){
  char    ledOffset           = led_offset(ledNum);
  set_led_color(ledOffset+0,val,test1);
  set_led_color(ledOffset+3,val,test1);
  set_led_color(ledOffset+1,val,test2);
  set_led_color(ledOffset+2,val,test3);
}
void write_led(char val, char* arg4){
  char    buf[64];
  sprintf(buf,"led == %d, val == '%s'",objNum,arg4);
  err_print(buf);
  set_led(objNum,val);
}
//  LED                                  LED                                  LED
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*


// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
//  BTN                                  BTN                                  BTN
void get_btn_state(int index){
  char    pin                 = pins[index];
  char    buf[64];
  char    result[10];
  if ( available[index] == 1 ) 
    if ( activated[index] == 1 ) {
      sprintf(buf,"cat /sys/class/gpio/gpio%d/value",pin);
      command(buf,result,ONE);
      err_print(buf);
      if(result[0]==48||result[0]==49){
        bits[index]           = result[0]-48;
      }
    }
}
void get_btn(char btnNum){
  get_btn_state(btn_offset(btnNum));
}
void read_btn(){
  int     offset              = btn_offset(objNum);
  char    buf[64];
  get_btn(objNum);
  sprintf(buf,"btn == %d, val == '%c'",objNum,bits[offset]+48);
  err_print(buf);
}
void read_btns(){
  objNum                      = 0;
  read_btn();
  objNum                      = 1;
  read_btn();
}
//  BTN                                  BTN                                  BTN
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*


void pre_parse_arguments(int argc, char** argv){
  int     i                   = -1,
          l                   = argc-1;
  char*   ai;
  argc_                       = argc;
  while(l>i++){
    if(strlen(argv[i])==7){
      ai                      = argv[i];
      if(ai[0]==118&&ai[1]==101&&ai[2]==114&&ai[3]==98&&ai[4]==111&&ai[5]==115&&ai[6]==101){
        verbose               = 1;
        argc_                 = argc-1;
        if(i<5)  indices[4]   = 5;
        if(i<4)  indices[3]   = 4;
        if(i<3)  indices[2]   = 3;
        if(i<2)  indices[1]   = 2;
      }
    }
  }
}
char parse_arguments(int argc, char** argv){
  char    val                 = VAL_NULL,obj;
  char    cmd                 = CMD_EMPTY;
  char    hasPinAndState      = 0,arg3B;
  char*   arg2;
  char*   arg3;
  char*   arg4;

  pre_parse_arguments(argc,argv);

  if(argc_>1)
    cmd                       = select_from_list(argv[indices[1]],commands,CMD_EMPTY,COMMANDCOUNT);

  if(cmd==CMD_ACTIVATE||cmd==CMD_DEACTIVATE||cmd==CMD_WRITE||cmd==CMD_READ||cmd==CMD_READALL){
    if(cmd==CMD_READALL){
      indices[3]              = indices[2];
    }
    if(argc_>3||((cmd==CMD_READALL)&&argc_>2)){
      arg2                    = argv[indices[2]];
      arg3                    = argv[indices[3]];
      if(((cmd==CMD_READALL)||strlen(arg2)==4)&&strlen(arg3)==12){
        arg3B                 = arg3[11];
        if(arg3B==49||arg3B==51||arg3B==53||arg3B==55){
          initialized         = 1;
          obj                 = valid_obj_and_state(cmd,arg2,arg3);
          if(obj==OBJ_LED||obj==OBJ_BTN)
            hasPinAndState    = parse_state(arg3);
        }else{
          cmd                 = CMD_NOT_INITIALIZED;
        }
      }
    }
  }
  if(cmd==CMD_WRITE&&hasPinAndState==1&&argc_>4){
    arg4                      = argv[indices[4]];
    val                       = select_from_list(arg4,values,VAL_NULL,VALUECOUNT);
  }

  if((hasPinAndState==0&&cmd!=CMD_NOT_INITIALIZED&&cmd!=CMD_INITIALIZE)||(cmd==CMD_WRITE&&val==VAL_NULL))
    cmd                       = CMD_ERROR_SYNTAX;

  if(cmd==CMD_INITIALIZE)       initialize_system();
  if(cmd==CMD_ACTIVATE)         activate(obj,objNum);
  if(cmd==CMD_DEACTIVATE)       deactivate(obj,objNum);
  if(cmd==CMD_WRITE)            write_led(val,arg4);
  if(cmd==CMD_READ)             read_btn();
  if(cmd==CMD_READALL)          read_btns();

  return cmd;
}

int main(int argc, char **argv) {
  char    cmd                 = parse_arguments(argc,argv);
  int     i                   = -1,
          l                   = 4;
  if(cmd==CMD_NOT_INITIALIZED)
    fprintf(stderr,"NOT INITIALIZED \n\n\n");
  if(cmd==CMD_ERROR_SYNTAX)
    fprintf(stderr,"SYNTAX ERROR \n\n\n");

  if(initialized==1)
    state[11]                 = 49;

  while(l>i++){
    state[i+i+1]              = octal_value(activated,i*4);
    state[i+i+2]              = octal_value(bits,i*4);
  }
  printf("%s",state);
}
