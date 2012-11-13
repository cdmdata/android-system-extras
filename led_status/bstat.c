/* Written by Steve Jardine 8/10/2012
   for CDM Data.
   Routine checks battery capacity value and whether the display is illuminated or not. 
   Meets the following criteria:

-  the GREEN status will iluminate whenever the display goes dark.
-  the ORANGE status will blink when the BATT is below 15% capacity (500ms ON, 1000ms OFF, 500ms ON, 1000ms OFF, 500ms ON, 27500ms OFF – repeat cycle)

*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h> 
#include <signal.h>

#define BON             0
#define BOFF            1
#define LOW_THRESHOLD   15 //percent



const unsigned MAXLINE=256;
char const* FCFG="/sys/class/power_supply/bq27200-0/capacity";
int green,orange, g_on, o_on;
FILE *fcfg;

void ctrl_led(int led_handler,int status)
{

   if ((write(led_handler,&status, sizeof(int))) == -1)
   { /* error reading file */
#ifdef DEBUG
      printf("Status = %d Unable to control LED\n", status);
#endif
      exit(1);
   }
}


void below_blink(int lhandle, int it)
{
   ctrl_led(lhandle,BON);
   usleep(500000); //500 mSec
   ctrl_led(lhandle,BOFF);

   if (it < 2)
   {
      sleep(1); //1 Sec
   } else {
      sleep(27);
      usleep(500000);      
   }
}

int get_curr_batt_capacity()
{ //Get current battery capacity in percent
   int bcap=0;
   char line[MAXLINE];

 /* Open capacity file */

   if ((fcfg=fopen( FCFG,"r")) == NULL)
   {  
#ifdef DEBUG
      printf("Can't open %s\n", FCFG);
#endif
      exit(1);
   }
   while ((fgets(line,256,fcfg))!= NULL)
   { 
      sscanf(line, "%d", &bcap);
   }

   fclose(fcfg);
   return(bcap);
}
int get_backlight_status()
{ //Get screen status
   int lit, scrn_handlr, bytes_read;
   char scrn_on;

/* Open Charging Status File */
   if ((scrn_handlr=open("/sys/class/graphics/fb0/state",O_RDONLY)) ==-1)
   { /* error file not open */
#ifdef DEBUG
      perror("Datafile scrn state");
#endif
      exit(1);
   }

   if ((bytes_read = read(scrn_handlr,&scrn_on, sizeof(char))) == -1)
             { /* error reading file */
                   exit(1);
             }
   /* Done check for charging */
   lit=scrn_on-0x30;
   close(scrn_handlr);
   return(lit);
}

int main() 
{
   int lwm=0, battcap=0, scrn_on, s_on;
   int lp=0;  //Which blink step (1-3);

   g_on=0;
   o_on=0;
   if ((green=open("/dev/LED4",O_WRONLY)) ==-1)
   { /* error file not open */
#ifdef DEBUG
      perror("Datafile 4");
#endif
      exit(1);
   }
   if ((orange=open("/dev/LED1",O_WRONLY)) ==-1)
   { /* error file not open */
#ifdef DEBUG
      perror("Datafile 1");
#endif
      exit(1);
   }

   while(1)
   { //Main Loop

      battcap=get_curr_batt_capacity(); // Capacity in percentage
      lwm=(battcap <= LOW_THRESHOLD) ? 1 : 0;
      scrn_on=get_backlight_status();
      s_on ^= scrn_on;
#ifdef DEBUG
      printf("S_on : %d scrn_on : %d\n", s_on, scrn_on);
#endif
      if ((s_on & 0x1) != g_on)
      {
#ifdef DEBUG
         printf("set green led\n");
#endif
         if ((s_on & 0x1) == 1)
            ctrl_led(green,BON);
         else
            ctrl_led(green,BOFF);

         g_on=(s_on & 0x1);
      }
#ifdef DEBUG
      printf("Capacity: %d LWM: %d\n", battcap, lwm);
#endif

      if(lwm)
         if (lp++<3) 
         { //Normal blink sequence
            below_blink(orange, lp);
         } else { lp=0; 
         }

      lwm=0;
      battcap=0;
      s_on=0;
      scrn_on=0;
      sleep(10);
   }
close(orange);
close(green);
fclose(fcfg);
return(0);
}

