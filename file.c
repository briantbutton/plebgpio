
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
void short_file_read(char *path, char *buff, int len) {
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
  }else{
    buff[0]                   = CHAR_ERROR;
  }
}
char read_file_digit(char *dir, char *subdir, char *fname){
  char    buff[4]             = {   0 ,   0 ,   0 ,   0 };
  char    fullpath[MAX_PATH];
  char    result              = VAL_ERROR;

  if(bngpiodir_ok){
    assemble_path(fullpath,dir,subdir,fname);
    short_file_read(fullpath,buff,one);
    if( buff[0]!=CHAR_ERROR ){
      if( buff[0]<128 && buff[0]>0 ){
        result                = b62ins[buff[0]];
      }
    }
  }
  return result;
}
char verify_dir_exists(char *path){
  DIR     *dp                 = opendir(path);
  char    exists              = 0;
  if(dp){
    closedir(dp);
    exists                    = 1;
#if VERBOSE == 3
    printf("found directory exists '%s'\n",path);
#endif
  }else{
#if VERBOSE == 1 || VERBOSE == 2 || VERBOSE == 3
    printf("did not find directory '%s'\n",path);
#endif
  }
  return exists;
}
//   FILE                               FILE                                 FILE
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*
