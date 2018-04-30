# Application Framework

libsolace provides a number of classes to help building various applications while allowing
for efficient resouce management. To achive a number of assumptions about application's
lifecycle has been made in the design process. Main assumption is that applications has dedicated
phases:
 - Initialization.
 - Event loop / processing.
 - clean up.

It is not a requirement to have all or any of that stages in your application.
All the classes has been designed to be used independently so that is possible to use any class from the library
without the rest.
For applicatoins that do follow the above phases that library has extra components.
