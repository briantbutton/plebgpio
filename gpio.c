#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define STATE_NOT_IN            0
#define STATE_IN                1
#define STATE_BAD               2
#define CMD_EMPTY               0
#define CMD_INITIALIZE          1
#define CMD_ACTIVATE            2
#define CMD_DEACTIVATE          3
#define CMD_WRITE               4
#define CMD_READ                5
#define CMD_READALL             6
#define CMD_ERROR_SYNTAX       10
#define CMD_NOT_INIT           11
#define CMD_NO_STATE           12
#define CMD_NO_WRITE_COLR      13
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
#define MAX_PATH               50
#define ZEROTXT                48
#define ONETXT                 49
#define forty                  40
#define zero                    0
#define one                     1
#define four                    4
#define five                    5
#define six                     6
#define eight                   8
#define COMMANDCOUNT            6
#define VALUECOUNT             17

static char initialized       = zero;
static char pattern           = zero;
static char btn_pins          = zero;
static char obj_avail         = zero;
static char objNum            = five;
static char objTyp            = OBJ_INVALID;
static char colrVal           = VAL_NULL;
static char verbose           = zero;
static char statestate        = STATE_NOT_IN;
static int  argc_             = one;
                              // 
                              // 
                              // 
                              // https://pinout.xyz/
                              // 
                              //                     LED0  LED2
                              //                       ||  || initialized
                              //                       ||  ||    |
                              //                       vv  vv    v
                              // Unactivated state == 400000000001
                              //    RGB LEDs [0,1,2,3]   ^^    ^^
                              //                         ||    ||
                              //                         ||    ||
                              //                        LED1 Buttons [0,1,2]
                              //                              BTN0 == 4
                              //                              BTN1 == 2
                              //                              BTN2 == 1

static char pattrns[16][24]   = {
                              //  ~   ~    ~     ~      ~       ~        ~         ~          ~           ~            ~             ~
                              // The pattern is set at initialization
                              // 
                              // There are eight patterns, 0 - 7
                              // If a pattern 8 is shown or entered, it means pattern 0
                              // 
                              // 
                              // 
                              // 2 LEDs 1 BTN B      https://www.button.network/gpio-guide/pinout/buttonnet21b.html
                              // It uses narrow grouping and follows the order ground-red-green-blue for both LEDs.
                              // This is for the prototype; the button placement, particularly, may change
                              // Addendum:  I added a BTN on 17, please note that this will require an entry in config.txt
                              // https://forums.raspberrypi.com/viewtopic.php?f=117&t=208748
                              // 
                              // gpio=17=pu
                              // 
                              //        ~  ~  ~   LED-0   ~  ~  ~       ~  ~  ~   LED-1   ~  ~  ~       ~  ~  ~   LED-2   ~  ~  ~       ~  ~  ~   LED-3   ~  ~  ~     BTN-0   BTN-1   BTN-2  empty
                              //        R       G       B       W       R       G       B       W       R       G       B       W       R       G       B       W
                                  {     17 ,    27 ,    22 ,    22 ,    25 ,     8 ,     7 ,     7 ,    11 ,     9 ,    10 ,    10 ,     6 ,    16 ,    26 ,    26 ,    17 ,     4 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 },
                                  {      0 ,     0 ,     0 ,     0 ,     1 ,     1 ,     1 ,     0 ,     1 ,     1 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 },
                              //        0       1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16      17      18      19      20      21      22      23      24      25      26      27      28      29 
                              // TRE LED TWO BTN A
                              //        ~  ~  ~   LED-0   ~  ~  ~       ~  ~  ~   LED-1   ~  ~  ~       ~  ~  ~   LED-2   ~  ~  ~       ~  ~  ~   LED-3   ~  ~  ~     BTN-0   BTN-1   BTN-2  empty
                              //        R       G       B       W       R       G       B       W       R       G       B       W       R       G       B       W
                                  {     17 ,    27 ,    22 ,    17 ,    23 ,    24 ,    25 ,    23 ,    10 ,     9 ,    11 ,    10 ,     6 ,    16 ,    26 ,     6 ,     0 ,     1 ,     5 ,     0 ,     0 ,     0 ,     0 ,     0 },
                                  {      1 ,     1 ,     1 ,     0 ,     1 ,     1 ,     1 ,     0 ,     1 ,     1 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     1 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 },
                              //        0       1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16      17      18      19      20      21      22      23      24      25      26      27      28      29 
                              // TWO LED ONE BTN A
                              //        ~  ~  ~   LED-0   ~  ~  ~       ~  ~  ~   LED-1   ~  ~  ~       ~  ~  ~   LED-2   ~  ~  ~       ~  ~  ~   LED-3   ~  ~  ~     BTN-0   BTN-1   BTN-2  empty
                              //        R       G       B       W       R       G       B       W       R       G       B       W       R       G       B       W
                                  {     17 ,    27 ,    22 ,    17 ,    23 ,    24 ,    25 ,    23 ,    10 ,     9 ,    11 ,    10 ,     6 ,    16 ,    26 ,     6 ,     0 ,     1 ,     5 ,     0 ,     0 ,     0 ,     0 ,     0 },
                                  {      0 ,     0 ,     0 ,     0 ,     1 ,     1 ,     1 ,     0 ,     1 ,     1 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 },
                              //        0       1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16      17      18      19      20      21      22      23      24      25      26      27      28      29 

                              // 2 LEDs 1 BTN X    
                              // This is the arrangment I mistakenly used for the first prototype
                              //        ~  ~  ~   LED-0   ~  ~  ~       ~  ~  ~   LED-1   ~  ~  ~       ~  ~  ~   LED-2   ~  ~  ~       ~  ~  ~   LED-3   ~  ~  ~     BTN-0   BTN-1   BTN-2  empty
                              //        R       G       B       W       R       G       B       W       R       G       B       W       R       G       B       W
                                  {     17 ,    27 ,    22 ,    17 ,     7 ,     8 ,    25 ,     7 ,    11 ,     9 ,    10 ,    11 ,     6 ,    16 ,    26 ,     6 ,     0 ,     1 ,     5 ,     0 ,     0 ,     0 ,     0 ,     0 },
                                  {      0 ,     0 ,     0 ,     0 ,     1 ,     1 ,     1 ,     0 ,     1 ,     1 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 },
                              //        0       1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16      17      18      19      20      21      22      23      24      25      26      27      28      29 
                              // TWO LED ONE BTN B
                              //        ~  ~  ~   LED-0   ~  ~  ~       ~  ~  ~   LED-1   ~  ~  ~       ~  ~  ~   LED-2   ~  ~  ~       ~  ~  ~   LED-3   ~  ~  ~     BTN-0   BTN-1   BTN-2  empty
                              //        R       G       B       W       R       G       B       W       R       G       B       W       R       G       B       W
                                  {     17 ,    27 ,    22 ,    17 ,    25 ,     8 ,     7 ,    25 ,    10 ,     9 ,    11 ,    10 ,     6 ,    16 ,    26 ,     6 ,     0 ,     1 ,     5 ,     0 ,     0 ,     0 ,     0 ,     0 },
                                  {      0 ,     0 ,     0 ,     0 ,     1 ,     1 ,     1 ,     0 ,     1 ,     1 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 },
                              //        0       1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16      17      18      19      20      21      22      23      24      25      26      27      28      29 
                              // TRE LED TWO BTN A
                              //        ~  ~  ~   LED-0   ~  ~  ~       ~  ~  ~   LED-1   ~  ~  ~       ~  ~  ~   LED-2   ~  ~  ~       ~  ~  ~   LED-3   ~  ~  ~     BTN-0   BTN-1   BTN-2  empty
                              //        R       G       B       W       R       G       B       W       R       G       B       W       R       G       B       W
                                  {     17 ,    27 ,    22 ,    17 ,    23 ,    24 ,    25 ,    23 ,    10 ,     9 ,    11 ,    10 ,     6 ,    16 ,    26 ,     6 ,     0 ,     1 ,     5 ,     0 ,     0 ,     0 ,     0 ,     0 },
                                  {      1 ,     1 ,     1 ,     0 ,     1 ,     1 ,     1 ,     0 ,     1 ,     1 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     1 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 },
                              //        0       1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16      17      18      19      20      21      22      23      24      25      26      27      28      29 
                              // TWO LED ONE BTN A
                              //        ~  ~  ~   LED-0   ~  ~  ~       ~  ~  ~   LED-1   ~  ~  ~       ~  ~  ~   LED-2   ~  ~  ~       ~  ~  ~   LED-3   ~  ~  ~     BTN-0   BTN-1   BTN-2  empty
                              //        R       G       B       W       R       G       B       W       R       G       B       W       R       G       B       W
                                  {     17 ,    27 ,    22 ,    17 ,    23 ,    24 ,    25 ,    23 ,    10 ,     9 ,    11 ,    10 ,     6 ,    16 ,    26 ,     6 ,     0 ,     1 ,     5 ,     0 ,     0 ,     0 ,     0 ,     0 },
                                  {      0 ,     0 ,     0 ,     0 ,     1 ,     1 ,     1 ,     0 ,     1 ,     1 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 },
                              //        0       1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16      17      18      19      20      21      22      23      24      25      26      27      28      29 

                              // TWO LED ONE BTN B
                              //        ~  ~  ~   LED-0   ~  ~  ~       ~  ~  ~   LED-1   ~  ~  ~       ~  ~  ~   LED-2   ~  ~  ~       ~  ~  ~   LED-3   ~  ~  ~     BTN-0   BTN-1   BTN-2  empty
                              //        R       G       B       W       R       G       B       W       R       G       B       W       R       G       B       W
                                  {     17 ,    27 ,    22 ,    17 ,    25 ,     8 ,     7 ,    25 ,    10 ,     9 ,    11 ,    10 ,     6 ,    16 ,    26 ,     6 ,     0 ,     1 ,     5 ,     0 ,     0 ,     0 ,     0 ,     0 },
                                  {      0 ,     0 ,     0 ,     0 ,     1 ,     1 ,     1 ,     0 ,     1 ,     1 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     1 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 }
                              //        0       1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16      17      18      19      20      21      22      23      24      25      26      27      28      29 
                                };

//  ~   ~    ~     ~      ~       ~        ~         ~          ~           ~            ~             ~
// If one wants buttons and lights to be auto-activated when a pattern is initialized, specify them here
// The table is pretty self-explanatory
static char initables[8][6][2]= {
                                  { { OBJ_INVALID , 0 } , { OBJ_LED     , 1 } , { OBJ_LED     , 2 } , { OBJ_INVALID , 2 } , { OBJ_BTN     , 0 } , { OBJ_BTN     , 1 } } ,
                                  { { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } } ,
                                  { { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } } ,
                                  { { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } } ,
                                  { { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } } ,
                                  { { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } } ,
                                  { { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } } ,
                                  { { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } , { OBJ_INVALID , 0 } } 
                                };

static char pattern_labels[8] = {     49 ,    50 ,    51 ,    52 ,    53 ,    54 ,    55 ,    56 };
static char oct_lows[4]       = {     48 ,    49 ,    50 ,    51 };
static char oct_ones[4]       = {     49 ,    51 ,    53 ,    55 };
static char oct_twos[4]       = {     50 ,    51 ,    54 ,    55 };
static char oct_fours[4]      = {     52 ,    53 ,    54 ,    55 };
/*
static char b36ins[128]       = {      0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,
                                       0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     1 ,     2 ,     3 ,     4 ,     5 ,     6 ,     7 ,     8 ,     9 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,
                                       0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,    10 ,    11 ,    12 ,    13 ,    14 ,    15 ,    16 ,    17 ,    18 ,    19 ,    20 ,    21 ,    22 ,    23 ,    24 ,    25 ,    26 ,    27 ,    28 ,    29 ,    30 ,    31 ,    32 ,
                                      33 ,    34 ,    35 ,     0 ,     0 ,     0 ,     0 ,     0 };
                                      */
static char b36tweens[128]    = {      0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,
                                       0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,    48 ,    49 ,    50 ,    51 ,    52 ,    53 ,    54 ,    55 ,    48 ,    49 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,
                                       0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,    50 ,    51 ,    52 ,    53 ,    54 ,    55 ,    48 ,    49 ,    50 ,    51 ,    52 ,    53 ,    54 ,    55 ,    48 ,    49 ,    50 ,    51 ,    52 ,    53 ,    54 ,    55 ,     0 ,
                                       0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 };
static char b36outs[36]       = {     48 ,    49 ,    50 ,    51 ,    52 ,    53 ,    54 ,    55 ,    56 ,    57 ,    97 ,    98 ,    99 ,   100 ,   101 ,   102 ,   103 ,   104 ,   105 ,   106 ,   107 ,   108 ,   109 ,   110 ,   111 ,   112 ,   113 ,   114 ,   115 ,   116 ,   117 ,   118 ,   119 ,   120 ,   121 ,   122 };


static char vals[24]          = {      0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 };
static char activated[24]     = {      0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 };


                                        //                7               
static char assigns[20]       = {        0 ,     0 ,     0 ,     0 , 
                                        //               17
                                         0 ,     0 ,     0 ,     0 ,     0 ,     0 ,
                                        //                4
                                         0 ,     0 ,     0 ,     0 ,     0 ,     0 ,
                                        // for consistency
                                         0 ,     0 ,     0 ,     0 };


static char state[13]         = {     52 ,    48 ,    48 ,    48 ,    48 ,    48 ,    48 ,    48 ,    48 ,    48 ,    48 ,    48 ,     0 };
static char indices[7]        = {      0 ,     1 ,     2 ,     3 ,     4 ,     5 ,     6 };
static char nouns[3][4]       = {
                                    {      0 ,     0 ,     0 ,     0 },
                                    {    108 ,   101 ,   100 ,     0 },
                                    {     98 ,   116 ,   110 ,     0 }
                                };
static char asciipins[30][3]  = {   { 48     ,     0 ,     0 } , { 49     ,     0 ,     0 } , { 50     ,     0 ,     0 } , { 51     ,     0 ,     0 } , { 52     ,     0 ,     0 } , { 53     ,     0 ,     0 } , { 54     ,     0 ,     0 } , { 55     ,     0 ,     0 } , { 56     ,     0 ,     0 } , { 57     ,     0 ,     0 } , 
                                    { 49     ,    48 ,     0 } , { 49     ,    49 ,     0 } , { 49     ,    50 ,     0 } , { 49     ,    51 ,     0 } , { 49     ,    52 ,     0 } , { 49     ,    53 ,     0 } , { 49     ,    54 ,     0 } , { 49     ,    55 ,     0 } , { 49     ,    56 ,     0 } , { 49     ,    57 ,     0 } , 
                                    { 50     ,    48 ,     0 } , { 50     ,    49 ,     0 } , { 50     ,    50 ,     0 } , { 50     ,    51 ,     0 } , { 50     ,    52 ,     0 } , { 50     ,    53 ,     0 } , { 50     ,    54 ,     0 } , { 50     ,    55 ,     0 } , { 50     ,    56 ,     0 } , { 50     ,    57 ,     0 } 
                                };
static char commands[6][12]   = {
                                    { CMD_INITIALIZE     ,    10 ,   105 ,   110 ,   105 ,   116 ,   105 ,    97 ,   108 ,   105 ,   122 ,   101 } , 
                                    { CMD_ACTIVATE       ,     8 ,    97 ,    99 ,   116 ,   105 ,   118 ,    97 ,   116 ,   101 ,     0 ,     0 } , 
                                    { CMD_DEACTIVATE     ,    10 ,   100 ,   101 ,    97 ,    99 ,   116 ,   105 ,   118 ,    97 ,   116 ,   101 } , 
                                    { CMD_WRITE          ,     5 ,   119 ,   114 ,   105 ,   116 ,   101 ,     0 ,     0 ,     0 ,     0 ,     0 } , 
                                    { CMD_READALL        ,     7 ,   114 ,   101 ,    97 ,   100 ,    97 ,   108 ,   108 ,     0 ,     0 ,     0 } , 
                                    { CMD_READ           ,     4 ,   114 ,   101 ,    97 ,   100 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } 
                                };
static char values[17][12]   = {
                                    { VAL_OFF            ,     3 ,   111 ,   102 ,   102 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_OFF            ,     1 ,    48 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_BLUE           ,     4 ,    98 ,   108 ,   117 ,   101 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_BLUE           ,     1 ,    49 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_GREEN          ,     5 ,   103 ,   114 ,   101 ,   101 ,   110 ,     0 ,     0 ,     0 ,     0 ,     0 } , 
                                    { VAL_GREEN          ,     1 ,    50 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_CYAN           ,     4 ,    99 ,   121 ,    97 ,   110 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } , 
                                    { VAL_CYAN           ,     1 ,    51 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_RED            ,     3 ,   114 ,   101 ,   100 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_RED            ,     1 ,    52 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_MAGENTA        ,     7 ,   109 ,    97 ,   103 ,   101 ,   110 ,   116 ,    97 ,     0 ,     0 ,     0 } ,
                                    { VAL_MAGENTA        ,     1 ,    53 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_YELLOW         ,     6 ,   121 ,   101 ,   108 ,   108 ,   111 ,   119 ,     0 ,     0 ,     0 ,     0 } , 
                                    { VAL_YELLOW         ,     1 ,    54 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_WHITE          ,     5 ,   119 ,   104 ,   105 ,   116 ,   101 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_WHITE          ,     2 ,   111 ,   110 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } ,
                                    { VAL_WHITE          ,     1 ,    55 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 ,     0 } 
                                };
static char cmds[6]           = { CMD_INITIALIZE , CMD_ACTIVATE , CMD_DEACTIVATE , CMD_WRITE , CMD_READ , CMD_READALL };
static char test1[5]          = {   VAL_WHITE ,     VAL_RED ,  VAL_YELLOW , VAL_MAGENTA };
static char test2[5]          = {   VAL_WHITE ,   VAL_GREEN ,  VAL_YELLOW ,    VAL_CYAN };
static char test3[5]          = {   VAL_WHITE ,    VAL_BLUE , VAL_MAGENTA ,    VAL_CYAN };
static char test4[5]          = {   VAL_WHITE ,    VAL_BLUE ,    VAL_BLUE ,    VAL_BLUE };
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


  INVOKE
  Initialize
  $ export GPIO="";export GPIO=$(gpio initialize 8);echo $GPIO
  Activate
  $ export GPIO=$(gpio activate led1 $GPIO);echo $GPIO
  Set Value
  $ export GPIO=$(gpio write led1 $GPIO red);echo $GPIO
  Read Value
  $ export GPIO=$(gpio read btn0 $GPIO);echo $GPIO
  Read Values
  $ export GPIO=$(gpio readall $GPIO);echo $GPIO
*/

// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
//   FILE                               FILE                                 FILE
void file_write(char *path, char *buff, char len) {
  FILE    *mb;
  int     i                   = 0,
          lim                 = len+1;
  char    c;
  mb                          = fopen(path, "w");
  if(mb){
    i                         = 0;
    c                         = buff[i++];
    while ( i < lim ) {
      putc(c,mb);
      c                       = buff[i++];
    }
    fclose(mb);
  }
}
void file_read(char *path, char *buff, int len) {
  FILE    *fp                 = fopen(path,"r");
  int     i                   = -1,
          l                   = len-1;
  char    c;
  if(fp){
    c                         = getc(fp);
    while( l>i++ && c!=EOF ){
      buff[i]                 = c;
      c                       = getc(fp);
    }
    fclose(fp);
  }
}
//   FILE                               FILE                                 FILE
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*


void err_print(char* string) {
  if(verbose==1)  fprintf(stderr,"%s\n",string);
}
char is_verbose(char* str) {
  char    verbose             = 0;
  if(strlen(str)==7&&str[0]==118&&str[1]==101&&str[2]==114&&str[3]==98&&str[4]==111&&str[5]==115&&str[6]==101)
    verbose                   = 1;
  return verbose;
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
char oct_subpart(char val, char* choices){
  return member_of(val,choices,four);
}
char recognized_pattern(char cmd) {
  return member_of(cmd,pattern_labels,eight);
}
char recognized_command(char cmd) {
  return member_of(cmd,cmds,six);
}
char shift_indices(char index) {
  argc_                       = argc_ - 1;
  if(index<2)   indices[1]    = indices[2];
  if(index<3)   indices[2]    = indices[3];
  if(index<4)   indices[3]    = indices[4];
  if(index<5)   indices[4]    = indices[5];
  if(index<6)   indices[5]    = indices[6];
  return -1;
}
int led_offset(char ledNum) {
  return 0+ledNum*4;
}
int btn_offset(char btnNum) {
  return 16+btnNum*1;
}
char digits(char pin) {
  if(pin>9){
    return 2;
  }else{
    return 1;
  }
}
char octal_value(char* array, int offset, int assign) {
  char    octal               = 0;
  if ( array[offset+0] == 1 )
    octal                     = octal+4;
  if ( array[offset+1] == 1 )
    octal                     = octal+2;
  if ( array[offset+2] == 1 || array[offset+3] == 1 )
    octal                     = octal+1;

  if ( assigns[assign+0] == 1 ) {
    octal                     = octal+16;
    assigns[assign+0]         = 0;
  }
  if ( assigns[assign+1] == 1 ) {
    octal                     = octal+8;
    assigns[assign+1]         = 0;
  }

  return b36outs[octal];
}
void assemble_path(char *path, char *a, char al, char *b, char bl, char *c, char cl){
  int i;
  for (i=0; i<MAX_PATH; i++)      path[i]        = 0;
  for (i=0; i<al; i++)            path[i]        = a[i];
  for (i=0; i<bl; i++)            path[al+i]     = b[i];
  for (i=0; i<cl; i++)            path[al+bl+i]  = c[i];
}
char match_string(char* a, char* b, char bl){
  int     i                   = 0;
  char    matches             = 0;
  if(strlen(a)==bl){
    matches                   = 1;
    while ( i<bl && matches==1 ) {
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
void set_pattern_from_state0(){
  char    stateNum            = state[0] - ZEROTXT;
  if ( stateNum==eight ){
    pattern                   = 0;
  } else {
    pattern                   = 2 * stateNum;
  }
}
// Returns 1 for led and 2 for btn; 0 for invalid
void set_obj_type(char cmd, char *arg2){
  char    isBtn               = 0,
          isLed               = 0;
  char    arg20,arg21,arg22,arg23;
  if ( cmd==CMD_READALL ) {
    objTyp                    = OBJ_BTN;
  } else {
    arg20                     = arg2[0];
    arg21                     = arg2[1];
    arg22                     = arg2[2];
    arg23                     = arg2[3];
    if ( oct_subpart(arg23,oct_lows)==1 ) {
      if((arg20==76&&arg21==69&&arg22==68)||(arg20==108&&arg21==101&&arg22==100)){
        isLed                 = 1;
        objNum                = arg23-ZEROTXT;
      }
      if(((arg20==66&&arg21==84&&arg22==78)||(arg20==98&&arg21==116&&arg22==110))&&arg23!=51){
        isBtn                 = 1;
        objNum                = arg23-ZEROTXT;
      }
      if(isLed==1&&cmd!=CMD_READ)
        objTyp                = OBJ_LED;
      if(isBtn==1&&cmd!=CMD_WRITE)
        objTyp                = OBJ_BTN;
    }
  }
}
void set_obj_state(char* array, char c, char o){
  char*   available           = pattrns[pattern+1];
  if ( oct_subpart(c,oct_fours)==1 )
    array[o+0]            = 1;
  if ( oct_subpart(c,oct_twos)==1 ) 
    array[o+1]            = 1;
  if ( oct_subpart(c,oct_ones)==1 ) {
    if(available[o+2]==1){
      array[o+2]          = 1;
    }else{
      array[o+3]          = 1;      
    }
  }
}
void set_led_activ(char c, char o){
  set_obj_state(activated,c,o);
}
void set_led_value(char c, char o){
  set_obj_state(vals,c,o);
}
void set_btn_activ(char c, char o){
  set_obj_state(activated,c,o);
}
void set_btn_value(char c, char o){
  set_obj_state(vals,c,o);
}
char parse_state(char *state){
  char    isValid             = 1;
  int     i                   = -1,
          l                   = 3;
  while(l>i++){
    set_led_activ(state[i+i+1],i*4);
    set_led_value(state[i+i+2],i*4);
  }
  set_btn_activ(state[i+i+1],i*4);
  set_btn_value(state[i+i+2],i*4);
  return isValid;
}



// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
//  MESSAGE                          MESSAGE                              MESSAGE
void copy3_to_assigns(char base, char num1, char num2, char num3) {
  assigns[base+0]             = num1;
  assigns[base+1]             = num2;
  assigns[base+2]             = num3;
}
void copy4_to_assigns(char base, char num1, char num2, char num3, char num4) {
  assigns[base+0]             = num1;
  assigns[base+1]             = num2;
  assigns[base+2]             = num3;
  assigns[base+3]             = num4;
}
void msg_btn_pins() {
  char*   pins                = pattrns[pattern+0];
  char*   available           = pattrns[pattern+1];
  char    offset,ref;
  if ( available[btn_offset(0)]==1 ) {
    assigns[1]                = 1;
    assigns[3]                = 1;
    offset                    = 4;
    ref                       = pins[btn_offset(0)];
    // printf("btn0 is available at pin %d\n",ref);
    copy3_to_assigns(offset, ((ref & 32) >> 5) , ((ref & 16) >> 4), ((ref &  8) >> 3));
    copy3_to_assigns(offset, ((ref &  4) >> 2) , ((ref &  2) >> 1), ((ref &  1) >> 0));
  }
  if ( available[btn_offset(1)]==1 ) {
    assigns[1]                = 1;
    assigns[2]                = 1;
    offset                    = 10;
    ref                       = pins[btn_offset(1)];
    // printf("btn1 is available at pin %d\n",ref);
    copy3_to_assigns(offset, ((ref & 32) >> 5) , ((ref & 16) >> 4), ((ref &  8) >> 3));
    copy3_to_assigns(offset, ((ref &  4) >> 2) , ((ref &  2) >> 1), ((ref &  1) >> 0));
  }
  btn_pins                    = 1;
}
void msg_obj_avail() {
  char*   available           = pattrns[pattern+1];
  char    offset;
  copy4_to_assigns(0,1,1,1,1);
  offset                      = 4;
  if ( available[led_offset(0)+2]==1 || available[led_offset(0)+3]==1 ) 
    assigns[offset+2]         = 1;
  if ( available[led_offset(1)+2]==1 || available[led_offset(1)+3]==1 ) 
    assigns[offset+3]         = 1;
  if ( available[led_offset(2)+2]==1 || available[led_offset(2)+3]==1 ) 
    assigns[offset+4]         = 1;
  if ( available[led_offset(3)+2]==1 || available[led_offset(3)+3]==1 ) 
    assigns[offset+5]         = 1;
  offset                      = 10;
  if ( available[btn_offset(0)]==1 ) 
    assigns[offset+3]         = 1;
  if ( available[btn_offset(1)]==1 ) 
    assigns[offset+4]         = 1;
  if ( available[btn_offset(2)]==1 ) 
    assigns[offset+5]         = 1;
  obj_avail                   = 1;
}
//  MESSAGE                          MESSAGE                              MESSAGE
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*


// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
//  ACTIVATE                           ACTIVATE                          ACTIVATE
void deactivate_obj(char count, int offset) {
  char*   pins                = pattrns[pattern+0];
  char*   available           = pattrns[pattern+1];
  char    i                   = 0,index,pin;
  while ( i < count ) {
    index                     = i+offset;
    if ( available[index]==1 ) 
      if ( activated[index]!=0 ) {
        pin                   = pins[index];
        file_write("/sys/class/gpio/unexport",asciipins[pin],digits(pin));
        activated[index]      = 0;
      }
    i                         = i+1;
  }
}
char activate_obj(char count, int offset, char *dir) {
  char*   pins                = pattrns[pattern+0];
  char*   available           = pattrns[pattern+1];
  char*   asciipin;
  char    i                   = 0,
          availabl            = 0,len,index,pin;
  char    path[MAX_PATH];
  while ( count > i++ ) {
    index                     = i+offset-1;
    if ( available[index]==1 ) {
      if ( activated[index]!=1 ) {
        pin                   = pins[index];
        asciipin              = asciipins[pin];
        file_write("/sys/class/gpio/export",asciipin,digits(pin));
        len                   = 2;
        if(dir[0]==111)
          len                 = 3;
        assemble_path(path,"/sys/class/gpio/gpio",20,asciipin,digits(pin),"/direction",10);
        file_write(path,dir,len);
        activated[index]      = 1;
      }
      availabl                = 1;
    }
  }
  return availabl;
}
char activate_led(char ledNum){
  char    dir[4]              = {   111 ,  117 ,  116 ,    0 } ;                       // 'out'
  return activate_obj(four,led_offset(ledNum),dir);
}
void deactivate_led(char ledNum){
  deactivate_obj(four,led_offset(ledNum));
}
char activate_btn(char btnNum){
  char    dir[4]              = {   105 ,  110 ,    0 ,    0 } ;                       // 'in'
  return activate_obj(one,btn_offset(btnNum),dir);
}
void deactivate_btn(char btnNum){
  deactivate_obj(one,btn_offset(btnNum));
}
void activate(char obj, char num){
  char    buf[64];
  char    availabl;
  int     objix               = obj;
  if ( obj==OBJ_LED || obj==OBJ_BTN ) {
    if ( verbose==1 ) {
      sprintf(buf,"activate, %s == %d",nouns[objix],num);
      err_print(buf);
    }
    if ( obj==OBJ_LED ) {
      availabl                = activate_led(num);
    } else {
      availabl                = activate_btn(num);
    }
    if ( availabl==0 ) {
      sprintf(buf,"%s%d is not available",nouns[objix],num);
      err_print(buf);
    }
  }
}
void deactivate(char obj, char num){
  char    buf[64];
  int     objix               = obj;
  if ( obj==OBJ_LED || obj==OBJ_BTN ) {
    if(verbose==1){
      sprintf(buf,"deactivate, %s == %d",nouns[objix],num);
      err_print(buf);
    }
    if ( obj==OBJ_LED ) {
      deactivate_led(num);
    }else{
      deactivate_btn(num);
    }
  }
}
//  ACTIVATE                           ACTIVATE                          ACTIVATE
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*

// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
//  LED / BTN                         LED / BTN                         LED / BTN
void set_led_pin(int index, char val, char* choices){
  char*   pins                = pattrns[pattern+0];
  char*   available           = pattrns[pattern+1];
  char    pin                 = pins[index],
          n                   = 0;
  char    asciival[2]         = { ZEROTXT ,  0 };
  char    path[MAX_PATH];
  char*   asciipin            = asciipins[pin];
  if ( available[index]==1 && activated[index]==1 ) {
    if ( member_of(val,choices,four)==1 ){
      n                       = 1;
      asciival[0]             = ONETXT;
    }
    if(vals[index]!=n){
      asciipin              = asciipins[pin];
      assemble_path(path,"/sys/class/gpio/gpio",20,asciipin,digits(pin),"/value",6);
      file_write(path,asciival,one);
      vals[index]             = n;
    }
  }
}
void set_led(char ledNum, char val){
  int     ledOffset           = led_offset(ledNum);
  set_led_pin(ledOffset+0,val,test1);
  set_led_pin(ledOffset+3,val,test4);
  set_led_pin(ledOffset+1,val,test2);
  set_led_pin(ledOffset+2,val,test3);
}
void get_btn(char btnNum){
  char*   available           = pattrns[pattern+1];
  char*   pins                = pattrns[pattern+0];
  char*   asciipin;
  char    btnOffset           = btn_offset(btnNum),
          pin                 = pins[btnOffset];
  char    result[10];
  char    path[MAX_PATH];
  if ( available[btnOffset]==1 && activated[btnOffset]==1 ) {
    asciipin                  = asciipins[pin];
    assemble_path(path,"/sys/class/gpio/gpio",20,asciipin,digits(pin),"/value",6);
    file_read(path,result,one);
    if(result[0]==ZEROTXT||result[0]==ONETXT){
      vals[btnOffset]         = result[0]-ZEROTXT;
    }
  }
}
void read_btn(char btnNum){
  get_btn(btnNum);
}
void read_btns(){
  get_btn(0);
  get_btn(1);
  get_btn(2);
}
//  LED / BTN                         LED / BTN                         LED / BTN
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*



// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
//  INITIALIZE                       INITIALIZE                        INITIALIZE
void initialize_system(){
  char    (*initable)[2]      = initables [ pattern / 2 ];
  char*   pair;
  char    i                   = 0,
          lim                 = 6;
  while ( i < lim ) {
    pair                      = initable[i++];
    if ( pair[0]==OBJ_LED || pair[0]==OBJ_BTN ) {
      activate ( pair[0] , pair[1] );
    }
  }
  initialized                 = 1;
}
//  INITIALIZE                       INITIALIZE                        INITIALIZE
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*


void pre_parse_arguments(int argc, char** argv){
  char    offset              = 0,
          i                   = 0,
          l                   = argc,
          ll                  = 12,
          maybe               = 1,ii,aiii;
  char*   ai;
  argc_                       = argc;
  while(l>i){
    if(strlen(argv[i])==7||strlen(argv[i])==12){
      ai                      = argv[i];
      if ( is_verbose(ai)==1 ) {
        verbose               = 1;
        offset                = shift_indices(i+offset);
      }
      if ( strlen(ai)==12 && recognized_pattern(ai[0])==1 ) {
        ii                    = 0;
        state[ii++]           = ai[0];
        while ( maybe==1 && ll>ii ) {
          aiii                = ai[ii];
          if ( ( aiii>47 && aiii<58 ) || ( aiii>96 && aiii<119 ) ) {
            state[ii]         = b36tweens[aiii];                         // Strips off higher-order bits
          }else{
            maybe             = 0;
          }
          if ( maybe==1 && ii==ll-1 ) {
            if ( oct_subpart(aiii,oct_ones)==1 )
              initialized     = 1;
            if ( oct_subpart(aiii,oct_twos)==1 )
              btn_pins        = 1;
            if ( oct_subpart(aiii,oct_fours)==1 )
              obj_avail       = 1;
          }
          ii                  = ii + 1;
        }
        if ( maybe==1 ) {
          statestate          = STATE_IN;
          offset              = shift_indices(i+offset);
          set_pattern_from_state0();
        }else{
          statestate          = STATE_BAD;
        }
      }
    }
    i                         = i+1;
  }
}
char parse_arguments(int argc, char** argv){
  char    cmd                 = CMD_EMPTY,
          hasState            = 0,
          shim                = 0,argv20;
  char*   arg2                = "no";
  char*   arg3;

  pre_parse_arguments(argc,argv);

  if ( argc_>1 )
    cmd                       = select_from_list(argv[indices[1]],commands,CMD_EMPTY,COMMANDCOUNT);

  if ( cmd==CMD_INITIALIZE ) {
    cmd                       = CMD_ERROR_SYNTAX;
      if ( argc_>2 && strlen(argv[2])==1 ) {
        argv20                = argv[2][0];
        if ( recognized_pattern(argv20)==1 || argv20==ZEROTXT ) {
          cmd                 = CMD_INITIALIZE;
          if ( argv20==ZEROTXT ) 
            shim              = eight;
          state[0]            = shim + argv20;
          set_pattern_from_state0();
        }
      }
  } else {
    if ( recognized_command(cmd)==1 && statestate==STATE_IN && initialized==1 ) {
      if ( argc_>2 ) 
        arg2                  = argv[indices[2]];
      if ( cmd==CMD_READALL || strlen(arg2)==4 ) {
        set_obj_type(cmd,arg2);
        if ( objTyp==OBJ_LED || objTyp==OBJ_BTN )
          hasState            = parse_state(state);
      }
    } else {
      cmd                     = CMD_ERROR_SYNTAX;
      if ( statestate!=STATE_IN || initialized!=1 ) {
        cmd                   = CMD_NOT_INIT;
        if ( statestate!=STATE_IN )
          cmd                 = CMD_NO_STATE;
      }
    }
    if ( cmd==CMD_WRITE && hasState==1 && argc_>3 ) {
      arg3                    = argv[indices[3]];
      colrVal                 = select_from_list(arg3,values,VAL_NULL,VALUECOUNT);
    }

    if ( hasState==0 && cmd!=CMD_NOT_INIT && cmd!=CMD_NO_STATE )
      cmd                     = CMD_ERROR_SYNTAX;
    if ( cmd==CMD_WRITE && colrVal==VAL_NULL ) 
      cmd                     = CMD_NO_WRITE_COLR;
  }
  return cmd;
}

int main(int argc, char **argv) {
  char    cmd                 = parse_arguments(argc,argv);
  int     i                   = -1,
          l                   = 4,
          info_bits           = 0,i2,i4;

  if ( cmd==CMD_INITIALIZE )     initialize_system();
  if ( cmd==CMD_ACTIVATE )       activate(objTyp,objNum);
  if ( cmd==CMD_DEACTIVATE )     deactivate(objTyp,objNum);
  if ( cmd==CMD_WRITE )          set_led(objNum,colrVal);
  if ( cmd==CMD_READ )           read_btn(objNum);
  if ( cmd==CMD_READALL )        read_btns();
  if ( cmd==CMD_NO_STATE )       fprintf(stderr,"NOT STATE RECEIVED \n\n\n");
  if ( cmd==CMD_NOT_INIT )       fprintf(stderr,"NOT INITIALIZED \n\n\n");
  if ( cmd==CMD_NO_WRITE_COLR )  fprintf(stderr,"NO WRITE COLOR \n\n\n");
  if ( cmd==CMD_ERROR_SYNTAX )   fprintf(stderr,"SYNTAX ERROR \n\n\n");

  if ( recognized_command(cmd) ) {

    if ( obj_avail==0 ) {
      msg_obj_avail();
    } else {
      if ( btn_pins==0 ) {
        msg_btn_pins();
      }
    }

    if ( initialized==1 )
      info_bits               = info_bits+1;
    if ( btn_pins==1 )
      info_bits               = info_bits+2;
    if ( obj_avail==1 )
      info_bits               = info_bits+4;
    state[11]                 = b36outs[info_bits];

    while ( l>i++ ) {
      i2                      = i+i;
      i4                      = i2+i2;
      state[i2+1]             = octal_value(activated,i4,i4);
      state[i2+2]             = octal_value(vals,i4,i4+2);
    } 
  }
  printf("%s",state);
}

