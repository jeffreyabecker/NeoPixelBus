#!/usr/bin/env python3
"""
Estimate how many uniformly spaced data points are needed so that piecewise
linear interpolation approximates the CCTWhiteBalanceShader kelvinToRgb
function within 1% error between 2000K and 6800K.

Accuracy criterion used by default:
- max absolute per-channel error <= 1% of full scale (255), i.e. 2.55 counts.

You can switch to relative-to-signal error with --mode relative.
"""

from __future__ import annotations

import argparse
import math
from typing import List, Tuple

RGB = Tuple[float, float, float]

MIN_K = 1200
MAX_K = 65000
RANGE_MIN = 2000
RANGE_MAX = 6800


def clamp_kelvin(k: float) -> float:
    return max(MIN_K, min(MAX_K, k))


def kelvin_to_rgb_255(kelvin: float) -> RGB:
    """Python port of CCTWhiteBalanceShader::kelvinToRgb before scale255ToComponent."""
    temp = clamp_kelvin(kelvin) / 100.0

    if temp <= 66.0:
        red = 255
        green = round(99.4708025861 * math.log(temp) - 161.1195681661)
        if temp <= 19.0:
            blue = 0
        else:
            blue = round(138.5177312231 * math.log(temp - 10.0) - 305.0447927307)
    else:
        red = round(329.698727446 * math.pow(temp - 60.0, -0.1332047592))
        green = round(288.1221695283 * math.pow(temp - 60.0, -0.0755148492))
        blue = 255

    red = max(0, min(255, int(red)))
    green = max(0, min(255, int(green)))
    blue = max(0, min(255, int(blue)))
    return float(red), float(green), float(blue)


def scale255_to_component(value_255: float, max_component: int) -> float:
    # Match shader rounding: (value * max + 127) / 255 with integer math.
    v = int(round(value_255))
    numerator = v * max_component + 127
    return float(numerator // 255)


def kelvin_to_rgb_component(kelvin: float, max_component: int) -> RGB:
    r8, g8, b8 = kelvin_to_rgb_255(kelvin)
    return (
        scale255_to_component(r8, max_component),
        scale255_to_component(g8, max_component),
        scale255_to_component(b8, max_component),
    )


def build_uniform_lut(
    n_points: int,
    k_min: int,
    k_max: int,
    max_component: int,
) -> List[Tuple[float, RGB]]:
    if n_points < 2:
        raise ValueError("n_points must be >= 2")

    step = (k_max - k_min) / (n_points - 1)
    lut: List[Tuple[float, RGB]] = []
    for i in range(n_points):
        k = k_min + i * step
        lut.append((k, kelvin_to_rgb_component(k, max_component)))
    return lut


def lerp(a: float, b: float, t: float) -> float:
    return a + (b - a) * t


def interpolate_rgb(k: float, lut: List[Tuple[float, RGB]]) -> RGB:
    if k <= lut[0][0]:
        return lut[0][1]
    if k >= lut[-1][0]:
        return lut[-1][1]

    lo = 0
    hi = len(lut) - 1
    while hi - lo > 1:
        mid = (lo + hi) // 2
        if lut[mid][0] <= k:
            lo = mid
        else:
            hi = mid

    k0, c0 = lut[lo]
    k1, c1 = lut[hi]
    t = (k - k0) / (k1 - k0)
    return (
        lerp(c0[0], c1[0], t),
        lerp(c0[1], c1[1], t),
        lerp(c0[2], c1[2], t),
    )


def error_percent(
    truth: RGB,
    approx: RGB,
    mode: str,
    relative_floor: float,
    max_component: int,
) -> float:
    """Return worst-channel error percentage for one sample."""
    errs = [abs(t - a) for t, a in zip(truth, approx)]

    if mode == "fullscale":
        return max(e / float(max_component) for e in errs) * 100.0

    # Relative-to-signal mode (with floor to avoid divide-by-zero blowups).
    rel = []
    for e, t in zip(errs, truth):
        denom = max(abs(t), relative_floor)
        rel.append(e / denom)
    return max(rel) * 100.0


def max_error_for_lut(
    lut: List[Tuple[float, RGB]],
    k_min: int,
    k_max: int,
    mode: str,
    relative_floor: float,
    max_component: int,
) -> float:
    worst = 0.0
    for k in range(k_min, k_max + 1):
        truth = kelvin_to_rgb_component(float(k), max_component)
        approx = interpolate_rgb(float(k), lut)
        e = error_percent(truth, approx, mode, relative_floor, max_component)
        if e > worst:
            worst = e
    return worst


def find_min_points(
    target_error_percent: float,
    k_min: int,
    k_max: int,
    mode: str,
    relative_floor: float,
    max_points: int,
    max_component: int,
) -> Tuple[int, float]:
    for n in range(2, max_points + 1):
        lut = build_uniform_lut(n, k_min, k_max, max_component)
        worst = max_error_for_lut(
            lut,
            k_min,
            k_max,
            mode,
            relative_floor,
            max_component,
        )
        if worst <= target_error_percent:
            return n, worst
    raise RuntimeError(
        f"No solution found up to {max_points} points. Increase --max-points."
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--k-min", type=int, default=RANGE_MIN)
    parser.add_argument("--k-max", type=int, default=RANGE_MAX)
    parser.add_argument("--target", type=float, default=1.0, help="Target error percent")
    parser.add_argument(
        "--mode",
        choices=["fullscale", "relative"],
        default="fullscale",
        help="Error metric: fullscale=%% of 255, relative=%% of signal",
    )
    parser.add_argument(
        "--relative-floor",
        type=float,
        default=1.0,
        help="Minimum denominator for relative mode",
    )
    parser.add_argument("--max-points", type=int, default=2000)
    parser.add_argument(
        "--bits",
        type=int,
        default=8,
        help="Channel bit depth for component-space evaluation (e.g. 8, 16)",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    if args.bits < 1 or args.bits > 31:
        raise ValueError("--bits must be in [1, 31]")
    max_component = (1 << args.bits) - 1

    n, worst = find_min_points(
        target_error_percent=args.target,
        k_min=args.k_min,
        k_max=args.k_max,
        mode=args.mode,
        relative_floor=args.relative_floor,
        max_points=args.max_points,
        max_component=max_component,
    )

    print(f"Range: {args.k_min}K to {args.k_max}K")
    print(f"Bits: {args.bits} (max component: {max_component})")
    print(f"Metric: {args.mode}")
    print(f"Target: <= {args.target:.4f}%")
    print(f"Minimum points required: {n}")
    print(f"Worst-case error at that size: {worst:.6f}%")


if __name__ == "__main__":
    main()
