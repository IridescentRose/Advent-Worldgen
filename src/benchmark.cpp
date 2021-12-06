#include "benchmark.hpp"
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <pspkernel.h>
#include <pspdebug.h>

#define MICROSECONDS 1000 * 1000

//Benchmarking utilities
auto start_time = 0;
auto begin_bench() -> void {
	start_time = sceKernelGetSystemTimeLow();
}

auto end_bench(const char* name) -> void {
	auto end = sceKernelGetSystemTimeLow();
	auto res = end - start_time;

    pspDebugScreenPrintf("[Test]: %s took %u microseconds!\n", name, res);
    printf("[Test]: %s took %u microseconds!\n", name, res);
}
