// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/cpu.h"
#include "base/memory/aligned_memory.h"
#include "base/memory/scoped_ptr.h"
#include "base/time/time.h"
#include "media/base/vector_math.h"
#include "media/base/vector_math_testing.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/perf/perf_test.h"

using base::TimeTicks;
using std::fill;

namespace media {

static const int kBenchmarkIterations = 200000;
static const float kScale = 0.5;
static const int kVectorSize = 8192;

class VectorMathPerfTest : public testing::Test {
 public:
  VectorMathPerfTest() {
    // Initialize input and output vectors.
    input_vector_.reset(static_cast<float*>(base::AlignedAlloc(
        sizeof(float) * kVectorSize, vector_math::kRequiredAlignment)));
    output_vector_.reset(static_cast<float*>(base::AlignedAlloc(
        sizeof(float) * kVectorSize, vector_math::kRequiredAlignment)));
    fill(input_vector_.get(), input_vector_.get() + kVectorSize, 1.0f);
    fill(output_vector_.get(), output_vector_.get() + kVectorSize, 0.0f);
  }

  void RunBenchmark(void (*fn)(const float[], float, int, float[]),
                    bool aligned,
                    const std::string& test_name,
                    const std::string& trace_name) {
    TimeTicks start = TimeTicks::HighResNow();
    for (int i = 0; i < kBenchmarkIterations; ++i) {
      fn(input_vector_.get(),
         kScale,
         kVectorSize - (aligned ? 0 : 1),
         output_vector_.get());
    }
    double total_time_seconds = (TimeTicks::HighResNow() - start).InSecondsF();
    perf_test::PrintResult(test_name,
                           "",
                           trace_name,
                           kBenchmarkIterations / total_time_seconds,
                           "runs/s",
                           true);
  }

 protected:
  scoped_ptr_malloc<float, base::ScopedPtrAlignedFree> input_vector_;
  scoped_ptr_malloc<float, base::ScopedPtrAlignedFree> output_vector_;

  DISALLOW_COPY_AND_ASSIGN(VectorMathPerfTest);
};

// Define platform independent function name for FMAC* perf tests.
#if defined(ARCH_CPU_X86_FAMILY)
#define FMAC_FUNC FMAC_SSE
#elif defined(ARCH_CPU_ARM_FAMILY) && defined(USE_NEON)
#define FMAC_FUNC FMAC_NEON
#endif

// Benchmark for each optimized vector_math::FMAC() method.
TEST_F(VectorMathPerfTest, FMAC) {
  // Benchmark FMAC_C().
  RunBenchmark(
      vector_math::FMAC_C, true, "vector_math_fmac", "unoptimized");
#if defined(FMAC_FUNC)
#if defined(ARCH_CPU_X86_FAMILY)
  ASSERT_TRUE(base::CPU().has_sse());
#endif
  // Benchmark FMAC_FUNC() with unaligned size.
  ASSERT_NE((kVectorSize - 1) % (vector_math::kRequiredAlignment /
                                 sizeof(float)), 0U);
  RunBenchmark(
      vector_math::FMAC_FUNC, false, "vector_math_fmac", "optimized_unaligned");
  // Benchmark FMAC_FUNC() with aligned size.
  ASSERT_EQ(kVectorSize % (vector_math::kRequiredAlignment / sizeof(float)),
            0U);
  RunBenchmark(
      vector_math::FMAC_FUNC, true, "vector_math_fmac", "optimized_aligned");
#endif
}

#undef FMAC_FUNC

// Define platform independent function name for FMULBenchmark* tests.
#if defined(ARCH_CPU_X86_FAMILY)
#define FMUL_FUNC FMUL_SSE
#elif defined(ARCH_CPU_ARM_FAMILY) && defined(USE_NEON)
#define FMUL_FUNC FMUL_NEON
#endif

// Benchmark for each optimized vector_math::FMUL() method.
TEST_F(VectorMathPerfTest, FMUL) {
  // Benchmark FMUL_C().
  RunBenchmark(
      vector_math::FMUL_C, true, "vector_math_fmul", "unoptimized");
#if defined(FMUL_FUNC)
#if defined(ARCH_CPU_X86_FAMILY)
  ASSERT_TRUE(base::CPU().has_sse());
#endif
  // Benchmark FMUL_FUNC() with unaligned size.
  ASSERT_NE((kVectorSize - 1) % (vector_math::kRequiredAlignment /
                                 sizeof(float)), 0U);
  RunBenchmark(
      vector_math::FMUL_FUNC, false, "vector_math_fmac", "optimized_unaligned");
  // Benchmark FMUL_FUNC() with aligned size.
  ASSERT_EQ(kVectorSize % (vector_math::kRequiredAlignment / sizeof(float)),
            0U);
  RunBenchmark(
      vector_math::FMUL_FUNC, true, "vector_math_fmac", "optimized_aligned");
#endif
}

#undef FMUL_FUNC

} // namespace media
