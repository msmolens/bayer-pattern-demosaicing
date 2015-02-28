LATEX=latex
BIBTEX=bibtex
PDFLATEX=pdflatex
XDVI=xdvi
EPSTOPDF=epstopdf
MAIN=bayer_renderer
MAINTEX=$(MAIN).tex
MAINDVI=$(MAIN).dvi

all: dvi pdf

dvi:
	$(LATEX) $(MAINTEX)
	$(BIBTEX) $(MAIN)
	$(LATEX) $(MAINTEX)
	$(LATEX) $(MAINTEX)

pdf:
	$(EPSTOPDF) bayer_pattern.eps
	$(EPSTOPDF) channel_extraction.eps
	$(EPSTOPDF) interpolation.eps
	$(PDFLATEX) $(MAINTEX)
	$(BIBTEX) $(MAIN)
	$(PDFLATEX) $(MAINTEX)
	$(PDFLATEX) $(MAINTEX)

view:
	$(XDVI) $(MAINDVI)

clean:
	rm -f *.pdf *.log *.aux *.bbl *.blg *.dvi

