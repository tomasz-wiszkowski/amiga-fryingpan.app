This is the FryingPan, version 1.2, native.

IMPORTANT NOTES:

This program has attached installer script. 
1) Please *DO* use it. 
2) Please *DO* install this program over the old install
3) Please *DO* add an assign unless you know you don't need it.

Reports about popping up requesters, library assertions, not loading configurations 
will be categorised as configuration error and ignored just like such user ignores 
the above points.

the change of the program has changed SIGNIFICANTLY. FryingPan performs 
operations in background, so the UI is pretty much independent to the operation.
therefore it is possible to launch i.e. track download or recording and shut down
application, the burning will continue, and this is the intended behavior.

FryingPan misses abort functionality (to be done in next days) and certain features
that older version had. However the current design allows implementation and these
features will be added within next few weeks. Major missing elements are: 
- multisession iso (with session import),
- speed setting,
- old and MMC3 incompatible drive support (if your drive refuses to initialize, you 
  will need to wait for compatibility updates - as i said - a few weeks top)

it is strongly advised to use native version of the fryingpan for any of the four 
platforms (AROS, MORPHOS, OS3 and OS4), however OS3 version can be used as well
on MoprhOS and OS4 (if you feel like using it). do NOT mix binaries from different
architectures.

FryingPan 1.0 is transparent to 0.41 from DOS/RAMLIB perspective (the vital files will
not be confused; furthermore, if you rename old FryingPan's name into something else,
it is okay to put 1.0 in the same directory). However, it is suggested to use only one in 
a session (that is: until next reboot, or until 'avail flush'). Do not use both: the 
controlling engine has been rewritten and may not neccessarily cooperate with old one.

THINGS TO BE DONE:
the current release is operational, but still a little bit limited when it comes to 
control. Still missing features are:

- ABORT button (to be done)
- GHOSTING UI ELEMENTS that will reject operations
- SESSION IMPORT (iso)
- MUI Settings needs to be available via menu option
- SPLIT ISO IMPORT: split iso already works with fryingpan, but there is no way
  to import previously splitted image.
- CoFFE will have to be recompiled to inherit changes done to the engine and other libraries
- PLUGINS: some plugins are partially ready. if you are a Gnu/C developer and feel like writing 
  a plugin to utilize the fryingpan capabilities in your program, write to me, too.


KNOWN ISSUES:
- please visit www.tbs-software.com/bugtracker to see the list of issues currently opened.

CHANGES:
  Please consult the CHANGES.txt for details.


FRYINGPAN can still be registered and its development continues.
Please support the Amiga Software by registering this product.

More information is available at:
http://www.tbs-software.com/fp

Please report bugs at:
http://www.tbs-software.com/bugtracker

If you have any questions that are not answered on my site, please contact me
Tomasz Wiszkowski <Tomasz.Wiszkowski@gmail.com>

