OBJS = config.o log.o
all : $(OBJS)
$(OBJS) : %.o : %.cpp
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE)

clean:
	rm -rf $(OBJS)
