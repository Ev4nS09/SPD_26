
###########
LATEX 		= latex
BIBTEX 		= bibtex
PDFLATEX 	= pdflatex
DVIPS		= dvips
PATCH		= patch

CLEAR		= @clear
DELETE		= rm -f
PRINT		= @echo

###########
BIBTEX_WARNING  = 'There were undefined references'
REF_WARNING 	= 'Rerun to get cross-references'
MAX_RUNS 	= 6
###########

fast:	01_recitation.tex 
	$(PDFLATEX) 01_recitation.tex
	$(PRINT) Done 

clean: 
	$(CLEAR)
	$(DELETE) *.aux *.bbl *.log *.blg *.dvi *.out *.dvi *.ps *.pdf
	$(DELETE) *\~
	$(DELETE) \#*
	$(PRINT) clean Done.
