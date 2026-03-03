PWD		= ./
usb		= 0
acm		= 0

.PHONY: start stop build-nuttx_builder

build-nuttx_builder:
	docker compose build nuttx_builder
start:
	docker compose --profile build up -d
stop:
	docker compose down

.PHONY: build nconfig configure
build:
	docker compose run --rm nuttx_builder make -C ./nuttx
nconfig:
	docker compose run --rm nuttx_builder make nconfig -C ./nuttx
configure:
	docker compose run --rm nuttx_builder ./nuttx/tools/configure.sh $(cfg)


.PHONY: stm32f4-st stm32f1-st stm32f1-j samd usb acm
stm32f4-st: build
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
		-c 'init' -c 'reset halt' -c 'flash write_image erase $(PWD)/nuttx/nuttx.bin 0x08000000' \
		-c 'reset' -c 'exit'
stm32f1-st: build
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
		-c 'init' -c 'reset halt' -c 'flash write_image erase $(PWD)/nuttx/nuttx.bin 0x08000000' \
		-c 'reset' -c 'exit'
stm32f1-j: build
	openocd -f interface/jlink.cfg -c "transport select swd" \
	-f target/stm32f1x.cfg -c 'init' -c 'reset halt' -c 'flash write_image erase $(PWD)/nuttx/nuttx.bin 0x08000000' \
	-c 'reset' -c 'exit'
samd: build
	openocd -f interface/jlink.cfg -c "transport select swd" \
	-f target/at91samdXX.cfg -c "program $(PWD)/nuttx/nuttx.bin verify reset exit 0x00000000"
usb:
	picocom -b 115200 /dev/ttyUSB$(usb)
acm:
	picocom -b 115200 /dev/ttyACM$(acm)


.PHONY: shell-build
shell-build:
	docker compose run --rm nuttx_builder bash


.PHONY: clean distclean
clean:
	docker compose run --rm nuttx_builder make clean -C ./nuttx
distclean:
	docker compose run --rm nuttx_builder make distclean -C ./nuttx
