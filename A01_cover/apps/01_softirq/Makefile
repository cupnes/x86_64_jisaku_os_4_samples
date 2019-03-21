TARGET = test
CFLAGS = -Wall -Wextra -nostdinc -nostdlib -fno-builtin -fno-common	\
	-Iinclude -fPIE
LDFLAGS = -Map app.map -s -x -T ../app.ld -pie
OBJS = app.o

$(TARGET): $(OBJS)
	ld $(LDFLAGS) -o $@ $+

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

deploy: $(TARGET)
	../../tools/create_fs.sh $(TARGET)
	mv fs.img ../../../fs/

clean:
	rm -f *~ *.o *.map $(TARGET) include/*~

.PHONY: deploy clean
