
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
    buff[0]                   = 126;                               // '~'
  }
}
char read_file_bit(char *dir, char *subdir, char *fname){
  char    buff[8];
  char    fullpath[MAX_PATH];
  char    result              = 0;
  char    found               = 0;

  if(bngpiodir_ok){
    assemble_path(fullpath,dir,subdir,fname);
    short_file_read(fullpath,buff,one);
    if( buff[0]!=126 ){
      found                   = 1;
      if( buff[0]==ZEROTXT || buff[0]==ONETXT ){
        result                = buff[0]-ZEROTXT;
      }
    }
  }
  if( found==1 ){
    if( result==1 ){
      printf("'%s' has value true\n",fname);
    }else{
      printf("'%s' has value false\n",fname);
    }
  }else{
    printf("'%s' not found (%s)\n",fname,fullpath);
  }
  return result;
}
char read_file_digit(char *dir, char *subdir, char *fname){
  char    buff[8];
  char    fullpath[MAX_PATH];
  char    result              = 0;
  char    found               = 0;

  if(bngpiodir_ok){
    assemble_path(fullpath,dir,subdir,fname);
    short_file_read(fullpath,buff,one);
    if( buff[0]!=126 ){
      found                   = 1;
      if( buff[0]<128 && buff[0]>0 ){
        result                = b36ins[buff[0]];
      }
    }
  }
  if( found==1 ){
    if (result!=program) printf("'%s' has value %d\n",fname,result);
  }else{
    // printf("'%s' not found  (%s)\n",fname,fullpath);
  }
  return result;
}
void read_file_digits(char *dir, char *subdir, char *fname, char *result){
  char    buff[8];
  char    fullpath[MAX_PATH];

  if(bngpiodir_ok){
    assemble_path(fullpath,dir,subdir,fname);
    short_file_read(fullpath,buff,five);
    if( buff[0]!=126 ){
      if( buff[0]<128 && buff[0]>0 && b36ins[buff[0]] < 36){
        result[0]             = b36ins[buff[0]];
      }
      if( buff[1]<128 && buff[1]>0 && b36ins[buff[1]] < 36){
        result[1]             = b36ins[buff[1]];
      }
      if( buff[2]<128 && buff[2]>0 && b36ins[buff[2]] < 36){
        result[2]             = b36ins[buff[2]];
      }
      if( buff[3]<128 && buff[3]>0 && b36ins[buff[3]] < 36){
        result[3]             = b36ins[buff[3]];
      }
      if( buff[4]<128 && buff[4]>0 && b36ins[buff[4]] < 36){
        result[4]             = b36ins[buff[4]];
      }
    }
  }
}
char verify_dir_exists(char *path){
  DIR     *dp                 = opendir(path);
  char    exists              = 0;
  if(dp){
    closedir(dp);
    exists                    = 1;
    printf("found directory exists '%s'\n",path);
  }else{
    printf("did not find directory '%s'\n",path);
  }
  return exists;
}
//   FILE                               FILE                                 FILE
// *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-* *-*=*  *=*-*  *=*-*  *=*-*

