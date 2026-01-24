# DF2 Circle Algorithm - Makefile
#
# Build all benchmark executables

CC = gcc
CFLAGS = -O3 -Wall -Wextra
LDFLAGS = -lm

TARGETS = df2_benchmark fair_comparison

all: $(TARGETS)

df2_benchmark: df2_circle_benchmark.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

fair_comparison: fair_comparison.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

test: all
	@echo "=== Running DF2 Benchmark ==="
	./df2_benchmark
	@echo ""
	@echo "=== Running Fair Comparison ==="
	./fair_comparison

.PHONY: all clean test
