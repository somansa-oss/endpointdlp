#define _GNU_SOURCE

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/signalfd.h>
#include <fcntl.h>

#include <linux/fanotify.h>
#include <sys/fanotify.h>

typedef struct {
    char *path;
} monitored_t;

#define FANOTIFY_BUFFER_SIZE 8192

enum {
    FD_POLL_SIGNAL = 0,
    FD_POLL_FANOTIFY,
    FD_POLL_MAX
};

static uint64_t event_mask =
(
 FAN_MARK_MOUNT |
 FAN_OPEN_PERM|
 // FAN_ACCESS |         /* File accessed */
 //  FAN_MODIFY |         /* File modified */
 FAN_CLOSE_WRITE |    /* Writtable file closed */
 FAN_CLOSE_NOWRITE |  /* Unwrittable file closed */
 //  FAN_OPEN |           /* File was opened */
 FAN_ONDIR |           /* We want to be reported of events in the directory */
 FAN_EVENT_ON_CHILD); /* We want to be reported of events in files of the directory */

static monitored_t *monitors = NULL;
static int n_monitors = 0;

static char *
get_program_name_from_pid (int     pid,
                           char   *buffer,
                           size_t  buffer_size)
{
    int fd = 0;
    ssize_t len = 0;
    char *aux = NULL;
    
    // Try to get program name by PID
    sprintf (buffer, "/proc/%d/cmdline", pid);
    if ((fd = open (buffer, O_RDONLY)) < 0)
        return NULL;
    
    if ((len = read (fd, buffer, buffer_size - 1)) <= 0)
    {
        close (fd);
        return NULL;
    }
    close (fd);
    
    buffer[len] = '\0';
    aux = strstr (buffer, "^@");
    if (aux)
        *aux = '\0';
    
    return buffer;
}

static char *
get_file_path_from_fd (int     fd,
                       char   *buffer,
                       size_t  buffer_size)
{
    ssize_t len = 0;
    
    if (fd <= 0)
        return NULL;
    
    sprintf (buffer, "/proc/self/fd/%d", fd);
    if ((len = readlink (buffer, buffer, buffer_size - 1)) < 0)
        return NULL;
    
    buffer[len] = '\0';
    return buffer;
}

static void
event_process (struct fanotify_event_metadata *event,
               int                             fanotify_fd)
{
    char file_path[PATH_MAX] = {0};
    char program_path[PATH_MAX] = {0};
    
    if (NULL == get_file_path_from_fd (event->fd, file_path, PATH_MAX))
    {
        struct fanotify_response access = {0};
        access.fd = event->fd;
        access.response = FAN_ALLOW;
        write (fanotify_fd, &access, sizeof (access));
        return;
    }
    
    printf ("Received event in path '%s'=>[%d]", file_path, event->mask);
    printf (" pid=%d (%s): \n",
            event->pid,
            (get_program_name_from_pid (event->pid, program_path, PATH_MAX) ?
             program_path :
             "unknown"));
    
    //if (event->mask & FAN_OPEN_PERM)
    {
        struct fanotify_response access = {0};
        
        access.fd = event->fd;
        
        //   if (strstr (file_path, "666") != NULL)
        // {
        //   printf ("\tFAN_OPEN_PERM: denying\n");
        //   access.response = FAN_DENY;
        // }
        //   else
        {
            //printf ("\tFAN_OPEN_PERM: allowing\n");
            access.response = FAN_ALLOW;
        }
        
        write (fanotify_fd, &access, sizeof (access));
    }
    
    fflush (stdout);
    close (event->fd);
}

static void
shutdown_fanotify (int fanotify_fd)
{
    int i;
    
    for (i = 0; i < n_monitors; ++i)
    {
        /* Remove the mark, using same event mask as when creating it */
        fanotify_mark (fanotify_fd,
                       FAN_MARK_REMOVE,
                       event_mask,
                       AT_FDCWD,
                       monitors[i].path);
        free (monitors[i].path);
    }
    free (monitors);
    close (fanotify_fd);
}

static int
initialize_fanotify (int          argc,
                     const char **argv)
{
    int i = 0;
    int fanotify_fd = 0;
    
    if ((fanotify_fd = fanotify_init (FAN_CLOEXEC| FAN_CLASS_CONTENT,
                                      O_RDONLY | O_CLOEXEC | O_LARGEFILE | O_NOATIME)) < 0)
    {
        fprintf (stderr,
                 "Couldn't setup new fanotify device: %s\n",
                 strerror (errno));
        return -1;
    }
    
    //n_monitors = argc - 1;
    //monitors = malloc (n_monitors * sizeof (monitored_t));
    n_monitors = 1;
    monitors = (monitored_t*)malloc (n_monitors * sizeof (monitored_t));
    
    /* Loop all input directories, setting up marks */
    //for (i = 0; i < n_monitors; ++i)
    {
        //monitors[i].path = strdup (argv[i + 1]);
        monitors[i].path = strdup ( "/media/somansa/1493-193A" );
        
        /* Add new fanotify mark */
        if (fanotify_mark (fanotify_fd,
                           FAN_MARK_ADD | FAN_MARK_MOUNT,
                           event_mask,
                           AT_FDCWD,
                           monitors[i].path) < 0)
        {
            fprintf (stderr,
                     "Couldn't add monitor in mount '%s': '%s'\n",
                     monitors[i].path,
                     strerror (errno));
            return -1;
        }
        
        printf ("Started monitoring mount '%s'...\n",
                monitors[i].path);
    }
    
    return fanotify_fd;
}

static void
shutdown_signals (int signal_fd)
{
    close (signal_fd);
}

static int
initialize_signals (void)
{
    int signal_fd;
    sigset_t sigmask;
    
    /* We want to handle SIGINT and SIGTERM in the signal_fd, so we block them. */
    sigemptyset (&sigmask);
    sigaddset (&sigmask, SIGINT);
    sigaddset (&sigmask, SIGTERM);
    
    if (sigprocmask (SIG_BLOCK, &sigmask, NULL) < 0)
    {
        fprintf (stderr,
                 "Couldn't block signals: '%s'\n",
                 strerror (errno));
        return -1;
    }
    
    /* Get new FD to read signals from it */
    if ((signal_fd = signalfd (-1, &sigmask, 0)) < 0)
    {
        fprintf (stderr,
                 "Couldn't setup signal FD: '%s'\n",
                 strerror (errno));
        return -1;
    }
    
    return signal_fd;
}

int
main (int          argc,
      const char **argv)
{
    int signal_fd = 0;
    int fanotify_fd = 0;
    struct pollfd fds[FD_POLL_MAX] = {0};
    
    /* Input arguments... */
    //   if (argc < 2)
    //     {
    //       fprintf (stderr, "Usage: %s directory1 [directory2 ...]\n", argv[0]);
    //       exit (EXIT_FAILURE);
    //     }
    
    /* Initialize signals FD */
    if ((signal_fd = initialize_signals ()) < 0)
    {
        fprintf (stderr, "Couldn't initialize signals\n");
        exit (EXIT_FAILURE);
    }
    
    /* Initialize fanotify FD and the marks */
    if ((fanotify_fd = initialize_fanotify (argc, argv)) < 0)
    {
        fprintf (stderr, "Couldn't initialize fanotify\n");
        exit (EXIT_FAILURE);
    }
    
    /* Setup polling */
    fds[FD_POLL_SIGNAL].fd = signal_fd;
    fds[FD_POLL_SIGNAL].events = POLLIN;
    fds[FD_POLL_FANOTIFY].fd = fanotify_fd;
    fds[FD_POLL_FANOTIFY].events = POLLIN;
    
    /* Now loop */
    for (;;)
    {
        /* Block until there is something to be read */
        if (poll (fds, FD_POLL_MAX, -1) < 0)
        {
            fprintf (stderr,
                     "Couldn't poll(): '%s'\n",
                     strerror (errno));
            exit (EXIT_FAILURE);
        }
        
        /* Signal received? */
        if (fds[FD_POLL_SIGNAL].revents & POLLIN)
        {
            struct signalfd_siginfo fdsi;
            
            if (read (fds[FD_POLL_SIGNAL].fd,
                      &fdsi,
                      sizeof (fdsi)) != sizeof (fdsi))
            {
                fprintf (stderr,
                         "Couldn't read signal, wrong size read\n");
                exit (EXIT_FAILURE);
            }
            
            /* Break loop if we got the expected signal */
            if (fdsi.ssi_signo == SIGINT ||
                fdsi.ssi_signo == SIGTERM)
            {
                break;
            }
            
            fprintf (stderr,
                     "Received unexpected signal\n");
        }
        
        /* fanotify event received? */
        if (fds[FD_POLL_FANOTIFY].revents & POLLIN)
        {
            char buffer[FANOTIFY_BUFFER_SIZE];
            ssize_t length;
            
            /* Read from the FD. It will read all events available up to
             * the given buffer size. */
            if ((length = read (fds[FD_POLL_FANOTIFY].fd,
                                buffer,
                                FANOTIFY_BUFFER_SIZE)) > 0)
            {
                struct fanotify_event_metadata *metadata;
                
                metadata = (struct fanotify_event_metadata *)buffer;
                while (FAN_EVENT_OK (metadata, length))
                {
                    event_process (metadata, fanotify_fd);
                    if (metadata->fd > 0)
                        close (metadata->fd);
                    metadata = FAN_EVENT_NEXT (metadata, length);
                }
            }
        }
    }
    
    /* Clean exit */
    shutdown_fanotify (fanotify_fd);
    shutdown_signals (signal_fd);
    
    printf ("Exiting fanotify example...\n");
    
    return EXIT_SUCCESS;
}
