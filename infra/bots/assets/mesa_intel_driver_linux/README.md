Creating the Mesa Intel Linux driver asset (which supports Vulkan)
============================================================

Using the automated asset python scripts requires that Docker be installed.

    $ sk asset upload mesa_intel_driver_linux

See below for steps on just building the driver.

Using Docker
------------
It is easiest to just use the pre-built docker image.

    docker build -t mesa-driver-builder:latest ./mesa-driver-builder/

    docker run --volume /tmp/out:/OUT --env MESA_VERSION=22.1.3 mesa-driver-builder:latest

You may change `/tmp/out` to be the desired output directory and `22.1.3` to be the desired
version of the mesa driver.

Finally, use `/tmp/out` as the input directory to the upload script.
