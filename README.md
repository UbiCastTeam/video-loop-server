# Video Loop Server (aka VLS)

This project provides a simple to use video streaming application
capable of streaming multiple videos in a seamless loop in order
to simulate e.g. IP cameras.

It is not a production ready server but rather use for development.

Supported protocols:
* RTSP (gst-rtsp-server)

## Building

The project is using the meson build system, and for building needs the
following dependencies

* gst-rtsp-server (will include glib, and all gst needed stuff)

```
sudo apt install build-essential meson libgstrtspserver-1.0-dev
```

### Native

```
meson build
meson compile -C build
```

You can then install it

```
sudo meson install -C build
```

The binary will be installed under /usr/local/bin/vls

### Docker

With Docker you can run the VLS without having to install it on you system, see
the next Running section

## Running

### Native

```
vls /path/to/media/folder/to/be/serverd
```

Will run the server and show the RTSP stream URI for all supported media in the
folder.

To see all available options check `vls --help`

## Systemd integration

To install or run it as a systemd service you can use the following command:

```
sudo systemctl enable vls@$(systemd-escape /path/to/media/folder).service --now
```

Note: as we cannot pass `/` as instance name, we use `systemd-escape` to escape
them. This is not supported on the systemd version included in Debian 11, so on old
systems copy the provided vls.service.example to /etc/systemd/system/

```
sudo copy vls.service.example /etc/systemd/system/vls.service
[EDIT THE /etc/systemd/system/vls.service FILE TO ADJUST THE PATHS]
sudo systemctl daemon-reload
sudo systemctl enable --now vls.service
```

Note 2: when started with systemd you can see the media resource URI with

```
systemctl status vls@$(systemd-escape /path/to/media/folder).service
Aug 19 17:38:56 myserver systemd[1]: Started Video Looped Server.
Aug 19 17:38:56 myserver vls[20799]: serving 2 media
Aug 19 17:38:56 myserver vls[20799]: RTSP stream ready at rtsp://0.0.0.0:8554/cam1-mp4
Aug 19 17:38:56 myserver vls[20799]: RTSP stream ready at rtsp://0.0.0.0:8554/cam2-mp4
```

If you add new videos you must reload the service with
```
systemctl restart vls.service
```

### Docker

```
make run MEDIA_FOLDER=/path/to/media/folder/to/be/serverd
```

Apart from the `MEDIA_FOLDER` variables, other customization can be by
overriding the following make variables:

* VLS_LISTEN_ADDR (default value: 0.0.0.0)
* VLS_RTSP_PORT (default value: 8554)

This command will launch the VLS server and display the video resource
available.

Ex: rtsp://0.0.0.0:8554/[slug-of-media-basename]
