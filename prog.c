
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
//   PROG                               PROG                                 PROG
char recognized_program(char prog){
  char result                 = 0;
  if( program==1 || program==2 || program==3 || program==4 || program==5 ){
    result                    = 1;
  }
  return result;
}
char retrieve_program(){
  char    program             = read_file_digit("","","prog");
  if ( recognized_program(program)==1 ) {
    return program;
  } else {
    return 0;
  }
}
int transition_program(char new_program){
  program                     = new_program;
  program_ix                  = 0;
  opcode                      = LOOP;
  return opcode;
}
// WARNING                                                        WARNING
// char is not a good type if there are more than 7 LED pins in total
// WARNING                                                        WARNING
int step_program(gpio_v2_t *pins, int write_mask){
  char    prog                = program*5,
          first_led           = 0,
          second_led          = 1,
          third_led           = 2,
          fourth_led          = 3,
          first_led_val,second_led_val,third_led_val,fourth_led_val,
          first_led_wid,second_led_wid,third_led_wid,fourth_led_wid,
          first_led_mod,second_led_mod,third_led_mod,fourth_led_mod,
          first_led_xer,second_led_xer,third_led_xer,fourth_led_xer,
          first_led_net,second_led_net,third_led_net,fourth_led_net;
  int     bits                = 0;
  char    pwr2[8]             = {    1 ,   2 ,   4 ,   8 ,  16 ,  32 ,  64 , 128 };
  bn_gpio_led *led0           = config.led0;
  bn_gpio_led *led1           = config.led1;
  bn_gpio_led *led2           = config.led2;
  bn_gpio_led *led3           = config.led3;

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

  if( opcode==CNTNU ){
    opcode                    = progs[prog][program_ix];
    if ( opcode==CNTNU || opcode==LOOP || opcode==STOP ) {

      // bits                      = progs[prog+1+first_led][program_ix] + progs[prog+1+second_led][program_ix]*8;

      first_led_val           = progs[prog+1+first_led][program_ix];
      first_led_wid           = led0->colrs;
      if ( led0->actv!=1 )       first_led_wid  = 0;
      first_led_mod           = pwr2[first_led_wid];
      first_led_xer           = 1;
      first_led_net           = ( first_led_val % first_led_mod ) * first_led_xer;

      second_led_val          = progs[prog+1+second_led][program_ix];
      second_led_wid          = led1->colrs;
      if ( led1->actv!=1 )       second_led_wid = 0;
      second_led_mod          = pwr2[second_led_wid];
      second_led_xer          = first_led_xer * first_led_mod;
      second_led_net          = ( second_led_val % second_led_mod ) * second_led_xer;

      third_led_val           = progs[prog+1+third_led][program_ix];
      third_led_wid           = led2->colrs;
      if ( led2->actv!=1 )       third_led_wid  = 0;
      third_led_mod           = pwr2[third_led_wid];
      third_led_xer           = second_led_xer * second_led_mod;
      third_led_net           = ( third_led_val % third_led_mod ) * third_led_xer;

      fourth_led_val          = progs[prog+1+fourth_led][program_ix];
      fourth_led_wid          = led3->colrs;
      if ( led3->actv!=1 )       fourth_led_wid = 0;
      fourth_led_mod          = pwr2[fourth_led_wid];
      fourth_led_xer          = third_led_xer * third_led_mod;
      fourth_led_net          = ( fourth_led_val % fourth_led_mod ) * fourth_led_xer;

      bits                    = first_led_net + second_led_net + third_led_net + fourth_led_net;

      // if ( (program_ix % 4)==0 && program_ix<8 )
      //   printf("LED0:  val == %d, wid == %d, mod == %d, xer == %d, net == %d\n",first_led_val,first_led_wid,first_led_mod,first_led_xer,first_led_net);
      // if ( (program_ix % 4)==1 && program_ix<8 )
      //   printf("LED1:  val == %d, wid == %d, mod == %d, xer == %d, net == %d\n",second_led_val,second_led_wid,second_led_mod,second_led_xer,second_led_net);
      // if ( (program_ix % 4)==2 && program_ix<8 )
      //   printf("LED2:  val == %d, wid == %d, mod == %d, xer == %d, net == %d\n",third_led_val,third_led_wid,third_led_mod,third_led_xer,third_led_net);
      // if ( (program_ix % 4)==3 && program_ix<8 )
      //   printf("LED3:  val == %d, wid == %d, mod == %d, xer == %d, net == %d\n",fourth_led_val,fourth_led_wid,fourth_led_mod,fourth_led_xer,fourth_led_net);

      // if(opcode==CNTNU) printf("opcode == 'continue', bits == %d, program_ix == %d, program == %d, write_mask == %d\n",bits,program_ix,program,write_mask);
      // if(opcode==LOOP)  printf("opcode == 'loop', bits == %d, program_ix == %d, program == %d, write_mask == %d\n",bits,program_ix,program,write_mask);
      // if(opcode==STOP)  printf("opcode == 'stop', bits == %d, program_ix == %d, program == %d, write_mask == %d\n",bits,program_ix,program,write_mask);

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
//   PROG                               PROG                                 PROG
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*

