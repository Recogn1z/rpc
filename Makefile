# ==== config ====
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -I./src/include -I./ -pthread
LIBS = -lprotobuf -lzookeeper_mt -lmuduo_net -lmuduo_base -lpthread -lglog


SRC_DIR = src
CALLEE_DIR = callee
CALLER_DIR = caller
BIN_DIR = bin


RPC_SRCS = $(SRC_DIR)/rpcapplication.cc $(SRC_DIR)/rpcconfig.cc \
           $(SRC_DIR)/rpccontroller.cc $(SRC_DIR)/rpcchannel.cc \
           $(SRC_DIR)/rpcprovider.cc $(SRC_DIR)/zookeeperutil.cc

# ==== Protobuf file ====
PROTO_SRCS = user.pb.cc src/rpcheader.pb.cc

# ==== executalbe file ====
SERVER = $(BIN_DIR)/server
CLIENT = $(BIN_DIR)/client

# ==== default target ====
all: $(SERVER) $(CLIENT)

# create bin path
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# ==== compile client ====
$(SERVER): $(BIN_DIR) $(RPC_SRCS) $(PROTO_SRCS) $(CALLEE_DIR)/server.cc
	$(CXX) $(CXXFLAGS) -o $@ $(CALLEE_DIR)/server.cc $(RPC_SRCS) $(PROTO_SRCS) $(LIBS)

# ==== compile server ====
$(CLIENT): $(BIN_DIR) $(RPC_SRCS) $(PROTO_SRCS) $(CALLER_DIR)/client.cc
	$(CXX) $(CXXFLAGS) -o $@ $(CALLER_DIR)/client.cc $(RPC_SRCS) $(PROTO_SRCS) $(LIBS)

# ==== clear ====
clean:
	rm -rf $(BIN_DIR)

# ==== run command ====
run-server: $(SERVER)
	./bin/server -i ./test.conf

run-client: $(CLIENT)
	./bin/client -i ./test.conf
