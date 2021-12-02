#pragma once

auto begin_bench() -> void;
auto end_bench(const char* name) -> void;

#define BENCHMARK(x, y) { begin_bench(); x; end_bench(y); }