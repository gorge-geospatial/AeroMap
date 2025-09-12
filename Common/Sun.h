/*
    Sun.h
    Perceived position/movement of the sun
*/

#ifndef SUN_H
#define SUN_H

#include "Calc.h"
#include "Angle.h"

class Sun
{
private:

    double m_date;       // "date" (time, daynum.dayfrac)

    // local time at last SetTime() call; actual local time = base + offset seconds passed in Update()
    int m_baseDay;
    int m_baseMonth;
    int m_baseYear;
    int m_baseHour;
    int m_baseMinute;
    float m_baseSeconds;

    // seconds since m_base*
    float m_offsetSeconds;

    // terrestrial lat/lon of our observer point
    Angle m_terrLat;
    Angle m_terrLon;

    Angle m_azimuth;        // apparent sun location
    Angle m_elevation;
    VEC3  m_Direction;      // direction (from origin) to sun

    void CalculateSunPosition();
    double JulianDay( int day, int month, int year, double UT );
    double rev( double x ) { return  x - floor(x/360.0)*360.0; }

public:

    Sun();
    virtual ~Sun();

    // seconds = virtual world seconds since last SetTime() parameters
    void Update( float seconds );   

    // set terrestrial (observer) location
    void SetLatLon( Angle lat, Angle lon );
    // set GMT
    void SetTime( int day, int month, int year, int hour, int minute, float seconds );  

    // get sun's apparent position
    void GetPosition( Angle& azimuth, Angle& elevation ) { azimuth = m_azimuth; elevation = m_elevation; }
    VEC3* GetDirection() { return &m_Direction; }

};

#endif // #ifndef SUN_H
