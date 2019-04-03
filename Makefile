
TARGET := aws-face-rekognition
SOURCES := main.cpp

OBJS := $(SOURCES:.cpp=.o)
LFLAGS := -Wl,-R/usr/local/lib -laws-cpp-sdk-core -laws-cpp-sdk-rekognition

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) -o $(TARGET) $(OBJS) $(LFLAGS) $(LIBS)

all: $(TARGET)

clean:
	rm -Rf $(TARGET)
	rm -Rf *.o

