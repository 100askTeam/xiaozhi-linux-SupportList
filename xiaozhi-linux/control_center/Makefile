# 设置编译器和链接器
CROSS_COMPILE =/home/ubuntu/tina-v853-100ask/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-glibc/toolchain
CXX = ${CROSS_COMPILE}/bin/arm-openwrt-linux-g++
LD = $(CXX)

# 设置 Boost、WebSocket++、OpenSSL 的路径
BOOST_DIR = /home/ubuntu/work/linux-xiaozhi/boost_1_69_0/boost_arm
WEBSOCKETPP_DIR = /home/ubuntu/work/linux-xiaozhi/websocketpp-0.8.1/build/websocketpp-arm
OTHER_DIR = /home/ubuntu/tina-v853-100ask/out/v853-100ask/staging_dir/target/usr

# 设置头文件搜索路径
CXXFLAGS = -I$(BOOST_DIR)/include -I$(WEBSOCKETPP_DIR)/include -I$(OTHER_DIR)/include -I. 

# 设置库文件搜索路径
LDFLAGS = -L$(BOOST_DIR)/lib -L$(OTHER_DIR)/lib -L$(CROSS_COMPILE)/lib 

# 设置编译选项
CXXFLAGS += -std=c++17 -Wall

# 设置要链接的库
LIBS = -lboost_system -lboost_thread -lssl -lcrypto -lpthread -lcurl -lnghttp2

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

