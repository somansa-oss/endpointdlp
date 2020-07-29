#ifndef	_INCLUDE_KERNEL_H
#define	_INCLUDE_KERNEL_H

#ifdef __APPLE__
#include "../../apple/include/KernelDataType.h"
#include "../../apple/include/KernelCommand.h"
#include "../../apple/include/KernelProtocol.h"
#else
#include "../../linux/include/KernelDataType.h"
#include "../../linux/include/KernelCommand.h"
#include "../../linux/include/KernelProtocol.h"
#endif // #ifdef __APPLE__

#endif // #ifndef	_INCLUDE_KERNEL_H

