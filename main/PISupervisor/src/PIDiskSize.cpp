#ifndef	_PIDISKSIZE_CPP
#define	_PIDISKSIZE_CPP

#include "PIDiskSize.h"
#include "include_system.h"
#include <sys/statvfs.h>

////////////////////////////////////////
// CPIDiskSize
CPIDiskSize::CPIDiskSize() {
	clear();
}

CPIDiskSize::~CPIDiskSize() {	
}

void CPIDiskSize::clear(void) {
	totalBytes = 0.0;
	freeBytes = 0.0;
	usedBytes = 0.0;
}
		
bool CPIDiskSize::getSystemDiskSize(void) {
	clear();

	struct statvfs sv;
	memset( &sv, 0, sizeof(sv) );
	int result = statvfs( "/", &sv );
	if(result != 0) {
		return false;
	}

	totalBytes = (double)(sv.f_blocks * sv.f_frsize);
	freeBytes = (double)(sv.f_bfree * sv.f_frsize);
	usedBytes = totalBytes - freeBytes;
	return true;
}

void CPIDiskSize::getUsedBytes(void) {
	usedBytes = totalBytes - freeBytes;
}
#endif
