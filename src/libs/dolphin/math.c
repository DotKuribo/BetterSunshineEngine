#include <Dolphin/PPCArch.h>
#include <SMS/ppc_intrinsics.h>

f64 sqrt(f64 d) {
    if (d <= 0.0)
        return d;

    const f64 sqrte = __frsqrte(d);
    return d * (-((d * sqrte * sqrte) - 3.0) * (sqrte * 0.5));
}

f32 sqrtf(f32 d) {
    if (d <= 0.0)
        return d;

    const f32 sqrte = __frsqrtes(d);
    return d * (-((d * sqrte * sqrte) - 3.0) * (sqrte * 0.5));
}