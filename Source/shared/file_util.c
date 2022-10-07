#include "options.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>
#ifdef pp_SLICETEST
#include <fcntl.h>
#ifdef pp_OSXLINUX
#include <sys/mman.h>
#endif
#endif
#ifdef pp_OSX
#include <unistd.h>
#endif
#include <math.h>
#ifdef WIN32
#ifdef __MINGW32__
#undef S_IFBLK
#undef S_ISBLK
#undef S_ISFIFO
#undef S_ISDIR
#undef S_ISCHR
#undef S_ISREG
#endif
#include <io.h>
#include <direct.h>
#include <dirent_win.h>
#else
#include <dirent.h>
#endif
#include "MALLOCC.h"

FILE *alt_stdout=NULL;

#ifdef pp_SLICETEST
#ifdef WIN32
#include <io.h>
#include <windows.h>
#include <sys/types.h>

/* mmap() replacement for Windows
 *
 * Author: Mike Frysinger <vapier@gentoo.org>
 * Placed into the public domain
 */

/* References:
 * CreateFileMapping: http://msdn.microsoft.com/en-us/library/aa366537(VS.85).aspx
 * CloseHandle:       http://msdn.microsoft.com/en-us/library/ms724211(VS.85).aspx
 * MapViewOfFile:     http://msdn.microsoft.com/en-us/library/aa366761(VS.85).aspx
 * UnmapViewOfFile:   http://msdn.microsoft.com/en-us/library/aa366882(VS.85).aspx
 */


#define PROT_READ     0x1
#define PROT_WRITE    0x2
/* This flag is only available in WinXP+ */
#ifdef FILE_MAP_EXECUTE
#define PROT_EXEC     0x4
#else
#define PROT_EXEC        0x0
#define FILE_MAP_EXECUTE 0
#endif

#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#define MAP_ANON      MAP_ANONYMOUS
#define MAP_FAILED    ((void *) -1)

#ifdef __USE_FILE_OFFSET64
# define DWORD_HI(x) (x >> 32)
# define DWORD_LO(x) ((x) & 0xffffffff)
#else
# define DWORD_HI(x) (0)
# define DWORD_LO(x) (x)
#endif

/* ------------------  mmap ------------------------ */

static void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
  if(prot&~(PROT_READ|PROT_WRITE|PROT_EXEC))
    return MAP_FAILED;
  if(fd==-1) {
    if(!(flags&MAP_ANON)||offset)
      return MAP_FAILED;
  }
  else if(flags&MAP_ANON)
    return MAP_FAILED;

  DWORD flProtect;
  if(prot&PROT_WRITE) {
    if(prot&PROT_EXEC)
      flProtect = PAGE_EXECUTE_READWRITE;
    else
      flProtect = PAGE_READWRITE;
  }
  else if(prot&PROT_EXEC) {
    if(prot&PROT_READ)
      flProtect = PAGE_EXECUTE_READ;
    else if(prot&PROT_EXEC)
      flProtect = PAGE_EXECUTE;
  }
  else
    flProtect = PAGE_READONLY;

  off_t end = length+offset;
  HANDLE mmap_fd, h;
  if(fd==-1)
    mmap_fd = INVALID_HANDLE_VALUE;
  else
    mmap_fd = (HANDLE)_get_osfhandle(fd);
  h = CreateFileMapping(mmap_fd, NULL, flProtect, DWORD_HI(end), DWORD_LO(end), NULL);
  if(h==NULL)
    return MAP_FAILED;

  DWORD dwDesiredAccess;
  if(prot&PROT_WRITE)
    dwDesiredAccess = FILE_MAP_WRITE;
  else
    dwDesiredAccess = FILE_MAP_READ;
  if(prot&PROT_EXEC)
    dwDesiredAccess |= FILE_MAP_EXECUTE;
  if(flags&MAP_PRIVATE)
    dwDesiredAccess |= FILE_MAP_COPY;
  void *ret = MapViewOfFile(h, dwDesiredAccess, DWORD_HI(offset), DWORD_LO(offset), length);
  if(ret==NULL) {
    CloseHandle(h);
    ret = MAP_FAILED;
  }
  return ret;
}

/* ------------------  munmap ------------------------ */

static void munmap(void *addr, size_t length)
{
  UnmapViewOfFile(addr);
  /* ruh-ro, we leaked handle from CreateFileMapping() ... */
}

#undef DWORD_HI
#undef DWORD_LO
#endif

/* ------------------  MemMap ------------------------ */

char *MemMap(char *file, size_t *size){
   int fd;
   char *ptr;

   if(file==NULL)return NULL;
   fd = open(file, O_RDONLY);
   struct stat statbuf;
   int err = fstat(fd, &statbuf);
   *size = statbuf.st_size;
   ptr = mmap(NULL, *size, PROT_READ|PROT_WRITE,MAP_SHARED, fd,0);
   if(ptr==MAP_FAILED)return NULL;
   return ptr;
}

/* ------------------  MemUnMap ------------------------ */

void MemUnMap(char *data, size_t size){
  munmap(data, size);
}
#endif

/* ------------------ TestWrite ------------------------ */

void TestWrite(char *scratchdir, char **fileptr){
  char *file, filecopy[1024], newfile[1024], *beg;

  if(Writable(".")==1||fileptr==NULL||scratchdir==NULL)return;
  strcpy(filecopy, *fileptr);
  FREEMEMORY(*fileptr);

  strcpy(newfile, scratchdir);
  strcat(newfile, dirseparator);
  file = filecopy;
  beg = strrchr(filecopy, SEP);
  if(beg!=NULL)file = beg+1;
  strcat(newfile, file);
  NewMemory((void **)fileptr, strlen(newfile)+1);
  strcpy(*fileptr, newfile);
}

/* ------------------ FFLUSH ------------------------ */

int FFLUSH(void){
  int return_val=0;

  if(alt_stdout!=NULL){
    return_val = fflush(alt_stdout);
  }
  return return_val;
}

/* ------------------ PRINTF ------------------------ */

int PRINTF(const char * format, ...){
  va_list args;
  int return_val=0;

  if(alt_stdout!=NULL){
    va_start(args, format);
    return_val=vfprintf(alt_stdout, format, args);
    va_end(args);
  }
  return return_val;
}

/* ------------------ SetStdOut ------------------------ */

void SetStdOut(FILE *stream){
  alt_stdout=stream;
}

#define FILE_BUFFER 1000

/* ------------------ CopyFile ------------------------ */

void CopyFILE(char *destdir, char *file_in, char *file_out, int mode){
  char buffer[FILE_BUFFER];
  FILE *streamin=NULL, *streamout=NULL;
  char *full_file_out=NULL;
  size_t chars_in;

  if(destdir==NULL||file_in==NULL)return;
  streamin=fopen(file_in,"rb");
  if(streamin==NULL)return;

  full_file_out=NULL;
  NewMemory((void **)&full_file_out,strlen(file_out)+strlen(destdir)+1+1);
  strcpy(full_file_out,destdir);
  if(destdir[strlen(destdir)-1]!=*dirseparator){
    strcat(full_file_out,dirseparator);
  }
  strcat(full_file_out,file_out);

  if(mode==REPLACE_FILE){
    streamout=fopen(full_file_out,"wb");
  }
  else if(mode==APPEND_FILE){
    streamout=fopen(full_file_out,"ab");
  }
  else{
    ASSERT(0);
  }

  if(streamout==NULL){
    FREEMEMORY(full_file_out);
    fclose(streamin);
    return;
  }
  PRINTF("  Copying %s to %s\n",file_in,file_out);
  for(;;){
    int end_of_file;

    end_of_file=0;
    chars_in=fread(buffer,1,FILE_BUFFER,streamin);
    if(chars_in!=FILE_BUFFER)end_of_file=1;
    if(chars_in>0)fwrite(buffer,chars_in,1,streamout);
    if(end_of_file==1)break;
  }
  FREEMEMORY(full_file_out);
  fclose(streamin);
  fclose(streamout);
}

/* ------------------ HaveProg ------------------------ */

int HaveProg(char *prog){
  if(system(prog) == 0)return 1;
  return 0;
}

/* ------------------ GetSmokeZipPath ------------------------ */

char *GetSmokeZipPath(char *progdir){
  char *zip_path;

  if(progdir!=NULL){
    NewMemory((void **)&zip_path,strlen(progdir)+20);
    strcpy(zip_path,progdir);
  }
  else{
    NewMemory((void **)&zip_path,2+20);
    strcpy(zip_path,".");
    strcat(zip_path,dirseparator);
  }

  strcat(zip_path,"smokezip");
#ifdef WIN32
  strcat(zip_path,".exe");
#endif
  if(FILE_EXISTS(zip_path)==YES)return zip_path;
  FREEMEMORY(zip_path);
  return NULL;
}

/* ------------------ SetDir ------------------------ */

char *SetDir(char *argdir){
  int lendir;
  char *dir;

  lendir=strlen(argdir);
  NewMemory((void **)&dir,lendir+2);
  strcpy(dir,argdir);
  if(dir[lendir-1]!=dirseparator[0]){
    strcat(dir,dirseparator);
  }
  return dir;
}

/* ------------------ GetBaseFileName ------------------------ */

char *GetBaseFileName(char *buffer,char *file){
  char *filebase,*ext;

  strcpy(buffer,file);
#ifdef WIN32
  filebase=strrchr(buffer,'\\');
#else
  filebase=strrchr(buffer,'/');
#endif
  if(filebase==NULL){
    filebase=buffer;
  }
  else{
    filebase++;
  }
  ext = strrchr(filebase,'.');
  if(ext!=NULL)*ext=0;
  return filebase;
}

/* ------------------ GetFileName ------------------------ */

char *GetFileName(char *temp_dir, char *file, int force_in_temp_dir){
  char *file2;
  char *file_out=NULL;
  FILE *stream=NULL;

  TrimBack(file);
  file2=TrimFront(file);
  if(force_in_temp_dir==NOT_FORCE_IN_DIR){
    stream=fopen(file2,"r");
    if(Writable(".")==YES||stream!=NULL){
      NewMemory((void **)&file_out,strlen(file2)+1);
      strcpy(file_out,file2);
    }
    if(stream!=NULL)fclose(stream);
  }
  if(file_out==NULL&&temp_dir!=NULL&&Writable(temp_dir)==YES){
    NewMemory((void **)&file_out,strlen(temp_dir)+1+strlen(file2)+1);
    strcpy(file_out, "");
    if(strcmp(temp_dir, ".")!=0){
      strcat(file_out, temp_dir);
      strcat(file_out, dirseparator);
    }
    strcat(file_out, file);
  }
  return file_out;
}

/* ------------------ FullFile ------------------------ */

void FullFile(char *file_out, char *dir, char *file){
  char *file2;

  TrimBack(file);
  file2=TrimFront(file);
  strcpy(file_out,"");
  if(dir!=NULL)strcat(file_out,dir);
  strcat(file_out,file2);
}

/* ------------------ StreamCopy ------------------------ */

unsigned int StreamCopy(FILE *stream_in, FILE *stream_out, int flag){
  int c;
  unsigned int nchars = 0;

  if(stream_in == NULL || stream_out == NULL)return 0;

  rewind(stream_in);
  c = fgetc(stream_in);
  while(c != EOF){
    if(flag == 0)return 1;
    fputc(c, stream_out);
    c = fgetc(stream_in);
    nchars++;
  }
  return nchars;
}

/* ------------------ FileCopy ------------------------ */

void FileCopy(char *file_in, char *file_out){
  FILE *stream_in, *stream_out;
  int c;

  if(file_in == NULL || file_out == NULL)return;
  stream_in = fopen(file_in, "rb");
  if(stream_in == NULL)return;
  stream_out = fopen(file_out, "wb");
  if(stream_out == NULL){
    fclose(stream_in);
    return;
  }

  c = fgetc(stream_in);
  while(c != EOF){
    fputc(c, stream_out);
    c = fgetc(stream_in);
  }
  fclose(stream_in);
  fclose(stream_out);
}

  /* ------------------ FileCat ------------------------ */

int FileCat(char *file_in1, char *file_in2, char *file_out){
  char buffer[FILE_BUFFER];
  FILE *stream_in1, *stream_in2, *stream_out;
  int chars_in;

  if(file_in1==NULL||file_in2==NULL)return -1;
  if(file_out==NULL)return -2;

  stream_in1=fopen(file_in1,"r");
  if(stream_in1==NULL)return -1;

  stream_in2=fopen(file_in2,"r");
  if(stream_in2==NULL){
    fclose(stream_in1);
    return -1;
  }

  stream_out=fopen(file_out,"w");
  if(stream_out==NULL){
    fclose(stream_in1);
    fclose(stream_in2);
    return -2;
  }

  for(;;){
    int end_of_file;

    end_of_file=0;
    chars_in=fread(buffer,1,FILE_BUFFER,stream_in1);
    if(chars_in!=FILE_BUFFER)end_of_file=1;
    if(chars_in>0)fwrite(buffer,chars_in,1,stream_out);
    if(end_of_file==1)break;
  }
  fclose(stream_in1);

  for(;;){
    int end_of_file;

    end_of_file=0;
    chars_in=fread(buffer,1,FILE_BUFFER,stream_in2);
    if(chars_in!=FILE_BUFFER)end_of_file=1;
    if(chars_in>0)fwrite(buffer,chars_in,1,stream_out);
    if(end_of_file==1)break;
  }
  fclose(stream_in2);
  fclose(stream_out);
  return 0;

}

/* ------------------ MakeOutFile ------------------------ */

void MakeOutFile(char *outfile, char *destdir, char *file1, char *ext){
  char filename_buffer[1024], *file1_noext;

  TrimBack(file1);
  strcpy(filename_buffer,TrimFront(file1));
  file1_noext=strstr(filename_buffer,ext);
  strcpy(outfile,"");
  if(file1_noext==NULL)return;
  file1_noext[0]='\0';
  if(destdir!=NULL){
    strcpy(outfile,destdir);
  }
  strcat(outfile,filename_buffer);
  strcat(outfile,"_diff");
  strcat(outfile,ext);
}


/* ------------------ Writable ------------------------ */

int Writable(char *dir){

  // returns 1 if the directory can be written to, 0 otherwise

  if(dir == NULL || strlen(dir) == 0)return NO;

#ifdef pp_LINUX
  if(ACCESS(dir,F_OK|W_OK)==-1){
    return NO;
  }
  else{
    return YES;
  }
#else
  {
    char tempfullfile[100], tempfile[40];
    FILE *stream;

    strcpy(tempfullfile,dir);
    strcat(tempfullfile,dirseparator);
    RandStr(tempfile,35);
    strcat(tempfullfile,tempfile);
    stream = fopen(tempfullfile,"w");
    if(stream==NULL){
      UNLINK(tempfullfile);
      return NO;
    }
    else{
      fclose(stream);
      UNLINK(tempfullfile);
      return YES;
    }
  }
#endif
}

/* ------------------ IfFirstLineBlank ------------------------ */

int IfFirstLineBlank(char *file){

  // returns 1 if first line of file is blank

  STRUCTSTAT statbuff1;
  int statfile1;
  FILE *stream = NULL;
  char buffer[255], *buffptr;

  if(file==NULL)return 1;

  statfile1 = STAT(file, &statbuff1);
  if(statfile1!=0)return 1;

  stream = fopen(file, "r");
  if(stream==NULL||fgets(buffer, 255, stream)==NULL){
    if(stream!=NULL)fclose(stream);
    return 1;
  }
  fclose(stream);
  buffptr = TrimFrontBack(buffer);
  if(strlen(buffptr)==0)return 1;
  return 0;
}

/* ------------------ IsFileNewer ------------------------ */

int IsFileNewer(char *file1, char *file2){

// returns 1 if file1 is newer than file2, 0 otherwise

  STRUCTSTAT statbuff1, statbuff2;
  int statfile1, statfile2;

  if(file1==NULL||file2==NULL)return -1;

  statfile1=STAT(file1,&statbuff1);
  statfile2=STAT(file2,&statbuff2);
  if(statfile1!=0||statfile2!=0)return -1;

  if(statbuff1.st_mtime>statbuff2.st_mtime)return 1;
  return 0;
}

  /* ------------------ GetFileInfo ------------------------ */

int GetFileInfo(char *filename, char *source_dir, FILE_SIZE *filesize){
  STRUCTSTAT statbuffer;
  int statfile;
  char buffer[1024];

  if(source_dir==NULL){
    strcpy(buffer,filename);
  }
  else{
    strcpy(buffer,source_dir);
    strcat(buffer,filename);
  }
  if(filesize!=NULL)*filesize=0;
  statfile=STAT(buffer,&statbuffer);
  if(statfile!=0)return statfile;
  if(filesize!=NULL)*filesize=statbuffer.st_size;
  return statfile;
}

/* ------------------ GetFileSizeSMV ------------------------ */

FILE_SIZE GetFileSizeSMV(const char *filename){
  STRUCTSTAT statbuffer;
  int statfile;
  FILE_SIZE return_val;

  return_val=0;
  if(filename==NULL)return return_val;
  statfile=STAT(filename,&statbuffer);
  if(statfile!=0)return return_val;
  return_val = statbuffer.st_size;
  return return_val;
}

/* ------------------ FileExistsOrig ------------------------ */

int FileExistsOrig(char *filename){
  if(ACCESS(filename, F_OK) == -1){
    return NO;
  }
  else{
    return YES;
  }
}

  /* ------------------ FileExists ------------------------ */

int FileExists(char *filename, filelistdata *filelist, int nfilelist, filelistdata *filelist2, int nfilelist2){

// returns YES if the file filename exists, NO otherwise

  if(filename == NULL)return NO;
  if(filelist != NULL&&nfilelist>0){
    if(FileInList(filename, filelist, nfilelist, filelist2, nfilelist2) != NULL){
      return YES;
    }
  }
  if(ACCESS(filename,F_OK)==-1){
    return NO;
  }
  else{
    return YES;
  }
}

/* ------------------ FreeFileList ------------------------ */

void FreeFileList(filelistdata *filelist, int *nfilelist){
  int i;

  for(i=0;i<*nfilelist;i++){
    FREEMEMORY(filelist[i].file);
  }
  FREEMEMORY(filelist);
  *nfilelist=0;
}

  /* ------------------ get_nfilelist ------------------------ */

int GetFileListSize(const char *path, char *filter){
  struct dirent *entry;
  DIR *dp;
  int maxfiles=0;

  dp = opendir(path);
  if(dp == NULL){
    perror("opendir");
    return 0;
  }
  while( (entry = readdir(dp))!=NULL ){
    if(((entry->d_type==DT_REG||entry->d_type==DT_UNKNOWN)&&MatchWild(entry->d_name,filter)==1))maxfiles++;
  }
  closedir(dp);
  return maxfiles;
}


/* ------------------ fopen_indir  ------------------------ */

FILE *fopen_indir(char *dir, char *file, char *mode){
  FILE *stream;

  if(file==NULL||strlen(file)==0)return NULL;
  if(dir==NULL||strlen(dir)==0){
    stream = fopen(file,mode);
  }
  else{
    char *filebuffer;
    int lenfile;

    lenfile = strlen(dir)+1+strlen(file)+1;
    NewMemory((void **)&filebuffer,lenfile*sizeof(char));
    strcpy(filebuffer,dir);
    strcat(filebuffer,dirseparator);
    strcat(filebuffer,file);
    stream = fopen(filebuffer,mode);
    FREEMEMORY(filebuffer);
  }
  return stream;
}

/* ------------------ CompareFileList ------------------------ */

int CompareFileList(const void *arg1, const void *arg2){
  filelistdata *x, *y;

  x = (filelistdata *)arg1;
  y = (filelistdata *)arg2;

  return strcmp(x->file, y->file);
}

/* ------------------ getfile ------------------------ */
filelistdata *FileInList(char *file, filelistdata *filelist, int nfiles, filelistdata *filelist2, int nfiles2){
  filelistdata *entry=NULL, fileitem;

  if(file==NULL)return NULL;
  fileitem.file = file;
  fileitem.type = 0;
  if(filelist!=NULL&&nfiles>0){
    entry = bsearch(&fileitem, (filelistdata *)filelist, (size_t)nfiles, sizeof(filelistdata), CompareFileList);
    if(entry!=NULL)return entry;
  }
  if(filelist2!=NULL&&nfiles2>0){
    entry = bsearch(&fileitem, (filelistdata *)filelist2, (size_t)nfiles2, sizeof(filelistdata), CompareFileList);
  }
  return entry;
}

/* ------------------ MakeFileList ------------------------ */

int MakeFileList(const char *path, char *filter, int maxfiles, int sort_files, filelistdata **filelist){
  struct dirent *entry;
  DIR *dp;
  int nfiles=0;
  filelistdata *flist;

  // DT_DIR - is a directory
  // DT_REG - is a regular file

  if (maxfiles == 0) {
    *filelist = NULL;
    return 0;
  }
  dp = opendir(path);
  if(dp == NULL){
    perror("opendir");
    *filelist=NULL;
    return 0;
  }
  NewMemory((void **)&flist,maxfiles*sizeof(filelistdata));
  while( (entry = readdir(dp))!=NULL&&nfiles<maxfiles ){
    if((entry->d_type==DT_REG||entry->d_type==DT_UNKNOWN)&&MatchWild(entry->d_name,filter)==1){
      char *file;
      filelistdata *flisti;

      flisti = flist + nfiles;
      NewMemory((void **)&file,strlen(entry->d_name)+1);
      strcpy(file,entry->d_name);
      flisti->file=file;
      flisti->type=0;
      nfiles++;
    }
  }
  closedir(dp);
  if(sort_files == YES&&nfiles>0){
    qsort((filelistdata *)flist, (size_t)nfiles, sizeof(filelistdata), CompareFileList);
  }
  *filelist=flist;
  return nfiles;
}

/* ------------------ GetFileSizeLabel ------------------------ */

void GetFileSizeLabel(int size, char *sizelabel){
  int leftsize,rightsize;

#define sizeGB   1000000000
#define size100MB 100000000
#define size10MB   10000000
#define sizeMB     1000000
#define size100KB    100000
#define size10KB      10000

  if(size>=sizeGB){
    size/=size10MB;
    leftsize=size/100;
    rightsize=size-100*leftsize;
    sprintf(sizelabel,"%i.%02i GB",leftsize,rightsize);
  }
  else if(size>=size100MB&&size<sizeGB){
    size/=sizeMB;
    leftsize=size;
    sprintf(sizelabel,"%i MB",leftsize);
  }
  else if(size>=size10MB&&size<size100MB){
    size/=size100KB;
    leftsize=size/10;
    rightsize=size-10*leftsize;
    sprintf(sizelabel,"%i.%i MB",leftsize,rightsize);
  }
  else if(size>=sizeMB&&size<size10MB){
    size/=size10KB;
    leftsize=size/100;
    rightsize=size-100*leftsize;
    sprintf(sizelabel,"%i.%02i MB",leftsize,rightsize);
  }
  else if(size>=size100KB&&size<sizeMB){
    size/=1000;
    leftsize=size;
    sprintf(sizelabel,"%i KB",leftsize);
  }
  else{
    size/=10;
    leftsize=size/100;
    rightsize=size-100*leftsize;
    sprintf(sizelabel,"%i.%02i KB",leftsize,rightsize);
  }
}

/* ------------------ GetFloatFileSizeLabel ------------------------ */

char *GetFloatFileSizeLabel(float size, char *sizelabel){
  int leftsize, rightsize;

  if(size>=sizeGB){
    size /= size10MB;
    leftsize = size/100;
    rightsize = size-100*leftsize;
    sprintf(sizelabel, "%i.%02i GB", leftsize, rightsize);
  }
  else if(size>=size100MB&&size<sizeGB){
    size /= sizeMB;
    leftsize = size;
    sprintf(sizelabel, "%i MB", leftsize);
  }
  else if(size>=size10MB&&size<size100MB){
    size /= size100KB;
    leftsize = size/10;
    rightsize = size-10*leftsize;
    sprintf(sizelabel, "%i.%i MB", leftsize, rightsize);
  }
  else if(size>=sizeMB&&size<size10MB){
    size /= size10KB;
    leftsize = size/100;
    rightsize = size-100*leftsize;
    sprintf(sizelabel, "%i.%02i MB", leftsize, rightsize);
  }
  else if(size>=size100KB&&size<sizeMB){
    size /= 1000;
    leftsize = size;
    sprintf(sizelabel, "%i KB", leftsize);
  }
  else{
    size /= 10;
    leftsize = size/100;
    rightsize = size-100*leftsize;
    sprintf(sizelabel, "%i.%02i KB", leftsize, rightsize);
  }
  return sizelabel;
}

/* ------------------ GetProgDir ------------------------ */

char *GetProgDir(char *progname, char **svpath){

// returns the directory containing the file progname

  char *progpath, *lastsep, *smokeviewpath2;

  lastsep=strrchr(progname,dirseparator[0]);
  if(lastsep==NULL){
    char *dir;

    dir = Which(progname);
    if(dir==NULL){
      NewMemory((void **)&progpath,(unsigned int)3);
      strcpy(progpath,".");
      strcat(progpath,dirseparator);
    }
    else{
      int lendir;

      lendir=strlen(dir);
      NewMemory((void **)&progpath,(unsigned int)(lendir+2));
      strcpy(progpath,dir);
      if(progpath[lendir-1]!=dirseparator[0])strcat(progpath,dirseparator);
    }
    NewMemory((void **)&smokeviewpath2,(unsigned int)(strlen(progpath)+strlen(progname)+1));
    strcpy(smokeviewpath2,progpath);
  }
  else{
    int lendir;

    lendir=lastsep-progname+1;
    NewMemory((void **)&progpath,(unsigned int)(lendir+1));
    strncpy(progpath,progname,lendir);
    progpath[lendir]=0;
    NewMemory((void **)&smokeviewpath2,(unsigned int)(strlen(progname)+1));
    strcpy(smokeviewpath2,"");;
  }
  strcat(smokeviewpath2,progname);
  *svpath=smokeviewpath2;
  return progpath;
}

/* ------------------ getprogdirabs ------------------------ */

#ifdef pp_LUA
char *getprogdirabs(char *progname, char **svpath){

// returns the absolute path of the directory containing the file progname
  char *progpath;
#ifdef WIN32
  NewMemory((void **)&progpath,_MAX_PATH);
  _fullpath(progpath,GetProgDir(progname,svpath),_MAX_PATH);
#else
  NewMemory((void **)&progpath,PATH_MAX);
  realpath(GetProgDir(progname,svpath),progpath);
#endif
  return progpath;
}
#endif

/* ------------------ LastName ------------------------ */

char *LastName(char *argi){

// returns the file name contained in the full path name argi

  char *lastdirsep;
  char *dir, *filename, cwdpath[1000];

  filename=argi;
  lastdirsep=strrchr(argi,SEP);
  if(lastdirsep!=NULL){
    dir=argi;
    filename=lastdirsep+1;
    lastdirsep[0]=0;
    GETCWD(cwdpath,1000);
    if(strcmp(cwdpath,dir)!=0){
      CHDIR(dir);
    }
  }
  return filename;
}

/* ------------------ GetZoneFileName ------------------------ */

char *GetZoneFileName(char *bufptr){
  char *full_name, *last_name, *filename;

  full_name=bufptr;
  if(FILE_EXISTS(full_name)==NO)full_name=NULL;

  last_name= LastName(bufptr);
  if(FILE_EXISTS(last_name)==NO)last_name=NULL;

  if(last_name!=NULL&&full_name!=NULL){
    if(strcmp(last_name,full_name)==0){
      last_name=NULL;
    }
  }

  if(last_name!=NULL&&full_name!=NULL){
    filename=last_name;
  }
  else if(last_name==NULL&&full_name!=NULL){
    filename=full_name;
  }
  else if(last_name!=NULL&&full_name==NULL){
    filename=last_name;
  }
  else{
    filename=NULL;
  }
  return filename;
}

/* ------------------ FileModtime ------------------------ */

time_t FileModtime(char *filename){

// returns the modification time of the file named filename

  STRUCTSTAT statbuffer;
  time_t return_val;
  int statfile;

  return_val=0;
  if(filename==NULL)return return_val;
  statfile=STAT(filename,&statbuffer);
  if(statfile!=0)return return_val;
  return_val = statbuffer.st_mtime;
  return return_val;
}

/* ------------------ Which ------------------------ */

char *Which(char *progname){

// returns the PATH directory containing the file progname

  char *pathlist, *pathlistcopy, *fullprogname, *prognamecopy;
  char *dir,*pathentry;
  char pathsep[2], dirsep[2];

#ifdef WIN32
  strcpy(pathsep,";");
  strcpy(dirsep,"\\");
#else
  strcpy(pathsep,":");
  strcpy(dirsep,"/");
#endif

  pathlist = getenv("PATH");
  if(pathlist==NULL||strlen(pathlist)==0||progname==NULL||strlen(progname)==0)return NULL;

  NewMemory((void **)&prognamecopy, (unsigned int)(strlen(progname)+4+1));
  strcpy(prognamecopy, progname);

  NewMemory((void **)&pathlistcopy, (unsigned int)(strlen(pathlist)+1));
  strcpy(pathlistcopy, pathlist);

#ifdef WIN32
  {
    const char *ext;

    ext = prognamecopy+strlen(progname)-4;
    if(strlen(progname)<=4||STRCMP(ext,".exe")!=0)strcat(prognamecopy, ".exe");
  }
#endif

  NewMemory((void **)&fullprogname, (unsigned int)(strlen(progname)+4+strlen(dirsep)+strlen(pathlist)+1));

  dir=strtok(pathlistcopy,pathsep);
  while(dir!=NULL&&strlen(dir)>0){
    strcpy(fullprogname,dir);
    strcat(fullprogname,dirsep);
    strcat(fullprogname,prognamecopy);
    if(FILE_EXISTS(fullprogname)==YES){
      NewMemory((void **)&pathentry,(unsigned int)(strlen(dir)+2));
      strcpy(pathentry,dir);
      strcat(pathentry,dirsep);
      FREEMEMORY(pathlistcopy);
      FREEMEMORY(fullprogname);
      FREEMEMORY(prognamecopy);
      return pathentry;
    }
    dir=strtok(NULL,pathsep);
  }
  FREEMEMORY(pathlistcopy);
  FREEMEMORY(fullprogname);
  FREEMEMORY(prognamecopy);
  return NULL;
}
