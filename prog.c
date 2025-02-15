
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
//   PROG                               PROG                                 PROG
void update_prog ( int prog_num , int offset , const char* values ) {
  int     limit                 = strlen(values)-1,
          i                     = -1,
          valid                 = 1,
          row                   = prog_num*5 + offset,
          all_matched           = 1;
  char    incumbent,value;
#if VERBOSE == 3
  printf("update_prog, '%s' is %d characters\n",values,limit+1);
#endif
  if ( offset==0 ) {
    while ( limit>i++ ) {
      if ( legit_next_val(values[i])==0 )
        valid                   = 0;
    }
  } else {
    while ( limit>i++ ) {
      if ( legit_led_val(values[i])==0 )
        valid                   = 0;
    }
  }
  i                             = -1;
  if ( valid==1 ) {
    if ( offset==0 ) {
      while ( limit>i++ ) {
        value                   = values[i];
        if ( value==TWOTXT || value==DASHTXT )               value            = 2;
        if ( value==TRETXT || value==BACKTXT )               value            = 3;
        if ( value==QUATXT || value==STOPTXT )               value            = 4;
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
        value                   = colorout[values[i]];
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
char recognized_program(char p){
  char    result              = VAL_ERROR;
  if ( p==0 || p==LAST_PROGRAM || ( p>0 && p<LAST_PROGRAM ) ) 
    result                    = p;
  return result;
}
char recognized_color(char c){
  char    result              = 0;
  if( c==0 || ( c>0 && c<VAL_ERROR ) )
    result                    = colorout[c];
  return result;
}
char retrieve_program(){
  char    prog                = read_file_digit("","","prog");
  return recognized_program(prog);
}
char retrieve_led(char led_num){
  char    label[5]            = { 108 , 101 , 100 , led_num+ZEROTXT , 0 },
          colr                = read_file_digit("","",label);
  return recognized_color(colr);
}
int transition_program(char new_program){
#if VERBOSE == 1 || VERBOSE == 2 || VERBOSE == 3
  printf("transition_program:  new_program == %d, program == %d\n",new_program,program);
#endif
  program                     = new_program;
  program_ix                  = 0;
  opcode                      = LOOP;
  return opcode;
}
char ledmod ( bn_gpio_led *led ) {
  char    wid                 = led->colrs,mod;
  if ( led->actv!=1 )
    wid                       = 0;
  mod                         = pwr2[wid];
  return mod;
}
// WARNING                                                        WARNING
// char is not a good type if there are more than 7 LED pins in total
// WARNING                                                        WARNING
int step_program(gpio_v2_t *pins){
  int     write_mask          = config.write_mask,
          prog5               = program*5,
          prog_led            = prog5+1;
  char    led_xer             = 1,
          led_mod             = 1,
          led_val             = 0;
  char    led_net[4]          = { 0 , 0 , 0 , 0 };
  int     bits                = 0,
          i                   = -1;

  // printf("step_program, prog5 == %d, program == %d, prog_led == %d\n",prog5,program,prog_led);

  if( opcode==STOP || opcode==LOOP ){
    program_ix                = 0;
    if( opcode==LOOP ){
      opcode                  = CNTNU;
    }
  }else{
    program_ix++;
    if( program_ix>(PROGRAM_LENGTH-1) ){
      opcode                  = STOP;
      program_ix              = 0;
    }
  }

  if ( opcode==CNTNU && write_mask>0 ) {
    opcode                    = progs[prog5][program_ix];
    if ( opcode==CNTNU || opcode==LOOP || opcode==STOP ) {

      while ( 3 > i++ ) {
        led_val               = progs[prog_led+i][program_ix];
        led_xer               = led_xer * led_mod;
        led_mod               = ledmod(leds[i]);
        led_net[i]            = ( led_val % led_mod ) * led_xer;
      }
      bits                    = led_net[0] + led_net[1] + led_net[2] + led_net[3];

      if ( old_led_bits!=bits ) {
        old_led_bits          = bits;
        if ( gpio_line_set_values ( pins , bits, write_mask) < 0 ) 
          problem             = FAILED_GPIO_WRITE;
      }
      return opcode;
    } else {
      return BAD_OPCODE;
    }
  } else {
    return opcode;
  }
}
void set_leds(gpio_v2_t *pins){
  int     write_mask          = config.write_mask;
  char    led_xer             = 1,
          led_mod             = 1,
          led_val             = 0;
  char    led_net[4]          = { 0 , 0 , 0 , 0 };
  int     bits                = 0,
          i                   = -1;

  while ( 3 > i++ ) {
    led_val                   = retrieve_led(i);
    led_xer                   = led_xer * led_mod;
    led_mod                   = ledmod(leds[i]);
    led_net[i]                = ( led_val % led_mod ) * led_xer;
  }
  bits                        = led_net[0] + led_net[1] + led_net[2] + led_net[3];
  if ( old_led_bits!=bits ) {
    old_led_bits              = bits;
    if ( gpio_line_set_values ( pins , bits, write_mask) < 0 ) 
      problem                 = FAILED_GPIO_WRITE;
  }
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
#if VERBOSE == 1 || VERBOSE == 2 || VERBOSE == 3
      printf("process_buttons: writing value '%c' to '%s'\n",string[0],fullpath);
#endif
      line->val               = curr_val;
    }
  }
}
void process_buttons ( int read_values ) {
  process_button ( read_values , config.btn0 ) ;
  process_button ( read_values , config.btn1 ) ;
  process_button ( read_values , config.btn2 ) ;
}

int read_and_process_buttons ( gpio_v2_t *pins ) {
  int     read_mask           = config.read_mask,
          failed_read         = 0,read_values;
  if ( read_mask>0 ) {
    read_values               = gpio_line_get_values ( pins , read_mask , read_mask ) ;
    if ( read_values<0 ) {
      failed_read             = 1;
      problem                 = FAILED_GPIO_READ;
    } else {
      process_buttons(read_values);
    }
  }
  return failed_read;
}
//   PROG                               PROG                                 PROG
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
