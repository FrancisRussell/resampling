#ifndef TINTL_INTERPOLATE_H
#define TINTL_INTERPOLATE_H

#include "timer.h"
#include "forward.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// \file
/// Functions for executing interpolation plans.

typedef enum
{
  STATISTIC_PLANNING,
  STATISTIC_EXECUTION,
  STATISTIC_UNKNOWN
} stat_type_t;

enum
{
  STATISTIC_EXECUTION_TIME = 1,
  STATISTIC_LAST_COMMON_VALUE
};

enum
{
  PREFER_PACKED_LAYOUT = 1,
  PREFER_SPLIT_LAYOUT = 2
};

/// Typedef for client use.
struct interpolate_plan_s;
typedef struct interpolate_plan_s *interpolate_plan;

/// Returns name of a plan
const char *interpolate_get_name(const interpolate_plan plan);

/// Executes an interleaved plan
void interpolate_execute_interleaved(const interpolate_plan plan, rs_complex *in, rs_complex *out);

/// Executes a split plan
void interpolate_execute_split(const interpolate_plan plan, double *rin, double *iin, double *rout, double *iout);

/// Executes a split-product plan
void interpolate_execute_split_product(const interpolate_plan plan, double *rin, double *iin, double *out);

/// Prints implementation-specific timing details to standard output
void interpolate_print_timings(const interpolate_plan plan);

/// Destroys a plan
void interpolate_destroy_plan(interpolate_plan plan);

/// Sets flags in a plan
void interpolate_set_flags(const interpolate_plan plan, const int flags);

/// Retrieves implementation-specific statistics from a plan
void interpolate_get_statistic_float(const interpolate_plan plan, int statistic, int index, stat_type_t *type, double *value);

/// Construct the best-performing interleaved interpolation plan from
/// multiple implementations.
interpolate_plan interpolate_plan_3d_interleaved_best(int n0, int n1, int n2, int flags);

/// Construct the best-performing split interpolation plan from
/// multiple implementations.
interpolate_plan interpolate_plan_3d_split_best(int n0, int n1, int n2, int flags);

/// Construct the best-performing split-product interpolation plan from
/// multiple implementations.
interpolate_plan interpolate_plan_3d_split_product_best(int n0, int n1, int n2, int flags);

/// Increment reference count and return count
int interpolate_inc_ref_count(interpolate_plan);

/// Decrement reference count and return count
int interpolate_dec_ref_count(interpolate_plan);

#ifdef __cplusplus
}
#endif

#endif
