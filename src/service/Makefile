OBJS = connection.o service.o
all : $(OBJS)
$(OBJS) : %.o : %.cpp
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE)

clean:
	rm -rf $(OBJS)
