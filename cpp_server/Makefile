OBJS = server.o serialport.o 
EXE = server
CFLAGS= -c -Wall -std=c++11

server: $(OBJS)
	g++ $(OBJS) -o $(EXE)

server.o: serialport.h server.cpp
	g++ server.cpp -o server.o $(CFLAGS)

serialport.o: serialport.h serialport.cpp
	g++ serialport.cpp -o serialport.o $(CFLAGS)

clean:
	@rm -f $(OBJS) $(EXE)
