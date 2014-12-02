##
## Makefile
##

MAIN = Master.cpp

## Compiler option
TARGET 	= $(MAIN:%.cpp=%)
SRCS	= $(wildcard *.cpp)
OBJS	= $(SRCS:%.cpp=%.o)
CXXFLAGS = -O3 -Wall
LFLAGS = -lpthread

CXX = g++ $(CFLAGS)



all: $(TARGET)

$(TARGET): $(OBJS)
	@ echo "<< creating executable $@ >>"
	$(CXX) $(LFLAGS) $(OBJS) $< -o $@
	@ echo "<< compilation succeeded! >>"
$(OBJS): $(SRCS)
	@ echo "<< compiling $@ >>"
	$(CXX) -c $< -o $@

mac:
	g++ -c $(SRCS)
	g++ -o $(TARGET) $(OBJS) $(LFLAGS)

.PHONY: clean echo
clean:
	rm -f *.o $(TARGET)

echo:
	@ echo "TARGET=$(TARGET)"
	@ echo "SRCS=$(SRCS)"
	@ echo "OBJS=$(OBJS)"
