#define _GNU_SOURCE
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <curses.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#include "include/hcxbtdumptool.h"
#include "include/rpigpio.h"

/*===========================================================================*/
/* global var */

static int fd_socket;
static int deviceid;
static bdaddr_t deviceaddr;
static struct hci_dev_info deviceinfo;

static int gpiostatusled;
static int gpiobutton;
static struct timespec sleepled;
static struct timespec sleepled2;

static bool wantstopflag;
static unsigned char ble[HCI_MAX_FRAME_SIZE +2];
/*===========================================================================*/
static void globalclose()
{
struct hci_dev_stats *devicestats = &deviceinfo.stat;

printf("\nterminating...\e[?25h\n");
sync();

if(fd_socket > 0)
	{
	if(ioctl(fd_socket, HCIGETDEVINFO, (void*) &deviceinfo) >= 0)
		{
		printf("INCOMING: %d ERROR: %d\n",
			devicestats->byte_rx, devicestats->err_rx);
		printf("OUTGOING: %d ERROR: %d\n",
			devicestats->byte_tx, devicestats->err_tx);
		}
	if(close(fd_socket) != 0) perror("failed to close HCI socket");
	}
printf("\n");
exit(EXIT_SUCCESS);
}
/*===========================================================================*/
static inline void process_ble_packet()
{ 


return;
}
/*===========================================================================*/
static void blescanloop()
{
struct hci_filter filter;
struct sockaddr_hci addr;

static int fdnum;
static fd_set readfds;
static struct timeval tvfd;

uint8_t scan_type = 0x00; /* passive - active */
uint16_t interval = htobs(0x0010);
uint16_t window = htobs(0x0010);
uint8_t own_type = 0x00;
uint8_t filter_policy = 0x00; /* whitelist - blacklist */

hci_filter_clear(&filter);
hci_filter_all_ptypes(&filter);
hci_filter_all_events(&filter);
    
if(setsockopt(fd_socket, SOL_HCI, HCI_FILTER, &filter, sizeof(filter)) == -1)
	{
	perror("failed to set socket options"); return;
	}

memset(&addr, 0, sizeof(addr));
addr.hci_family = AF_BLUETOOTH;
addr.hci_dev = 0;
if(bind(fd_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
	perror("failed to bind socket"); return;
	}

if(hci_le_set_scan_parameters(fd_socket, scan_type, interval, window, own_type, filter_policy, 1000) < 0)
	{
	perror("failed to set scan parameters"); return;
	}
    
if(hci_le_set_scan_enable(fd_socket, 1, 0, 1000) < 0)
	{
	perror("failed to enable BLE scan"); return;
	} 
tvfd.tv_sec = FDUSECTIMER;
tvfd.tv_usec = 0;
while(1)
	{
	if(wantstopflag == true) return;
	FD_ZERO(&readfds);
	FD_SET(fd_socket, &readfds);
	fdnum = select(fd_socket +1, &readfds, NULL, NULL, &tvfd);
	if(fdnum < 0)
		{
		continue;
		}
	if(FD_ISSET(fd_socket, &readfds)) process_ble_packet();
	else
		{
		tvfd.tv_sec = FDUSECTIMER;
		tvfd.tv_usec = 0;
		}
	}
return;
}
/*===========================================================================*/
static void bdscanloop()
{
static struct hci_inquiry_req *hir;
static inquiry_info *zeiger;
static uint8_t mc;
static uint8_t nrsp = 255;
static uint8_t len = 8;
static uint16_t flags = IREQ_CACHE_FLUSH;
uint8_t hirb[sizeof(struct hci_inquiry_req) + (sizeof(inquiry_info) *nrsp)];
int socket;

char name[248] = { 0 };

socket = hci_open_dev( deviceid );


hir = (struct hci_inquiry_req*)hirb;
while(1)
	{
	if(wantstopflag == true) return;
	hir->dev_id  = deviceid;
	hir->flags   = flags;
	hir->lap[0] = 0x33;
	hir->lap[1] = 0x8b;
	hir->lap[2] = 0x9e;
	hir->length  = len;
	hir->num_rsp = nrsp;
	if(ioctl(fd_socket, HCIINQUIRY, hir) < 0) return;

	zeiger = (inquiry_info*)(hirb + sizeof(*hir));
	for(mc = 0; mc < hir->num_rsp; mc++)
		{
		if(hci_read_remote_name(socket, &zeiger->bdaddr, sizeof(name), name, 0) < 0) strcpy(name, "[unknown]");
		printf("%02x%02x%02x%02x%02x%02x %s\n", zeiger->bdaddr.b[5], zeiger->bdaddr.b[4], zeiger->bdaddr.b[3], zeiger->bdaddr.b[2], zeiger->bdaddr.b[1], zeiger->bdaddr.b[0], name);
		zeiger++;
		}
	}
return;
}
/*===========================================================================*/
/*===========================================================================*/
static inline void programmende(int signum)
{
if((signum == SIGINT) || (signum == SIGTERM) || (signum == SIGKILL)) wantstopflag = true;
return;
}
/*===========================================================================*/
static inline size_t chop(char *buffer, size_t len)
{
static char *ptr;

ptr = buffer +len -1;
while(len)
	{
	if (*ptr != '\n') break;
	*ptr-- = 0;
	len--;
	}
while(len)
	{
	if (*ptr != '\r') break;
	*ptr-- = 0;
	len--;
	}
return len;
}
/*---------------------------------------------------------------------------*/
static inline int fgetline(FILE *inputstream, size_t size, char *buffer)
{
static size_t len;
static char *buffptr;

if(feof(inputstream)) return -1;
buffptr = fgets (buffer, size, inputstream);
if(buffptr == NULL) return -1;
len = strlen(buffptr);
len = chop(buffptr, len);
return len;
}
/*===========================================================================*/
static inline bool initgpio(int gpioperi)
{
static int fd_mem;

fd_mem = open("/dev/mem", O_RDWR|O_SYNC);
if(fd_mem < 0)
	{
	fprintf(stderr, "failed to get device memory\n");
	return false;
	}
gpio_map = mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, GPIO_BASE +gpioperi);
close(fd_mem);
if(gpio_map == MAP_FAILED)
	{
	fprintf(stderr, "failed to map GPIO memory\n");
	return false;
	}
gpio = (volatile unsigned *)gpio_map;
return true;
}
/*===========================================================================*/
static inline int getrpirev()
{
static FILE *fh_rpi;
static int len;
static int rpi = 0;
static int rev = 0;
static int gpioperibase = 0;
static char *revptr = NULL;
static const char *revstr = "Revision";
static const char *hwstr = "Hardware";
static const char *snstr = "Serial";
static char linein[128];

fh_rpi = fopen("/proc/cpuinfo", "r");
if(fh_rpi == NULL)
	{
	perror("failed to retrieve cpuinfo");
	return gpioperibase;
	}
while(1)
	{
	if((len = fgetline(fh_rpi, 128, linein)) == -1) break;
	if(len < 15) continue;
	if(memcmp(&linein, hwstr, 8) == 0)
		{
		rpi |= 1;
		continue;
		}
	if(memcmp(&linein, revstr, 8) == 0)
		{
		rpirevision = strtol(&linein[len -6], &revptr, 16);
		if((revptr - linein) == len)
			{
			rev = (rpirevision >> 4) &0xff;
			if(rev <= 3)
				{
				gpioperibase = GPIO_PERI_BASE_OLD;
				rpi |= 2;
				continue;
				}
			if(rev == 0x09)
				{
				gpioperibase = GPIO_PERI_BASE_OLD;
				rpi |= 2;
				continue;
				}
			if(rev == 0x0c)
				{
				gpioperibase = GPIO_PERI_BASE_OLD;
				rpi |= 2;
				continue;
				}
			if((rev == 0x04) || (rev == 0x08) || (rev == 0x0d) || (rev == 0x0e) || (rev == 0x11))
				{
				gpioperibase = GPIO_PERI_BASE_NEW;
				rpi |= 2;
				continue;
				}
			continue;
			}
		rpirevision = strtol(&linein[len -4], &revptr, 16);
		if((revptr - linein) == len)
			{
			if((rpirevision < 0x02) || (rpirevision > 0x15)) continue;
			if((rpirevision == 0x11) || (rpirevision == 0x14)) continue;
			gpioperibase = GPIO_PERI_BASE_OLD;
			rpi |= 2;
			}
		continue;
		}
	if(memcmp(&linein, snstr, 6) == 0)
		{
		rpi |= 4;
		continue;
		}
	}
fclose(fh_rpi);
if(rpi < 0x7) return 0;
return gpioperibase;
}
/*===========================================================================*/
static inline bool globalinit()
{
static int c;
static int gpiobasemem = 0;

wantstopflag = false;
fd_socket = -1;
sleepled.tv_sec = 0;
sleepled.tv_nsec = GPIO_LED_DELAY;
sleepled2.tv_sec = 0;
sleepled2.tv_nsec = GPIO_LED_DELAY +GPIO_LED_DELAY;
rpirevision = 0;
if((gpiobutton > 0) || (gpiostatusled > 0))
	{
	if(gpiobutton == gpiostatusled)
		{
		fprintf(stderr, "same value for wpi_button and wpi_statusled is not allowed\n");
		return false;
		}
	gpiobasemem = getrpirev();
	if(gpiobasemem == 0)
		{
		fprintf(stderr, "failed to locate GPIO\n");
		return false;
		}
	if(initgpio(gpiobasemem) == false)
		{
		fprintf(stderr, "failed to init GPIO\n");
		return false;
		}
	if(gpiostatusled > 0)
		{
		INP_GPIO(gpiostatusled);
		OUT_GPIO(gpiostatusled);
		}
	if(gpiobutton > 0)
		{
		INP_GPIO(gpiobutton);
		}
	}
if(gpiostatusled > 0)
	{
	for (c = 0; c < 5; c++)
		{
		GPIO_SET = 1 << gpiostatusled;
		nanosleep(&sleepled, NULL);
		GPIO_CLR = 1 << gpiostatusled;
		nanosleep(&sleepled2, NULL);
		}
	}

signal(SIGINT, programmende);
return true;
}
/*===========================================================================*/
static inline bool opensocket()
{
if(deviceid == -1)
	{
	printf("no device selected, using first reachable device\n");
	deviceid = hci_devid(NULL);
	}
if(deviceid < 0)
	{
	fprintf(stderr, "found no suitable device");
	return false;
	}

if((fd_socket = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI)) < 0)
	{
	perror("failed to open HCI socket");
	return false;
	}

if(ioctl(fd_socket, HCIDEVDOWN, deviceid) < 0)
	{
	fprintf(stderr, "failed to bring down device %d down: %s\n", deviceid, strerror(errno));
	return false;
	}

if(ioctl(fd_socket, HCIDEVUP, deviceid) < 0)
	{
	if(errno == EALREADY) return false;
	fprintf(stderr, "failed to inititialize device %d: %s\n", deviceid, strerror(errno));
	return false;
	}

if(ioctl(fd_socket, HCIDEVRESTAT, deviceid) < 0)
	{
	fprintf(stderr, "failed to reset stats counters on device %d: %s", deviceid, strerror(errno));
	return false;
	}

if(ioctl(fd_socket, HCIGETDEVINFO, (void*) &deviceinfo) < 0)
	{
	perror("failed to get device information");
	return false;
	}
return true;
}
/*===========================================================================*/
static void showdevices()
{
static bool btlflag = false;

printf("\nID MAC\n---------------\n"); 
for(deviceid = 0; deviceid < 255; deviceid++)
if(hci_devba(deviceid, &deviceaddr) >= 0)
	{
	btlflag = true;
	printf("%02d %02x%02x%02x%02x%02x%02x\n", deviceid, deviceaddr.b[5], deviceaddr.b[4], deviceaddr.b[3], deviceaddr.b[2], deviceaddr.b[1], deviceaddr.b[0]);
	}

if(btlflag == false)
	{
	printf("found no suitable device or Bleutooth not activated!\n"
		"Procedure to active Bluetooth:\n"
		"systemctl start bluetooth\n"
		"bluetoothctl power on\n");
	}
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
	"-d <digit> : input device id\n"
	"-D         : show available devices\n"
	"-h         : show this help\n"
	"-v         : show version\n"
	"\n"
	"long options:\n"
	"--bdscan          : scan for Bluetooth devices in range\n"
	"--help            : show this help\n"
	"--version         : show version\n"
	"\n"
	"Procedure to active Bluetooth:\n"
	"systemctl start bluetooth\n"
	"bluetoothctl power on\n"
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
static bool bdscanflag;
static const char *short_options = "d:Dhv";
static const struct option long_options[] =
{
	{"bdscan",			no_argument,		NULL,	HCX_BDSCAN},
	{"gpio_button",			required_argument,	NULL,	HCX_GPIO_BUTTON},
	{"gpio_statusled",		required_argument,	NULL,	HCX_GPIO_STATUSLED},
	{"version",			no_argument,		NULL,	HCX_VERSION},
	{"help",			no_argument,		NULL,	HCX_HELP},
	{NULL,				0,			NULL,	0}
};

auswahl = -1;
index = 0;
optind = 1;
optopt = 0;
deviceid = -1;
gpiobutton = 0;
gpiostatusled = 0;
showdeviceflag = false;
bdscanflag = false;

while((auswahl = getopt_long(argc, argv, short_options, long_options, &index)) != -1)
	{
	switch (auswahl)
		{
		case HCX_DEVICEID:
		deviceid = strtol(optarg, NULL, 10);
		break;

		case HCX_BDSCAN:
		bdscanflag = true;
		break;

		case HCX_GPIO_BUTTON:
		gpiobutton = strtol(optarg, NULL, 10);
		if((gpiobutton < 2) || (gpiobutton > 27))
			{
			fprintf(stderr, "only 2...27 allowed\n");
			exit(EXIT_FAILURE);
			}
		break;

		case HCX_GPIO_STATUSLED:
		gpiostatusled = strtol(optarg, NULL, 10);
		if((gpiostatusled < 2) || (gpiostatusled > 27))
			{
			fprintf(stderr, "only 2...27 allowed\n");
			exit(EXIT_FAILURE);
			}
		break;

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

if(showdeviceflag == true)
	{
	showdevices();
	return EXIT_SUCCESS;
	}

if(getuid() != 0)
	{
	fprintf(stderr, "this program requires root privileges\n");
	globalclose();
	}

printf("initialization...\n");
if(globalinit() == false)
	{
	fprintf(stderr, "initialization failed\n");
	globalclose();
	}

if(opensocket() == false)
	{
	fprintf(stderr, "initialization failed\n");
	globalclose();
	}

if(bdscanflag == true)
	{
	bdscanloop();
	globalclose();
	}

globalclose();
return EXIT_SUCCESS;
}
/*===========================================================================*/
