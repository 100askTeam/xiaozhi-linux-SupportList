# 设置编译器和链接器
CROSS_COMPILE =/home/ubuntu/k230_linux_sdk/output/k230_canmv_dongshanpi_defconfig/host
CXX = ${CROSS_COMPILE}/bin/riscv64-unknown-linux-gnu-g++
LD = $(CXX)

#SYSROOT = $(CROSS_COMPILE)/sysroot

# 设置 Boost、WebSocket++、OpenSSL 的路径
OTHER_DIR = /home/ubuntu/k230_linux_sdk/output/k230_canmv_dongshanpi_defconfig/host/riscv64-buildroot-linux-gnu/sysroot/usr

# 设置头文件搜索路径
#CXXFLAGS = --sysroot=$(SYSROOT) -I$(BOOST_DIR)/include -I$(OTHER_DIR)/include -I.
CXXFLAGS = -I${CROSS_COMPILE}/include -I$(OTHER_DIR)/include -I.

# 设置库文件搜索路径
#LDFLAGS = -L$(BOOST_DIR)/lib -L$(OTHER_DIR)/lib -L$(CROSS_COMPILE)/lib -L$(SYSROOT)/lib64 -L$(SYSROOT)/usr/lib 
LDFLAGS = -L$(OTHER_DIR)/lib -L$(CROSS_COMPILE)/lib

# 设置编译选项
CXXFLAGS += -std=c++17 -Wall

# 设置要链接的库
LIBS = -lboost_system -lssl -lcrypto -lpthread -lcurl -lnghttp2 -lgcc -lgcc_s

# 设置源文件和目标文件
SRCS =  $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

# 设置最终的可执行文件
TARGET = control_center

# 默认目标
all: $(TARGET)

# 链接生成可执行文件
$(TARGET): $(OBJS)
	$(LD) $(OBJS) -o $(TARGET) $(LDFLAGS) $(LIBS)

# 需要判断是否存在依赖文件
# .main.o.d .sub.o.d
dep_files := $(foreach f, $(objs), .$(f).d)
dep_files := $(wildcard $(dep_files))

# 把依赖文件包含进来
ifneq ($(dep_files),)
  include $(dep_files)
endif

# 编译源文件为目标文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(OBJS) $(TARGET)

