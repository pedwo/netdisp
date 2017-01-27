## netdisp

A Qt app for showing network and CPU load for the Renesas RZ processor.

Assumes Qt5 dev libraries/headers are already installed.

### Build

    $ qmake && make

### Operation

You can enable/disable networks by pressing them.
    
### Run

Make sure you have calibrated the screen with tslib first.

    $ ts_calibrate

Then run the app

    $ export QT_QPA_PLATFORM=linuxfb
    $ export QT_QPA_FB_TSLIB=1
    $ ./netdisp
