SRCDIR := src
TARGET := build
INCLUDE := -I include

CLIENT = \
	$(SRCDIR)/client.cpp \
	$(SRCDIR)/helpers.cpp

SERVER = \
	$(SRCDIR)/database_server.cpp \
	$(SRCDIR)/memory_manager.cpp \
	$(SRCDIR)/helpers.cpp

default:
	mkdir -p $(TARGET)
	g++ $(CLIENT) $(INCLUDE) -o $(TARGET)/client -w
	g++ $(SERVER) $(INCLUDE) -o $(TARGET)/server -w -lpthread

client:
	mkdir -p $(TARGET)
	g++ $(CLIENT) $(INCLUDE) -o $(TARGET)/client -w

server:
	mkdir -p $(TARGET)
	g++ $(SERVER) $(INCLUDE) -o $(TARGET)/server -w -lpthread

clean:
	rm -rf $(TARGET)
