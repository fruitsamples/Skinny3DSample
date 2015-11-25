#include <Printing.h>#include <AppleEvents.h>#include <Windows.h>#include <Fonts.h>#include <ToolUtils.h>#include <TextUtils.h>#include <LowMem.h>#include <SegLoad.h>#include <Devices.h>#include <Quickdraw.h>#include	"SkinnyMain.h"#include 	"3DAppSpecific.h"#define		kMinHeap	100000enum { aboutBoxID = 129 }; // resource IDsBoolean		gDone = false;				/* true when Quit is selected */Boolean		gInBackGround = false;		/* true if we are in the background */THPrint		gPrintH;CursHandle	gWatchCursor;// local prototypesstatic void MainEventLoop(void);static void DoClick(EventRecord *evt);static void DoKey(EventRecord *evt);static void DoUpdate(EventRecord *evt);static void DoActivate(EventRecord *evt);static void DoOSEvent(EventRecord *evt);static void UpdateMenus(void);static void DoMenu(long msel);static void ChooseApple(short itemNumber);static void ChooseFile(short item);static void ChooseEdit(short item);static void DoPageSetup(void);static OSErr InstallAEHandlers(void);static pascal OSErr AEOpenHandler(AppleEvent *messagein, AppleEvent *reply, long refIn);static pascal OSErr AEOpenDocHandler(AppleEvent *messagein, AppleEvent *reply, long refIn);static pascal OSErr AEPrintHandler(AppleEvent *messagein, AppleEvent *reply, long refIn);static pascal OSErr AEQuitHandler(AppleEvent *messagein, AppleEvent *reply, long refIn);//==============================================================void main(void){	long	masterBlocks = 10;	long 	err;	MaxApplZone();	while ( masterBlocks-- )		MoreMasters();	InitGraf(&qd.thePort);	InitFonts();	InitWindows();	InitMenus();	InitCursor();	TEInit();	FlushEvents(everyEvent, 0);	InitDialogs(0L);	gWatchCursor = GetCursor(watchCursor);	if (!gWatchCursor)		return;	HNoPurge((Handle) gWatchCursor);	err = InitializeApplication(); // app-specific	if (err != noErr) {		ErrMsg("\pInitialization failed.");		return;	}		DoNew(); // app-specific	MainEventLoop();	Cleanup();}//------------------------------------static void MainEventLoop(void){	EventRecord	event;	Boolean		gotEvent;	long		grow;		while	(!gDone) {		gotEvent = WaitNextEvent(everyEvent, &event, 10, nil);		if (gotEvent) {			switch(event.what) {				case nullEvent:								break;				case mouseDown:		DoClick(&event);		break;				case mouseUp:		 						break;				case keyDown:		DoKey(&event);			break;				case keyUp:			 						break;				case autoKey:		DoKey(&event);			break;				case updateEvt:		DoUpdate(&event);		break;				case diskEvt:		 						break;				case activateEvt:	DoActivate(&event);		break;				case osEvt:			DoOSEvent(&event);		break;				default:									break;			}		}		if (MaxMem(&grow) < kMinHeap) {		ErrMsg("\pSorry, need to quit (too many memory leaks)");		gDone = true;		}	}}//------------------------------------static void DoKey(EventRecord *evt){	char c = (char)evt->message & charCodeMask;		if ((evt->modifiers & cmdKey)) {		UpdateMenus();		DoMenu(MenuKey(evt->message & charCodeMask));	}}//------------------------------------static void DoUpdate(EventRecord *evt){	WindowPtr	updateWindow;	GrafPtr		savePort;		GetPort(&savePort);	updateWindow = (WindowPtr)evt->message;	SetPort(updateWindow);	BeginUpdate(updateWindow);							DrawAppSpecificContent(updateWindow); // app-specific; erase as required	EndUpdate(updateWindow);	SetPort(savePort);}//------------------------------------static void DoActivate(EventRecord *evt){	ActivateWindow((WindowPtr)evt->message, (evt->modifiers & activeFlag));}//------------------------------------static void DoOSEvent(EventRecord *evt){	if ( (evt->message >> 24) == suspendResumeMessage)		gInBackGround = !(evt->message & resumeFlag);}//------------------------------------static void DoClick(EventRecord *evt){	WindowPtr	w;		switch (FindWindow(evt->where, &w)) {		case inDesk:		break;		case inMenuBar:		UpdateMenus();							DoMenu(MenuSelect(evt->where));							break;		case inSysWindow:	SystemClick(evt, w);							break;		case inContent:		if (w != FrontWindow())								SelectWindow(w);							else {								SetPort(w);								DoClickInContent(evt, w);							}							break;		case inDrag:		DragWindow(w, evt->where, &qd.screenBits.bounds);							break;		case inGoAway:		if (TrackGoAway(w, evt->where))								DoClose();							break;		default:			break;	}}//------------------------------------static void UpdateMenus(void){	MenuHandle	menu = GetMenuHandle(mFile);	if (PreflightNew()) // application-specific		EnableItem(menu, iNew);	else		DisableItem(menu, iNew);	if (FrontWindow())		EnableItem(menu, iClose);	else		DisableItem(menu, iClose);	UpdateAppMenus();}//------------------------------------static void DoMenu(long msel){	short item = LoWord(msel);	short menu = HiWord(msel);	switch (menu) {		case mApple: ChooseApple(item);		break;		case mFile : ChooseFile(item);		break;		case mEdit : ChooseEdit(item);		break;		default :	 DoAppSpecificMenu(menu, item); // app-specific	}	HiliteMenu(0);}//------------------------------------static void ChooseApple(short itemNumber){	short dontCare;		if (itemNumber == iAbout)	{		dontCare = Alert(aboutBoxID, nil);	} 	else 	{		GrafPtr		savePort;		Str255		daName;		short		daRefNum;				GetPort(&savePort);		GetMenuItemText(GetMenuHandle(mApple), itemNumber, daName);		daRefNum = OpenDeskAcc(daName);		SetPort(savePort);	}}//------------------------------------static void ChooseFile(short item){	switch(item) {		case iNew	:		DoNew();			break;		case iOpen	:		DoOpen();			break;		case iClose	:		DoClose();			break;		case iSave	:		DoSave();			break;		case iSaveAs:		DoSaveAs();			break;		case iPageSetup	:	DoPageSetup();			break;		case iPrint	:		DoPrint();			break;		case iQuit	:		gDone = true;			break;	}}//------------------------------------static void ChooseEdit(short item)// Does not support edit menu right now{	SystemEdit(item-1);}//------------------------------------static void DoPageSetup( void ){	Boolean	ignore;	short	err;		PrOpen();	if ((err = PrError()) != 0) {		ErrMsgCode("\p PrOpen failed.",err);		return;	}	ignore = PrStlDialog(gPrintH);	PrClose();}//-------------------------------static OSErr InstallAEHandlers(void){	OSErr	err;		err = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, NewAEEventHandlerProc(AEOpenHandler), 0, false);	if ( err ) goto problem;	err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, NewAEEventHandlerProc(AEOpenDocHandler), 0, false);	if ( err ) goto problem;	err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerProc(AEQuitHandler), 0, false);	if ( err ) goto problem;	err = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, NewAEEventHandlerProc(AEPrintHandler), 0, false);	if ( err ) goto problem;	return noErr;	problem:	ErrMsgCode("\pError installing AppleEvent handlers.", err);	return err;}// ------------------------------------------------------------------------- static pascal OSErr AEOpenHandler(AppleEvent *messagein, AppleEvent *reply, long refIn){	return ( noErr );}static pascal OSErr AEOpenDocHandler(AppleEvent *messagein, AppleEvent *reply, long refIn){	return ( noErr );}static pascal OSErr AEPrintHandler(AppleEvent *messagein, AppleEvent *reply, long refIn){	return ( noErr );}static pascal OSErr AEQuitHandler(AppleEvent *messagein, AppleEvent *reply, long refIn){	gDone = true;	return ( noErr );}// ------------------------------------------------------------------------- //----------------------------------------------------// Error handling#define KEEP_GOING 1#define DEBUGGER 2#define EXITTOSHELL 3static void Msg(Str255 msg) // Display an Alert with the string passed.{	ParamText(msg,nil,nil,nil);	Alert(130, nil);}void ErrMsgCode(Str255 msg, short code)//	Display error alert with error code. Will also display MemErr and ResErr for you.{ 	Str31	codeStr;	Str31	memErrStr;	Str31	resErrStr;	short	disposition;		if (code)		NumToString(code, codeStr);	else		codeStr[0] = 0;	NumToString(LMGetMemErr(), memErrStr);	NumToString(LMGetResErr(), resErrStr);	ParamText(msg, codeStr, memErrStr, resErrStr);	disposition = Alert(128, nil);		switch (disposition)	{		case	KEEP_GOING:		return; break;		case	DEBUGGER:		DebugStr("\p Doing a Stack Crawl;sc6"); break;		case	EXITTOSHELL:	ExitToShell(); break;	}}//----------------------------------------------------void ErrMsg(Str255 msg) // No error code desired.{	ErrMsgCode(msg, 0);}//------------------------------------void pcat(StringPtr d, StringPtr s){	short	i, j;	if (((j = s[0]) + d[0]) > 255)		j = 255 - d[0]; // Limit dest string to 255	for (i = 0; i < j;) d[++d[0]] = s[++i];}