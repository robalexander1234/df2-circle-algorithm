# DF2 Circle Algorithm

A Direct Form 2 digital filter algorithm for circle rasterization that outperforms Bresenham's midpoint algorithm for medium-to-large radii.

**Author:** Robert Alexander  
**Email:** alexander.robert.b@gmail.com  
*Retired Software Engineer — formerly Intel, Google, Motorola, Texas Instruments, Boeing*

## Overview

This repository contains an implementation and analysis of a circle-drawing algorithm based on a second-order IIR digital filter. The algorithm exploits the well-known resonator recurrence:

```
w[n] = 2·cos(ω)·w[n-1] - w[n-2]
```

to generate sine and cosine sequences that trace a circle parametrically. Unlike Bresenham's algorithm, which requires explicit 8-way symmetry exploitation and contains data-dependent branches, the DF2 approach uses a tight, branch-free inner loop that pipelines efficiently on modern processors.

## Key Results

| Radius | DF2 Fixed (full circle) | Bresenham (8-way sym) | Speedup |
|--------|-------------------------|----------------------|---------|
| 25     | 0.93 µs                 | 0.80 µs              | 0.86×   |
| 50     | **1.83 µs**             | 2.61 µs              | **1.43×** |
| 75     | **2.81 µs**             | 5.47 µs              | **1.95×** |
| 100    | **4.09 µs**             | 9.05 µs              | **2.21×** |

**The DF2 algorithm beats Bresenham for radii ≥ 50 pixels**, with speedups increasing to over 2× at radius 100.

### Why Does It Win?

Despite performing ~8× more iterations (full circle vs. one octant), DF2 wins because:

1. **Branch-free execution**: The inner loop contains no data-dependent branches, enabling full pipelining
2. **No symmetry overhead**: Bresenham's `plot8()` function requires 8 coordinate calculations and memory accesses per iteration
3. **Predictable memory access**: Pixels are plotted in angular order, which is cache-friendly

### Stability Limitation

The algorithm becomes numerically unstable at large radii because the coefficient `2·cos(ω)` approaches 2.0 (the stability boundary). The critical radius depends on arithmetic precision:

| Format | Fractional Bits | Critical Radius |
|--------|-----------------|-----------------|
| Q8.8   | 8               | ~8              |
| Q16.16 | 16              | ~120            |
| Q1.31  | 31              | ~21,800         |
| Float64| 52              | ~31,000,000     |

## Building

```bash
cd src
make
```

Or compile individually:

```bash
gcc -O3 -o df2_benchmark df2_circle_benchmark.c -lm
gcc -O3 -o fair_comparison fair_comparison.c -lm
```

## Running Benchmarks

```bash
# Full benchmark with all algorithms
./df2_benchmark

# Direct comparison: DF2 full-circle vs Bresenham 8-way
./fair_comparison
```

## Algorithm

The core algorithm in C:

```c
void circle_df2(Framebuffer *fb, int cx, int cy, int r) {
    double omega = 1.0 / (1.5 * r);
    double coeff = 2.0 * cos(omega);
    double scale = -1.0 / omega;
    
    double w0 = r * cos(omega);  // w[n-2]
    double w1 = r;               // w[n-1]
    
    int steps = (int)(2.0 * M_PI / omega) + 1;
    
    for (int i = 0; i < steps; i++) {
        int x = (int)round(w1);
        int y = (int)round((w1 - w0) * scale);
        
        plot(fb, cx + x, cy + y);
        
        // DF2 recurrence: one multiply
        double w2 = coeff * w1 - w0;
        w0 = w1;
        w1 = w2;
    }
}
```

## Mathematical Background

The algorithm is derived from the z-transform of discrete sinusoids. The transfer function:

```
H(z) = 1 / (1 - 2cos(ω)z⁻¹ + z⁻²)
```

has poles at `z = e^(±jω)`, which lie exactly on the unit circle. This corresponds to a marginally stable oscillator—one that neither grows nor decays with exact arithmetic.

The recurrence generates `w[n] = r·cos(ωn)`. The sine component is derived from the scaled difference of adjacent samples:

```
y[n] = -(w[n] - w[n-1]) / ω ≈ r·sin(ωn)
```

## Files

```
df2-circle-algorithm/
├── README.md
├── LICENSE
├── Makefile
├── src/
│   ├── df2_circle_benchmark.c   # Full benchmark suite
│   ├── fair_comparison.c        # DF2 vs Bresenham comparison
│   └── Makefile
└── paper/
    ├── df2_circle_paper.tex     # LaTeX source
    └── df2_circle_paper.pdf     # Technical paper
```

## History

This algorithm was originally developed by the author circa 1985 using the Direct Form 2 filter realization. It was found to beat Bresenham's algorithm for medium radii but exhibited instability at larger radii—a tradeoff that is now fully characterized in terms of fixed-point precision and pole placement on the unit circle.

The algorithm was never published at the time. Recent analysis confirms its novelty as a graphics technique: while recursive sinusoidal oscillators are well-known in DSP, applying the DF2 realization specifically to circle rasterization with performance comparison against Bresenham appears to be original.

## Applications

The DF2 circle algorithm is particularly suited for:

- **Embedded systems** with hardware multipliers but limited branch prediction
- **Real-time graphics** where circles of bounded radius are drawn repeatedly
- **FPGA/ASIC implementations** where the regular dataflow maps well to hardware
- **Arc drawing** where the parametric approach naturally handles partial circles

## Extensions

The algorithm naturally extends to:

- **Ellipses**: Use different scale factors for x and y
- **Spirals**: Multiply the coefficient by a decay factor < 1 each iteration
- **Arcs**: Adjust iteration count and initial phase
- **Antialiasing**: Sub-pixel coordinates are naturally available

## Citation

If you use this algorithm in your research, please cite:

```bibtex
@misc{alexander2025df2circle,
  author = {Alexander, Robert},
  title = {A Direct Form 2 Digital Filter Algorithm for Circle Rasterization},
  year = {2025},
  howpublished = {\url{https://github.com/robalexander1234/df2-circle-algorithm}}
}
```

## License

MIT License - see [LICENSE](LICENSE) for details.

## References

1. J. E. Bresenham, "A linear algorithm for incremental digital display of circular arcs," Communications of the ACM, 1977.
2. M. Minsky, "Circle algorithm," in HAKMEM, MIT AI Memo 239, 1972.
3. J. O. Smith, "Introduction to Digital Filters with Audio Applications," W3K Publishing, 2007.
4. J. L. Cieśliński and L. V. Moroz, "Fast exact digital differential analyzer for circle generation," Applied Mathematics and Computation, 2015.
