#ifndef SFP_H
#define SFP_H

#include "a0h.h"
#include "a2h.h"

#include<stdint.h>
#include<stdbool.h>

typedef struct {
  sfp_a0h_base_t a0;
  sfp_a0h_extended_t a0e;
  sfp_a2h_t a2;

} sfp_t;

bool sfp_init(sfp_t *sfp);
bool sfp_update(sfp_t *sfp);
bool sfp_update_a0(sfp_t *sfp);
bool sfp_update_a2(sfp_t *sfp);



#endif
