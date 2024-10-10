#include "Device.h"

#pragma unmanaged
int CloseDevice(fz_context* ctx, fz_device* dev) {
	MuTry(ctx, fz_close_device(ctx, dev));
}
