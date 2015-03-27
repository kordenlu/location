BASE_DIR= ${HOME}/develop/server
SERVER_DIR = $(BASE_DIR)/location
SERVER_INCLUDE_DIR = $(SERVER_DIR)
SERVER_OBJ_DIR = $(SERVER_DIR)/.objs

SRC = $(wildcard *.cpp)
OBJS = $(addprefix $(SERVER_OBJ_DIR)/, $(subst .cpp,.o,$(SRC)))

MONGOC_INCLUDE_DIR = /usr/local/include/libmongoc-1.0
BSON_INCLUDE_DIR = /usr/local/include/libbson-1.0/

INC = -I$(MONGOC_INCLUDE_DIR) -I$(BSON_INCLUDE_DIR)

CPPFLAGS= -g -gdwarf-2 -fPIC -Wall -DDEBUG $(INC) -Wno-invalid-offsetof

all : $(OBJS)

$(SERVER_OBJ_DIR)/%.o : %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)