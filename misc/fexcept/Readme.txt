  FAR Exception handler used to replace existing FAR exception
  procedure, but saves to file full exception dump using plugin MAP
  files and disassembles the place of exception.

  Warning: To use exception replacement you must have a FAR manager version
           above "1.70 beta 5 (build 15XX)".

  Warning: To execute handler replacement you should have FAR
           ExceptionRule enabled. See in file "FAR installed dir"\TechInfo.txt
           #17 Rule.

  Warning: For correct execution of replacement you must have Create and Write
           permissions into FAR directory (or change sources to write into
           other directory!).

  Warning: To compile or run exception replacement you need to have an
           ExcDump library with release date closest to FExcept release date.

  Warning: To use you must have ExcDump library. Req ExcDump version: 08-07-2004
           or later.

--------------------------------------------------------------------------------
  0. If you still do not do so: download the latest ExcDump library
       ftp://far:far@194.226.246.33/ExcDump/ExcDump_DDMMYY.zip

      where:
        dd - latest day
        mm - latest month
        yy - latest year

     in additional for demangle function download
       ftp://far:far@194.226.246.33/HaronDemangle.rar
     and put near ExcDump.dll
--------------------------------------------------------------------------------

  1. Unpack all files from archive to any directory

  2. Copy ExcDump.dll, fexcept.dll (and HaronDemangle.dll optional)
     into the FAR directory  (directory from where far.exe run)

  3. Import SetFarExceptionHandler.reg into registry

  4. Run FAR as normal.
     In case you do all things correctly, the next time any plugin traps
     you should see a message:
      +========== Trap log ===========+
      �   Generating trap log file... �
      +===============================+

     followed by message:
      +================= Exception error... =================+
      � Plugin:                                              �
      �   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  �
      � executes an error and will be unloaded.              �
      � The trap log file has been saved to file:            �
      �   yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy  �
      �------------------------------------------------------�
      � Do you want to terminate FAR itself ? (recommended)  �
      �                       Yes  No                        �
      +======================================================+

     where instead "x" line will be plugin name or FAR.exe itself
     and instead "y" line the name of generated file.
