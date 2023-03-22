# Makefile de exemplo

# compilador a utilizar
CC= gcc
# flags para compilar
CFLAGS= -g -Wall
# flags para linkar, incluíndo a biblioteca matemática
LDFLAGS= -lm
# gerar variáveis com listas ficheiros de cada tipo para usar nas várias regras
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c, %.o, $(SRCS))
HEADERS=$(wildcard *.h)
PDFS=$(wildcard *.pdf)

# nome do executável a produzir, compilar com "make"
TARGET= cot

all: $(TARGET)

# compilar executável com optimização, dar o comando "make release"
release: CFLAGS+= -O3
release: clean
release: $(TARGET)


$(TARGET): $(OBJS) Makefile
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

# listar dependências. para simplificar, cada ficheiro .o depende do correspondente .c, de todos os ficheiros .h e da Makefile
%.o: %.c $(HEADERS) Makefile
	$(CC) -c $(CFLAGS) -o $@ $<

# para criar um zip com todos os ficheiros modificados, dar o comando "make zip"
zip: $(TARGET).zip

$(TARGET).zip: Makefile $(SRCS) $(HEADERS) $(PDFS)
	zip -9 $@ $?

# para criar um tar.gz com todos os ficheiros, dar o comando "make tgz"
tgz: $(TARGET).tar.gz

$(TARGET).tar.gz: Makefile $(SRCS) $(HEADERS) $(PDFS)
	tar czvf $@ $^

# para forçar recompilação, dar o comando "make clean" e depois "make" ou o comando "make clean ; make"
clean:
	rm -f *.o $(TARGET)
