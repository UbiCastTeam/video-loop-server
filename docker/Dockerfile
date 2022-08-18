FROM debian:bullseye

RUN apt-get update
RUN apt-get install -y --no-install-recommends \
		build-essential meson libgstrtspserver-1.0-dev 

WORKDIR /opt/vls

# add and compile vls (note that file update will trigger a new copy hence new
# compilation)
ADD meson.build .
ADD vls@.service.in .
ADD src ./src

RUN meson setup build .
RUN meson compile -C build

ENTRYPOINT ["/opt/vls/build/vls"]
CMD ["-h"]
