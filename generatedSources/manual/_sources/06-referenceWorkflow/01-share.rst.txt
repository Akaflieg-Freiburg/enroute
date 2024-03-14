.. _importData:

Import Data
===========

**Enroute Flight Navigation** offers a robust file import feature, allowing
users to import data from various external sources. This document provides a
step-by-step guide on how to utilize this functionality.  A list of supported
file formats and text data items is found at the end of this section

There are four ways to import files into **Enroute Flight Navigation**.

- Import Files and Text Data Shared by Other Apps
- Import Files from Local or Remote Storage
- Drag-and-Drop (Linux Desktop only)
- Copy-and-Paste (Linux Desktop only)
- Command line (Linux Desktop only)

We expect that most users on mobile devices will import files that are shared by
other apps.  Users on Linux desktop computers will probably prefer drag-and-drop.


Import Files and Text Data Shared by Other Apps
-----------------------------------------------

**Enroute Flight Navigation** is able to import files from all apps that allow
file sharing.  On Android, **Enroute Flight Navigation** also accepts text data
shared by other apps.  While the list of these apps is endless, we expect that 
most users employ one of the following programs to transfer and access files
and data.

- Web browsers allow downloading and opening files from the internet.
- Apps such as Google Maps share text data items. 
- File management apps can open files stored on your device. 
- Most file management apps also allow opening files stored on cloud services
  connected to your devices. Some cloud service come with specialized apps.
- Email apps can open files attached to your mail messages, instant messaging
  apps can open files attached to chat messages.
- Both Android and iOS come with specialized apps that allow sharing files
  between devices, using Bluetooth or Wi-Fi connections.

The precise procedure for opening a file depends on the app in use. In a typical
scenario, do the following.

1. Identify and open an app that is able to access the file. This could be a web
   browser for files from the internet, a file manager for files stored on your
   device, or an email app for files attached to your mail messages.
2. Navigate to the file that you with to import. You might have to point your
   browser to the correct website, open the correct folder in your file manager,
   or open an email that has the relevant file attached.
3. Open the file. In some scenarios, it suffices to click on the file name or
   file icon.  Other apps require you to click on an appropriate button or menu
   entry, typically called "Open" or "Share".
4. The operating system will identify the file type and present you with a list
   of apps known to handle files of this type. Choose **Enroute Flight
   Navigation** from this list.
5. **Enroute Flight Navigation** will open and import the file.

.. note:: TripKits are ZIP files with specialized content. Trying to open a 
  TripKit file, some file management utilities will automatically unpack the ZIP 
  file rather than offering to open it in **Enroute Flight Navigation**.  Along 
  similar lines, GeoTIFF files are image files with specialized metadata and some
  file management utilities will launch an image viewing application rather than
  offering to open a GeoTIFF file in **Enroute Flight Navigation**.
  
  If you encounter problems opening a TripKit or GeoTIFF file, look for an icon
  or menu item labeled "Open with…".  Some utilities open an appropriate context 
  menu after a tap-and-hold gesture.  Alternatively, import the file from local 
  or remote storage, as explained in the next section.


Import Files from Local or Remote Storage
-----------------------------------------

There are scenarios where the operating system cannot identify the file type and
does not offer to open a given file with **Enroute Flight Navigation**.  In
these settings, **Enroute Flight Navigation** offers an alternative mechanism to
import files.

1. Transfer the file to your device and save it in the local file storage.
   Alternatively, save the file in cloud storage service that is connected to
   your device. 
2. Open **Enroute Flight Navigation**. Use the main menu to navigate to one of
   the pages listed below.
3. Open the three-dot-menu in the top right corner of the screen and choose the
   item "Import".  A file dialog will open.  
4. Select the file and click on the button "Import".

.. note:: There are systems where the file dialog shows only files of one specific 
   type. If you cannot see your file, look for a button or drop-down-menu in the 
   file dialog that allows choosing the file types.


Pages with File Import Functionality
....................................

The following pages of **Enroute Flight Navigation** offer import functionality.

===================== =========== ====================
Page                  File Type   Function
===================== =========== ====================
Route                 GeoJSON     Flight Route (replaces current flight route)
Route                 GPX         Flight Route (replaces current flight route)
Library/Route         GeoJSON     Flight Route (added to library)
Library/Route         GPX         Flight Route (added to library)
Library/Maps and Data MBTILES     Raster Maps
Library/Maps and Data MBTILES     Vector Maps
Library/Maps and Data OpenAir     Airspace Data
Library/Maps and Data GeoTIFF     Approach Chart
Library/Maps and Data ZIP/TripKit Approach Chart Collection
Library/Waypoints     CUP         Waypoint Collection
Library/Waypoints     GeoJSON     Waypoint Collection
Library/Waypoints     GPX         Waypoint Collection
===================== =========== ====================


Drag-and-Drop (Linux Desktop Only)
----------------------------------

If you are running **Enroute Flight Navigation** on a Linux desktop computer,
you can import a file by dragging its icon from the desktop and drop it anywhere
in the **Enroute Flight Navigation** window.  You can import text data items by
dragging the text into the **Enroute Flight Navigation** window.


Command Line (Linux Desktop Only)
---------------------------------

When starting **Enroute Flight Navigation** from the Unix command line, it is
possible to pass file names as command line arguments.


Supported File Formats
----------------------

**Enroute Flight Navigation** accepts data in the following formats.

========================= =================================== 
Functionality             File Format
========================= ===================================
Airspace Data             `OpenAir <https://pyopenair.readthedocs.io/en/latest/openair.html>`_ 
Approach Charts           `GeoTIFF <https://trac.osgeo.org/geotiff>`_
Approach Chart Collection `ZIP <https://en.wikipedia.org/wiki/ZIP_(file_format)>`_/`TripKit <https://mpmediasoft.de/products/AIPBrowserDE/help/AIPBrowserDE%20-%20Trip-Kit-Spezifikation.html>`_
FLARM Test Data           Text file
Flight Routes             `GPX <https://en.wikipedia.org/wiki/GPS_Exchange_Format>`_  
Flight Routes             `GeoJSON <https://en.wikipedia.org/wiki/GeoJSON>`_          
Raster Maps               `MBTILES <https://docs.mapbox.com/help/glossary/mbtiles/>`_
Vector Maps               `MBTILES <https://docs.mapbox.com/help/glossary/mbtiles/>`_
Waypoint Collections      `GeoJSON <https://en.wikipedia.org/wiki/GeoJSON>`_
Waypoints                 `CUP <https://downloads.naviter.com/docs/SeeYou_CUP_file_format.pdf>`_ 
========================= ===================================


Supported Text Data Items
-------------------------

**Enroute Flight Navigation** accepts data in the following formats.

========================= =================================== 
Functionality             Sample
========================= ===================================
Google Map Link
Shortened Google Map Link xx
OpenStreetMap Link
WeGo Link
Shortened WeGo Link
GEO URLs
========================= ===================================
