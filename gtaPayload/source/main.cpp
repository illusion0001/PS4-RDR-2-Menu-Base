#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>

#include "natives.h"
#include "menu.h"
#include <Common.h>

bool init = false;
int frameCount = 0;
uint64_t base_address = 0;
Menu menu;

struct proc_info procInfo = {};

#define PROC_MODE_READ 0
#define PROC_MODE_WRITE 1

#define TEX_ICON_SYSTEM "cxml://psnotification/tex_icon_system"
#define attr_module_hidden __attribute__((weak)) __attribute__((visibility("hidden")))
#define attr_public __attribute__((visibility("default")))

// Thanks to OSM
// https://github.com/OSM-Made/PS4-Notify/blob/c6d259bc5bd4aa519f5b0ce4f5e27ef7cb01ffdd/Notify.cpp

// For pre formatted strings
void NotifyStatic(const char* IconUri, const char* text) {
	OrbisNotificationRequest Buffer{};
	Buffer.type = NotificationRequest;
	Buffer.unk3 = 0;
	Buffer.useIconImageUri = 1;
	Buffer.targetId = -1;
	strcpy(Buffer.message, text);
	strcpy(Buffer.iconUri, IconUri);
	sceKernelSendNotificationRequest(0, &Buffer, sizeof(Buffer), 0);
}

void sys_proc_rw(u64 address, void *data, u64 length) {
	struct proc_rw process_rw_data;
	process_rw_data.address = address;
	process_rw_data.data = data;
	process_rw_data.length = length;
	process_rw_data.write_flags = PROC_MODE_WRITE;
	sys_sdk_proc_rw(&process_rw_data);
	return;
}

void mainMenu() {
	menu.banner("Main Menu");
	menu.option("Self Options");
	menu.option("Vehicle Spawner");
	menu.option("Settings");
}

bool shownNotification = false;

void ScriptMain()
{
	if (!shownNotification)
	{
		NotifyStatic(TEX_ICON_SYSTEM, "ScriptMain reached");
		menu = Menu(mainMenu);
		shownNotification = true;
	}
	int newFrameCount = MISC::GET_FRAME_COUNT();
	if (newFrameCount > frameCount) {
		frameCount = newFrameCount;

		menu.monitorButtons();

		if (menu.open) {
			menu.run();
		}
	}
}

void Clock_GET_CLOCK_DAY_OF_MONTH_Hook(struct NativeArg_s* arg)
{
	ScriptMain();
	arg->returnValue[0] = *(int32_t*)(procInfo.base_address + (0x07e414a4 - NO_ASLR_ADDR));
}

void* rdr3_thread(void* args)
{
	if (sys_sdk_proc_info(&procInfo) == 0)
	{
		u8 hook_array[] =
		{ 
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, // jmp qword ptr [$+6]
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // ptr
		};
		base_address = procInfo.base_address;
		uint64_t hookAddress = base_address + (0x021c5f80 - NO_ASLR_ADDR);
		uint64_t targetFunc =  (uint64_t)(void*)Clock_GET_CLOCK_DAY_OF_MONTH_Hook;
		sys_proc_rw(hookAddress, hook_array, sizeof(hook_array));
		sys_proc_rw(hookAddress+6, &targetFunc, sizeof(void*));
	}
	else
	{
		NotifyStatic(TEX_ICON_SYSTEM, "Failed to get process info");
	}
	scePthreadExit(NULL);
	return NULL;
}

extern "C" int32_t attr_module_hidden module_start(size_t argc, const void *args)
{
	OrbisPthread thread;
	scePthreadCreate(&thread, NULL, rdr3_thread, NULL, STRINGIFY(rdr3_thread));
	return 0;
}

extern "C" int32_t attr_module_hidden module_stop(size_t argc, const void *args)
{
	return 0;
}
