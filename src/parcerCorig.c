#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
int main(int argc, char** argv) 
{
   char * line;
   char ini[2];
   char well[20];
   char open[20];
   char resv[20];
   char water[20];
   char slash[100];
   float  opr;
   float  wpr;
   float  gpr;
   float  wat;
   size_t len = 0;
   ssize_t read;
   int nrlines=0;

   int iens;

   char histinfile[200];
   char histoutfile[200];
   char Cfile[200];

   char* wellfile = "wells.txt";


/*********************************************************************************
* Processing arguments:
* Note that if iens (arg1) equals -1 then iens is extracted from runpath
*/
   if ( argc != 5 ) { printf("expects arguments: iens schedule.infile schedule.outfile\n"); exit(-1); }

/*
   for (int i = 0; i < argc; ++i) 
       printf("%s %d \n", argv[i], argc); 
*/

   sscanf(argv[1], "%d", &iens);
   printf("Ensemble member: %d\n", iens);

   sscanf(argv[2], "%s", histinfile);
   printf("Input file     : %s\n", histinfile);

   sscanf(argv[3], "%s", histoutfile);
   printf("Output file    : %s\n", histoutfile);

   sscanf(argv[4], "%s", Cfile);
   printf("Control file   : %s\n", Cfile);

   printf("Processing realization=%d\n", iens);

/*********************************************************************************/
// Open and read wells.txt file
   
   printf("Reading =%s \n", wellfile);
   FILE* fpw = fopen(wellfile, "r");
   if (fpw == NULL) { printf("fopen failed to open the file %s\n", wellfile); exit(-1); }






/*********************************************************************************/
// Open and read control.dat file
   int nrens,nrwells, nrdata, nx;
   char *cnrens = malloc(10 * sizeof(char));
   char *cnrwells = malloc(10 * sizeof(char));
   char *cnrdata = malloc(10 * sizeof(char));
   char *cnx = malloc(10 * sizeof(char));

   printf("Reading =%s \n", Cfile);
   FILE* fpe = fopen(Cfile, "r");
   if (fpe == NULL) { printf("fopen failed to open the file %s\n", Cfile); exit(-1); }

   fscanf(fpe,"%s %d %s %d %s %d %s %d ", cnrens, &nrens, cnrwells, &nrwells, cnrdata, &nrdata, cnx, &nx);
   printf("%s %d, %s %d, %s %d, %s %d\n", cnrens, nrens, cnrwells, nrwells, cnrdata, nrdata, cnx, nx );

   float Cin[nx][nrdata][nrwells] ;
   int dummyj;  // ensemble index
   int dummyi;  // well index
   int dummyl;  // ratetype index
   int dummyk;  // time index

      for(int i = 0; i < nrwells; i++){ // wellname index
         for(int l = 0; l < nrdata; l++){  // ratetype index
            for(int k = 0; k < nx; k++){  // time index
               float value;
               fscanf(fpe, "%d %d %d %d %f", &dummyk, &dummyl, &dummyi, &dummyj, &value);
               if (k==dummyk-1  && l==dummyl-1 && i==dummyi-1 && j==dummyj-1){
                  Cin[k][l][i]=value;
               } else {
                  printf("Error reading %s: %d %d %d %d %d %d %d\n",Cfile, dummyj-1, i, dummyi-1, l, dummyl-1, k, dummyk-1);
               }
            }
         }
      }
   fclose(fpe);
   printf("Done reading =%s \n", Efile);



/*********************************************************************************/
// Open history.sch file
   FILE* fp = fopen(histinfile, "r");
   if (fp == NULL) { printf("fopen failed to open the file %s\n", histinfile); exit(-1); }

// Count number of lines in history.sch file
   while ((read = getline(&line, &len, fp)) != -1) { nrlines++; }
   rewind(fp);

// Read history.sch file into string
   char string[nrlines][200];
   for (int i=0; i< nrlines; i++){
      fgets(string[i], 200, fp);
   }
   fclose(fp);

// Write back updated history file
   FILE* fpout = fopen(histoutfile, "w");
   if (fpout == NULL) { printf("fopen failed to open the file %s\n", histoutfile); exit(-1); }
   int i=0;
   int k=0;
   while (i < nrlines){
      fprintf(fpout,"%s",string[i]);

      sscanf(string[i], "%s", line);
      if (strcmp(line,"WCONHIST")==0){
        k=k+1;
        i=i+1;
        sscanf(string[i], "%s[^/]", line);
        while (strcmp(line,"/") != 0) {
           strncpy(ini,string[i],2);
           ini[2] = '\0';
           if (strcmp(ini,"--") == 0){
              fprintf(fpout,"%s",string[i]);
              i=i+1;
           } else {
              sscanf(string[i], "%s %s %s %f %f %f %[^\n]", well, open, resv, &opr, &wpr, &gpr, slash);
              int iwell;
              sscanf(&well[3], "%d", &iwell);
              //fprintf(fpout,"Well number iwell= %d k=%d opr=%12.4f\n", iwell, k, E0[k-1][0][iwell-1][iens]);

              fprintf(fpout,"--%s  %s %s %10.3f %10.3f %10.0f %s\n",well,open,resv,opr,wpr,gpr,slash);
              float oprt=E0[k-1][0][iwell-1][iens];
              float wprt=E0[k-1][1][iwell-1][iens];
              float gprt=E0[k-1][2][iwell-1][iens];
              fprintf(fpout,"  %s  %s %s %10.3f %10.3lf %10.0f %s\n",well,open,resv,opr+oprt,fabs(wpr+wprt),gpr+gprt,slash);

              i=i+1;
              sscanf(string[i], "%s[^/]", line);
           }
        }
        fprintf(fpout,"%s",string[i]);
      }

      else if (strcmp(line,"WCONINJH")==0){
        i=i+1;
        sscanf(string[i], "%s[^/]", line);
        while (strcmp(line,"/") != 0) {
           strncpy(ini,string[i],2);
           ini[2] = '\0';
           if (strcmp(ini,"--") == 0){
              fprintf(fpout,"%s",string[i]);
              i=i+1;
           } else {
              sscanf(string[i], "%s %s %s %f %[^\n]", well, water, open, &wat, slash );
              fprintf(fpout,"   %s %s %s %9.2f %s\n",well, water, open, wat, slash);
           }

           i=i+1;
           sscanf(string[i], "%s[^/]", line);
        }
        fprintf(fpout,"%s",string[i]);
      }

      i=i+1;
   }
}
