#ifndef	_INCLUDESYSTEM_H
#define	_INCLUDESYSTEM_H

/*	Standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
//#include <getopt.h>
#include <limits.h>

/*	System header */
#include <pwd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>

/*	Measuring Time */
#include <time.h>
#include <sys/times.h>
#include <sys/time.h>

/*	Thread */
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>

/*	IPC */
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>

/*	Socket */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*	File system */
#include <dirent.h>
#include <fcntl.h>

/*	Platform specific */
#ifndef	_HPUX_SOURCE
#include <stdarg.h>
#include <inttypes.h>
#endif
#ifdef	_AIX
#include <standards.h>
#endif

/*	Additional feature */
#include <dlfcn.h>
// #include <aio.h>	

//#include <string>
//#include <sstream>
#include <vector>
#include <list>
//#include <map>
#include <stdexcept>
//#include <algorithm>

#endif
