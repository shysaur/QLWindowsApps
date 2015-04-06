QLWindowsApps
=============

**QLWindowsApps** is a QuickLook Plugin for Microsoft Windows DLL and EXE 
files, intended as a companion to Wine on Mac OS X. It is based on a modified
version of [icoutils](http://www.nongnu.org/icoutils/) by Oskar Liljeblad and
Frank Richter.


Features
--------

- EXEs and DLLs can be Quick-Looked in the Finder. Plus, EXEs will have an icon
  just like native apps.
- Supports 16-bit, 32-bit and 64-bit EXEs and DLLs.
- Shows all the embedded version info, localized in the current language if
  localizations are available.
- It refuses to read big executables (>32 MB) over a network share (yes, this
  is a feature).
  
### Supported OS X versions

QLWindowsApps was tested on OS X Snow Leopard and later, up to OS X Yosemite.
It should also work on Leopard, but it was never tested on it.
  

Known Issues
------------

### QuickLook thumbnail frames

QuickLook by default adds a frame to thumbnails, and there's no API available
for disabling this frame. To work around this, the plugin simply sets the
file's custom icon (the one you can change in the Get Info window in the
Finder).

Because of this, filesystems that do not support Finder metadata writing
(like native NTFS) are stuck in QL-framed land, and I can't do anything about
it.

### 16-bit binary trouble

Like it's said in the icoutils webpage, "There is no relocation support for
16-bit (NE) binaries." Fortunately, relocatable 16-bit binaries seem to be
quite uncommon.

For some reason, even when the binary is non-relocatable, icoutils chokes a bit
on 16 bit resources, even when it extracts them fine.


Copyright & License
-------------------

The icoutils are copyright (c) 1998 Oskar Liljeblad.
QLWindowsApps is copyright (c) 2013-2015 Daniele Cattaneo.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses/).


