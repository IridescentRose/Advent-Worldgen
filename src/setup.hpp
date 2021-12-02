#pragma once
#include <pspkernel.h>
#include <pspdebug.h>

auto exit_callback(int arg1, int arg2, void* common) -> int;
auto CallbackThread(SceSize args, void* argp) -> int;
auto SetupCallbacks() -> int;