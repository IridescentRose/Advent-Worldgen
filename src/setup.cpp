#include "setup.hpp"

auto exit_callback(int arg1, int arg2, void* common) -> int {
	sceKernelExitGame();
	return 0;
}
 
auto CallbackThread(SceSize args, void* argp) -> int {
	int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
 
	return 0;
}
 
auto SetupCallbacks() -> int {
	int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if (thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	}
	return thid;
}