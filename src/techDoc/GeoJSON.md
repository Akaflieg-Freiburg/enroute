# GeoJSON files used in the Enroute app

**Enroute** uses standard GeoJSON files to encode geographic maps containing aeronautical information. This text describes the files in more detail.

There are three properties contained in every feature of the GeoJSON file:

* **TYP** - Describes the main type of the feature. 
  
  - **AD** - Airfields
  
  - **AS** - Airspaces
  
  - **NAV** - NavAids
  
  - **PRC** - Procedures. This includes traffic circuits, holding patterns in control zones, are arrival/departure procedures for control zones
  
  - **WP** - Waypoints. This includes (mandatory) reporting point and generic user-defined waypoints.
  
  All features of the same **TYP** have the same geometry, and the same list of optional and required properties. The **TYP**s are further subdivided into categories, as described below.  

* **CAT** - A string that describes the category of the feature, for instance as **AD-GLD** (=glider site) or **CTR** (=control zone). The possible values of **CAT** are described below for every value of **TYP**.

* **NAM** - Name of the feature, as a human-readable string.

The types are described below in more detail.

## TYP AD: Airfields

Airfields are GeoJSON features whose the geometry is always a point. The airspace is described by the properties listed below.

#### Property: CAT - required

Type of the airfield. This is exactly one of the following strings.

* **AD** - Civil or civil/military airfield or airport, without information about runways

* **AD-GRASS** - Civil or civil/military airfield or airport, without paved runway

* **AD-GRASS** - Civil or civil/military airfield or airport, with paved runway

* **AD-INOP** - Abandoned airfield

* **AD-GLD** - Glider site

* **AD-MIL** - Military airfield or airport, without information about runways

* **AD-MIL-GRASS** - Military airfield, without paved runway

* **AD-MIL-PAVED** - Military airfield, with paved runway

* **AD-UL** - Ultralight flying site

* **AD-WATER** - Sea airfield

#### Property: COD - optional

ICAO code of the airfield. A typical entry reads "EDDE"

#### Property: COM - optional

Communication radio frequencies (e.g. ATIS). Frequencies are separated by newline characters. Entries should be sorted as follows:

1. Tower frequencies

2. Ground frequencies

3. Apron frequencies

4. Other

A typical entry reads "TWR 121.150 MHz\nGROUND 121.750 MHz\nAPRON 121.900 MHz"

#### Property: ELE - required

Elevation of the airfield as a number, in meters above main sea level

#### Property: INF - optional

Information radio frequencies (e.g. ATIS). Frequencies are separated by newline characters. A typical entry reads "ATIS 133.450 MHz"

#### Property: NAM - required

Name of the airfield. A typical entry reads "ERFURT-WEIMAR"

#### Property: NAV - optional

Navigation frequencies pertaining to the airfield, such as ILS frequences. Frequencies are separated by newline characters. A typical entry reads "ATIS 133.450 MHz"

#### Property: ORI - optional

Orientation of the most important runway, as a number between 0 and 360. Paved runways are always more important than unpaved runways. Among runways of the same type, longer runways are more important than unpaved ones.

#### Property: OTH - optional

Navigation frequencies pertaining to the airfield, such as frequences that can be used to switch on the lights or frequencies for nearby airspaces. Frequencies are separated by newline characters.

#### Property: RWY - optional

Description of all operational runways. Runways are separated by newline characters. A typical entry reads "10/28, 2600x50m, ASPH, 095°"

#### Property: TYP - required

Always "AD"

## TYP AS: Airspaces

Airspaces are GeoJSON features whose the geometry is always a polygon. The airspace is described by the properties listed below.

#### Property: BOT - required

Lower boundary of the airspace, specified as a string in one of the following forms.

* **GND** - Ground level

* **5500** - 5500 ft above sea level

* **1000 AGL** - 1000 ft above ground level

* **FL 130** - flight level 130

#### Property: CAT - required

Category of the airspace. This is exactly one of the following strings.

* **A** - Airspace class A

* **B** - Airspace class B

* **CTR** - Control zone

* **C** - Airspace class C

* **D** - Airspace class D

* **DNG** - Danger area, except parachute jumping exercise zones (for these, see **PJE** below)

* **P** - Prohibited area

* **PJE** - Parachute jumping exercise zone 

* **R** - Restricted area

* **TMZ** - Transponder mandatory zone

#### Property: ID - required

Free-text string with the identifier of the item, as found in the openAIP database.

#### Property: NAM - required

Free-text string with the name of the airspace

#### Property: TOP - required

Upper boundary of the airspace, specified as a string in the form described under "Property BOT".

#### Property: TYP - required

Constant string "AS".

## TYP NAV: NavAids

Airspaces are GeoJSON features whose the geometry is always a polygon. The airspace is described by the properties listed below.

#### Property: CAT - required

One of the following strings that describes the category of the navaid. 

* **NDB**

* **VOR**

* **VOR-DME**

* **VORTAC**

* **DVOR**

* **DVOR-DME**

* **DVORTAC**

#### Property: COD - required

Code name of the navaid, such as "KRH"

#### Property: ELE - optional

Elevation of the navaid, in meters over sea level.

#### Property: NAM - required

Full-text name of the navaid, such as "Karlsruhe"

#### Property: NAV - required

Radio frequency of the navaid, as a string of the form "341.0 kHz", "113.60 MHz" or "113.60 MHz • 83X".

#### Property: MOR - required

Morse code of the station.

#### Property: TYP - required

Constant string "NAV"

## TYP PRC: Procedures

Procedures are GeoJSON features whose the geometry is always a line. The airspace is described by the properties listed below.

#### Property: CAT - required

Constant string "PRC"

#### Property: GAC - optional

Guess if the procedure is meant for standard aircraft, or for Glider/UL/Helicopters. This is "green" for standard aircraft and "red" for the others.

#### Property: NAM - optional

Name of the procedure, such as "TFC Glider"

#### Property: TYP - required

Constant string "PRC"

## TYP WP: Waypoints

Waypoints are GeoJSON features whose the geometry is always a point. The airspace is described by the properties listed below.

#### Property: CAT - required

One of the following strings that describes the category of the waypoint.

* **MRP** - Mandatory reporting point

* **RP** - Reporting point

* **WP** - Generic waypoint

#### Property: COD - required for CAT == MRP and CAT == RP

 A code name of the waypoint, such as "EDDE-S1".

#### Property: COM - optional

A string that describes an associated frequency, such as "EDDE - TWR 121.150 MHz".

#### Property: ICA - optional

A string with the ICAO code of an associated airfield, such as "EDDE".

#### Property: MID - optional

Internal ID of the waypoint in the AIXM database. This is a string such as "7295".

#### Property: NAM - required

Name of the waypoint, such as "ERFURT-WEIMAR (SIERRA1)",

#### Property: SCO - required for CAT == MRP and CAT == RP

Short description of the waypoint, such as "S1".

#### Property: TYP - required

Constand string "WP"
