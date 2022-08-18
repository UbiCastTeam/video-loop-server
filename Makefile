SHELL=/bin/bash

MEDIA_FOLDER=/tmp/media
DOCKER_IMAGE_NAME=vls
DOCKER_CONTAINER_NAME=vls
VLS_LISTEN_ADDR=0.0.0.0
VLS_RTSP_PORT=8554

all: help

help:
	@echo "this Makefile is for Docker only, if you want to build and install on \
		you machine use meson"
	@echo "for Docker use you can type `make run MEDIA_FOLDER=/path/to/folder`"

build .docker_build: src/*.c meson.build
	docker build -t $(DOCKER_IMAGE_NAME) -f docker/Dockerfile .
	touch .docker_build

run: .docker_build
	@if [[ ! -d $(MEDIA_FOLDER) ]]; then                                     \
		echo "$(MEDIA_FOLDER) does not exist";                               \
	elif [[ -z "$(shell ls -A $(MEDIA_FOLDER))" ]]; then                     \
		echo "$(MEDIA_FOLDER) is empty";                                     \
	else                                                                     \
		docker run --rm --name $(DOCKER_CONTAINER_NAME)                      \
		-v $(abspath $(MEDIA_FOLDER)):/media                                 \
		-p $(VLS_RTSP_PORT):$(VLS_RTSP_PORT)                                 \
		$(DOCKER_IMAGE_NAME) -s --listening_address $(VLS_LISTEN_ADDR)       \
		--rtsp_port $(VLS_RTSP_PORT) /media;                                 \
	fi

.PHONY: build
