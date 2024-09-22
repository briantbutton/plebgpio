
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
//   PROG                               PROG                                 PROG
void update_prog ( int prog_num , int offset , const char* values ) {
  int     limit                 = strlen(values)-1,
          i                     = -1,
          valid                 = 1,
          row                   = prog_num*5 + offset,            //   16
          all_matched           = 1;
  char    incumbent,value;
#if VERBOSE == 3
  printf("update_prog, '%s' is %d characters\n",values,limit+1);
#endif
  if ( offset==0 ) {
    while ( limit>i++ ) {
      value                     = values[i];
      if ( value!=TWOTXT && value!=DASHTXT && value!=TRETXT && value!=BACKTXT && value!=QUATXT && value!=STOPTXT )
        valid                   = 0;
    }
  } else {
    while ( limit>i++ ) {
      value                     = values[i];
      if ( value!=ZEROTXT && value!=ZEROTXT_ && value!=ONETXT  && value!=ONETXT_  &&
           value!=TWOTXT  && value!=TWOTXT_  && value!=TRETXT  && value!=TRETXT_  &&
           value!=QUATXT  && value!=QUATXT_  && value!=QUITXT  && value!=QUITXT_  &&
           value!=SESTXT  && value!=SESTXT_  && value!=SETTXT  && value!=SETTXT_
          )
        valid                   = 0;
    }
  }
  i                             = -1;
  if ( valid==1 ) {
    if ( offset==0 ) {
      while ( limit>i++ ) {
        value                   = values[i];
        if ( value==TWOTXT || value==DASHTXT )               value                 = 2;
        if ( value==TRETXT || value==BACKTXT )               value                 = 3;
        if ( value==QUATXT || value==STOPTXT )               value                 = 4;
        incumbent               = progs[row][i];
        progs[row][i]           = value;
        if ( all_matched==1 && value!=incumbent ) {
#if VERBOSE == 3
          printf ( "update_program: in row %d, there was a difference in column %d - %d replacing %d\n",row,i,value,incumbent);
#endif
          all_matched           = 0;
        }
      }
#if VERBOSE == 3
      if ( all_matched==1 )
        printf ( "update_program: in row %d, everything matched for %d values\n",row,limit+1);
#endif
    } else {
      while ( limit>i++ ) {
        value                   = values[i];
        if ( value==ZEROTXT || value==ZEROTXT_ )             value                 = 0;
        if ( value==ONETXT || value==ONETXT_ )               value                 = 1;
        if ( value==TWOTXT || value==TWOTXT_ )               value                 = 2;
        if ( value==TRETXT || value==TRETXT_ )               value                 = 3;
        if ( value==QUATXT || value==QUATXT_ )               value                 = 4;
        if ( value==QUITXT || value==QUITXT_ )               value                 = 5;
        if ( value==SESTXT || value==SESTXT_ )               value                 = 6;
        if ( value==SETTXT || value==SETTXT_ )               value                 = 7;
        incumbent               = progs[row][i];
        progs[row][i]           = value;
        if ( all_matched==1 && value!=incumbent ){
#if VERBOSE == 3
          printf ( "update_program: in row %d, there was a difference in column %d - %d replacing %d\n",row,i,value,incumbent);
#endif
          all_matched           = 0;
        }
      }
#if VERBOSE == 3
      if ( all_matched==1 )
        printf ( "update_program: in row %d, everything matched for %d values\n",row,limit+1);
#endif
    }
  }
}
char recognized_program(char prog){
  char    result              = 0;
  if( prog==1 || prog==2 || prog==3 || prog==4 || prog==5 || prog==6 || prog==7 || prog==8 || prog==9 ){
    result                    = 1;
  }
  return result;
}
char retrieve_program(){
  char    prog                = read_file_digit("","","prog");
  if ( recognized_program(prog)==1 ) {
    return prog;
  } else {
    return 0;
  }
}
int transition_program(char new_program){
  printf("transition_program:  new_program == %d, program == %d\n",new_program,program);
  program                     = new_program;
  program_ix                  = 0;
  opcode                      = LOOP;
  return opcode;
}
char led_mod ( bn_gpio_led *led ) {
  char    wid                 = led->colrs,mod;
  if ( led->actv!=1 )
    wid                       = 0;
  mod                         = pwr2[wid];
  return mod;
}
// WARNING                                                        WARNING
// char is not a good type if there are more than 7 LED pins in total
// WARNING                                                        WARNING
int step_program(gpio_v2_t *pins, int write_mask){
  char    prog                = program*5,
          prog_led            = prog+1,
          first_led_val,second_led_val,third_led_val,fourth_led_val,
          first_led_mod,second_led_mod,third_led_mod,fourth_led_mod,
          first_led_xer,second_led_xer,third_led_xer,fourth_led_xer,
          first_led_net,second_led_net,third_led_net,fourth_led_net;
  int     bits                = 0;

  // printf("step_program, prog == %d, program == %d, prog_led == %d\n",prog,program,prog_led);

  if( opcode==STOP || opcode==LOOP ){
    program_ix                = 0;
    if( opcode==LOOP ){
      opcode                  = CNTNU;
    }
  }else{
    if( 70<program_ix++ ){
      opcode                  = STOP;
    }
  }

  if( opcode==CNTNU && write_mask>0 ){
    opcode                    = progs[prog][program_ix];
    if ( opcode==CNTNU || opcode==LOOP || opcode==STOP ) {

      first_led_val           = progs[prog_led+0][program_ix];
      first_led_mod           = led_mod(config.led0);
      first_led_xer           = 1;
      first_led_net           = ( first_led_val % first_led_mod ) * first_led_xer;

      second_led_val          = progs[prog_led+1][program_ix];
      second_led_mod          = led_mod(config.led1);
      second_led_xer          = first_led_xer * first_led_mod;
      second_led_net          = ( second_led_val % second_led_mod ) * second_led_xer;

      third_led_val           = progs[prog_led+2][program_ix];
      third_led_mod           = led_mod(config.led2);
      third_led_xer           = second_led_xer * second_led_mod;
      third_led_net           = ( third_led_val % third_led_mod ) * third_led_xer;

      fourth_led_val          = progs[prog_led+3][program_ix];
      fourth_led_mod          = led_mod(config.led3);
      fourth_led_xer          = third_led_xer * third_led_mod;
      fourth_led_net          = ( fourth_led_val % fourth_led_mod ) * fourth_led_xer;

      bits                    = first_led_net + second_led_net + third_led_net + fourth_led_net;

      if ( gpio_line_set_values ( pins , bits, write_mask) == -1 ) {
        return FAILED_GPIO_WRITE;
      } else {
        return opcode;
      }
    } else {
      return BAD_OPCODE;
    }
  } else {
    return opcode;
  }
    // if (gpio_line_set_values (&pins, 0x0009, read_mask-1) == -1)  {
    //   break;
    // }
}
void process_button ( int read_values , bn_gpio_btn* btn ) {
  char    string[4]           = {  32 ,  32 ,  32 ,   0 };
  char    fname[5]            = {  98 , 116 , 110 ,  48 ,   0 };
  char    fullpath[MAX_PATH];
  char    curr_val;
  bn_gpio_line* line;
  if ( btn->actv==1 ) {
    line                      = btn->hot;
    if ( ( line->weight & read_values ) > 0 ) {
      curr_val                = 1;
    } else {
      curr_val                = 0;
    }
    if ( curr_val!=line->val ) {
      fname[3]                = btn->num+ZEROTXT;
      string[0]               = curr_val+ZEROTXT;
      assemble_path(fullpath,"","",fname);
      file_write ( fullpath , string , 1 );
      printf("process_buttons: writing value '%c' to '%s'\n",string[0],fullpath);
      line->val               = curr_val;
    }
  }
}
void process_buttons ( int read_values ) {
  process_button ( read_values , config.btn0 ) ;
  process_button ( read_values , config.btn1 ) ;
  process_button ( read_values , config.btn2 ) ;
}

int read_and_process_buttons ( gpio_v2_t *pins , int read_mask ) {
  int     failed_read         = 0,read_values;
  if ( read_mask>0 ) {
    read_values               = gpio_line_get_values ( pins , read_mask , read_mask ) ;
    if ( read_values<0 ) {
      failed_read             = 1;
    } else {
      process_buttons(read_values);
    }
  }
  return failed_read;
}
//   PROG                               PROG                                 PROG
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*

