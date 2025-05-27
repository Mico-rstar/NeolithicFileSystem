CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra

# 目标可执行文件
TARGET := ./test/testDisk.o

# 源文件
SRCS := ./test/testDisk.cpp ./src/disk.cpp

# 生成目标
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# 清理
clean:
	rm -f $(TARGET) diskdata.bin testfile.bin

.PHONY: all clean