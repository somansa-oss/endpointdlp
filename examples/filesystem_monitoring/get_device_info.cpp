// sudo apt-get install libudev-dev
// build with: gcc -o udevl -ludev -Wall -g udevl.c
#include <stdio.h>
#include <string.h>
#include <libudev.h>

static void
print_children(
     struct udev* udev, struct udev_device* parent)
{
  struct udev_device* child = NULL;
  struct udev_enumerate *enumerate = udev_enumerate_new(udev);

  udev_enumerate_add_match_parent(enumerate, parent);
  //udev_enumerate_add_match_subsystem(enumerate, subsystem);
  udev_enumerate_scan_devices(enumerate);

  struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
  struct udev_list_entry *entry = NULL;
  struct udev_list_entry *sysattrs = NULL;
  struct udev_list_entry *sysattr_list_entry = NULL;
  struct udev_list_entry *props = NULL;
  struct udev_list_entry *props_list_entry = NULL;

  udev_list_entry_foreach(entry, devices) 
  {
     const char *path = udev_list_entry_get_name(entry);
     child = udev_device_new_from_syspath(udev, path);
     printf("\tchild-path: %s\n", path);

    // sysattrs = udev_device_get_sysattr_list_entry( child );
    // udev_list_entry_foreach( sysattr_list_entry, sysattrs )
    // {
    //     const char *attr = udev_list_entry_get_name( sysattr_list_entry );
    //     const char *value = udev_device_get_sysattr_value(child, attr);
    //     printf("\t\tattribute [%s][%s]\n", attr, value);
    // }

    props = udev_device_get_properties_list_entry( child );
    udev_list_entry_foreach( props_list_entry, props )
    {
        const char *attr = udev_list_entry_get_name( props_list_entry );
        const char *value = udev_device_get_property_value(child, attr);
        printf("\t\tproperty [%s][%s]\n", attr, value);
    }
  }

  udev_enumerate_unref(enumerate);
}

static struct udev_device*
get_child(
     struct udev* udev, struct udev_device* parent, const char* subsystem)
{
  struct udev_device* child = NULL;
  struct udev_enumerate *enumerate = udev_enumerate_new(udev);

  udev_enumerate_add_match_parent(enumerate, parent);
  udev_enumerate_scan_devices(enumerate);

  struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
  struct udev_list_entry *entry;

  udev_list_entry_foreach(entry, devices) {
    const char *path = udev_list_entry_get_name(entry);
    child = udev_device_new_from_syspath(udev, path);
    break;
  }

  udev_enumerate_unref(enumerate);
  return child;
}

static void enumerate_usb_mass_storage(struct udev* udev)
{
  struct udev_enumerate* enumerate = udev_enumerate_new(udev);
  if (enumerate == NULL)
    return;

  udev_enumerate_add_match_subsystem(enumerate, "usb");
  //udev_enumerate_add_match_property(enumerate, "DEVTYPE", "scsi_device");
  udev_enumerate_scan_devices(enumerate);

  struct udev_list_entry *sysattrs = NULL;
  struct udev_list_entry *sysattr_list_entry = NULL;

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

    //[DEVTYPE][usb_device]    
    const char *value = udev_device_get_property_value(parent, "DEVTYPE");
    if (NULL == value || 0 != strcmp(value, "usb_device"))
    {
        continue;
    }

    const char *value2 = udev_device_get_property_value(parent, "ID_MODEL_FROM_DATABASE");
    if (NULL == value2 
      || ((NULL == strstr(value2, "hub") && NULL == strstr(value2, "Hub"))))
    {
        continue;
    }

    //ID_MODEL_FROM_DATABASE][1.1 root hub]

    //  struct udev_device* block = get_child(udev, parent, "block");
    //  if (block == NULL)
    //  {
    //     //do nothing
    //  }
    //  else
     {
        struct udev_list_entry *props = NULL;
        struct udev_list_entry *props_list_entry = NULL;

        printf("path = %s\n", path);

        // sysattrs = udev_device_get_sysattr_list_entry( parent );
        // udev_list_entry_foreach( sysattr_list_entry, sysattrs )
        // {
        //     const char *attr = udev_list_entry_get_name( sysattr_list_entry );
        //     const char *value = udev_device_get_sysattr_value(parent, attr);
        //     printf("\tattribute [%s][%s]\n", attr, value);
        // }
        // props = udev_device_get_properties_list_entry( parent );
        // udev_list_entry_foreach( props_list_entry, props )
        // {
        //     const char *attr = udev_list_entry_get_name( props_list_entry );
        //     const char *value = udev_device_get_property_value(parent, attr);
        //     printf("\tproperty [%s][%s]\n", attr, value);
        // }

        //print_children(udev, parent);


        struct udev_device* child = NULL;
        struct udev_enumerate *child_enumerate = udev_enumerate_new(udev);

        udev_enumerate_add_match_parent(child_enumerate, parent);
        udev_enumerate_scan_devices(child_enumerate);

        struct udev_list_entry *devices = udev_enumerate_get_list_entry(child_enumerate);
        struct udev_list_entry *child_entry = NULL;

        udev_list_entry_foreach(child_entry, devices) {
          const char *child_path = udev_list_entry_get_name(child_entry);
          child = udev_device_new_from_syspath(udev, child_path);
          if (child != NULL)
          {
            const char *value_devtype = udev_device_get_property_value(child, "DEVTYPE");
            const char *value_cdrom = udev_device_get_property_value(child, "ID_CDROM");
            if ((NULL != value_devtype && 0 == strcmp(value_devtype, "partition"))
              || (NULL != value_cdrom && 0 == strcmp(value_cdrom, "1")))
            {
              print_children(udev, child);
            }
            else
            {
              // do nothing
            }
            
          }
        }

        udev_enumerate_unref(child_enumerate);

        
     }
     
    // struct udev_device* scsi_disk = get_child(udev, scsi, "scsi_disk");
    // if (scsi_disk == NULL)
    //     continue;    
    // struct udev_device* usb
    //   = udev_device_get_parent_with_subsystem_devtype(
    //       scsi, "usb", "usb_device");

    // if (block && scsi_disk && usb) {
    //     printf("block = %s, usb = %s:%s:%s, product = %s, scsi = %s, path = %s\n",
    //       udev_device_get_devnode(block),
    //       udev_device_get_sysattr_value(usb, "idVendor"),
    //       udev_device_get_sysattr_value(usb, "idProduct"),
    //       udev_device_get_sysattr_value(usb, "iSerial"),
    //       udev_device_get_sysattr_value(usb, "iProduct"),
    //       udev_device_get_sysattr_value(scsi, "vendor"),
    //       path);
    // }

    // if (block)
    //   udev_device_unref(block);

    // if (scsi_disk)
    //   udev_device_unref(scsi_disk);

    // udev_device_unref(scsi);
  }

  udev_enumerate_unref(enumerate);
}

int main()
{
  struct udev* udev = udev_new();

  enumerate_usb_mass_storage(udev);

  udev_unref(udev);
  return 0;
}