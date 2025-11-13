echo -e '_user/%: user/%.c\n\t$(CC) $(CFLAGS) -fno-pic -static -fno-builtin -fno-stack-protector -Wall -Wextra -Wno-unused-parameter -m32 -MD -ggdb -I. -Iuser -o $@ $<' >> Makefile
