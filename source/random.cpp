#include "random.h"

random_generator MakeRandomGenerator(u32 seed) {
    return seed;
}

u32 NextRandomValue(random_generator *generator) {
    u32 result = *generator;

    result ^= result << 13;
    result ^= result >> 17;
    result ^= result << 5;
    *generator = result;

    return result;
}

u32 RandomU32(random_generator *generator, u32 min, u32 max) {
    return (NextRandomValue(generator) % (max - min)) + min;
}

f32 RandomZeroToOne(random_generator *generator) {
    const u32 max = -1;

    return NextRandomValue(generator) / (f32) max;	
}

f32 RandomF32(random_generator *generator, f32 min, f32 max) {
    return (RandomZeroToOne(generator) * (max - min) + min);
}
