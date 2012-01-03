#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#define XIDUMP_VERSION "0.8.3"

#include "./util-config.h"

/*****************************************************************************
** XInput
*****************************************************************************/

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>
#include <X11/keysym.h>

	enum
	{
		INPUTEVENT_KEY_PRESS,
		INPUTEVENT_KEY_RELEASE,
		INPUTEVENT_FOCUS_IN,
		INPUTEVENT_FOCUS_OUT,
		INPUTEVENT_BTN_PRESS,
		INPUTEVENT_BTN_RELEASE,
		INPUTEVENT_PROXIMITY_IN,
		INPUTEVENT_PROXIMITY_OUT,
		INPUTEVENT_MOTION_NOTIFY,
		INPUTEVENT_DEVICE_STATE_NOTIFY,
		INPUTEVENT_DEVICE_MAPPING_NOTIFY,
		INPUTEVENT_CHANGE_DEVICE_NOTIFY,
		INPUTEVENT_DEVICE_POINTER_MOTION_HINT,
		INPUTEVENT_DEVICE_BUTTON_MOTION,
		INPUTEVENT_DEVICE_BUTTON1_MOTION,
		INPUTEVENT_DEVICE_BUTTON2_MOTION,
		INPUTEVENT_DEVICE_BUTTON3_MOTION,
		INPUTEVENT_DEVICE_BUTTON4_MOTION,
		INPUTEVENT_DEVICE_BUTTON5_MOTION,

		INPUTEVENT_MAX
	};

	int gnDevListCnt = 0;
	XDeviceInfoPtr gpDevList = NULL;
	int gnLastXError = 0;
	int gnVerbose = 0;
	int gnSuppress = 4;
	int gnInputEvent[INPUTEVENT_MAX] = { 0 };

int ErrorHandler(Display* pDisp, XErrorEvent* pEvent)
{
	char chBuf[64];
	XGetErrorText(pDisp,pEvent->error_code,chBuf,sizeof(chBuf));
	fprintf(stderr,"X Error: %d %s\n", pEvent->error_code, chBuf);
	gnLastXError  = pEvent->error_code;
	return 0;
}

int GetLastXError(void)
{
	return gnLastXError;
}

Display* InitXInput(void)
{
	Display* pDisp;
	int nMajor, nFEV, nFER;

	pDisp = XOpenDisplay(NULL);
	if (!pDisp)
	{
		fprintf(stderr,"Failed to connect to X server.\n");
		return NULL;
	}

	XSetErrorHandler(ErrorHandler);

	XSynchronize(pDisp,1 /*sync on*/);
	
	if (!XQueryExtension(pDisp,INAME,&nMajor,&nFEV,&nFER))
	{
		fprintf(stderr,"Server does not support XInput extension.\n");
		XCloseDisplay(pDisp);
		return NULL;
	}

	return pDisp;
}

XDeviceInfoPtr GetDevice(Display* pDisp, const char* pszDeviceName)
{
	int i;

	/* get list of devices */
	if (!gpDevList)
	{
		gpDevList = (XDeviceInfoPtr) XListInputDevices(pDisp, &gnDevListCnt);
		if (!gpDevList)
		{
			fprintf(stderr,"Failed to get input device list.\n");
			return NULL;
		}
	}

	/* find device by name */
	for (i=0; i<gnDevListCnt; ++i)
	{
		if (!strcasecmp(gpDevList[i].name,pszDeviceName) &&
			gpDevList[i].num_classes)
			return gpDevList + i;
	}

	return NULL;
}

static const char* GetEventName(int nType)
{
	static char xchBuf[64];

	switch (nType)
	{
		case KeyPress: return "KeyPress";
		case KeyRelease: return "KeyRelease";
		case ButtonPress: return "ButtonPress";
		case ButtonRelease: return "ButtonRelease";
		case MotionNotify: return "MotionNotify";
		case EnterNotify: return "EnterNotify";
		case LeaveNotify: return "LeaveNotify";
		case FocusIn: return "FocusIn";
		case FocusOut: return "FocusOut";
		case KeymapNotify: return "KeymapNotify";
		case Expose: return "Expose";
		case GraphicsExpose: return "GraphicsExpose";
		case NoExpose: return "NoExpose";
		case VisibilityNotify: return "VisibilityNotify";
		case CreateNotify: return "CreateNotify";
		case DestroyNotify: return "DestroyNotify";
		case UnmapNotify: return "UnmapNotify";
		case MapNotify: return "MapNotify";
		case MapRequest: return "MapRequest";
		case ReparentNotify: return "ReparentNotify";
		case ConfigureNotify: return "ConfigureNotify";
		case ConfigureRequest: return "ConfigureRequest";
		case GravityNotify: return "GravityNotify";
		case ResizeRequest: return "ResizeRequest";
		case CirculateNotify: return "CirculateNotify";
		case CirculateRequest: return "CirculateRequest";
		case PropertyNotify: return "PropertyNotify";
		case SelectionClear: return "SelectionClear";
		case SelectionRequest: return "SelectionRequest";
		case SelectionNotify: return "SelectionNotify";
		case ColormapNotify: return "ColormapNotify";
		case ClientMessage: return "ClientMessage";
		case MappingNotify: return "MappingNotify";

		default:
		if (nType == gnInputEvent[INPUTEVENT_KEY_PRESS])
			return "XIKeyPress";
		else if (nType == gnInputEvent[INPUTEVENT_KEY_RELEASE])
			return "XIKeyRelease";
		else if (nType == gnInputEvent[INPUTEVENT_FOCUS_IN])
			return "XIFocusIn";
		else if (nType == gnInputEvent[INPUTEVENT_FOCUS_OUT])
			return "XIFocusOut";
		else if (nType == gnInputEvent[INPUTEVENT_BTN_PRESS])
			return "XIButtonPress";
		else if (nType == gnInputEvent[INPUTEVENT_BTN_RELEASE])
			return "XIButtonRelease";
		else if (nType == gnInputEvent[INPUTEVENT_PROXIMITY_IN])
			return "XIProximityIn";
		else if (nType == gnInputEvent[INPUTEVENT_PROXIMITY_OUT])
			return "XIProximityOut";
		else if (nType == gnInputEvent[INPUTEVENT_MOTION_NOTIFY])
			return "XIMotionNotify";
		else if (nType == gnInputEvent[INPUTEVENT_DEVICE_STATE_NOTIFY])
			return "XIDeviceStateNotify";
		else if (nType == gnInputEvent[INPUTEVENT_DEVICE_MAPPING_NOTIFY])
			return "XIDeviceMappingNotify";
		else if (nType == gnInputEvent[INPUTEVENT_CHANGE_DEVICE_NOTIFY])
			return "XIChangeDeviceNotify";
		else if (nType == gnInputEvent[INPUTEVENT_DEVICE_POINTER_MOTION_HINT])
			return "XIDevicePointerMotionHint";
		else if (nType == gnInputEvent[INPUTEVENT_DEVICE_BUTTON_MOTION])
			return "XIDeviceButtonMotion";
		else if (nType == gnInputEvent[INPUTEVENT_DEVICE_BUTTON1_MOTION])
			return "XIDeviceButton1Motion";
		else if (nType == gnInputEvent[INPUTEVENT_DEVICE_BUTTON2_MOTION])
			return "XIDeviceButton2Motion";
		else if (nType == gnInputEvent[INPUTEVENT_DEVICE_BUTTON3_MOTION])
			return "XIDeviceButton3Motion";
		else if (nType == gnInputEvent[INPUTEVENT_DEVICE_BUTTON4_MOTION])
			return "XIDeviceButton4Motion";
		else if (nType == gnInputEvent[INPUTEVENT_DEVICE_BUTTON5_MOTION])
			return "XIDeviceButton5Motion";
	}

	snprintf(xchBuf,sizeof(xchBuf),"Event_%d",nType);
	return xchBuf;
}


/*****************************************************************************
** FORMAT
*****************************************************************************/

typedef enum
{
	FORMATTYPE_DEFAULT,
	FORMATTYPE_ACCELERATION
} FORMATTYPE;

typedef struct _FORMAT FORMAT;
struct _FORMAT
{
	const char* pszName;
	FORMATTYPE type;
};

	FORMAT gFormats[] =
	{
		{ "default", FORMATTYPE_DEFAULT },
		{ "accel", FORMATTYPE_ACCELERATION },
		{ NULL }
	};


/*****************************************************************************
** UI
*****************************************************************************/

typedef struct _UI UI;
struct _UI
{
	const char* pszName;
	int (*Init)(void);
	void (*Term)(void);
	int (*Run)(Display* pDisp, XDeviceInfo* pDevInfo, FORMATTYPE fmt);
};

/*****************************************************************************
** Raw UI
*****************************************************************************/

static int RawInit(void)
{
	return 0;
}

static void RawTerm(void)
{
}

static int RawRunDefault(Display* pDisp, XDeviceInfo* pDevInfo)
{
	XEvent event;
	XAnyEvent* pAny;
	struct timeval tv;
	double dStart, dNow;

	gettimeofday(&tv,NULL);
	dStart = tv.tv_sec + (double)tv.tv_usec / 1E6;

	while (1)
	{
		XNextEvent(pDisp,&event);

		pAny = (XAnyEvent*)&event;
		/* printf("event: type=%s\n",GetEventName(pAny->type)); */

		/* display time */
		gettimeofday(&tv,NULL);
		dNow = tv.tv_sec + (double)tv.tv_usec / 1E6;
		printf("%.8f: ",(dNow - dStart));

		if (pAny->type == gnInputEvent[INPUTEVENT_PROXIMITY_IN])
			printf("Proximity In\n");
		else if (pAny->type == gnInputEvent[INPUTEVENT_PROXIMITY_OUT])
			printf("Proximity Out\n");
		else if (pAny->type == gnInputEvent[INPUTEVENT_FOCUS_IN])
			printf("Focus In\n");
		else if (pAny->type == gnInputEvent[INPUTEVENT_FOCUS_OUT])
			printf("Focus Out\n");
		else if (pAny->type == gnInputEvent[INPUTEVENT_MOTION_NOTIFY])
		{
			printf("Motion: x=%+6d y=%+6d p=%4d tx=%+4d ty=%+4d "
				"w=%+5d \n",
					((XDeviceMotionEvent*)pAny)->axis_data[0],
					((XDeviceMotionEvent*)pAny)->axis_data[1],
					((XDeviceMotionEvent*)pAny)->axis_data[2],
					(short)(((XDeviceMotionEvent*)pAny)->axis_data[3]&0xffff),
					(short)(((XDeviceMotionEvent*)pAny)->axis_data[4]&0xffff),
					(short)(((XDeviceMotionEvent*)pAny)->axis_data[5]&0xffff));

		}
		else if ((pAny->type == gnInputEvent[INPUTEVENT_BTN_PRESS]) ||
				(pAny->type == gnInputEvent[INPUTEVENT_BTN_RELEASE]))
		{
			XDeviceButtonEvent* pBtn = (XDeviceButtonEvent*)pAny;
			printf("Button: %d %s\n",pBtn->button,
					pAny->type == gnInputEvent[INPUTEVENT_BTN_PRESS] ?
						"DOWN" : "UP");
		}
		else if ((pAny->type == gnInputEvent[INPUTEVENT_KEY_PRESS]) ||
				(pAny->type == gnInputEvent[INPUTEVENT_KEY_RELEASE]))
		{
			XDeviceKeyEvent* pKey = (XDeviceKeyEvent*)pAny;
			printf("Key: %d %s\n", pKey->keycode - 7,
			       (pAny->type == gnInputEvent[INPUTEVENT_KEY_PRESS]) ?
			       "DOWN" : "UP");
		}
		else
		{
			printf("Event: %s\n",GetEventName(pAny->type));
		}

		/* flush data to terminal */
		fflush(stdout);
	}

	return 0;
}

static int RawRun(Display* pDisp, XDeviceInfo* pDevInfo, FORMATTYPE fmt)
{
	return RawRunDefault(pDisp,pDevInfo);
}

	UI gRawUI = { "raw", RawInit, RawTerm, RawRun };

/****************************************************************************/

	UI* gpUIs[] =
	{
		/* Raw UI is always available */
		&gRawUI,
		NULL
	};



/****************************************************************************/

int Run(Display* pDisp, UI* pUI, FORMATTYPE fmt, const char* pszDeviceName)
{
	int nRtn;
	XDevice* pDev;
	XDeviceInfoPtr pDevInfo;
	int nEventListCnt = 0;
	XEventClass eventList[32];
	XEventClass cls;

	/* get the device by name */
	pDevInfo = GetDevice(pDisp,pszDeviceName);
	if (!pDevInfo)
	{
		fprintf(stderr,"Unable to find input device '%s'\n",pszDeviceName);
		return 1;
	}

	/* open device */
	pDev = XOpenDevice(pDisp,pDevInfo->id);
	if (!pDev)
	{
		fprintf(stderr,"Unable to open input device '%s'\n",pszDeviceName);
		return 1;
	}

	/* key events */
	DeviceKeyPress(pDev,gnInputEvent[INPUTEVENT_KEY_PRESS],cls);
	if (cls) eventList[nEventListCnt++] = cls;
	DeviceKeyRelease(pDev,gnInputEvent[INPUTEVENT_KEY_RELEASE],cls);
	if (cls) eventList[nEventListCnt++] = cls;

	/* focus events */
	DeviceFocusIn(pDev,gnInputEvent[INPUTEVENT_FOCUS_IN],cls);
	if (cls) eventList[nEventListCnt++] = cls;
	DeviceFocusOut(pDev,gnInputEvent[INPUTEVENT_FOCUS_OUT],cls);
	if (cls) eventList[nEventListCnt++] = cls;

	/* button events */
	DeviceButtonPress(pDev,gnInputEvent[INPUTEVENT_BTN_PRESS],cls);
	if (cls) eventList[nEventListCnt++] = cls;
	DeviceButtonRelease(pDev,gnInputEvent[INPUTEVENT_BTN_RELEASE],cls);
	if (cls) eventList[nEventListCnt++] = cls;

	/* proximity events */
	ProximityIn(pDev,gnInputEvent[INPUTEVENT_PROXIMITY_IN],cls);
	if (cls) eventList[nEventListCnt++] = cls;
	ProximityOut(pDev,gnInputEvent[INPUTEVENT_PROXIMITY_OUT],cls);
	if (cls) eventList[nEventListCnt++] = cls;

	/* motion events */
	DeviceMotionNotify(pDev,gnInputEvent[INPUTEVENT_MOTION_NOTIFY],cls);
	if (cls) eventList[nEventListCnt++] = cls;

	/* device state */
	DeviceStateNotify(pDev,gnInputEvent[INPUTEVENT_DEVICE_STATE_NOTIFY],cls);
	if (cls) eventList[nEventListCnt++] = cls;
	DeviceMappingNotify(pDev,
			gnInputEvent[INPUTEVENT_DEVICE_MAPPING_NOTIFY],cls);
	if (cls) eventList[nEventListCnt++] = cls;
	ChangeDeviceNotify(pDev,gnInputEvent[INPUTEVENT_CHANGE_DEVICE_NOTIFY],cls);
	if (cls) eventList[nEventListCnt++] = cls;

#if 0
	/* this cuts the motion data down - not sure if this is useful */
	DevicePointerMotionHint(pDev,
			gnInputEvent[INPUTEVENT_DEVICE_POINTER_MOTION_HINT],cls);
	if (cls) eventList[nEventListCnt++] = cls;
#endif

	/* button motion */
	DeviceButtonMotion(pDev,
			gnInputEvent[INPUTEVENT_DEVICE_BUTTON_MOTION],cls);
	if (cls) eventList[nEventListCnt++] = cls;
	DeviceButton1Motion(pDev,
			gnInputEvent[INPUTEVENT_DEVICE_BUTTON1_MOTION],cls);
	if (cls) eventList[nEventListCnt++] = cls;
	DeviceButton2Motion(pDev,
			gnInputEvent[INPUTEVENT_DEVICE_BUTTON2_MOTION],cls);
	if (cls) eventList[nEventListCnt++] = cls;
	DeviceButton3Motion(pDev,
			gnInputEvent[INPUTEVENT_DEVICE_BUTTON3_MOTION],cls);
	if (cls) eventList[nEventListCnt++] = cls;
	DeviceButton4Motion(pDev,
			gnInputEvent[INPUTEVENT_DEVICE_BUTTON4_MOTION],cls);
	if (cls) eventList[nEventListCnt++] = cls;
	DeviceButton5Motion(pDev,
			gnInputEvent[INPUTEVENT_DEVICE_BUTTON5_MOTION],cls);
	if (cls) eventList[nEventListCnt++] = cls;

	/* specify which events to report */
	/* XSelectInput(pDisp,wnd,0x00FFFFFF ^ PointerMotionHintMask); */
	/* XSelectExtensionEvent(pDisp,wnd,eventList,nEventListCnt); */

	/* grab device - work whether pointer is in active window or not */
	XGrabDevice(pDisp,pDev,DefaultRootWindow(pDisp),
			0, /* no owner events */
			nEventListCnt, eventList, /* events */
			GrabModeAsync, /* don't queue, give me whatever you got */
			GrabModeAsync, /* same */
			CurrentTime);

	/* fire up the UI */
	if ((nRtn=pUI->Init()) != 0)
		fprintf(stderr,"failed to initialize UI\n");
	else
	{
		if ((nRtn=pUI->Run(pDisp,pDevInfo,fmt)) != 0)
			fprintf(stderr,"failed to run UI\n");
		pUI->Term();
	}

	XUngrabDevice(pDisp,pDev,CurrentTime);
	XFree(pDev);
	XCloseDisplay(pDisp);

	return nRtn;
}

/*****************************************************************************
** main
*****************************************************************************/

int main(int argc, char** argv)
{
	int nRtn;
	int bList = 0;
	UI* pUI=NULL, **ppUI;
	FORMAT* pFmt;
	FORMATTYPE fmt=FORMATTYPE_DEFAULT;
	const char* pa;
	Display* pDisp = NULL;
	const char* pszDeviceName = NULL;

	++argv;
	pa = *(argv++); 
	pszDeviceName = pa;

	/* device must be specified */
	if (!pszDeviceName && !bList)
	{
		fprintf(stderr,"input_device not specified\n");
	}


	/* default to first valid UI, if not specified */
	if (pUI == NULL)
		pUI = gpUIs[0];
	
	/* open connection to XServer with XInput */
	pDisp = InitXInput();
	if (!pDisp) exit(1);

	nRtn = Run(pDisp,pUI,fmt,pszDeviceName);

	/* release device list */
	if (gpDevList)
		XFreeDeviceList(gpDevList);

	XCloseDisplay(pDisp);

	return nRtn;
}
