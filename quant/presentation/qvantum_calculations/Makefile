
NAME=slides

.PHONY: all
all: $(NAME).pdf

%.pdf: %.tex
	pdflatex $<

.PHONY: clean
clean:
	rm -f *.aux *.out *.log *.pdf *.snm *.toc *.nav *.dvi

