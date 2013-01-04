/* This code reads the current Ethernet MAC id. The MAC id is 
 * incremented by one. The TiWi firmware file is read. The new MAC 
 * id is written to the firmware file.
 * 
 * Written by Steve Jardine CDM Data
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define maxval 0xFF
#define MAC_ADDR_LEN 6

unsigned char inc_byte (unsigned char mac_id_member, int *increment)
{
   unsigned char incel = mac_id_member+1;
   *increment=0;
   
   if ((int)mac_id_member == (int)maxval)
   {
      *increment=1;
      return(0);
   } else { return (incel); }
   
}

int readfile (unsigned char inbuf[])
{
   FILE *fp;
   unsigned long fileLen;
   int numread=0,i;

   fp=fopen("/system/etc/firmware/ti-connectivity/wl1271-nvs.bin","rb"); 
   if(fp==NULL)
   {
      puts("cannot open file");
      exit(0);
   }

   fseek(fp, 1, SEEK_END);
   fileLen=ftell(fp);
   fseek(fp, 0, SEEK_SET);

   numread=fread(inbuf,1,fileLen,fp);//1 indicate 1 byte i.e. 8 bits of data.

fclose(fp);
return(numread);
}

void write_nvs_file (unsigned char wmid[], unsigned char outpbuf[])
{
   const unsigned char vals[] = {
      0x0, 0x1, 0x6d, 0x54, 0x71
    };

   int i,rc;

   outpbuf[0]=vals[1];
   outpbuf[1]=vals[2];
   outpbuf[2]=vals[3];

   /* write down MAC address in new NVS file */
   outpbuf[3]=wmid[5];
   outpbuf[4]=wmid[4];
   outpbuf[5]=wmid[3];
   outpbuf[6]=wmid[2];

   outpbuf[7]=vals[1];
   outpbuf[8]=vals[4];
   outpbuf[9]=vals[3];
  
   outpbuf[10]=wmid[1];
   outpbuf[11]=wmid[0];

}

void writefile (unsigned char outbuf[], int blen)
{
   FILE *fo;

   fo=fopen("/system/etc/firmware/ti-connectivity/wl1271-nvs.bin","wb"); 
   if(fo==NULL)
   {
      puts("cannot open file");
      exit(0);
   }

   fwrite(outbuf, 1, blen, fo);
   fclose(fo);
}

int main( int argc, char *argv[] )
{
   int s, x, oct_ptr,tt;
   struct ifreq buffer;
   int update, buflen;
   unsigned char ret_val;
   unsigned char bmid[6];
   unsigned char fdbuf[1000] = {0};

   // Get original nvs file

   buflen=readfile (fdbuf);

   //Get current Ethernet MAC ID

   s = socket(PF_INET, SOCK_DGRAM, 0);

   memset(&buffer, 0x00, sizeof(buffer));
   
   strcpy(buffer.ifr_name, "eth0");

   ioctl(s, SIOCGIFHWADDR, &buffer);

   close(s);

   //Increment MAC ID

   update=0;
   for(oct_ptr=6;oct_ptr>0;oct_ptr--) 
   {
      if ((update) || (oct_ptr==6))
      {
         ret_val = inc_byte((unsigned char)buffer.ifr_hwaddr.sa_data[oct_ptr-1], &update);
         bmid[oct_ptr-1]=ret_val;
      } else { bmid[oct_ptr-1]= (unsigned char)buffer.ifr_hwaddr.sa_data[oct_ptr-1];}
   }

#ifdef DEBUG

   printf("\n");

   for( s = 0; s < 6; s++ )
   {
      printf("%.2X", (unsigned char)buffer.ifr_hwaddr.sa_data[s]);
      if (s<5)
	printf(":");

   }

   printf("\n");

   for( s = 0; s < 6; s++ )
   {
      printf("%.2X", (unsigned char)bmid[s]);
      if (s<5)
        printf(":");
   }

   printf("\n");

#endif

   // Update NVS file

   write_nvs_file (bmid, fdbuf);

#ifdef DEBUG
   printf("First 20 bytes of output file are:\n");
   for(tt=0;tt<20;tt++) 
   {
      printf ("0x%.2x ", fdbuf[tt]);
   }
   printf("\n");
#endif

   // Write out NVS file

    writefile (fdbuf,buflen);

 return EXIT_SUCCESS;
}
