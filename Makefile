# DF2 Circle Algorithm
# Top-level Makefile

.PHONY: all clean test paper

all:
	$(MAKE) -C src all

clean:
	$(MAKE) -C src clean

test:
	$(MAKE) -C src test

paper:
	cd paper && pdflatex df2_circle_paper.tex && pdflatex df2_circle_paper.tex
