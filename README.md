configuration
-------------

 set `ADS8321_PIN_XX` macros to appropriate <PORT, PIN>


build and install
-----------------
assuming avrdude installed and /dev/ttyUSB0
```
make clean
make
make install
```



remarks
-------
- note: non-blocking read untested!
- 5V logic
- max 100kHz clk



