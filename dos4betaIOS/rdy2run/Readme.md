# Built Image
Copy disks to SD card. 

# How to run it
**This version requires J4F's EDITED beta IOS
https://github.com/SuperFabius/A250220_DEVEL

EDIT line 214 and 215 to the following:
#define   SERIALRXIRQ_VECTOR    0x40         
#define   SYSTICKIRQ_VECTOR     0x41         


Copy the fiels in this directory to the image provided by J4F. 
(back up your SD FIRST!!)

trigger IOS by holding down reset+user and letting up reset
Change default disk set to disk set 2 (MSDOS). 

Image contains masm, link, dos utils, basic, and msc3.

MSC4 does run.  

If you use terraterm, make sure to set language to english
and select a font. By default the \ char is mapped to Yen

## Adding software with dosbox-x- 
mount the disk image in dosbox-x

	imgmount f ds2n00.dsk -t hdd -fs fat

You can now copy files to the F: which will put them on the image

Close dosbox-x and recopy disk file to your sd. 
