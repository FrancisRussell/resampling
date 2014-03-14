#ifndef COMMON_CUDA_H
#define COMMON_CUDA_H

#include <cuComplex.h>
#include <cufft.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CUDA_CHECK(err) printCudaDiagnostics( err, __FILE__, __LINE__)
#define CUFFT_CHECK(err) printCuFFTDiagnostics( err, __FILE__, __LINE__)

void halve_nyquist_components_cuda(interpolate_properties_t *props, block_info_t *block_info, cuDoubleComplex *coarse);

void pad_coarse_to_fine_interleaved_cuda(interpolate_properties_t *props,
  const block_info_t *from_info, const cuDoubleComplex *from,
  const block_info_t *to_info, cuDoubleComplex *to,
  const int positive_only);

void transpose_block_cuda(const block_info_t *from_info, const cuDoubleComplex *from, block_info_t *to_info, 
  cuDoubleComplex *to, int count);

void printCudaDiagnostics(cudaError_t code, const char *file, int line);
void printCuFFTDiagnostics(cufftResult code, const char *file, int line);

void calcGridDim(const dim3& total_dim, const dim3& block_dim, dim3& grid_dim);

#ifdef __cplusplus
}
#endif

#endif