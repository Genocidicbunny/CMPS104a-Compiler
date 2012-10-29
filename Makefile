
GCC     = gcc -g -O0 -Wall -Wextra -std=gnu99
LINT 	= lint -errchk=%all
SPLINT	= splint -namechecks +posixlib +standard 
MKDEPS	= gcc -MM

SRCS_C 	= astree.c lyutils.c auxlib.c strhash.c stringtable.c main.c
SRCS_H 	= astree.h astree.rep.h lyutils.h auxlib.h strhash.h stringtable.h
SRCS_L 	= scanner.l
SRCS_Y 	= parser.y
CLGEN	= yylex.c
CYGEN	= yyparse.c
HYGEN	= yyparse.h
GENS	= ${CLGEN} ${CYGEN} ${HYGEN}
ALLSRCC	= ${SRCS_C} ${CLGEN} ${CYGEN}
ALLSRC 	= ${SRCS_C} ${SRCS_H} ${SRCS_L} ${SRCS_Y}
OBJS 	= ${ALLSRCC:.c=.o}
LREPORT	= yylex.output
YREPORT	= yyparse.output
REPORTS	= ${LREPORT} ${YREPORT}
DEPS 	= Makefile.deps
BIN 	= oc

all: ${BIN}

${BIN}: ${OBJS}
	gcc -o${BIN} ${OBJS}


${CLGEN} : ${SRCS_L}
	flex -o${CLGEN} ${SRCS_L} 2>${LREPORT}
	rm lex.backup

${CYGEN} ${HYGEN} : ${SRCS_Y}
	bison -dtv -o${CYGEN} ${SRCS_Y}

clean:
	rm -f ${OBJS} ${GENS} ${DEPS} ${REPORTS}

cleanout:
	rm -f *.str
	rm -f *.tok

spotless: clean
	rm -f ${BIN}

deps: ${ALLSRCC}
	${MKDEPS} ${ALLSRCC} >> ${DEPS}

${DEPS}:
	touch ${DEPS}
	make deps

lint: ${SRCS_C}
	${LINT} ${SRCS_C}

splint: ${SRCS_C}
	${SPLINT} ${SRCS_C}
ci:
	ci -u ${ALLSRC} Makefile README

%.o: %.c
	${GCC} -c $<

include ${DEPS}
