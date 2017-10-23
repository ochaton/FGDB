# FGDB

This is the skeleton of futher project FGDB

You must have preinstalled:
cmake >= 3.1
gcc >= 4.8
libev >= 4.2
libev-devel >= 4.2
libmsgpackc >= 2.1
perl >= 5.010

## Build
```
mkdir build/
cd build
cmake ..
make
```

## Run
After stage build
```
./server_test
```

## Run tests
After stage run
```
make -C client/
```

## Client-Protocol
Uses Msgpack arrays
