/*
 * Fair comparison: Both algorithms with AND without 8-way symmetry
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

#define FP_BITS 16
#define FP_ONE (1 << FP_BITS)
#define FP_HALF (1 << (FP_BITS - 1))
typedef int32_t fixed_t;

static inline fixed_t to_fixed(double d) { return (fixed_t)(d * FP_ONE + (d >= 0 ? 0.5 : -0.5)); }
static inline int fixed_to_int(fixed_t f) { return (f + FP_HALF) >> FP_BITS; }
static inline fixed_t fp_mul(fixed_t a, fixed_t b) { return (fixed_t)(((int64_t)a * b) >> FP_BITS); }

typedef struct { int w, h; uint8_t *px; } FB;
FB* mkfb(int w, int h) { FB *f = malloc(sizeof(FB)); f->w=w; f->h=h; f->px=calloc(w*h,1); return f; }
void clrfb(FB *f) { memset(f->px, 0, f->w * f->h); }
void freefb(FB *f) { free(f->px); free(f); }

static inline void plot(FB *f, int x, int y) {
    x += f->w/2; y += f->h/2;
    if (x >= 0 && x < f->w && y >= 0 && y < f->h) f->px[y * f->w + x] = 1;
}

static inline void plot8(FB *f, int x, int y) {
    plot(f, x, y); plot(f, -x, y); plot(f, x, -y); plot(f, -x, -y);
    plot(f, y, x); plot(f, -y, x); plot(f, y, -x); plot(f, -y, -x);
}

int count_px(FB *f) { int n=0; for(int i=0;i<f->w*f->h;i++) n+=f->px[i]; return n; }

/* DF2 - Full circle (no symmetry) */
int df2_full(FB *fb, int r) {
    if (r <= 0) return 0;
    double omega = 1.0 / (1.5 * r);
    fixed_t coeff = to_fixed(2.0 * cos(omega));
    fixed_t scale = to_fixed(-1.0 / omega);
    fixed_t w0 = to_fixed(r * cos(omega));
    fixed_t w1 = to_fixed((double)r);
    int steps = (int)(2.0 * M_PI / omega) + 10;
    int drawn = 0;
    for (int i = 0; i < steps; i++) {
        int x = fixed_to_int(w1);
        int y = fixed_to_int(fp_mul(w1 - w0, scale));
        int px = x + fb->w/2, py = y + fb->h/2;
        if (px >= 0 && px < fb->w && py >= 0 && py < fb->h) {
            if (!fb->px[py * fb->w + px]) { fb->px[py * fb->w + px] = 1; drawn++; }
        }
        fixed_t w2 = fp_mul(coeff, w1) - w0;
        w0 = w1; w1 = w2;
    }
    return drawn;
}

/* DF2 - With 8-way symmetry (one octant) */
int df2_sym8(FB *fb, int r) {
    if (r <= 0) return 0;
    double omega = 1.0 / (1.5 * r);
    fixed_t coeff = to_fixed(2.0 * cos(omega));
    fixed_t scale = to_fixed(-1.0 / omega);
    fixed_t w0 = to_fixed(r * cos(omega));
    fixed_t w1 = to_fixed((double)r);
    while (1) {
        int x = fixed_to_int(w1);
        int y = fixed_to_int(fp_mul(w1 - w0, scale));
        if (y > x) break;
        plot8(fb, x, y);
        fixed_t w2 = fp_mul(coeff, w1) - w0;
        w0 = w1; w1 = w2;
    }
    return count_px(fb);
}

/* Bresenham - With 8-way symmetry (standard) */
int bres_sym8(FB *fb, int r) {
    if (r <= 0) return 0;
    int x = 0, y = r, d = 3 - 2*r;
    while (x <= y) {
        plot8(fb, x, y);
        if (d < 0) d += 4*x + 6;
        else { d += 4*(x-y) + 10; y--; }
        x++;
    }
    return count_px(fb);
}

/* Bresenham - Full circle (no symmetry, for comparison) */
int bres_full(FB *fb, int r) {
    if (r <= 0) return 0;
    int x = 0, y = r, d = 3 - 2*r;
    /* Octant 1 */
    while (x <= y) {
        plot(fb, x, y); 
        if (d < 0) d += 4*x + 6;
        else { d += 4*(x-y) + 10; y--; }
        x++;
    }
    /* Reset and do octant 2 */
    x = 0; y = r; d = 3 - 2*r;
    while (x <= y) {
        plot(fb, y, x);
        if (d < 0) d += 4*x + 6;
        else { d += 4*(x-y) + 10; y--; }
        x++;
    }
    /* Octant 3 */
    x = 0; y = r; d = 3 - 2*r;
    while (x <= y) {
        plot(fb, y, -x);
        if (d < 0) d += 4*x + 6;
        else { d += 4*(x-y) + 10; y--; }
        x++;
    }
    /* Continue for all 8... */
    x = 0; y = r; d = 3 - 2*r;
    while (x <= y) {
        plot(fb, x, -y);
        if (d < 0) d += 4*x + 6;
        else { d += 4*(x-y) + 10; y--; }
        x++;
    }
    x = 0; y = r; d = 3 - 2*r;
    while (x <= y) {
        plot(fb, -x, -y);
        if (d < 0) d += 4*x + 6;
        else { d += 4*(x-y) + 10; y--; }
        x++;
    }
    x = 0; y = r; d = 3 - 2*r;
    while (x <= y) {
        plot(fb, -y, -x);
        if (d < 0) d += 4*x + 6;
        else { d += 4*(x-y) + 10; y--; }
        x++;
    }
    x = 0; y = r; d = 3 - 2*r;
    while (x <= y) {
        plot(fb, -y, x);
        if (d < 0) d += 4*x + 6;
        else { d += 4*(x-y) + 10; y--; }
        x++;
    }
    x = 0; y = r; d = 3 - 2*r;
    while (x <= y) {
        plot(fb, -x, y);
        if (d < 0) d += 4*x + 6;
        else { d += 4*(x-y) + 10; y--; }
        x++;
    }
    return count_px(fb);
}

double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e9 + ts.tv_nsec;
}

int main(void) {
    printf("================================================================\n");
    printf("  Fair Comparison: With and Without 8-way Symmetry\n");
    printf("================================================================\n\n");
    
    int radii[] = {25, 50, 75, 100};
    int nradii = 4;
    int iters = 50000;
    
    for (int ri = 0; ri < nradii; ri++) {
        int r = radii[ri];
        FB *fb = mkfb(r*3, r*3);
        
        printf("Radius = %d:\n", r);
        printf("%-30s %10s %8s %10s\n", "Algorithm", "Time(us)", "Pixels", "ns/pixel");
        printf("----------------------------------------------------------------\n");
        
        /* Test each */
        typedef struct { const char *name; int (*fn)(FB*, int); } Alg;
        Alg algs[] = {
            {"DF2 Fixed (full circle)", df2_full},
            {"DF2 Fixed (8-way sym)", df2_sym8},
            {"Bresenham (8-way sym)", bres_sym8},
            {"Bresenham (full circle)", bres_full}
        };
        
        for (int ai = 0; ai < 4; ai++) {
            double t = 0;
            int px = 0;
            for (int i = 0; i < iters; i++) {
                clrfb(fb);
                double s = now_ns();
                algs[ai].fn(fb, r);
                t += now_ns() - s;
                px = count_px(fb);
            }
            t /= iters;
            if (px > 0) {
                printf("%-30s %10.2f %8d %10.2f\n", 
                       algs[ai].name, t/1000, px, t/px);
            } else {
                printf("%-30s %10s %8s %10s\n", 
                       algs[ai].name, "UNSTABLE", "---", "---");
            }
        }
        printf("\n");
        freefb(fb);
    }
    
    return 0;
}
