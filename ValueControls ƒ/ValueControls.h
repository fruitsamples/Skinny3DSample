#ifndef _VALUECTL_#define _VALUECTL_typedef	struct ValueCtlCluster *VccPtr; // ValueCtlCluster is opaque (private)VccPtr	NewVCluster(long tag, Str255 title, long numEntries, Rect *bounds);void	AddValueCtl(VccPtr vcc, Str255 title, float initV, float mini, float maxi, float delta);void	AddSeparator(VccPtr vcc);void	DisposeVCluster(VccPtr	vcc);void	DrawVCluster(VccPtr	vcc);float	GetCurrentValue(VccPtr vcc, long index);void	SetResetValue(VccPtr vcc, long index, float value);OSType	GetIdTag(VccPtr	vcc);Boolean	TakeHit(Point clickPt, VccPtr vcc); // true if change in vcc data#endif