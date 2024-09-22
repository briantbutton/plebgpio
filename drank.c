
/**
 * @brief open gpiochipX device for control of gpio pins via ioctl().
 * @param gpiodev path to gpiochipX in filesystem, e.g. "/dev/gpiochip0".
 * @return returns file descriptor for device on success, -1 otherwise.
 */
int gpio_dev_open (const char *gpiodev)
{
  int gpiofd;

  /* open gpiochipX */
  if ((gpiofd = open (gpiodev, O_RDONLY)) < 0) {
    perror ("open-GPIOCHIP");
  }

  return gpiofd;
}


/**
 * @brief closes gpio file descriptor associated with "/dev/gpiochipX"
 * @param gpiofd file descriptor retured by previos call to gpio_dev_open().
 * @return returns 0 on success, -1 otherwise.
 */
int gpio_dev_close (int gpiofd)
{
  if (close (gpiofd) < 0) {         /* close gpiochipX fd */
    perror ("close-gpiofd");
    return -1;
  }

  return 0;
}


/**
 * @brief configure ioctl for gpio control using ABI V2 access.
 * @param gpio gpio v2_struct holding pointers to linecfg and linereq
 * structs and open file descriptor returned by prior call to
 * gpio_dev_open()..
 * linecfg - v2 ioctl config struct for gpio pins (lines).
 * linereq - v2 ioctl request struct member of linecfg containing
 * separate file-descriptor used for read/write access of configured
 * gpio pins (lines).
 * @return returns 0 on success, -1 otherwise.
 */
int gpio_line_cfg_ioctl (gpio_v2_t *gpio)
{
  /* get ioctl values for line request */
  if (ioctl (gpio->fd, GPIO_V2_GET_LINE_IOCTL, gpio->linereq) < 0) {
    perror ("ioctl-GPIO_V2_GET_LINE_IOCTL");
    return -1;
  }

  /* set the line config for the retured linereq file descriptor */
  if (ioctl (gpio->linereq->fd, GPIO_V2_LINE_SET_CONFIG_IOCTL, gpio->linecfg) < 0) {
    perror ("ioctl-GPIO_V2_LINE_SET_CONFIG_IOCTL");
    return -1;
  }

  return 0;
}


/**
 * @brief print the GPIO number and function for each GPIO line (pin) in
 * a 3-column format.
 * @param gpiofd open file descriptor for gpiochipX returned from prior call
 * to gpio_dev_open().
 * @return returns 0 on success, -1 otherwise.
 */
int prn_gpio_v2_ghip_info (int gpiofd)
{
  __u8  rows = 0;
  struct gpiochip_info chip_info = { .name = "" };

  /* gpio_v2 ioctl call to get chip information */
  if (ioctl (gpiofd, GPIO_GET_CHIPINFO_IOCTL, &chip_info) == -1) {
    perror ("ioctl-GPIO_GET_CHIPINFO_IOCTL");
    return -1;
  }

  /* validate lines returned (integer division intentional) */
  if ((rows = chip_info.lines / INFOCOLS) == 0) {
    fputs ("error: GPIO_GET_CHIPINFO_IOCTL no GPIO lines.\n", stderr);
    return -1;
  }

#if VERBOSE == 3
__u8 remstart = rows * INFOCOLS;    /* compute no. of rows to print at end */

  /* output chip information */
  printf ("\nGPIO chip information\n\n"
          "  name  : %s\n"
          "  label : %s\n"
          "  lines : %u\n\n",
          chip_info.name, chip_info.label, chip_info.lines);

  /* loop producing output of gpios in 3-column format */
  for (__u8 r = 0; r < rows; r++) {
    for (__u32 c = 0; c < INFOCOLS; c++) {
      __u32 chip = r * INFOCOLS + c;
      struct gpio_v2_line_info line_info = { .name = "", .consumer = "", .offset = chip };
      if (ioctl (gpiofd, GPIO_V2_GET_LINEINFO_IOCTL, &line_info) == -1) {
        perror ("ioctl-GPIO_GET_LINEINFO_IOCTL");
        fprintf (stderr, "Failed getting line %u info.\n", chip);
        continue;
      }
      printf ("  %2hhu :  %-18s", chip, line_info.name);
    }
    putchar ('\n');
  }

  /* output any remaining lines (e.g. r * c <= line < chip_info.lines) */
  for (__u32 c = remstart; c < chip_info.lines; c++) {
    struct gpio_v2_line_info line_info = { .name = "", .consumer = "", .offset = c };
    if (ioctl (gpiofd, GPIO_V2_GET_LINEINFO_IOCTL, &line_info) == -1) {
      perror ("ioctl-GPIO_GET_LINEINFO_IOCTL");
      fprintf (stderr, "Failed getting line %u info.\n", c);
      continue;
    }
    printf ("  %2hhu :  %-18s", c, line_info.name);
  }
  puts ("\n");    /* tidy up with an additional (2) newlines */
#endif

  return 0;
}


/**
 * @brief write gpio pin (line) values (HI/LO) from bits set or clearned
 * in gpio->linevals->bits for pins with bit set high in gpio->linevals->mask
 * from mask using gpio_v2 ioct line request.
 * @param gpio gpio v2_t struct holding linereq with open linereq file
 * descriptor set by prior call to gpio_line_cfg_ioctl() used to write
 * linevals to gpio pin index(s) in linereq->offsets specified by bits HI
 * in mask.
 * @param mask bitmap with bits 1 (HI) that correspond to index in
 * gpio->linereq->offsets pin array that will be set.
 * @param bits gpio value to write (0 - LO, 1 - HI) to set bit in
 * linereq->bits for bits specified in mask.
 * @return returns 0 on success, -1 otherwise.
 */
int gpio_line_set_values (gpio_v2_t *gpio, __u64 bits,  __u64 mask)
{
  /* set or clear linereq .bits pin values (bitmap of pin values set to
   * bits that correspond to pin bits (bitmap indexes) set HI in mask.
   */
  // if ((bits & mask) > 0) {
  //   gpio->linevals->bits |= mask;
  // } else {
  //   gpio->linevals->bits &= (0xffffffffffffffff & ~mask);
  // }
  gpio->linevals->bits = bits;                    // new line

  /* set linevals mask to mask */
  gpio->linevals->mask = mask;
  /* set GPIO pin value to bit in lineval->bits (0 or 1) for pins with
   * bit == 1 in mask.
   */
  if (ioctl (gpio->linereq->fd, GPIO_V2_LINE_SET_VALUES_IOCTL, gpio->linevals) < 0) {
    perror ("ioctl-GPIO_V2_LINE_SET_VALUES_IOCTL-1");
    return -1;
  }
  return 0;
}


/**
 * @brief write gpio pin (line) values (HI/LO) from bits set or clearned
 * in gpio->linevals->bits for pins with bit set high in gpio->linevals->mask
 * from mask using gpio_v2 ioct line request.
 * @param gpio gpio v2_t struct holding linereq with open linereq file
 * descriptor set by prior call to gpio_line_cfg_ioctl() used to write
 * linevals to gpio pin index(s) in linereq->offsets specified by bits HI
 * in mask.
 * @param mask bitmap with bits 1 (HI) that correspond to index in
 * gpio->linereq->offsets pin array that will be set.
 * @param bits gpio value to write (0 - LO, 1 - HI) to set bit in
 * linereq->bits for bits specified in mask.
 * @return returns 0 on success, -1 otherwise.
 */
int gpio_line_get_values (gpio_v2_t *gpio, __u64 bits,  __u64 mask) {

  gpio->linevals->bits = bits;
  gpio->linevals->mask = mask;
  if (ioctl (gpio->linereq->fd, GPIO_V2_LINE_GET_VALUES_IOCTL, gpio->linevals) < 0) {
    perror ("ioctl-GPIO_V2_LINE_GET_VALUES_IOCTL-1");
    return -1;
  }
  bits   = gpio->linevals->bits;
  if ( bits!=old_bits ) {
#if VERBOSE == 3
    if ( ( bits & bits ) > 0 ){
      printf("'bits' is non-zero   %d\n",bits);
    }else{
      printf("'bits' is zero   %d\n",bits);
    }
#endif
    old_bits = bits;
  }
  return old_bits;
}


/**
 * @brief closes the open line request file descriptor for v2 linereq.
 * @param gpio gpio v2 data struct holding line request struct with open
 * linereq file descriptor opened by prior call to gpio_line_cfg_ioctl().
 * @return returns 0 on success, -1 otherwise.
 */
int gpio_line_close_fd (gpio_v2_t *gpio)
{
  if (close (gpio->linereq->fd) < 0) {    /* close linereq fd */
    perror ("close-linereq.fd");
    return -1;
  }

  return 0;
}

