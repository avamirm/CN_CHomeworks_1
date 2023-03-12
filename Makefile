CC=g++ -g --std=c++20

OUT_SERVER = server
OUT_CLIENT = client
PATH_SRC := src
PATH_BUILD := build
PATH_INC := $(PATH_SRC)/include
PATH_OBJ := $(PATH_BUILD)/obj

vpath %.hpp $(PATH_INC)
vpath %.cpp $(PATH_SRC)

OBJS_SERVER := server.o commandHandler.o hotel.o user.o room.o reservation.o
OBJS_CLIENT := client.o

all: $(PATH_BUILD) $(PATH_BIN) $(PATH_OBJ) $(OUT_CLIENT) $(OUT_SERVER)

$(OUT_SERVER): $(addprefix $(PATH_OBJ)/, $(OBJS_SERVER))
	$(CC) -o $@ $^

$(OUT_CLIENT): $(addprefix $(PATH_OBJ)/, $(OBJS_CLIENT))
	$(CC) -o $@ $^

$(PATH_OBJ)/server.o: server.cpp server.hpp commandHandler.hpp hotel.hpp user.hpp room.hpp reservation.hpp json.hpp date.hpp errors.hpp
	$(CC) -c $< -o $@

$(PATH_OBJ)/client.o: client.cpp client.hpp json.hpp date.hpp errors.hpp
	$(CC) -c $< -o $@	

$(PATH_OBJ)/commandHandler.o: commandHandler.cpp commandHandler.hpp hotel.hpp user.hpp room.hpp reservation.hpp json.hpp date.hpp errors.hpp
	$(CC) -c $< -o $@

$(PATH_OBJ)/hotel.o: hotel.cpp hotel.hpp user.hpp room.hpp reservation.hpp json.hpp date.hpp errors.hpp
	$(CC) -c $< -o $@

$(PATH_OBJ)/user.o: user.cpp user.hpp json.hpp errors.hpp
	$(CC) -c $< -o $@

$(PATH_OBJ)/room.o: room.cpp room.hpp
	$(CC) -c $< -o $@

$(PATH_OBJ)/reservation.o: reservation.cpp reservation.hpp
	$(CC) -c $< -o $@

$(PATH_BUILD): ; mkdir -p $@
$(PATH_OBJ): ; mkdir -p $@

.PHONY: all clean

clean:
	rm -r $(PATH_BUILD) > /dev/null
	rm -f $(OUTPUT) > /dev/null