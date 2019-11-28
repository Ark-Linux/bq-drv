DESTDIR=/usr/local

bq25703a_drv: gpio_config.c bq25703a_drv.c 

install:
	install -d $(DESTDIR)/bin
	install bq25703a_drv $(DESTDIR)/bin

clean:
	rm -rf bq25703a_drv *.o
