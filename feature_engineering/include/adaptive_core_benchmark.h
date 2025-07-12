#pragma once

// Function to run adaptive core allocation benchmark
// Allocates 8+ cores to slow operations (< 20 GFLOPS) and fewer cores to fast operations
void run_adaptive_core_benchmark();
