# Built Image
copy autoboot.bin and ds1n00.dsk

# How to run it
Copy these files to the root of your v20mbc
(back up your SD FIRST!!)

trigger IOS by holding down reset+user and letting up reset
Change default disk set (normally cpm-86)
Select autoboot

Image contains masm, link, dos utils, and basic. 

If you use terraterm, make sure to set language to english
and select a font. By default the \ char is mapped to Yen

## Adding software with dosbox-x- 
mount the disk image in dosbox-x

	imgmount f ds1n00.dsk -t hdd -fs fat

You can now copy files to the F: which will put them on the image

Close dosbox-x and recopy disk file to your sd. 
