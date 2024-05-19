# Compiler and Compiler Flags
CC=gcc
CFLAGS=-Wall -g -Iinclude
# Linker flags
LDFLAGS=-lreadline

# The build target executable:
TARGET=miniOS
# Source, Object files
SRCS=kernel/kernel.c kernel/top/top.c
OBJS=$(SRCS:.c=.o) 

# Include directory
INCLUDE_DIR=include

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)


$(OBJDIR)/%.o: $(NEWDIR)/%.c
	$(cc) $(CFLAGS) -c -o $@ $<

# To obtain object fils
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# Clean up:
clean:
	rm -f $(OBJS) $(TARGET)
