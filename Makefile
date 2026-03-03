PWD		= ./
usb		= 0
acm		= 0

# БАЗОВЫЕ КОМАНДЫ УПРАВЛЕНИЯ
.PHONY: start stop status build nconfig configure build-nuttx_builder

build-nuttx_builder:
	docker compose build nuttx_builder

start:
	@echo "🚀 Запуск всех контейнеров..."
	docker compose --profile build up -d
	@make status

stop:
	@echo "🛑 Остановка всех контейнеров..."
	docker compose down

status:
	@echo "📊 Статус контейнеров:"
	@docker compose ps || echo "Контейнеры не запущены"


build:
	@echo "🔨 Сборка в контейнере 'nuttx_builder'..."
	docker compose run --rm nuttx_builder make -C ./nuttx

nconfig:
	@echo "🔨 Вывод меню"
	docker compose run --rm nuttx_builder make nconfig -C ./nuttx

configure:
	@echo "🔨 Конфигурация"
	docker compose run --rm nuttx_builder ./nuttx/tools/configure.sh $(cfg)


.PHONY: stm32f4-st stm32f1-st stm32f1-j samd usb

stm32f4-st: build
	@echo "⚡ Прошивка через контейнер 'flasher'..."
	docker compose run --rm flasher openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
		-c 'init' -c 'reset halt' -c 'flash write_image erase $(PWD)/nuttx/nuttx.bin 0x08000000' \
		-c 'reset' -c 'exit'

stm32f1-st: build
	@echo "⚡ Прошивка через контейнер 'flasher'..."
	docker compose run --rm flasher openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
		-c 'init' -c 'reset halt' -c 'flash write_image erase $(PWD)/nuttx/nuttx.bin 0x08000000' \
		-c 'reset' -c 'exit'

stm32f1-j: build
	@echo "⚡ Прошивка через контейнер 'flasher'..."
	docker compose run --rm flasher openocd -f interface/jlink.cfg -c "transport select swd" \
	-f target/stm32f1x.cfg -c 'init' -c 'reset halt' -c 'flash write_image erase $(PWD)/nuttx/nuttx.bin 0x08000000' \
	-c 'reset' -c 'exit'

samd: build
	@echo "⚡ Прошивка через контейнер 'flasher'..."
	docker compose run --rm flasher openocd -f interface/jlink.cfg -c "transport select swd" \
	-f target/at91samdXX.cfg -c "program $(PWD)/nuttx/nuttx.bin verify reset exit 0x00000000"


usb:
	@echo "📟 Открытие последовательного порта через контейнер 'serial'..."
	docker compose run --rm serial picocom -b 115200 /dev/ttyUSB$(usb)

acm:
	@echo "📟 Открытие последовательного порта через контейнер 'serial'..."
	docker compose run --rm serial picocom -b 115200 /dev/ttyACM$(acm)


# ИНТЕРАКТИВНЫЕ СЕССИИ С КОНКРЕТНЫМИ КОНТЕЙНЕРАМИ
.PHONY: shell-build shell-flash shell-serial

shell-build:
	@echo "💻 Вход в контейнер сборки..."
	docker compose run --rm nuttx_builder bash

shell-flash:
	@echo "💻 Вход в контейнер прошивки..."
	docker compose run --rm flasher bash

shell-serial:
	@echo "💻 Вход в контейнер последовательного порта..."
	docker compose run --rm serial bash


# УТИЛИТЫ
.PHONY: clean distclean logs

clean:
	@echo "🧹 Очистка сборки..."
	docker compose run --rm nuttx_builder make clean -C ./nuttx

distclean:
	@echo "🧹 Очистка сборки..."
	docker compose run --rm nuttx_builder make distclean -C ./nuttx


logs:
	@echo "📝 Логи всех контейнеров:"
	docker compose logs -f


# Commands to show microcontrollers datashits
.PHONY: samd21-rm
samd21-rm:
	@okular /home/Workspace/Library/Base_Datasheets/Integrated\ \circuits/Microcontrollers/ARM/Cortex-M0+/Microchip/ATSAMD21/ATSAMD21\ \RM.pdf


# Commands to show schematics of the board with target devices
# Switchcore private
.PHONY: switchchore
switchcore:
	@okular /mnt/Server/untr-private/Якунин/BG/Documents/Э3/Коммутирующая.pdf
