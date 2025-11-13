_user/%: user/%.c
	$(CC) $(CFLAGS) -fno-pic -static -fno-builtin -fno-stack-protector \
	-Wall -Wextra -Wno-unused-parameter -m32 -MD -ggdb -I. -Iuser \
	-o $@ $<