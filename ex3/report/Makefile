TARGET = parlab08-a3-report
CC = xelatex
CCFLAGS = -shell-escape -synctex=1


all: $(TARGET).pdf aux


aux:
	find . -iname "*.aux" -not -name $(TARGET).aux -delete



%.pdf:	%.tex title/* files/*
	$(CC) $(CCFLAGS) $<
	$(CC) $(CCFLAGS) $<


.PHONY:	clean all spell
clean:
	rm -f $(TARGET).{pdf,log,aux,pyg}

spell:
	aspell -l el_gr --ignore-case=true -c $(TARGET).tex
