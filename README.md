# CERMon
## CLAS12 EVIO Rapid Monitoring 

![Main window of CERMon](https://github.com/JeffersonLab/CERMon/wiki/images/main_window.png "Main window of CERMon")

This is a ROOT based GUI code to write fast monitoring programs for CLAS12, or any other experiment using EVIO.
No attempt is made to make this a "universally configurable" program. It is flexible, but for a lot of features you
will need to change the source code. 

## Using this code at JLab

At JLab, on the CLON machines, there is a pre-compiled version in /home/clasrun/CERMon/bin and the
libraries in /home/clasrun/CERMon/lib. Using the "clasrun" account, an alias called `cermon` is
setup to run this code. From a terminal, to run RasterMon with the default ET parameters, you can simply type:

```shell
cermon
```

If you are not using the "clasrun" account, you can run the code with:

```shell
/home/clasrun/CERMon/cermon.sh  # Add optional arguments.
```

If you want to setup the environment for the code but not run it, you can source this using the bash shell, after
which you can run the code directly without a script:

```bash
. /home/clasrun/CERMon/cermon.sh
CERMon # arguments
```

The standard behavior of the code is to connect to the ET ring at clondaq6. You can specify a different ET with
options:
```shell
cermon --et --host clondaq7 --port 11111 --etname /et/clasprod 
```

Or you can analyze a set of files from disk with:
```shell
cermon /cache/clas12/rg-c/data/clas_999999/clas_999999.evio.*   # replace with proper file path.
```

This will run through the set of EVIO files you specify on the commandline. You
can also use the GUI to select files or switch the ET under the "file" menu.

See ```CERMon --help``` for details on the commandline options.

## Details

More details for running this code are in the 
[CERMon GitHub wiki](https://github.com/JeffersonLab/CERMon/wiki). 
