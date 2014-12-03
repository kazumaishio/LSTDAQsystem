#gcc -lrt -std=gnu99 -o FakeFEB src/FakeFEB.c src/AcceptTCPConnection.c src/AddressUtility.c src/CreateTCPServerSocket.c src/DieWithError.c src/DieWithMessage.c src/HandleTCPClient.c

##
## Makefile
##



SRCDIR  =src
TMPDIR  =tmp
INCDIR  =include
MAIN	=$(SRCDIR)/FakeFEB.c

CFLAGS  = -lrt -std=gnu99
CC      = gcc $(CFLAGS)

TARGET  = $(MAIN:$(SRCDIR)/%.c=%)
SRCS	= $(wildcard $(SRCDIR)/*.c)
OBJS    = $(SRCS:$(SRCDIR)/%.c=$(TMPDIR)/%.o)

all: $(TARGET)

$(TARGET): $(MAIN) $(OBJS)
	@ echo "<< creating executable $@ >>"
	$(CC)  $(OBJS) $< -o $@
	@ echo "<< compilation succeeded! >>"
$(OBJS): $(SRCS)
	@ echo "<< compiling $@ >>"
	$(CC) -c $< -o $@ -I $(INCDIR)

.PHONY: clean echo
clean:
	rm -f $(TMPDIR)/*.o *.o $(TARGET)

echo:
	@ echo "TARGET=$(TARGET)"
	@ echo "SRCS=$(SRCS)"
	@ echo "OBJS=$(OBJS)"
