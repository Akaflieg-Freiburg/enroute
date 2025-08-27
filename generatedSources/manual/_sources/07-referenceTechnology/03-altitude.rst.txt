
Altitude Measurement
====================


Types of Altitude used in **Enroute Flight Navigation**
-------------------------------------------------------

**Enroute Flight Navigation** uses several notions of altitude.


True Altitudes
^^^^^^^^^^^^^^

- **Geometric Altitude** or **True Altitude AMSL** is the vertical distance
  between your aircraft and the main sea level.  To avoid confusion, geometric
  altitudes are typically indicated in the form "T.ALT 6500 AMSL".
  
- **Absolute Altitude** or **True Altitude AGL** is the vertical distance
  between your aircraft and the terrain. To avoid confusion, absolute altitudes
  are typically indicated in the form "T.ALT 6500 AGL".
  
**Enroute Flight Navigation** computes the absolute altitude is computed using
satellite navigation data and a terrain elevation database.


Barometric Altitudes
^^^^^^^^^^^^^^^^^^^^

- **Pressure altitude** is the value shown by your altimeter when set to the
  standard value 1013.2hPa. To avoid confusion, pressure altitudes are typically
  indicated in the form "FL 65".
  
- **Altitude** is the value shown by your altimeter when set to QNH.  To avoid
  confusion, altitudes are typically indicated in the form "6500".

- **Cabin altitude** is the altitude at which an aircraft flying in the ICAO
  standard atmosphere experiences a static pressure equal to the pressure in the
  cabin of your aircraft. 

Altitude and pressure altitude is used to define airspace boundaries and
vertical aircraft positions.

.. warning:: In Central Europe, true altitudes and barometric altitudes typically 
  differ by 5-10%, but we have seen differences of 15% on warm summer days in 
  Germany. Never use true altitudes to judge vertical distances to airspaces!

**Enroute Flight Navigation** computes altitude and pressure altitude from
static pressure data reported by traffic data receivers and from QNH data
reported by nearby airfields.  If **Enroute Flight Navigation** is not connected
to a traffic data receiver or if the traffic data receiver does not report
static pressure, then no pressure altitude is available.  **Enroute Flight
Navigation** measures cabin altitude using the pressure sensor installed in your
mobile device.

.. note:: As a rule of thumb, any traffic data receiver that handles ADS-B 
  signals will report static pressure, as static pressure is required to 
  interpret ADS-B data. In particular, all PowerFLARM devices use and 
  report static pressure.


Density Altitude
^^^^^^^^^^^^^^^^

**Density altitude** is the altitude at which an aircraft flying in the ICAO
standard atmosphere experiences an air density equal to the density measured by
the ambient pressure/temperature sensors of your device. 
  
**Enroute Flight Navigation** computes the density altitude from METAR data, but
only for airfields reporting the necessary data.


Airspace Side View
------------------

.. _sideViewStaticPressure:

Starting with version 3.0.0, **Enroute Flight Navigation** is able to show a
sideways view of the airspace. However, the side view is only available if
**Enroute Flight Navigation** has access to static pressure information.  This
section of the manual explains why.


Why does **Enroute** need static pressure for the airspace side view?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Vertical airspace boundaries are defined as barometric altitudes, either over
QNH or over the standard pressure level.  As a consequence, the geometric
altitude of airspaces changes with the weather: Airspaces are typically much
lower on cold winter days than they are in summer. In order to show your
aircraft in relation to airspaces, **Enroute Flight Navigation** therefore needs
to know the barometric altitudes of your aircraft, or equivalently, the static
pressure.  

.. warning:: Never use true altitude to judge vertical distances to airspaces.


How can I provide static pressure data?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Follow the steps outlined in chapter :ref:`traffic` to connect **Enroute Flight
Navigation** to a traffic data receiver that provides static pressure data.  

If you fly an aircraft where static pressure and cabin pressure agree, you can
run the additional app `CCAS <https://ccas.aero/>`__, which runs in the
background, uses pressure sensors in your mobile device and reports the pressure
as static pressure to **Enroute Flight Navigation**.

.. warning:: We strongly recommend connecting **Enroute Flight Navigation** to a 
  proper traffic data receiver.  While CCAS might have its use for pilots flying 
  balloons, paragliders or gyrocopters, think twice before using CCAS in a 
  motorized plane or glider. In typical GA aircraft, static pressure and cabin 
  pressure do not necessarily agree.  The precise error typically depends on 
  airspeed and on the configuration of the heating and ventilation systems.


But other apps show side views without static pressure data!
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

We do not know the internal workings of other apps.  However, we do not see how
sufficiently reliable information can possibly be provided without static
pressure data. 

We fly general aviation aircraft in Germany and Switzerland, where vertical
separation between jet aircraft and airspace limits is sometimes no more than
500ft.  In view of the extremely severe consequences of airspace violations, we
decided against showing questionable data.
