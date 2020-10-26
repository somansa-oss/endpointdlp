#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mntent.h>
#include <libudev.h>

int main_org(void)
{
  struct mntent *ent;
  FILE *aFile;

  aFile = setmntent("/proc/mounts", "r");
  if (aFile == NULL) {
    perror("setmntent");
    exit(1);
  }

//    char *mnt_fsname;		/* Device or server for filesystem.  */
//    char *mnt_dir;		/* Directory mounted on.  */
//    char *mnt_type;		/* Type of filesystem: ufs, nfs, etc.  */
//    char *mnt_opts;		/* Comma-separated options for fs.  */
//    int mnt_freq;		/* Dump frequency (in days).  */
//    int mnt_passno;		/* Pass number for `fsck'.  */

  while (NULL != (ent = getmntent(aFile))) {
    printf("%s %s %s %s \n", ent->mnt_fsname, ent->mnt_dir, ent->mnt_type, ent->mnt_opts);
  }
  endmntent(aFile);
}

static void enumerate_usb_mass_storage(struct udev* udev, char* mnt_fsname)
{
  struct udev_enumerate* enumerate = NULL;
  if (udev == NULL || mnt_fsname == NULL)
    return;
  
  enumerate = udev_enumerate_new(udev);
  if (enumerate == NULL)
    return;

  printf("DEVNAME = %s\n", mnt_fsname);

  udev_enumerate_add_match_property(enumerate, "DEVNAME", mnt_fsname);
  //udev_enumerate_add_match_property(enumerate, "DEVTYPE", "partition");
  udev_enumerate_scan_devices(enumerate);

  struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
  struct udev_list_entry *entry = NULL;

  udev_list_entry_foreach(entry, devices) 
  {
    const char* path = udev_list_entry_get_name(entry);
    if (path == NULL)
        continue;

     struct udev_device* parent = udev_device_new_from_syspath(udev, path);
     if (parent == NULL)
         continue;

    //  {
    //     struct udev_list_entry *props = NULL;
    //     struct udev_list_entry *props_list_entry = NULL;

    //     printf("path = %s\n", path);

    //     props = udev_device_get_properties_list_entry( parent );
    //     udev_list_entry_foreach( props_list_entry, props )
    //     {
    //         const char *attr = udev_list_entry_get_name( props_list_entry );
    //         const char *value = udev_device_get_property_value(parent, attr);
    //         printf("\tproperty [%s][%s]\n", attr, value);
    //     }
    //  }

    const char *id_cdrom = udev_device_get_property_value(parent, "ID_CDROM");
    printf("\tproperty [ID_CDROM][%s]\n", id_cdrom);

    const char *devtype = udev_device_get_property_value(parent, "DEVTYPE");
    printf("\tproperty [DEVTYPE][%s]\n", devtype);

     break;
  }

  udev_enumerate_unref(enumerate);
}


int EnumMountCallback(void *pMount, void* pParam)
{
  struct mntent *ent = NULL;
  FILE *aFile = NULL;
  struct udev* udev = NULL;

  aFile = setmntent("/proc/mounts", "r");
  if (aFile == NULL) 
  {
    printf("[DLP][%s] setmntent() failed \n", __FUNCTION__);
    return -1;
  }

  udev = udev_new();
  if (udev == NULL)
  {
    printf("[DLP][%s] udev_new() failed \n", __FUNCTION__);
    endmntent(aFile);
    return -1;
  }

  while (NULL != (ent = getmntent(aFile))) 
  {
    // ent->mnt_fsname, ent->mnt_dir, ent->mnt_type, ent->mnt_opts
    //  /dev/sdb1  /media/somansa/PRIVACY-I  vfat  rw,nosuid,nodev,relatime,uid=1001,gid=1001,fmask=0022,dmask=0022,codepage=437,iocharset=ascii,shortname=mixed,showexec,utf8,flush,errors=remount-ro
    //  /dev/sda3 / ext4  rw,relatime,errors=remount-ro 
    if (strstr(ent->mnt_fsname, "/dev/") == NULL)
    {
      if (ent->mnt_fsname[0] == '/' && ent->mnt_fsname[1] == '/')
      {
        // //192.168.181.1/tmp
      }
      else
      {
        continue;
      }
    }

    if (strlen(ent->mnt_dir) <= 1)
    {
      continue;
    }
    printf("%s %s %s %s \n", ent->mnt_fsname, ent->mnt_dir, ent->mnt_type, ent->mnt_opts);

    enumerate_usb_mass_storage(udev, ent->mnt_fsname);
  }
  endmntent(aFile);
  udev_unref(udev);

  return 0;
}

int main(void)
{
  EnumMountCallback(NULL, NULL);
}