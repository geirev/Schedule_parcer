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
   float E0[36][3][5][100];
   float Ein[36][3][5][100];

   int iter;
   int iens;

   char histinfile[200];
   char histoutfile[200];
   char Efile[200];
   char Epert[200];

/*********************************************************************************
* Processing arguments:
* Note that if iens (arg1) equals -1 then iens and iter is extracted from runpath
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

   sscanf(argv[4], "%s", Epert);
   strcpy(Efile,Epert);
   printf("EPERT file     : %s\n", Epert);


   if (iens == -1){
// Extract realization number and iteration number from runpath variable
//      /home/geve/Dropbox/Statoil/ert/Rundir/IEStest/real-0/it-0/history.sch
      printf("IENS = -1: \n");
      int count=0;
      int pos[100];
      int i=0;
      do {
         if (histoutfile[i] == '/' || histoutfile[i] == '-'){
            count++;
            pos[count]=i;
            // printf("string count %d %d\n", count, pos[count]);
         }
      } while (histoutfile[i++]);

//      printf("Aiter %.*s\n", pos[count-2] - pos[count-3] -1 , histoutfile + pos[count-3] +1 );
//      printf("Aiens %.*s\n", pos[count] - pos[count-1] -1 , histoutfile + pos[count-1] +1 );

      int iensS=pos[count-3] +1;
      int iensL=pos[count-2] - pos[count-3] -1;

      int iterS=pos[count-1] +1;
      int iterL=pos[count] - pos[count-1] -1;

//      printf("Biens %.*s\n", iensL , histoutfile+iensS );
//      printf("Biter %.*s\n", iterL , histoutfile+iterS );

      char  ciens[iensL];
      char  citer[iterL];

      strncpy(ciens, histoutfile+iensS , iensL);
      strncpy(citer, histoutfile+iterS , iterL);

      sscanf(citer, "%d", &iter);
      sscanf(ciens, "%d", &iens);
//      printf("citer= %s  %d\n", citer, iter);
//      printf("ciens= %s  %d\n", ciens, iens);

      sprintf(Efile, "%s_%s.dat",Epert,citer);
   }

   printf("Processing iteration=%d and realization=%d\n", iter,iens);

/*********************************************************************************/
// Open and read Epert_X.dat file
   int nrens,nrwells, nrdata, nx;
   char *cnrens = malloc(10 * sizeof(char));
   char *cnrwells = malloc(10 * sizeof(char));
   char *cnrdata = malloc(10 * sizeof(char));
   char *cnx = malloc(10 * sizeof(char));

   printf("Reading =%s version A\n", Efile);
   FILE* fpe = fopen(Efile, "r");
   if (fpe == NULL) { printf("fopen failed to open the file %s\n", Efile); exit(-1); }

   fscanf(fpe,"%s %d %s %d %s %d %s %d ", cnrens, &nrens, cnrwells, &nrwells, cnrdata, &nrdata, cnx, &nx);
   printf("%s= %d, %s= %d, %s= %d, %s= %d\n", cnrens, nrens, cnrwells, nrwells, cnrdata, nrdata, cnx, nx );

//   float Ein[nx][nrdata][nrwells][nrens]

   for(int j = 0; j < nrens; j++){ // Ensemble number index
      for(int i = 0; i < nrwells; i++){ // wellname index
         for(int l = 0; l < nrdata; l++){  // ratetype index
            for(int k = 0; k < nx; k++){  // time index
               fscanf(fpe, "%d %d %d %d %f", &k, &l, &i, &j, Ein[k][l][i][j]);
            }
         }
      }
   }
   fclose(fpe);

   printf("Reading =%s version B\n", Efile);
   FILE* fpe2 = fopen(Efile, "r");
   if (fpe2 == NULL) { printf("fopen failed to open the file %s\n", Efile); exit(-1); }

   fscanf(fpe2,"%s %d %s %d %s %d %s %d ", cnrens, &nrens, cnrwells, &nrwells, cnrdata, &nrdata, cnx, &nx);
   printf("%s= %d, %s= %d, %s= %d, %s= %d\n", cnrens, nrens, cnrwells, nrwells, cnrdata, nrdata, cnx, nx );

   for(int i = 0; i < nrens*nrwells*nrdata*nx; i++)
   {
      int j;  // ensemble index
      int i;  // well index
      int l;  // ratetype index
      int k;  // time index
      float value;
      fscanf(fpe2, "%d %d %d %d %f", &k, &l, &i, &j, &value);
      E0[k-1][l-1][i-1][j-1]=value;
      // printf("%d %d %d %d %12.4f %12.4f \n", k, l, i, j, E0[k-1][l-1][i-1][j-1], value); 
   }
   fclose(fpe2);

   for(int k = 0; k < nx; k++){
      for(int l = 0; l < nrdata; l++){
         for(int i = 0; i < nrwells; i++){
            for(int j = 0; j < 10; j++){
               printf("%12.4f %12.4f ", E0[k][l][i][j], Ein[k][l][i][j]);
            }
            printf("\n");
         }
      }
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
