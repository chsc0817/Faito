#pragma once

#include "basic.h"

typedef u32 random_generator;

random_generator MakeRandomGenerator(u32 seed);

u32 NextRandomValue(random_generator *generator);
u32 RandomU32(random_generator *generator, u32 min, u32 max);
f32 RandomF32(random_generator *generator, f32 min, f32 max);
f32 RandomZeroToOne(random_generator *generator);