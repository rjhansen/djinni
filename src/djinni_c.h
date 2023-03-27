#ifndef DJINNI_C_H
#define DJINNI_C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void *new_compressed_penalty_function(double expPower, double pcap,
                                      double cperc);

void delete_compressed_penalty_function(void *_c);

void *new_traveling_salesman(const char *filename);

void delete_traveling_salesman(void *_tsp);

void *new_ca_tsp(void *_ca, void *_tsp, double multT, double accept,
                 uint32_t tBI, uint32_t minIter, uint32_t maxIter);

void delete_ca_tsp(void *_ca_tsp);

void run_ca_tsp(void *_ca_tsp);

double ca_tsp_get_cost(void *_ca_tsp);

double ca_tsp_get_penalty(void *_ca_tsp);

uint32_t ca_tsp_get_best_iteration(void *_ca_tsp);

uint32_t ca_tsp_get_current_iteration(void *_ca_tsp);

uint32_t ca_tsp_get_max_iterations(void *_ca_tsp);

uint32_t ca_tsp_get_min_iterations(void *_ca_tsp);

double ca_tsp_get_multiplier(void *_ca_tsp);

double ca_tsp_get_probability(void *_ca_tsp);

double ca_tsp_get_lambda(void *_ca_tsp);

#ifdef __cplusplus
}
#endif
#endif
