CXX      = g++
CXXFLAGS = -std=c++20 -g -Wall -Wextra
TARGET   = myshell
SRCS     = command.cc tokenizer.cc
HDRS     = command.h tokenizer.h
OBJS     = $(SRCS:.cc=.o)
all: $(TARGET)
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
%.o: %.cc $(HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f $(OBJS) $(TARGET)
.PHONY: all clean