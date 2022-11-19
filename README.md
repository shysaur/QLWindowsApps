QLWindowsApps
=============

**QLWindowsApps** includes a QuickLook Plugin and a Spotlight importer
for Microsoft Windows DLL and EXE files, intended as companions to Wine on
macOS. It is based on a modified version of
[icoutils](http://www.nongnu.org/icoutils/) by Oskar Liljeblad and Frank Richter.


Features
--------

- EXEs and DLLs can be Quick-Looked in the Finder and their properties
  (version info and bitness)  will be visible to Spotlight.
- EXEs will have an icon just like native apps.
- Supports 16-bit, 32-bit and 64-bit EXEs and DLLs.
- Shows all the embedded version info, localized in the current language if
  localizations are available.
- It refuses to read big executables (>32 MB) over a network share (yes, this
  is a feature).

### Supported OS X versions

QLWindowsApps 1.3.3 works on macOS High Sierra and later,
and was tested up to macOS Ventura.

If you are using a macOS version older than High Sierra, you can download
QLWindowsApps 1.3.2, which works on OS X Mavericks and later, or
QLWindowsApps 1.0.2, which works on Mac OS X Leopard and later.
  

Known Issues
------------

### 16-bit binary support

The icoutils webpage states: "There is no relocation support for
16-bit (NE) binaries." Fortunately, relocatable 16-bit binaries seem to be
quite uncommon.

For some reason, even when the binary is non-relocatable, icoutils chokes a bit
on 16 bit resources, even when it extracts them fine.


Warning for releases before 1.3.0
---------------------------------

In versions before 1.3.0, QLWindowsApps used to modify the custom icon of EXE
files in order to hide the frame added to the icon by QuickLook. But from version
1.3.0, QLWindowsApps uses a recently discovered private QuickLook API to
hide the icon frame without the need to modify the file.

It is possible to disable changing the icon of files in older versions of
QLWindowsApps by executing the following command in Terminal.app:

```Shell
defaults write com.danielecattaneo.qlgenerator.qlwindowsapps DisableIconChange 1
```


Copyright & License
-------------------

The icoutils are copyright (c) 1998 Oskar Liljeblad.
QLWindowsApps is copyright (c) 2012-2019 Daniele Cattaneo.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses/).


