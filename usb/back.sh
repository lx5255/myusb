make clean
cd ..
tar -cvf usb.tar usb
#echo usb.tar>> $(date -d "today" +"%Y%m%d").tar	 
cp usb.tar /mnt/hgfs/back/usb_back_$(date -d "today" +"%Y%m%d").tar	 
rm  usb.tar
