#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
int main( int argc, char **argv ) {
  char target[20]; /* monitoring directory name */
  int fd;
  int wd; /* watch desc */
  char buffer[BUF_LEN];
  fd = inotify_init();
  if (fd < 0) {
    perror("inotify_init");
  }
  if (argc < 2) {
    fprintf (stderr, "Watching the current directory\n");
    strcpy (target, ".");
  } else {
    fprintf (stderr, "Watching '%s' directory\n", argv[1]);
    strcpy (target, argv[1]);
  }
  wd = inotify_add_watch(fd, "/home1/knight", IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_MOVE_SELF);
  while(1) {
    int length, i = 0;
    length = read(fd, buffer, BUF_LEN);
    if (length < 0) {
      perror("read");
    }
    while (i < length) {
      struct inotify_event *event = (struct inotify_event *) &buffer[i];
      printf ("[debug] wd=%d mask=%d cookie=%d len=%d dir=%s\n", event->wd, event->mask, event->cookie, event->len, (event->mask & IN_ISDIR)?"yes":"no");
      if (event->len) {
        if (event->mask & IN_CREATE) {
          if (event->mask & IN_ISDIR) {
            printf("The directory %s was created.\n", event->name);      
          } else {
            printf("The file %s was created.\n", event->name);
          }
        } else if (event->mask & IN_DELETE) {
          if (event->mask & IN_ISDIR) {
            printf("The directory %s was deleted.\n", event->name);      
          } else {
            printf("The file %s was deleted.\n", event->name);
          }
        } else if (event->mask & IN_MODIFY) {
          if (event->mask & IN_ISDIR) {
            printf("The directory %s was modified.\n", event->name);
          } else {
            printf("The file %s was modified.\n", event->name);
          }
        } else if (event->mask & IN_MOVED_FROM || event->mask & IN_MOVED_TO || event->mask & IN_MOVE_SELF) {

 
          if (event->mask & IN_ISDIR) {
            printf("The directory %s was moved.\n", event->name);
          } else {
            printf("The file %s was moved.\n", event->name);
          }
        }
      }
      i += EVENT_SIZE + event->len;
    }
  }
  /*
  inotify_rm_watch(fd, wd);
  close(fd);
  */
  return 0;
}