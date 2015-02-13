SRCDIR := src
TARGET := build
INCLUDE := -I include

default:
	mkdir -p $(TARGET)
	g++ $(SRCDIR)/client.cpp $(SRCDIR)/helpers.cpp $(INCLUDE) -o $(TARGET)/client -w
	g++ $(SRCDIR)/database_server.cpp $(SRCDIR)/helpers.cpp $(INCLUDE) -o $(TARGET)/server -lpthread -w

clean:
	rm -rf $(TARGET)
