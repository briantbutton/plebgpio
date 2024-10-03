//  ~   ~    ~     ~      ~       ~        ~         ~          ~           ~            ~             ~
// Called against the section string:
//     ~ LEDs: LED-number plus 1
//     ~ buttons: 0 - button-number - 1
//     ~ program: 10 + program-number
//     ~ 'pleb': -11
//     ~ no match: 0
int is_device_string ( const char* string ) {
  char    str0,str1,str2,str3,str4;
  int     is                  = 0;
  if ( strlen(string)==4 || strlen(string)==5 ) {
    str0                      = string[0];
    str1                      = string[1];
    str2                      = string[2];
    str3                      = string[3];
    if ( strlen(string) == 4 ) {
      // led0, led1, led2, led3
      if ( ( str0==108 && str1==101 && str2==100 ) && ( str3==48 || str3==49 || str3==50 || str3==51 ) )
        is                    = 0 + 1 + (str3-ZEROTXT);
      // btn0, btn1, btn2
      if ( ( str0==98  && str1==116 && str2==110 ) && ( str3==48 || str3==49 || str3==50 ) )
        is                    = 0 - 1 - (str3-ZEROTXT);
      // pleb
      if ( str0==112  && str1==108 && str2==101 && str3==98 )
        is                    = 0 - 11;
    } else {
      str4                    = string[4];
      // progx, where x is 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E or F
      if ( ( str4>ONETXT-1 && str4<NINETXT+1 ) || ( str4>64 && str4<71 ) || ( str4>96 && str4<103 ) ) {
        if ( str4>64 && str4<71 )
          str4                = str4 - 7;
        if ( str4>96 && str4<103 )
          str4                = str4 - 39;
        if ( str0==112 && str1==114 && str2==111 && str3==103 )
          is                  = 10 + str4 - ZEROTXT;
      }
    }
  }
  return is;
}
//  ~   ~    ~     ~      ~       ~        ~         ~          ~           ~            ~             ~
// Called for each statement line of config.txt
// Amends structures of global variable 'config', which holds configuration of buttons and LEDs
// 
int handler(void* conf, const char* sect, const char* name, const char* valu){
  // bn_gpio_config* config      = (bn_gpio_config*)conf;
  bn_gpio_led* led;
  bn_gpio_btn* btn;
  char    nam0                = name[0];
  int     is                  = is_device_string(sect),prog_num;
#if VERBOSE == 3
  printf("handler: sect == '%s', name == '%s', valu == '%s'\n",sect,name,valu);
#endif
  if ( unmatched_sect(is) ) {
    return 0;
  } else {
    // printf("handler: matched; sect == '%s' (%d), name == '%s', valu == '%s'\n",sect,is,name,valu);
    // LED                                      LED                                      LED
    if ( led_sect(is) ) {
      led                     = leds[is-1];
      if( strcmp("colors",name)==0 || strcmp("pwr",name)==0 || strcmp("red",name)==0 || strcmp("green",name)==0 || strcmp("blue",name)==0 ) {
        if( nam0==99 ) {
          if ( strlen(valu)==1 && is_octal_value(valu,3)==1 )
            led->colrs        = valu[0] - ZEROTXT;
        } else {
          if ( nam0==112 )                   led->blu->pin    = pinOf(valu);                      // pwr
          if ( nam0==114 )                   led->red->pin    = pinOf(valu);                      // red
          if ( nam0==103 )                   led->grn->pin    = pinOf(valu);                      // green
          if ( nam0==98  )                   led->blu->pin    = pinOf(valu);                      // blue
#if VERBOSE == 3
          if ( pinOf(valu)>0 )               printf("handler: led%d %s set to '%s'\n",(is-1),name,valu);
#endif
        }
      }
    }

    // BTN                                      BTN                                      BTN
    if ( btn_sect(is) ) {
      btn                     = btns[0-1-is];
      if(strcmp("hot",name)==0)              btn->hot->pin    = pinOf(valu);
#if VERBOSE == 3
      if(strcmp("hot",name)==0)              printf("handler: btn%d hot set to '%s'\n",(0-1-is),valu);
#endif
    }

    // PROG                                     PROG                                     PROG
    if ( prog_sect(is) && strlen(valu)<PROGRAM_LENGTH+1 ) {
      prog_num                = is-10;
      if ( strcmp("next",name)==0 )          update_prog(prog_num,0,valu);
      if ( strcmp("led0",name)==0 )          update_prog(prog_num,1,valu);
      if ( strcmp("led1",name)==0 )          update_prog(prog_num,2,valu);
      if ( strcmp("led2",name)==0 )          update_prog(prog_num,3,valu);
      if ( strcmp("led3",name)==0 )          update_prog(prog_num,4,valu);
    }

    // PLEB                                     PLEB                                     PLEB
    if ( pleb_sect(is) ) {
      if ( strcmp("step",name)==0 && is_octal_value(valu,1)==1 ) {
        overspeed             = valu[0] - ZEROTXT;
#if VERBOSE == 3
        printf("handler: step set to %s\n",valu);
#endif
      }
      if ( strcmp("prog",name)==0 && is_octal_value(valu,9)==1 ) {
        program               = valu[0] - ZEROTXT;
#if VERBOSE == 3
        printf("handler: program set to %s\n",valu);
#endif
      }

    }
    return 1;
  }
}
//  ~   ~    ~     ~      ~       ~        ~         ~          ~           ~            ~             ~
// Reads config file and sets runtime data structures accordingly
// 
int initialize_config(int argc, char** argv){
  int     parse_result        = -1;;
  char    fullpath[MAX_PATH];
  char    string[4]           = {  32 ,  32 ,   0 };

  setlinebuf(stdout);

  bngpiodir_ok                = verify_dir_exists(bngpiodir);

  if( bngpiodir_ok ){

    assemble_path ( fullpath , "" , "" , "config.txt" ) ;
    parse_result              = ini_parse ( fullpath ,handler , &config ) ;

#if VERBOSE == 3
    printf("result from ini_parse == %d\n",parse_result);
#endif
#if VERBOSE == 2 || VERBOSE == 3
    printf("initial program == %d\n",program);
#endif

    string[0]                 = program + ZEROTXT;
    assemble_path ( fullpath , "" , "" , "prog" ) ;
    file_write ( fullpath , string , 1 );
  }
  if ( parse_result==0 ) {
    count_lines();
    print_config(config);
    return program;
  } else {
    if ( bngpiodir_ok ) {
      problem                 = CONFIG_PARSE_ERROR;
    } else {
      problem                 = MISSING_DIR;
    }
    return 0;
  }
}
