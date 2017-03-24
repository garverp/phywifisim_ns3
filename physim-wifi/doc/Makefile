TEXI2HTML = texi2html
TEXI2PDF = texi2dvi --pdf
INKSCAPE = inkscape
CONVERT = convert
CSS = --css-include=manual.css
SPLIT = --split section

FIGURES = figures
DOC_FIGURES = ..
VPATH = $(FIGURES)

IMAGES_SVG = \
	${FIGURES}/figure1-architecture.svg \
	${FIGURES}/figure2-frameformat.svg \
	${FIGURES}/figure3-signalprocessing.svg \
	${FIGURES}/figure4-reception.svg \
	${FIGURES}/figure5-statemachine.svg

IMAGES_PDF = ${IMAGES_SVG:.svg=.pdf}
IMAGES_PNG = ${IMAGES_SVG:.svg=.png}

CHAPTERS = \
	manual.texi \
	architecture.texi \
	implementation.texi \
	frame-construction.texi \
	wireless-channel.texi \
	frame-reception.texi \
	phy-state-machine.texi \
	mac-integration.texi \
	remarks.texi \
	examples.texi \
	changelog.texi

%.png : %.svg; $(INKSCAPE) --without-gui --file=$(patsubst %.png,%.svg,$@) --export-png=$@
%.pdf : %.svg; $(INKSCAPE) --without-gui --file=$(patsubst %.pdf,%.svg,$@) --export-pdf=$@

all:  manual.pdf manual.html manual/manual.html after-all-clean

manual.pdf: $($IMAGES_SVG) $(IMAGES_PDF) $(CHAPTERS)
	$(TEXI2PDF) manual.texi

manual.html: $($IMAGES_SVG) $(IMAGES_PNG) $(CHAPTERS)
	$(TEXI2HTML) ${CSS} manual.texi

manual/manual.html: $($IMAGES_SVG) $(IMAGES_PNG) $(CHAPTERS)
	$(TEXI2HTML) ${CSS} ${SPLIT} --output manual manual.texi

figures-clean:
	rm -rf $(IMAGES_PNG) $(IMAGES_PDF)

texi-clean: 	
	rm -rf manual.aux manual.cp manual.cps manual.fn manual.ky manual.pg manual.tp
	rm -rf manual.vr manual.toc manual.log manual.pdf manual.html manual/ 

clean:	figures-clean texi-clean

after-all-clean: figures-clean
	rm -rf manual.aux manual.cp manual.cps manual.fn manual.ky manual.pg manual.tp
	rm -rf manual.vr manual.toc manual.log
