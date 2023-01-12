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


   char histinfile[200];
   char histoutfile[200];
   char controlfile[200];
   char Cfile[200];


/*********************************************************************************
* Processing arguments:
*/
   if ( argc != 5 ) { printf("expects arguments: schedule.infile schedule.outfile control.txt control.dat\n"); exit(-1); }

/*
   for (int i = 0; i < argc; ++i)
       printf("%s %d \n", argv[i], argc);
*/


   sscanf(argv[1], "%s", histinfile);
   printf("Input file     : %s\n", histinfile);

   sscanf(argv[2], "%s", histoutfile);
   printf("Output file    : %s\n", histoutfile);

   sscanf(argv[3], "%s", controlfile);
   printf("Control file   : %s\n", controlfile);

   sscanf(argv[4], "%s", Cfile);
   printf("Control file   : %s\n", Cfile);



/*********************************************************************************/
// Open and read control.txt file with information about wells etc
   printf("Reading =\n\n%s \n", controlfile);
   FILE* fpc = fopen(controlfile, "r");
   if (fpc == NULL) { printf("fopen failed to open the file %s\n", controlfile); exit(-1); }
   int nrc=0;
   while ((read = getline(&line, &len, fpc)) != -1) { nrc++; }
   rewind(fpc);

   typedef struct
   {
   char  wellname[10];
   char  variable[10];
   int  nrdates;
   } controlfields;
   controlfields controls[nrc];

   for (int i=0; i< nrc; i++){
      fscanf(fpc,"%s %s %d", controls[i].wellname,controls[i].variable,&controls[i].nrdates);
      printf("line %d: %s %s %d\n",i+1,controls[i].wellname, controls[i].variable,controls[i].nrdates);
   }
   fclose(fpc);

   int nrw=0;
   int jw;
   for (int i = 0; i < nrc; i++) {
      for (jw = i+1; jw < nrc; jw++){
         if (strcmp(controls[i].wellname, controls[jw].wellname) == 0) break;
      }
      if (jw == nrc) nrw++;
   }
   printf("number of wells are: %d\n", nrw);

   int nrv=0;
   int jv;
   for (int i = 0; i < nrc; i++) {
      for (jv = i+1; jv < nrc; jv++){
         if (strcmp(controls[i].variable, controls[jv].variable) == 0) break;
      }
      if (jv == nrc) nrv++;
   }
   printf("number of variables are: %d\n", nrv);


   for (int i = 1; i < nrc; i++) {
      if (controls[i].nrdates != controls[i-1].nrdates ) { printf("ERROR: nrdates must be the same for all wells and variables\n"); exit(-1); }
   }
   int nrd=controls[1].nrdates;

   printf("number of dates are: %d\n", nrd);



/*********************************************************************************/
// Open and read control.dat file
   printf("\n\nReading =%s \n", Cfile);
   int nrcdata=nrw*nrv*nrd;

   typedef struct
   {
   char  wellname[10];
   char  variable[10];
   int   date;
   float value;
   } controlvars;

   controlvars ccc[nrcdata];

   FILE* fpe = fopen(Cfile, "r");
   if (fpe == NULL) { printf("fopen failed to open the file %s\n", Cfile); exit(-1); }

   for(int i = 0; i < nrcdata; i++){
      fscanf(fpe,"%s %s %d %g", ccc[i].wellname, ccc[i].variable, &ccc[i].date, &ccc[i].value);
      printf("%d %s %s %d %g\n", i+1, ccc[i].wellname, ccc[i].variable, ccc[i].date, ccc[i].value);
   }



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

              fprintf(fpout,"--%s  %s %s %10.3f %10.3f %10.0f %s\n",well,open,resv,opr,wpr,gpr,slash);

              // Select the iw corresponding to the current well, variable and date
              int iopr=0;
              int igpr=0;
              int iwpr=0;
              float oprt,gprt,wprt;

              for (int iw=0; iw < nrcdata; iw++){
                 if (strcmp(well,ccc[iw].wellname) == 0 && strcmp("WOPR",ccc[iw].variable) == 0 && k == ccc[iw].date){
                    oprt=ccc[iw].value;
                    iopr=++iopr;
                 }
                 if (strcmp(well,ccc[iw].wellname) == 0 && strcmp("WGPR",ccc[iw].variable) == 0 && k == ccc[iw].date){
                    gprt=ccc[iw].value;
                    igpr=++igpr;
                 }
                 if (strcmp(well,ccc[iw].wellname) == 0 && strcmp("WWPR",ccc[iw].variable) == 0 && k == ccc[iw].date){
                    wprt=ccc[iw].value;
                    iwpr=++iwpr;
                 }
                 if (iopr==1 && igpr==1 && iwpr==1) {
//                    printf("perturbations: %f %f %f\n", oprt, gprt, wprt);
                    break;
                 }
                 if (iopr > 1 || igpr > 1 || iwpr > 1){
                    printf("Duplicates in control.dat file: %d %d %d\n", iopr, igpr, iwpr);
                    exit(-1);
                 }
              }

              if (iopr == 0 || igpr == 0 || iwpr == 0){
                 printf("did not find perturbations: %d %d %d\n", iopr, igpr, iwpr);
                 for (int iw=0; iw < nrcdata; iw++){
                    printf("%d: %s=%s, %s, %d=%d\n",iw, well, ccc[iw].wellname, ccc[iw].variable, k,ccc[iw].date);
                 }
                 exit(-1);
              }



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
