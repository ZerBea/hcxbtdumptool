#define _GNU_SOURCE
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <curses.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "include/hcxbtdumptool.h"
/*===========================================================================*/
/* global var */

static int fd_socket;
static int deviceid;
bdaddr_t deviceaddr;

/*===========================================================================*/
static void globalclose()
{
if(fd_socket > 0)
	{
	if(close(fd_socket) != 0) perror("failed to close HCI socket");
	}
exit(EXIT_SUCCESS);
}
/*===========================================================================*/
static inline bool globalinit()
{
fd_socket = -1;

return true;
}
/*===========================================================================*/
static inline bool opensocket()
{
if((fd_socket = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
	{
	perror("failed to open HCI socket");
	return false;
	}



return true;
}
/*===========================================================================*/
static void showdevices()
{
printf("\nID MAC\n---------------\n"); 
for(deviceid = 0; deviceid < 255; deviceid++)
if(hci_devba(deviceid, &deviceaddr) >= 0) printf("%02d %02x%02x%02x%02x%02x%02x\n", deviceid, deviceaddr.b[5], deviceaddr.b[4], deviceaddr.b[3], deviceaddr.b[2], deviceaddr.b[1], deviceaddr.b[0]);
printf("\n");
return;
}
/*===========================================================================*/
__attribute__ ((noreturn))
static inline void version(char *eigenname)
{
printf("%s %s (C) %s ZeroBeat\n", eigenname, VERSION_TAG, VERSION_YEAR);
exit(EXIT_SUCCESS);
}
/*---------------------------------------------------------------------------*/
__attribute__ ((noreturn))
static inline void usage(char *eigenname)
{
printf("%s %s  (C) %s ZeroBeat\n"
	"usage  : %s <options>\n"
	"         press the switch to terminate hcxbtdumptool\n"
	"         hardware modification is necessary, read more:\n"
	"         https://github.com/ZerBea/hcxdumptool/tree/master/docs\n"
	"\n"
	"short options:\n"
	"-D         : show available devices\n"
	"-h         : show this help\n"
	"-v         : show version\n"
	"\n"
	"long options:\n"
	"--help            : show this help\n"
	"--version         : show version\n"
	"\n"
	"\n",
	eigenname, VERSION_TAG, VERSION_YEAR, eigenname);
exit(EXIT_SUCCESS);
}
/*---------------------------------------------------------------------------*/
__attribute__ ((noreturn))
static inline void usageerror(char *eigenname)
{
printf("%s %s (C) %s by ZeroBeat\n"
	"usage: %s -h for help\n", eigenname, VERSION_TAG, VERSION_YEAR, eigenname);
exit(EXIT_FAILURE);
}
/*===========================================================================*/
int main(int argc, char *argv[])
{
static int auswahl;
static int index;
static bool showdeviceflag;
static const char *short_options = "Dhv";
static const struct option long_options[] =
{
	{"version",			no_argument,		NULL,	HCX_VERSION},
	{"help",			no_argument,		NULL,	HCX_HELP},
	{NULL,				0,			NULL,	0}
};

auswahl = -1;
index = 0;
optind = 1;
optopt = 0;
showdeviceflag = false;
while((auswahl = getopt_long(argc, argv, short_options, long_options, &index)) != -1)
	{
	switch (auswahl)
		{

		case HCX_SHOWDEVICELIST:
		showdeviceflag = true;
		break;

		case HCX_HELP:
		usage(basename(argv[0]));
		break;

		case HCX_VERSION:
		version(basename(argv[0]));
		break;

		case '?':
		usageerror(basename(argv[0]));
		break;
		}
	}

setbuf(stdout, NULL);
if(argc < 2)
	{
	fprintf(stderr, "no option selected\n");
	exit(EXIT_FAILURE);
	}
if(getuid() != 0)
	{
	fprintf(stderr, "this program requires root privileges\n");
	globalclose();
	}

if(globalinit() == false)
	{
	fprintf(stderr, "initialization failed\n");
	globalclose();
	}

if(showdeviceflag == true)
	{
	showdevices();
	globalclose();
	}

globalclose();
return EXIT_SUCCESS;
}
/*===========================================================================*/
