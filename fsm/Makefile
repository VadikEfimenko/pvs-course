
DIR = generate
MAIN_FILE = server.def
DOT_FILE = server.dot
TEX_FILE = server.tex

fsm: $(MAIN_FILE)
	autogen $<
	python2 fsm2dot $< > $(DOT_FILE)
	dot2tex -ftikz --autosize --crop $(DOT_FILE) > $(TEX_FILE) 
	pdflatex $(TEX_FILE)

PYCFLOW2DOT = ~/.local/bin/cflow2dot
SRC_FILE = ../main.c ../fsm/socket.c ../process/process.c
OUTPUT_FORMAT = pdf
OUTPUT_NAME = server-flow
flow:
	$(PYCFLOW2DOT) -i $(SRC_FILE) -f $(OUTPUT_FORMAT) -o $(OUTPUT_NAME)
