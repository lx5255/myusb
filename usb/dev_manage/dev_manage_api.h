#ifndef _DEV_MG_API_
#define _DEV_MG_API_

int dev_manage_init();
int get_avail_major();
dev_io *get_dev_io_byname(const char *dev_name);
int get_dev_msg(dev_hd *hd, unsigned int *event, unsigned int *data);
void dev_event_callback_run();
int dev_mount_api(dev_io *io, int major, void *parm);
int dev_umount_api(dev_hd *hd);
int dev_open_api(dev_hd **hd, const char *dev_name);
char *get_dev_name_by_hd(dev_hd *hd);
int dev_read_api(dev_hd *hd, unsigned int addr,  void *buffer,  int buffer_len);
int dev_write_api(dev_hd *hd, unsigned int addr, void *buffer, int buffer_len);
int dev_ioctrl_api(dev_hd *hd, int cmd, void *parm);
#endif
