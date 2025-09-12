/*
    Sun.cpp

    The orbital elements consist of 6 quantities which completely define a circular, elliptic, parabolic or hyperbolic orbit. 
    Three of these quantities describe the shape and size of the orbit, and the position of the planet in the orbit:

        a  Mean distance, or semi-major axis
        e  Eccentricity
        T  Time at perihelion

    A circular orbit has zero eccentricity. An elliptical orbit has an eccentricity between zero and one. A parabolic orbit 
    has an eccentricity of exactly one. Finally, a hyperbolic orbit has an eccentricity larger than one. A parabolic orbit 
    has an infinite semi-major axis, a, therefore one instead gives the perihelion distance, q, for a parabolic orbit:

        q  Perihelion distance  = a * (1 - e)

    It is customary to give q instead of a also for hyperbolic orbit, and for elliptical orbits with eccentricity close to one.

    The three remaining orbital elements define the orientation of the orbit in space:

        i   Inclination, i.e. the "tilt" of the orbit relative to the
            ecliptic.  The inclination varies from 0 to 180 degrees. If
            the inclination is larger than 90 degrees, the planet is in
            a retrogade orbit, i.e. it moves "backwards".  The most
            well-known celestial body with retrogade motion is Comet Halley.

        N   (usually written as "Capital Omega") Longitude of Ascending
            Node. This is the angle, along the ecliptic, from the Vernal
            Point to the Ascending Node, which is the intersection between
            the orbit and the ecliptic, where the planet moves from south
            of to north of the ecliptic, i.e. from negative to positive
            latitudes.

        w   (usually written as "small Omega") The angle from the Ascending
            node to the Perihelion, along the orbit.

    These are the primary orbital elements. From these many secondary orbital elements can be computed:

        q  Perihelion distance  = a * (1 - e)

        Q  Aphelion distance    = a * (1 + e)

        P  Orbital period       = 365.256898326 * a**1.5/sqrt(1+m) days,
           where m = the mass of the planet in solar masses (0 for
           comets and asteroids). sqrt() is the square root function.

        n  Daily motion         = 360_deg / P    degrees/day

        t  Some epoch as a day count, e.g. Julian Day Number. The Time
           at Perihelion, T, should then be expressed as the same day count.

        t - T   Time since Perihelion, usually in days

        M  Mean Anomaly         = n * (t - T)  =  (t - T) * 360_deg / P
           Mean Anomaly is 0 at perihelion and 180 degrees at aphelion

        L  Mean Longitude       = M + w + N

        E  Eccentric anomaly, defined by Kepler's equation:   M = E - e * sin(E)
           An auxiliary angle to compute the position in an elliptic orbit

        v  True anomaly: the angle from perihelion to the planet, as seen
           from the Sun

        r  Heliocentric distance: the planet's distance from the Sun.

    * from: http://www.stjarnhimlen.se/comp/tutorial.html

    Other elements:

        RA      Right Ascension
        Decl    Declination

         Right Ascension and Declination serve as an absolute coordinate system fixed on the sky, rather than a relative system like the 
         zenith/horizon system. Right Ascension is the equivalent of longitude, only measured in hours, minutes and seconds (since the 
         Earth rotates in the same units). Declination is the equivalent of latitude measured in degrees from the celestial equator (0 to 90). 
         Any point of the celestial (i.e. the position of a star or planet) can be referenced with a unique Right Ascension and Declination.
*/

#include "Sun.h"

Sun::Sun()
    : m_date(0.0)    // current date/time
    , m_baseDay(0)
    , m_baseMonth(0)
    , m_baseYear(0)
    , m_baseHour(0)
    , m_baseMinute(0)
    , m_baseSeconds(0.0)
    , m_offsetSeconds(0.0)
{
    m_Direction.x = 0;
    m_Direction.y = 0;
    m_Direction.z = 0;
}

Sun::~Sun()
{
}

void Sun::SetTime( int day, int month, int year, int hour, int minute, float seconds )
{
    m_baseDay = day;
    m_baseMonth = month;
    m_baseYear = year;
    m_baseHour = hour;
    m_baseMinute = minute;
    m_baseSeconds = seconds;

    CalculateSunPosition();
}

void Sun::SetLatLon( Angle lat, Angle lon )
{ 
    m_terrLat = lat; 
    m_terrLon = lon; 

    CalculateSunPosition();
}

double Sun::JulianDay( int day, int month, int year, double UT )
{
  //  Julian Day (valid from 1900/3/1 to 2100/2/28)

    //Julian day: 86400 s, Julian year: 365.25 d, Julian Century: 36525 d

    double julianDay = 0.0;
    if (month<=2) 
    {
        month = month+12; 
        year = year-1;
    }
    julianDay = (int)(365.25*year) + (int)(30.6001*(month+1)) - 15 + 1720996.5 + day + UT/24.0;
    return julianDay;
}

void Sun::Update( float seconds )
{
    m_offsetSeconds += seconds;
    CalculateSunPosition();
}

void Sun::CalculateSunPosition()
{
    const double a = 1.0;     // mean distance, a.u.

    Angle M;            // mean anomaly
    Angle L;            // mean longitude (M + w + N)
    Angle E;            // eccentric anomaly, defined by Kepler's equation:   M = E - e * sin(E)
    Angle w;            // longitude of perihelion
    double e = 0.0;     // eccentricity
    double oblecl;      // obliquity of the ecliptic
    Angle sunLon;       // longitude of the sun

    double x = 0.0;     // work vars
    double y = 0.0;
    double z = 0.0;

    // THIS ASSUMES SetTime() passed in UT, not local

    // UT time, hours
    double UT = (m_baseHour + m_baseMinute/60.0 + (m_baseSeconds+m_offsetSeconds)/3600.0);

    double julianDayTgt = JulianDay( m_baseDay, m_baseMonth, m_baseYear, UT );
    double julianDayRef = JulianDay( 31, 12, 1999, 0.0 );    // reference
    m_date = julianDayTgt-julianDayRef;     // # of julian days since ref
    
    // these orbital elements are valid for the sun's apparent orbit around earth
    w.SetDegrees( 282.9404 + 4.70935E-5 * m_date );         // longitude of perihelion
    e = 0.016709 - 1.151E-9 * m_date;                       // eccentricity
    M.SetDegrees( 356.0470 + 0.9856002585 * m_date );       // mean anomaly
    M.SetDegrees( rev( M.Degrees() ) );                   // reduce M to 0-360

    // we also need the obliquity of the ecliptic, oblecl:
    oblecl = 23.4393 - 3.563E-7 * m_date;

    // and the Sun's mean longitude, L:
    L.SetDegrees( w.Degrees() + M.Degrees() );
    L.SetDegrees( rev( L.Degrees() ) );

    // E = eccentric anomaly
    E.SetDegrees( M.Degrees() + (180.0/PI) * e * sin(M.Radians()) * (1.0 + e * cos(M.Radians())) );

    // now we compute the Sun's rectangular coordinates in the plane of the ecliptic, 
    // where the X axis points towards the perihelion:
    x = cos(E.Radians()) - e;
    y = sin(E.Radians()) * sqrt(1.0 - e*e);

    // convert to distance and true anomaly:
    double r = sqrt( x*x + y*y );
    double v = RadianToDegree(atan2( y, x ));

    // now we can compute the longitude of the Sun:
    sunLon.SetDegrees( v + w.Degrees() );
    sunLon.SetDegrees( rev( sunLon.Degrees() ) );

    // --> the longitude of the sun is important, but we still don't know where in the
    //     sky the sun is located

    // Finally we'll compute the Sun's ecliptic rectangular coordinates, rotate these to equatorial coordinates
    // and then compute the Sun's RA and Decl:

    x = r * cos( sunLon.Radians() );
    y = r * sin( sunLon.Radians() );
    z = 0.0;

    // using oblecl, we rotate these coordinates:

    double xequat = x;
    double yequat = y * cos(DegreeToRadian(oblecl));    // - 0.0 * sin(oblecl);
    double zequat = y * sin(DegreeToRadian(oblecl));    // + 0.0 * cos(oblecl);

    // Convert to RA and Decl:

    double RA_deg = RadianToDegree(atan2( yequat, xequat ));        // calc RA in degrees
    Angle Decl;
    Decl.SetRadians( atan2( zequat, sqrt( xequat*xequat + yequat*yequat ) ) );

    // *** SIDEREAL TIME ***

    // the Sidereal Time tells the Right Ascension of the part of the sky that's precisely south, i.e. in the 
    // meridian. Sidereal Time is a local time, which can be computed from:

    //SIDTIME = GMST0 + UT + LON/15

    // where
    //      SIDTIME, hours + decimals
    //      GMST0 = GMST0 is the Sidereal Time at the Greenwich meridian at 00:00 right now, hours + decimals
    //      UT = same as Greenwich time, ours + decimals
    //      LON = terrestial longitude in degrees (western longitude is negative, eastern positive)

    // Now, how do we compute GMST0? Simple - we add (or subtract) 180 degrees to (from) L, 
    // the Sun's mean longitude, which we've already computed earlier. Then we normalise the
    // result to between 0 and 360 degrees, by applying the rev() function. Finally we divide by 15 to convert degrees to hours:

    double GMST0_hr = L.Degrees()/15.0 + 12.0;   // calc in hours

    double SIDTIME_hr = (GMST0_hr + UT + m_terrLon.Degrees()/15.0);  // calc SIDTIME in hours
    double SIDTIME_deg = SIDTIME_hr*15.0;                // convert to degrees
    SIDTIME_deg = rev( SIDTIME_deg );       // keep SIDTIME 0-360

    // to compute the altitude and azimuth we also need to know the Hour Angle, HA. The Hour Angle 
    // is zero when the clestial body is in the meridian i.e. in the south (or, from the southern 
    // hemisphere, in the north) - this is the moment when the celestial body is at its highest above the horizon.

    // the Hour Angle increases with time (unless the object is moving faster than the Earth rotates; this is the 
    // case for most artificial satellites). It is computed from:

    Angle HA;
    HA.SetDegrees( SIDTIME_deg - RA_deg );

    // now we'll convert the Sun's HA and Decl to a rectangular (x,y,z) 
    // coordinate system where the X axis points to the celestial equator in the south, the Y axis to 
    // the horizon in the west, and the Z axis to the north celestial pole: The distance, r, is here 
    // irrelevant so we set r = 1 for simplicity:

    x = cos(HA.Radians()) * cos(Decl.Radians());
    y = sin(HA.Radians()) * cos(Decl.Radians());
    z = sin(Decl.Radians());

    // now we'll rotate this x,y,z system along an axis going east-west, i.e. the Y axis, in such a 
    // way that the Z axis will point to the zenith. At the North Pole the angle of rotation will be 
    // zero since there the north celestial pole already is in the zenith. At other latitudes the 
    // angle of rotation becomes 90_deg - latitude. This yields:

    double xhor = x * sin(m_terrLat.Radians()) - z * cos(m_terrLat.Radians());
    double yhor = y;
    double zhor = x * cos(m_terrLat.Radians()) + z * sin(m_terrLat.Radians());

    // finally (!) we compute our azimuth and altitude:

    double azimuth = atan2( yhor, xhor ) + PI; //+ 180_deg
    double elevation = atan2( zhor, sqrt(xhor*xhor+yhor*yhor) );
    m_azimuth.SetRadians( azimuth );
    m_elevation.SetRadians( elevation );

    // align DIR vector with outside world's Y = elevation, X = E/W, Z=N/S
    m_Direction.x = yhor;
    m_Direction.y = zhor;
    m_Direction.z = xhor;
    m_Direction = Normalize( m_Direction );
}
