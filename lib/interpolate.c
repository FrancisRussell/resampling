#include <complex.h>
#include <float.h>
#include <assert.h>
#include <stdlib.h>
#include "tintl/interpolate.h"
#include "tintl/forward.h"
#include "common.h"
#include "tintl/naive.h"
#include "tintl/padding_aware.h"
#include "tintl/phase_shift.h"
#include "plan_cache.h"

static const int best_plan_cache_enabled = 1;

const char *interpolate_get_name(const interpolate_plan plan)
{
  validate_plan(plan);
  return plan->get_name(plan);
}

void interpolate_execute_interleaved(const interpolate_plan plan, rs_complex *in, rs_complex *out)
{
  validate_plan(plan);
  time_point_save(&plan->before);
  plan->execute_interleaved(plan, in, out);
  time_point_save(&plan->after);
}

void interpolate_execute_split(const interpolate_plan plan, double *rin, double *iin, double *rout, double *iout)
{
  validate_plan(plan);
  time_point_save(&plan->before);
  plan->execute_split(plan, rin, iin, rout, iout);
  time_point_save(&plan->after);
}

void interpolate_execute_split_product(const interpolate_plan plan, double *rin, double *iin, double *out)
{
  validate_plan(plan);
  time_point_save(&plan->before);
  plan->execute_split_product(plan, rin, iin, out);
  time_point_save(&plan->after);
}

void interpolate_print_timings(const interpolate_plan plan)
{
  validate_plan(plan);
  plan->print_timings(plan);
}

void interpolate_get_statistic_float(const interpolate_plan plan, int statistic, int index, stat_type_t *type, double *value)
{
  validate_plan(plan);
  switch(statistic)
  {
    case STATISTIC_EXECUTION_TIME:
      *type = STATISTIC_EXECUTION;
      *value = time_point_delta(&plan->before, &plan->after);
      break;
    default:
      plan->get_statistic_float(plan, statistic, index, type, value);
      break;
  }
}

void interpolate_set_flags(const interpolate_plan plan, const int flags)
{
  validate_plan(plan);
  plan->set_flags(plan, flags);
}

void interpolate_destroy_plan(interpolate_plan plan)
{
  validate_plan(plan);
  interpolate_dec_ref_count(plan);
}

int interpolate_inc_ref_count(interpolate_plan plan)
{
  validate_plan(plan);
  int count;

#ifdef _OPENMP
  #pragma omp critical
#endif
  count = ++plan->ref_cnt;

  return count;
}

int interpolate_dec_ref_count(interpolate_plan plan)
{
  validate_plan(plan);
  int count;

#ifdef _OPENMP
  #pragma omp critical
#endif
  count = --plan->ref_cnt;

  assert(count >= 0);

  if (count == 0)
  {
    plan->destroy_detail(plan);
    plan->magic = 0;
    free(plan);
  }

  return count;
}

static plan_cache_t *global_plan_cache(void)
{
  plan_cache_t *cache_ptr;

#ifdef _OPENMP
  #pragma omp critical
  {
#endif

  static int cache_initialised = 0;
  static plan_cache_t cache;

  if (cache_initialised == 0)
  {
    plan_cache_init(&cache);
    cache_initialised = 1;
  }

  cache_ptr = &cache;

#ifdef _OPENMP
  }
#endif

  return cache_ptr;
}

static interpolate_plan global_plan_cache_get(const plan_key_t *key)
{
  plan_cache_t *const cache = global_plan_cache();
  interpolate_plan cached_plan = plan_cache_get(cache, key);
  return cached_plan;
}

static int global_plan_cache_insert(const plan_key_t *key, interpolate_plan plan)
{
  plan_cache_t *const cache = global_plan_cache();
  const int inserted = plan_cache_insert(cache, key, plan);
  return inserted;
}

typedef interpolate_plan (*plan_constructor_t)(int n0, int n1, int n2, int flags);

static interpolate_plan find_best_plan(double (*timer)(interpolate_plan),
  plan_constructor_t *constructors,
  int n0, int n1, int n2, interpolation_t type, int flags)
{
  plan_key_t key;
  key.n0 = n0;
  key.n1 = n1;
  key.n2 = n2;
  key.type = type;

  if (best_plan_cache_enabled)
  {
    interpolate_plan cached_plan = global_plan_cache_get(&key);
    if (cached_plan != NULL)
      return cached_plan;
  }

  int plan_type_count = 0;

  while(constructors[plan_type_count] != NULL)
    ++plan_type_count;

  interpolate_plan best_plan = NULL;
  double best_time = DBL_MAX;

  for(size_t plan_id = 0; plan_id < plan_type_count; ++plan_id)
  {
    interpolate_plan plan = constructors[plan_id](n0, n1, n2, flags);
    const double time = timer(plan);

    if (time < best_time)
    {
      best_time = time;
      if (best_plan != NULL)
        interpolate_destroy_plan(best_plan);
      best_plan = plan;
    }
    else
    {
      interpolate_destroy_plan(plan);
    }
  }

  assert(best_plan != NULL);

  if (best_plan_cache_enabled)
    global_plan_cache_insert(&key, best_plan);

  return best_plan;
}

interpolate_plan interpolate_plan_3d_interleaved_best(int n0, int n1, int n2, int flags)
{
  plan_constructor_t constructors[] = {
    interpolate_plan_3d_naive_interleaved,
    interpolate_plan_3d_padding_aware_interleaved,
    interpolate_plan_3d_phase_shift_interleaved,
    NULL
  };

  return find_best_plan(time_interpolate_interleaved, constructors, n0, n1, n2, INTERPOLATE_INTERLEAVED, flags);
}

interpolate_plan interpolate_plan_3d_split_best(int n0, int n1, int n2, int flags)
{
  plan_constructor_t constructors[] = {
    interpolate_plan_3d_naive_split,
    interpolate_plan_3d_padding_aware_split,
    interpolate_plan_3d_phase_shift_split,
    NULL
  };
  return find_best_plan(time_interpolate_split, constructors, n0, n1, n2, INTERPOLATE_SPLIT, flags);
}

interpolate_plan interpolate_plan_3d_split_product_best(int n0, int n1, int n2, int flags)
{
  plan_constructor_t constructors[] = {
    interpolate_plan_3d_naive_product,
    interpolate_plan_3d_padding_aware_product,
    interpolate_plan_3d_phase_shift_product,
    NULL
  };
  return find_best_plan(time_interpolate_split_product, constructors, n0, n1, n2, INTERPOLATE_SPLIT_PRODUCT, flags);
}
