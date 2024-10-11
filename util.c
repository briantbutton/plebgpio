
int  unmatched_sect ( int is ) {
  if ( is==0 ) {
    return 1;
  } else {
    return 0;
  }
}
int  led_sect ( int is ) {
  if ( is>0 && is<5 ) {
    return 1;
  } else {
    return 0;
  }
}
int  btn_sect ( int is ) {
  if ( 0>is && -4<is ) {
    return 1;
  } else {
    return 0;
  }
}
int  prog_sect ( int is ) {
  if ( is>10 ) {
    return 1;
  } else {
    return 0;
  }
}
int  pleb_sect ( int is ) {
  if ( -11==is ) {
    return 1;
  } else {
    return 0;
  }
}
char b36_text_value_plus_1 ( const char* string, char limit , char incl0 ) {
  char    val;
  if ( strlen(string)==1 || strlen(string)==2 ) {
    if ( string[0] < 128 ) {
      val                     = b36ins[string[0]];
      if ( val==0 ) {
        if ( incl0==1 ) {
          return 1;
        } else {
          return 0;
        }
      } else {
        if ( val < limit+1 ) {
          return val+1;
        } else {
          return 0;
        }
      }
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}
char legit_next_val (char val) {
  if ( val==TWOTXT || val==DASHTXT || val==TRETXT || val==BACKTXT || val==QUATXT || val==STOPTXT ) {
    return 1;
  } else {
    return 0;
  }
}
char legit_led_val (char val) {
  if ( val==ZERTXT || val==ZERTXT_ || val==ONETXT || val==ONETXT_ ||
       val==TWOTXT || val==TWOTXT_ || val==TRETXT || val==TRETXT_ ||
       val==QUATXT || val==QUATXT_ || val==QUITXT || val==QUITXT_ ||
       val==SESTXT || val==SESTXT_ || val==SETTXT || val==SETTXT_ ) {
    return 1;
  } else {
    return 0;
  }
}
char member_of(char val, char* choices, char count)
{
  char    member              = 0;
  if ( count==four )
    if ( val==choices[0] || val==choices[1] || val==choices[2] || val==choices[3] )
      member                  = 1;
  if ( count==five )
    if ( val==choices[0] || val==choices[1] || val==choices[2] || val==choices[3] || val==choices[4] )
      member                  = 1;
  if ( count==six )
    if ( val==choices[0] || val==choices[1] || val==choices[2] || val==choices[3] || val==choices[4] || val==choices[5] )
      member                  = 1;
  if ( count==eight )
    if ( val==choices[0] || val==choices[1] || val==choices[2] || val==choices[3] || val==choices[4] || val==choices[5] || val==choices[6] || val==choices[7] )
      member                  = 1;
  return member;
}
void count_line(bn_gpio_line* line){
  int     weight              = config.curr_weight,
          count               = config.line_count;
  line->weight                = weight;
  if ( line->out==1 ) {
    config.write_mask         = config.write_mask+weight;
  } else {
    config.read_mask          = config.read_mask+weight;
  }
  config.curr_weight          = weight+weight;
  config.offsets[count]       = line;
  config.line_count           = count + 1;
}
char count_led_lines(bn_gpio_led* led){
  char    actv                = 0;
  if ( led->colrs==3 ) {
    if ( led->red->pin>0 && led->grn->pin>0 && led->blu->pin>0 ) {
      count_line(led->blu);
      count_line(led->grn);
      count_line(led->red);
      actv                    = 1;
    }
  }
  if ( led->colrs==2 ) {
    if ( led->red->pin==0 && led->grn->pin>0 && led->blu->pin>0 ) {
      count_line(led->blu);
      count_line(led->grn);
      actv                    = 1;
    }
    if ( led->red->pin>0 && led->grn->pin==0 && led->blu->pin>0 ) {
      count_line(led->blu);
      count_line(led->red);
      actv                    = 1;
    }
    if ( led->red->pin>0 && led->grn->pin>0 && led->blu->pin==0 ) {
      count_line(led->grn);
      count_line(led->red);
      actv                    = 1;
    }
  }
  if ( led->colrs==0 || led->colrs==1 ) {
    if ( led->red->pin==0 && led->grn->pin==0 && led->blu->pin>0 ) {
      led->colrs              = 1;                                  // 1 is the default # of colors if the other config matches
      count_line(led->blu);
      actv                    = 1;
    }
  }
  led->actv                   = actv;
  return actv;
}
char count_btn_lines(bn_gpio_btn* btn){
  char    actv                = 0;
  if ( btn->hot->pin>0 ) {
    count_line(btn->hot);
    actv                      = 1;
#if VERBOSE == 3
    printf("count_btn_lines, btn%d hot is pin %d\n",btn->num,btn->hot->pin);
#endif
  } else {
#if VERBOSE == 3
    printf("count_btn_lines, btn%d is inactive\n",btn->num);
#endif
  }
  btn->actv                   = actv;
  return actv;
}
#if VERBOSE == 2 || VERBOSE == 3
void print_led_pin(bn_gpio_led* led, bn_gpio_line* pin, char* colour){
  printf("      led%d-%s is GPIO #%d, and has weight %d\n",led->num,colour,pin->pin,pin->weight);
}
void print_btn_pin(bn_gpio_btn* btn, bn_gpio_line* pin, char* colour){
  printf("      btn%d-%s is GPIO #%d, and has weight %d\n",btn->num,colour,pin->pin,pin->weight);
}
void print_led(bn_gpio_led* led){
  if ( led->actv==1 ){
    printf("   led%d has %d colours\n",led->num,led->colrs);
    if ( led->blu->pin>0 )       print_led_pin(led,led->blu,"blue");
    if ( led->grn->pin>0 )       print_led_pin(led,led->grn,"green");
    if ( led->red->pin>0 )       print_led_pin(led,led->red,"red");
  } else {
    printf("   led%d is inactive\n",led->num);
  }
}
void print_btn(bn_gpio_btn* btn){
  if ( btn->actv==1 ){
    printf("   btn%d is active\n",btn->num);
    if ( btn->hot->pin>0 )       print_btn_pin(btn,btn->hot,"hot");
  } else {
    printf("   btn%d is inactive\n",btn->num);
  }
}
#endif
void print_config(){
#if VERBOSE == 2 || VERBOSE == 3
  printf("\nconfig.curr_weight == %d, config.line_count == %d\n",config.curr_weight,config.line_count);
  print_led(config.led0);
  print_led(config.led1);
  print_led(config.led2);
  print_led(config.led3);
  print_btn(config.btn0);
  print_btn(config.btn1);
  print_btn(config.btn2);
  printf("offset 0 has pin #%d\n",config.offsets[0]->pin);
  printf("offset 1 has pin #%d\n",config.offsets[1]->pin);
  printf("offset 2 has pin #%d\n",config.offsets[2]->pin);
  printf("offset 3 has pin #%d\n",config.offsets[3]->pin);
  printf("offset 4 has pin #%d\n",config.offsets[4]->pin);
  printf("offset 5 has pin #%d\n",config.offsets[5]->pin);
  printf("offset 6 has pin #%d\n",config.offsets[6]->pin);
  printf("offset 7 has pin #%d\n",config.offsets[7]->pin);
  printf("\n");
#endif
}
void count_lines(){
  config.line_count           = 0;
  config.curr_weight          = 1;
  config.read_mask            = 0;
  config.write_mask           = 0;
  count_led_lines(config.led0);
  count_led_lines(config.led1);
  count_led_lines(config.led2);
  count_led_lines(config.led3);
  count_btn_lines(config.btn0);
  count_btn_lines(config.btn1);
  count_btn_lines(config.btn2);
}
int pinOf(const char* string) {
  int     pin                 = atoi(string);
  if ( pin<0 || pin>26 )
    pin                       = 0;
  return pin;
}
void assemble_path(char *path, char *a, char *b, char *c){
  char    l                   = strlen(bngpiodir),
          al                  = strlen(a),
          bl                  = strlen(b),
          cl                  = strlen(c),
          sl                  = 1;
  char    i,j;
  char    s[1]                = {47};
  for (i=0; i<MAX_PATH; i++)          path[i]          = 0;
  i                           = 0;
  j                           = 0;
  while ( i < l )                     path[j++]        = bngpiodir[i++];
  if ( al > 0 ) {
    i                         = 0;
    while ( i < al )                  path[j++]        = a[i++];
    i                         = 0;
    while ( i < sl )                  path[j++]        = s[i++];
  }
  if ( bl > 0 ) {
    i                         = 0;
    while ( i < bl )                  path[j++]        = b[i++];
    i                         = 0;
    while ( i < sl )                  path[j++]        = s[i++];
  }
  i                           = 0;
  while ( i < cl )                    path[j++]        = c[i++];
}
