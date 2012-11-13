/* Written by Steve Jardine 8/10/2012
   for CDM Data.
   Routine turns on blue status LED for
   Nitrogenb 53k tablet whenever any connectivity 
   device is turned on (wifi, bluetooth, or cell)
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h> 

// LED Status 

#define BON    0
#define BOFF   1


const unsigned MAXLINE=256;
char const* FCFG="/proc/modules";
char const* findkey1="hci_uart";
char const* findkey2="wl12xx_sdio";

int fd, l_stat, g_on;

void ctrl_led(int status)
{

   if ((write(fd,&status, sizeof(int))) == -1)
   { /* error reading file */
#ifdef DEFINE
      printf("Status = %d Unable to control LED\n", status);
#endif
      exit(1);
   }
}

int main() {
   FILE *fcfg;
   char line[MAXLINE], modle[MAXLINE], dum1[MAXLINE], dum2[MAXLINE],dum3[MAXLINE];
   int wifi=0, bt=0, fst, sec, b_stat, b_on=0;

   l_stat=0;
   g_on=0;

   if ((fd=open("/dev/LED2",O_WRONLY)) ==-1)
   { /* error file not open */
#ifdef DEFINE
      perror("Datafile 2");
#endif
      exit(1);
   }

   if ((fcfg=fopen( FCFG,"r")) == NULL)
   {  
#ifdef DEFINE
      printf("Can't open %s\n", FCFG);
#endif
      exit(1);
   }

   while (1==1)
   {
      while ((fgets(line,256,fcfg))!= NULL)
      {  // Parse file for wifi and bt
   
         sscanf(line, "%s %d %d %s %s %s", modle,&fst, &sec, dum1, dum2, dum3);
#ifdef DEBUG
      printf("Looking at %d -- %s strnlen: %d %d %d\n",strlen(line), modle, strlen(modle), fst, sec);
#endif
         if (!(strcmp(modle, findkey2)))
         {//Found WIFI
#ifdef DEFINE
            printf("\nFound key 2!\n");
#endif
            wifi=1;
         }
         if ((!(strcmp(modle, findkey1))) && (sec>1))
         {//Found BT
#ifdef DEFINE
            printf("\nFound key 1! - sec=%d\n",sec);
#endif
            bt=1;
         }
      }
#ifdef DEFINE
      printf("wifi=%d bt=%d\n", wifi, bt);
#endif

      if (wifi || bt)
         b_stat=1;
      else
         b_stat=0;
      if (b_stat != g_on)
      {
#ifdef DEBUG
         printf("set blue led b_stat %d g_on :%d\n", b_stat, g_on);
#endif
         if (b_stat)
            ctrl_led(BON);
         else
            ctrl_led(BOFF);

         g_on=b_stat;
      }
      rewind(fcfg);
      wifi=0;
      bt=0;
      sleep(2);
   }
   fclose(fcfg);
   close(fd);
   return 0;
}

