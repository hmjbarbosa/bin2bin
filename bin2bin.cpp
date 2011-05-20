#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
//#include<limits.h>
//#include<endian.h>
#include<iostream>

#define DEBUG_BIN

void help(char* prog) {
  std::cout<<"NAME\n";
  std::cout<<"\t"<<prog<<" - convert between data formats in a binary file\n";
  std::cout<<std::endl;
  std::cout<<"SYNOPSIS\n";
  std::cout<<"\t"<<prog<<" -i in-file -o out-file options\n";
  std::cout<<std::endl;
  std::cout<<"DESCRIPTION\n";
  std::cout<<"\t"<<prog<<" converts a binary file by changing between little and big endian, and\n";
  std::cout<<"\tbetween data type representation int*1, int*2, int*4, float*4 and float*8.\n";
  std::cout<<std::endl;
  std::cout<<"OPTIONS\n";
  std::cout<<"\t-i input_file \n";
  std::cout<<std::endl;
  std::cout<<"\t\t Opens file input_file for input\n";
  std::cout<<std::endl;
  std::cout<<"\t-iend le|be \n";
  std::cout<<"\t\t Selects endiness of input file: le for little_endian, or be for big_endian\n";
  std::cout<<std::endl;
  std::cout<<"\t-iprec si1|si2|si4|ui1|ui2|ui4|r4|r8 \n";
  std::cout<<"\t\t Selects the precision of input data. (s|u is for signed vs unsigned. (i|r \n";
  std::cout<<"\t\t is for integer or float. (1|2|4|8 is the number of bytes. Standard 32-bit\n";
  std::cout<<"\t\t float uses 4-bytes, while double precision, or 64-bit float, uses 8-byte.\n";
  std::cout<<std::endl;
  std::cout<<"\t-o output_file \n";
  std::cout<<"\t\t Open file output_file for output\n";
  std::cout<<std::endl;
  std::cout<<"\t-oend le|be \n";
  std::cout<<"\t\t Select endiness of output file: le for little_endian, or be for big_endian\n";
  std::cout<<std::endl;
  std::cout<<"\t-oprec si1|si2|si4|ui1|ui2|ui4|r4|r8 \n";
  std::cout<<"\t\t Selects the precision of output data. (s|u is for signed vs unsigned. (i|r \n";
  std::cout<<"\t\t is for integer or float. (1|2|4|8 is the number of bytes. Standard 32-bit\n";
  std::cout<<"\t\t float uses 4-bytes, while double precision, or 64-bit float, uses 8-byte.\n";
  exit(0);
}

// Check if the sytem where the program was compiled is big endian or not
bool is_big_endian(){
    long x = 0x34333231;
    char *y = (char *) &x;
    if(strncmp(y,"1234",4)) 
      return true;
    else
      return false;
}

#ifdef DEBUG_BIN
// Search for max/min 
template <class myType>
void printmaxmin(const myType* data, int32_t n, const char* msg) {
  long double min, max;
  max=data[0];
  min=data[0];
  for (int32_t i=1; i<n; i++) {
    if (max < data[i]) max=data[i];
    if (min > data[i]) min=data[i];
  }
  std::cout << msg;
  std::cout << "\tmax=" << max << std::endl;
  std::cout << "\tmin=" << min << std::endl;
  return;
}
#endif

// Swap bytes in a variable of any kind
template <class myType>
myType swap( const myType in )
{
  myType out;
  char *cin = ( char* ) & in;
  char *cout = ( char* ) & out;
  int8_t i=0; int8_t j=sizeof(out)-1;
  for (; j>=0; j--) cout[i++] = cin[j];
  return out;
}

template <class writeType, class readType>
void writefile(char *arq, char* end, readType *toconvert, int32_t n) {
  writeType *data;
  FILE *file;
  int32_t typesize;

  typesize=sizeof(writeType);

  // allocate memory of new data type
  data = (writeType*) malloc (n*sizeof(writeType));
  for (int32_t i=0; i<n; i++) {
    data[i] = (writeType) toconvert[i];
    //std::cout << toconvert[i] << " => " << data[i] << std::endl;
  }

#ifdef DEBUG_BIN
  printmaxmin<writeType>(data, n, "After converting data type\n");
#endif

  // decide if we need to swap
  if (is_big_endian() != (strcmp(end,"be")==0)) {
    // swap 
    for (int32_t i=0; i<n; i++) data[i] = swap<readType>(data[i]);
  }

  /*
#ifdef DEBUG_BIN
  printmaxmin<writeType>(data, n, "After converting be/le for writing\n");
#endif
  */

  // write output file
  file = fopen(arq,"w");
  fwrite(data, typesize, n, file);
  fclose(file);  

  // cleanup
  free(data);
}

template <class readType>
void readwritefile (char *iarq, char* iend, char* oarq, char* oend, char* oprec) {
  readType *data;
  FILE *file;
  int32_t n, filesize, typesize, readsize;
  
  file = fopen(iarq,"r");
  typesize=sizeof(readType);

  // get file size in units of readType
  fseek(file, 0, SEEK_END);
  filesize=ftell(file);
  n=filesize/typesize;
  rewind(file);

  // read all data with the right type of variable
  data = (readType*) malloc(n*typesize);
  readsize = fread(data,typesize,n,file);
  fclose(file);
  if (readsize!=filesize) {
    fprintf(stderr,"Read data not equal to file on disk: %d %d\n",
            readsize,filesize);
    exit(1);
  }

  /*
#ifdef DEBUG_BIN
  printmaxmin<readType>(data, n, "After reading from disk\n");
#endif
  */

  // decide if we need to swap input data
  if (is_big_endian() != (strcmp(iend,"be")==0)) {
    for (int32_t i=0; i<n; i++) data[i] = swap<readType>(data[i]);
  }
  
#ifdef DEBUG_BIN
  printmaxmin<readType>(data, n, "After converting le/be\n");
#endif
  
  // write file converting data
  if (strcmp(oprec,"si1") == 0)
    writefile<int8_t,readType>(oarq, oend, data, n);

  else if (strcmp(oprec,"si2") == 0)
    writefile<int16_t,readType>(oarq, oend, data, n);

  else if (strcmp(oprec,"si4") == 0)
    writefile<int32_t,readType>(oarq, oend, data, n);

  else if (strcmp(oprec,"ui1") == 0)
    writefile<uint8_t,readType>(oarq, oend, data, n);

  else if (strcmp(oprec,"ui2") == 0)
    writefile<uint16_t,readType>(oarq, oend, data, n);

  else if (strcmp(oprec,"ui4") == 0)
    writefile<uint32_t,readType>(oarq, oend, data, n);

  else if (strcmp(oprec,"r4") == 0)
    writefile<float,readType>(oarq, oend, data, n);

  else if (strcmp(oprec,"r8") == 0)
    writefile<double,readType>(oarq, oend, data, n);

  else {
    fprintf(stderr,"Wrong precision of output data: %s\n",oprec);
    exit(9);
  }

  free(data);
  return;
}


int main(int argc, char *argv[]) {
  
  //char *prog=NULL, *iarq=NULL, *oarq=NULL, *iend=NULL, *oend=NULL, *iprec=NULL, *oprec=NULL;
  char *prog = 0, *iarq, *oarq, *iend, *oend, *iprec, *oprec;

  // initialize
  prog=NULL;
  iarq=NULL;
  iend=NULL;
  iprec=NULL;
  oarq=NULL;
  oend=NULL;
  oprec=NULL;

  // process command line options
  prog = argv[0];
  for(int32_t i=1; i<argc; i++) {
    if (strcmp(argv[i],"-i")==0) {
      if (i+1>=argc) { fprintf(stderr,"Missing parameter after -i\n"); exit(1);}
      iarq = argv[i+1]; i++;
    } else if (strcmp(argv[i],"-o")==0) {
      if (i+1>=argc) { fprintf(stderr,"Missing parameter after -o\n"); exit(1);}
      oarq = argv[i+1]; i++;
    } else if (strcmp(argv[i],"-iend")==0) {
      if (i+1>=argc) { fprintf(stderr,"Missing parameter after -iend\n"); exit(1);}
      iend=argv[i+1]; i++;
    } else if (strcmp(argv[i],"-oend")==0) {
      if (i+1>=argc) { fprintf(stderr,"Missing parameter after -oend\n"); exit(1);}
      oend=argv[i+1]; i++;
    } else if (strcmp(argv[i],"-iprec")==0) {
      if (i+1>=argc) { fprintf(stderr,"Missing parameter after -iprec\n"); exit(1);}
      iprec=argv[i+1]; i++;
    } else if (strcmp(argv[i],"-oprec")==0) {
      if (i+1>=argc) { fprintf(stderr,"Missing parameter after -oprec\n"); exit(1);}
      oprec=argv[i+1]; i++;
    } else if (strcmp(argv[i],"-h")==0) {
      help(prog);
    } else {
      fprintf(stderr,"Invalid option: %s\n",argv[i]);
      exit(1);
    };
  };

  // verify options
  if (iarq==NULL||strcmp(iarq,"")==0) {
    fprintf(stderr,"Enter the input file name!\n");
    exit(2);
  }
  if (iend==NULL||(strcmp(iend,"le") && strcmp(iend,"be"))) {
    fprintf(stderr,"Invalid endian for input file: %s\n",iend);
    exit(2);
  }
  if (iprec==NULL||strcmp(iprec,"")==0) {
    fprintf(stderr,"Enter the input file data type!\n");
    exit(2);
  }
  if (strcmp(iprec,"si1") && strcmp(iprec,"si2") && strcmp(iprec,"si4") && 
      strcmp(iprec,"ui1") && strcmp(iprec,"ui2") && strcmp(iprec,"si4") && 
      strcmp(iprec,"r4") && strcmp(iprec,"r8") ) {
    fprintf(stderr,"Invalid data type for input file: %s\n",iprec);
    exit(2);
  }
  if (oarq==NULL||strcmp(oarq,"")==0) {
    fprintf(stderr,"Enter the output file name!\n");
    exit(2);
  }
  if (oend==NULL||(strcmp(oend,"le") && strcmp(oend,"be"))) {
    fprintf(stderr,"Invalid endian for output file: %s\n",oend);
    exit(2);
  }
  if (oprec==NULL||strcmp(oprec,"")==0) {
    fprintf(stderr,"Enter the output file data type!\n");
    exit(2);
  }
  if (strcmp(oprec,"si1") && strcmp(oprec,"si2") && strcmp(oprec,"si4") && 
      strcmp(oprec,"ui1") && strcmp(oprec,"ui2") && strcmp(oprec,"ui4") && 
      strcmp(oprec,"r4") && strcmp(oprec,"r8") ) {
    fprintf(stderr,"Invalid data type for output file: %s\n",oprec);
    exit(2);
  }

  // output something to the user
  std::cout << "INPUT\n";
  std::cout << "\tfile: " << iarq << std::endl;
  std::cout << "\tendiness: " << iend << std::endl;
  std::cout << "\tdata type: " << iprec << std::endl;
  std::cout << "OUTPUT\n";
  std::cout << "\tfile: " << oarq << std::endl;
  std::cout << "\tendiness: " << oend << std::endl;
  std::cout << "\tdata type: " << oprec << std::endl;

  // read file, converting it, then write
  if (strcmp(iprec,"si1") == 0)
    readwritefile<int8_t>(iarq, iend, oarq, oend, oprec);

  else if (strcmp(iprec,"si2") == 0)
    readwritefile<int16_t>(iarq, iend, oarq, oend, oprec);

  else if (strcmp(iprec,"si4") == 0)
    readwritefile<int32_t>(iarq, iend, oarq, oend, oprec);

  else if (strcmp(iprec,"ui1") == 0)
    readwritefile<uint8_t>(iarq, iend, oarq, oend, oprec);

  else if (strcmp(iprec,"ui2") == 0)
    readwritefile<uint16_t>(iarq, iend, oarq, oend, oprec);

  else if (strcmp(iprec,"ui4") == 0)
    readwritefile<uint32_t>(iarq, iend, oarq, oend, oprec);

  else if (strcmp(iprec,"r4") == 0)
    readwritefile<float>(iarq, iend, oarq, oend, oprec);

  else if (strcmp(iprec,"r8") == 0)
    readwritefile<double>(iarq, iend, oarq, oend, oprec);

  else {
    fprintf(stderr,"Wrong precision of input data: %s\n",iprec);
    exit(9);
  }

  return(0);
}
