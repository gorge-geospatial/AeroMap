// Gis.cpp
// General-purpose geospatial functions.
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include <math.h>
#include <string.h>
#include <assert.h>

#include "XString.h"
#include "Gis.h"

// some projections need a lat/lon "origin", unless otherwise specified,
// they will all use this
#define DEFAULT_PHI0	 23.0		// default origin
#define DEFAULT_LAMBDA0	-96.0

// UTM origin, all zones

#define UTM_EASTING  500000.0		// false easting
#define UTM_NORTHING      0.0		// false northing

#define UTM_NORTHING_LAT_N    0.0	// latitude of origin, north half (equator to 84 30 N)
#define UTM_NORTHING_LAT_S  -80.5	// latitude of origin, south half (equator to 80 30 S)

// define ellipsoids

struct EllipsoidType	// ellipsoid descriptor
{
	GIS::Ellipsoid Id;	// Ellipsoid ID
	char Name[16];
	int  Year;
	double a;			// equatorial radius, meters
	double b;			// polar radius, meters
	double f;			// flattening
	double e;			// eccentricity (e^2 = 2f - f^2)
};

struct EllipsoidType m_Ellipsoid[] = 
{
	{GIS::Ellipsoid::Airy,       "Airy",        1830, 6377563.396,	6356256.9,		1.0 / 299.3249646,   0.0816733738741419},
	{GIS::Ellipsoid::Australian, "Australian",  1965, 6378160.0,	6356774.7,		1.0 / 298.25,        0.0818201799960599},
	{GIS::Ellipsoid::Bessel,     "Bessel",      1841, 6377397.2,    6356079.0,      1.0 / 299.15,        0.0816972146615756},
	{GIS::Ellipsoid::Clarke1866, "Clarke1866",  1866, 6378206.4,    6356583.8,      1.0 / 294.9787,      0.0822718539743484},
	{GIS::Ellipsoid::Clarke1880, "Clarke1880",  1880, 6378249.1,    6356514.9,      1.0 / 293.46,        0.0824841015220188},
	{GIS::Ellipsoid::Everest,    "Everest",     1830, 6377276.345,  6356075.4,      1.0 / 300.8017,      0.0814729809826527},
	{GIS::Ellipsoid::GRS_80,     "GRS_80",      1980, 6378137.0,	6356752.3,		1.0 / 298.257,       0.0818192214555232},
	{GIS::Ellipsoid::Intl_1924,  "Intl_1924",   1924, 6378388.0,	6356911.9,		1.0 / 297.0,		 0.0819918899790298},
	{GIS::Ellipsoid::Krasovsky,  "Krasovsky",   1940, 6378245.0,	6356863.0,		1.0 / 298.3,		 0.0818133340169312},
	{GIS::Ellipsoid::WGS_72,     "WGS_72",      1972, 6378135.0,	6356750.5,		1.0 / 298.26,		 0.0818188106627487},
	{GIS::Ellipsoid::WGS_84,     "WGS_84",      1984, 6378137.0,	6356752.3142,	1.0 / 298.257223563, 0.0818191908426215},
};

GIS::GIS()
{

}

GIS::~GIS()
{

}

void GIS::LatLonToXY_AlbersEqualAreaConic(double lat, double lon, double& xm, double& ym,
									      double stdParallel1 /* = 29.5 */, double stdParallel2 /* = 45.5*/,
										  Ellipsoid ellipsoid /* = Ellipsoid::CLARKE_1866 */)
{
	//	Albers Equal-Area Conic - formulae for the ellipsoid
	//	
	//		x = rho sin theta											[14-1]
	//		y = rho0 - rho cos theta									[14-2]
	//	
	//	where:
	//	
	//		rho = a (C-nq)^(1/2) / n									[14-12] 
	//		theta = n (lambda-lambda0)									[14-4]
	//		rho0 = a (C-nq0)^(1/2) / n									[14-12a] 
	//		C = m1^2 + nq1												[14-13]
	//		n = (m1^2-m2^2) / (q2-q1)									[14-14]
	//		m = cos(phi) / (1-e^2 sin^2(phi))^1/2						[14-15]
	//		q = (1-e^2) {sin(phi)/(1-e^2 sin^2(phi)) - [1/(2e)]			
	//				     ln[(1-e sin(phi))/(1 + e sin(phi))]}			[3-12]
	//
	//	source:
	//		Map Projections - a working manual, John Parr Snyder
	//

	const double phi = DegreeToRadian(lat);
	const double lambda = DegreeToRadian(lon);

	// get geoid properties
	const int geoID = GetEllipsoidIndex(ellipsoid);
	assert(geoID >= 0);

	const double a = m_Ellipsoid[geoID].a;
	const double e = m_Ellipsoid[geoID].e;
	const double ee = e*e;

	// convert standard parallels to radians
	const double phi1 = DegreeToRadian(stdParallel1);
	const double phi2 = DegreeToRadian(stdParallel2);

	// convert origin to radians
	const double phi0 = DegreeToRadian(DEFAULT_PHI0);
	const double lambda0 = DegreeToRadian(DEFAULT_LAMBDA0);

	const double sin_phi = sin(phi);
	const double sin_phi0 = sin(phi0);
	const double sin_phi1 = sin(phi1);
	const double sin_phi2 = sin(phi2);

	// from equation 14-15
	const double m1 = cos(phi1) / sqrt(1.0 - ee*sin_phi1*sin_phi1);
	const double m2 = cos(phi2) / sqrt(1.0 - ee*sin_phi2*sin_phi2);

	// from equation 3-12
	const double q = (1.0 - ee) *
				(
					sin_phi / (1.0 - ee*sin_phi*sin_phi) - 1.0/(2.0*e) * log((1.0 - e*sin_phi) / (1.0 + e*sin_phi))
				);
	const double q0 = (1.0 - ee) *
				(
					sin_phi0 / (1.0 - ee*sin_phi0*sin_phi0) - 1.0/(2.0*e) * log((1.0 - e*sin_phi0) / (1.0 + e*sin_phi0))
				);
	const double q1 = (1.0 - ee) *
				(
					sin_phi1 / (1.0 - ee*sin_phi1*sin_phi1) - 1.0/(2.0*e) * log((1.0 - e*sin_phi1) / (1.0 + e*sin_phi1))
				);
	const double q2 = (1.0 - ee) *
				(
					sin_phi2 / (1.0 - ee*sin_phi2*sin_phi2) - 1.0/(2.0*e) * log((1.0 - e*sin_phi2) / (1.0 + e*sin_phi2))
				);

	
	const double n = (m1*m1 - m2*m2) / (q2 - q1);		// [14-14]
	const double C = m1*m1 + n*q1;						// [14-13]
	const double rho0 = a * sqrt(C - n*q0) / n;			// [14-12a]
	const double rho = a * sqrt(C - n*q) / n;			// [14-12]
	const double theta = n * (lambda - lambda0);		// [14-4]

	xm = rho * sin(theta);					// [14-1]
	ym = rho0 - rho * cos(theta);			// [14-2]
}

void GIS::XYToLatLon_AlbersEqualAreaConic(double xm, double ym, double& lat, double& lon,
										  double stdParallel1 /* = 29.5 */, double stdParallel2 /* = 45.5*/,
										  Ellipsoid ellipsoid /* = Ellipsoid::CLARKE_1866 */)
{
	//	Albers Equal-Area Conic - formulae for the ellipsoid
	//	
	//										 /                                                     \
	//		phi = phi + (1-e^2sin^2(phi))^2 |   q         sin(phi)        1     / (1-e sin(phi)) \  |
	//					------------------- | ----- -  --------------- + -- ln |  --------------  | |      [3-16]
	//						2 cos(phi)      | 1-e^2    1-e^2sin^2(phi)   2e     \ (1+e sin(phi)) /  |
	//										 \                                                     /
	//									 
	//		lambda = lambda0 + theta / n                                                                   [14-9]
	//	
	//	where:
	//	
	//		q = (C - rho^2 n^2 / a^2) / n      	[14-19]
	//		rho = (x^2 + (rho0-y)^2)^1/2		[14-10]
	//		theta = atan[x/(rho0-y)]			[14-11]
	//
	//	source:
	//		Map Projections - a working manual, John Parr Snyder
	//

	// get geoid properties
	const int geoID = GetEllipsoidIndex(ellipsoid);
	assert(geoID >= 0);

	const double a = m_Ellipsoid[geoID].a;
	const double e = m_Ellipsoid[geoID].e;
	const double ee = e*e;
	const double e4 = ee*ee;
	const double e6 = e4*ee;

	// convert standard parallels to radians
	const double phi1 = DegreeToRadian(stdParallel1);
	const double phi2 = DegreeToRadian(stdParallel2);

	// convert origin to radians
	const double phi0 = DegreeToRadian(DEFAULT_PHI0);
	const double lambda0 = DegreeToRadian(DEFAULT_LAMBDA0);

	const double sin_phi0 = sin(phi0);
	const double sin_phi1 = sin(phi1);
	const double sin_phi2 = sin(phi2);

	// from equation 14-15
	const double m1 = cos(phi1) / sqrt(1.0 - ee*sin_phi1*sin_phi1);
	const double m2 = cos(phi2) / sqrt(1.0 - ee*sin_phi2*sin_phi2);

	// from equation 3-12
	const double q0 = (1.0 - ee) *
				(
					sin_phi0 / (1.0 - ee*sin_phi0*sin_phi0) - 1.0/(2.0*e) * log((1.0 - e*sin_phi0) / (1.0 + e*sin_phi0))
				);
	const double q1 = (1.0 - ee) *
				(
					sin_phi1 / (1.0 - ee*sin_phi1*sin_phi1) - 1.0/(2.0*e) * log((1.0 - e*sin_phi1) / (1.0 + e*sin_phi1))
				);
	const double q2 = (1.0 - ee) *
				(
					sin_phi2 / (1.0 - ee*sin_phi2*sin_phi2) - 1.0/(2.0*e) * log((1.0 - e*sin_phi2) / (1.0 + e*sin_phi2))
				);

	
	const double n = (m1*m1 - m2*m2) / (q2 - q1);					// [14-14]
	const double C = m1*m1 + n*q1;									// [14-13]
	const double rho0 = a * sqrt(C - n*q0) / n;						// [14-12a]
	const double rho = sqrt(xm*xm + (rho0 - ym)*(rho0 - ym));		// [14-10]
	const double theta = atan(xm / (rho0 - ym));					// [14-11]
	const double q = (C - (rho*n / a)*(rho*n / a)) / n;				// [14-19]

	// an iterative equation [3-16] exists for phi, but chose to use
	// equation [14-21] instead (beta is the authalic latitude):

	double beta = asin( q / (1.0-((1.0-ee)/(2.0*e)) * log(((1.0-e)/(1.0+e)))) );

	double phi = beta + (ee/3.0 + 31*e4/180.0 + 517*e6/5040.0) * sin(2.0*beta)		// [3-18]
					  + (23.0*e4/360.0 + 251.0*e6/3780.0) * sin(4.0*beta)
					  + (761.0*e6/45360) * sin(6.0*beta);

	double lambda = lambda0 + theta / n;					// [14-9]

    // return latitude & longitude
    
	lat = RadianToDegree( phi );
	lon = RadianToDegree( lambda );
}

void GIS::LatLonToXY_BipolarObliqueConicConformal(double lat, double lon, double& xm, double& ym, double R /* = 6,370,997m */)
{
	//	Bipolar Oblique Conic Conformal projection - formulae for sphere (no ellipsoidal form exists)
	//	
	//	Developed specifically for the Americas
	//	
	//		x = -x' cos Azc - y' sin Azc			[17-32]
	//		y = -y' cos Azc + x' sin Azc			[17-33]
	//
	//	where:
	//	
	//		lambdaB = -110° + arccos((cos(104°)-sin(-20°)sin(45°)) / (cos(-20°) cos(45°)))			[17-1]
	//		        = -19°59'36"
	//				= longitude of B
	//				
	//		n 		= [(ln sin(31°)-ln sin(78°)] / [(ln tag(31°/2)-ln tan(73°/2))]					[17-2]
	//				= 0.63056, the cone constant for both conic projections
	//		
	//		F0 		= R sin(31°)/(n tan^n(31°/2))													[17-3]
	//				= 1.83376R, where R = radius of globe at map scale
	//			
	//		k0 		= 2/[1+F0 tan^n(26°)/(R sin(52°))]												[17-4]
	//				= 1.03462, the scale factor by which the coordinates are multiplied
	//				  to balance the errors
	//		
	//		F		= k0 * F0																		[17-5]
	//				= 1.89725 R, a convenient constant
	//			
	//		AzAB 	= arccos {[cos(-20°)sin(45°)-sin(-20°)cos(45°)cos(lambdaB+110°)]/sin(104°)} 	[17 -6]
	//				= 46.78203°, the azimuth east of north of B from A
	//				
	//		AzBA	= arccos ([cos(45°)sin(-20°)-sin(45°)cos(-20°)cos(lambdaB+110°)]/sin(104°)]		[17-7]
	//				= 104.42834°, the azimuth west of north of A from B
	//				
	//		T		= tan^n (31°/2) + tan^n(73°/2)													[17-8]
	//				= 1.27247, a convenient constant
	//				
	//		rhoc	= l/2FT 																		[17-9]
	//				= 1.20709 R, the radius of the center point of the axis from either pole
	//				
	//		zc		= 2 arctan (T/2)^1/n 															[17-10]
	//				= 52.03888°, the polar distance of the center point from either pole
	//				
	//				
	//		phic 	= arcsin [sin ( -20°) cos zc +cos ( -20°) sin zc cos AzAB]						[17-11]
	//				= 17°16'28" N. lat., the latitude of the center point, on the
	//				southern-cone side of the axis
	//
	//		lambdac = arcsin (sin zc sin AzAB / cos phic) - 110° 									[17 -12]
	//				= -73°00'27" long., the longitude of the center point, on the
	//				southern-cone side of the axis
	//
	//		Azc 	= arcsin [cos ( -20°) sin AzAB / cos phic] 										[17 -13]
	//				= 45.81997°, the azimuth east of north of the axis at the center point,
	//				  relative to meridian A.c on the southern-cone side of the axis
	//				
	//		It must be established first whether point (phi,lambda) is north or south of the axis, 
	//		to determine which conic projection is involved. With these formulas, it is done by 
	//		comparing the azimuth of point (phi,lambda) with the azimuth of the axis, all as viewed from B.
	//
	//
	//		zB 		= arccos [sin 45° sin(phi) + cos 45° cos(phi) cos (lambdaB-lambda)]				[17-14]
	//				= polar distance of (phi,lambda) from pole B
	//			
	//		AzB 	= arctan {sin(lambdaB-lambda)/[cos 45o tan(phi)-sin 45° cos(lambdaB-lambda)]}	[17-15]
	//				= azimuth of (phi,lambda) west of north, viewed from B
	//		
	//		
	//		If AzB is greater than AzBA (from equation 17-7), go to equation 17-23.
	//		Otherwise proceed to equation 17-16 for the projection from pole B.
	//
	//		
	//		rhoB 	= F tan^n l/2 zB																[17-16]
	//				(in example rhoB = F * R * tan^n l/2 zB, but in def there is no R?)
	//
	//		k 		= rhoB n /(R sin zB )															[17-17]
	//				= scale factor at point (phi,lambda), disregarding small adjustment near axis
	//		
	//		alpha	= arccos [[tan^n 1/2 zB + tan^n l/2(104°-zB )]/T)								[17-18]
	//				If | n (AzBA - AzB) | is less than alpha
	//				
	//					rhoB' = rhoB / cos[alpha - n(AzBA-AzB)]										[17-19]
	//					
	//				If the above expression is equal to or greater than alpha,
	//					rhoB' = rhoB																[17-20]
	//
	//				Then
	//					x' = rhoB' sin [n (AzBA - AzB)]												[17-21]
	//					y' = rhoB' cos [n (AzBA - AzB)] - rhoc										[17-22]
	//					
	//			To change to nonskewed rectangular coordinates, go to equations (17-32) and (17-33). 
	//			The following formulas give coordinates for the projection from pole A:
	//			
	//				zA 	= arccos [sin(-20°) sin phi + cos(-20°) cos(phi) cos(lambda+110°)]			[17-23]
	//					= polar distance of (phi,lambda) from pole A
	//				AzA = arctan [sin (A.+ 110°)/(cos ( -20°) tan <j>-sin ( -20°) cos (A.+ 110°)])	[17-24]
	//					= azimuth of (phi,lambda) east of north, viewed from A
	//				rhoA = F tan^n 1/2 zA															[17-25]
	//				k	= rhoA n / R sin zA 														[17-26]
	//				    = scale factor at (phi,lambda)		
	//				alpha = arccos {[tan^n 1/2 zA + tan^n 1/2(104°-zA)]/T}							[17-27]
	//				
	//			if | n (AzAB - AzA) | is less than alpha,
	//			
	//				rhoA' = rhoA / cos[alpha + n (AzAB - AzA)]										[17-28]
	//				
	//			if the above expression is qual to or greater than alpha,
	//			
	//				rhoA' = rhoA																	[17-29]
	//				
	//			
	//			then
	//			
	//				x' =  rhoA' sin[n(AzAB-AzA)]													[17-30]
	//				y' = -rhoA' cos[n(AzAB-AzA)] + rhoc												[17-31]
	//				
	//				x = -x' cos Azc - y' sin Ac														[17-32]
	//				y = -y' cos Azc + x'sin Azc														[17-33]
	//
	//	source:
	//		Map Projections - a working manual, John Parr Snyder
	//

	const double phi = DegreeToRadian(lat);
	const double lambda = DegreeToRadian(lon);

	const double sin45 = sin(DegreeToRadian(45.0));
	const double cos45 = cos(DegreeToRadian(45.0));

	const double lambdaB = -0.34894950354874305;					// [17-1]
	const double n = 0.63056;										// [17-2]
	const double F = 1.89725;										// [17-5]
	const double AzAB = DegreeToRadian(46.78203);					// [17-6]
	const double AzBA = DegreeToRadian(104.42834);					// [17-7]
	const double T = 1.27247;										// [17-8]
	const double rhoc = 0.5*F*T;									// [17-9]
	const double Azc = DegreeToRadian(45.81997);					// [17-13]

	const double zB = acos(sin45*sin(phi) + cos45*cos(phi)*cos(lambdaB - lambda));						// [17-14]
	const double AzB = atan(sin(lambdaB - lambda) / (cos45*tan(phi) - sin45*cos(lambdaB - lambda)));	// [17-15]

	double xprime;
	double yprime;

	if (AzB > AzBA)
	{
		// proceed to equation [17-23]

		//TODO:
		//no test coverage this code path

		const double zA	= acos( sin(DegreeToRadian(-20))*sin(phi) + cos(DegreeToRadian(-20))*cos(phi)*cos(lambda + DegreeToRadian(110)) );	// [17-23]
		const double AzA = atan( sin(lambda + DegreeToRadian(110)) / cos(DegreeToRadian(-20))*tan(phi) - sin(DegreeToRadian(-20))*cos(lambda + DegreeToRadian(110)) );	// [17-24]
		const double rhoA = R * F * pow(tan(0.5*zA), n);	// [17-25]
		//const double k	= rhoA * n / R * sin(zA);			// [17-26]
		const double alpha = acos( (pow(tan(zA/2.0), n) + pow(tan(0.5*(DegreeToRadian(104)-zA)), n)) / T );	// [17-27]
				
		double rhoAprime;
		if (fabs(n * (AzAB - AzA)) < alpha)
		{
			rhoAprime = rhoA / cos(alpha + n * (AzAB - AzA));	// [17-28]
		}
		else
		{
			rhoAprime = rhoA;	// [17-29]
		}
			
		xprime =  rhoAprime * sin(n * (AzAB-AzA));				// [17-30]
		yprime = -rhoAprime * cos(n * (AzAB-AzA)) + rhoc;		// [17-31]
	}
	else
	{
		// proceed to equation [17-16] for projection from pole B
		
		// TODO:
		// even though the factors are the same as the worked example, i
		// get 7496093.31 for rhoB, while he gets 7496100
		const double rhoB = R * F * pow(tan(zB*0.5), n);		// [17-16]
		//const double k = rhoB * n / (R * sin(zB) );				// [17-17]
		const double alpha = acos((pow( tan(zB*0.5), n) + pow(tan(0.5*(DegreeToRadian(104)-zB)), n)) / T);		// [17-18]
				
		double rhoBprime;
		if (fabs(n * (AzBA - AzB)) < alpha)
		{
			rhoBprime = rhoB / cos(alpha - n*(AzBA-AzB));		// [17-19]
		}
		else
		{
			rhoBprime = rhoB;									// [17-20]
		}

		xprime = rhoBprime * sin(n * (AzBA - AzB));				// [17-21]
		yprime = rhoBprime * cos(n * (AzBA - AzB)) - rhoc * R;	// [17-22]
	}

	xm = -xprime * cos( Azc )- yprime * sin(Azc);		// [17-32]
	ym = -yprime * cos( Azc) + xprime * sin(Azc);		// [17-33]
}

void GIS::XYToLatLon_BipolarObliqueConicConformal( double xm, double ym, double& lat, double& lon, double R /* = 6,370,997m */ )
{
	//	Bipolar Oblique Conic Conformal projection - formulae for sphere (no ellipsoidal form exists)
	//	
	//	Developed specifically for the Americas
	//	
	//		x' = -x cos Azc + y sin Azc		[17-34]
	//		y' = -x sin Azc - y cos Azc		[17-35]
	//
	//	If x' is equal to or greater than zero, go to equation [17-36]. If x' is negative,
	//	go to equation (17-45).
	//
	//		rhoB' = [x'^2 + (rhoc+y')^2]^1/2							[17-36]
	//		AzB' = arctan [x' /(pc + y')]								[17-37]
	//	
	//		rhoB = rhoB'												[17-38]
	//		zB = 2 arctan(rhoB / F)^1/n									[17-39]
	//		alpha = arccos{[tan^n(1/2 zB) + tan^n(1/2(104°-zB))]/T}		[17-40]
	//	
	//	If |AzB'| is equal to or greater than alpha, go to equation [17-42]. If |AzB'| is less
	//	than alpha, calculate:
	//
	//		rhoB = rhoB' cos(alpha-AzB')								[17-41]
	//		
	//	and use this value to recalculate equations (17-39), (17-40), and (17-41), repeating
	//	until rhoB found in (17 -41) changes by less than a predetermined convergence.
	//	Then,		
	//	
	//		AzB = AzBA - AzB' / n										[17-42]
	//		
	//	Using AzB and the final value of zB,
	//
	//		phi 	= arcsin (sin 45° cos zB + cos 45° sin zB cos AzB )					[17-43]
	//		lambda 	= lambdaB - arctan {sin AzB/[cos 45°/tan zB - sin 45° cos AzB ]}	[17-44]
	//		
	//	The remaining equations are for the southern cone only (negative x'):
	//	
	//		rhoA' = [x'^2 + (rhoc - y')^2]^1/2								[17-45]
	//		AzA'  = arctan [x' / (rhoc - y')]								[17-46]
	//		
	//	let
	//		rhoA = rhoA'													[17-47]
	//		zA = 2 arcan(rhoA / F)^1/n										[17-48]
	//		alpha = arccos {[tan^n 1/2 zA + tan ^ n 1/2 (104-zA)]/T}		[17-49]
	//		
	//	If |AzA'| is equal to or greater than alpha, go to equation (17-51). If |AzA'| is less
	//	than alpha, calculate
	//	
	//		rhoA = rhoA' cos(alpha + AzA')									[17-50]
	//		
	//	and use this value to recalculate equations (17-48), (17-49), and (17-50), repeating
	//	until pA found in equation (17-50) changes by less than a predetermined
	//	convergence. Then,
	//
	//		AzA = AzAB - AzA' / n											[17-51]
	//		
	//	Using AzA and the final value of zA,
	//	
	//		phi 	= arcsin [sin (-20°) cos zA + cos 20° sin zA cos AzA]					[17-52]
	//		lambda 	= arctan [sin AzA/[cos(-20°)/tan zA - sin(-20°) cos AzA]}-110°			[17-53]
	//
	//	source:
	//		Map Projections - a working manual, John Parr Snyder
	//

	const double lambdaB =-0.34894950354874305;						// [17-1]
	const double n = 0.63056;										// [17-2]
	const double F = 1.89725;										// [17-5]
	const double AzAB = DegreeToRadian(46.78203);					// [17-6]
	const double AzBA = DegreeToRadian(104.42834);					// [17-7]
	const double T = 1.27247;										// [17-8]
	const double rhoc = 0.5*F*T;									// [17-9]
	const double Azc = DegreeToRadian(45.81997);					// [17-13]

	const double sin45 = sin(DegreeToRadian(45.0));
	const double cos45 = cos(DegreeToRadian(45.0));

	const double cos_azc = cos(Azc);
	const double sin_azc = sin(Azc);

	// convert x/y to skew coordinates

	const double xprime = -xm * cos_azc + ym * sin_azc;				// [17-34]
	const double yprime = -xm * sin_azc - ym * cos_azc;				// [17-35]

	// If x' is equal to or greater than zero, go to equation [17-36]. If x' is negative,
	// go to equation (17-45).

	double alpha;
	double phi, lambda;		// return values

	if (xprime >= 0.0)
	{
		const double rhoBprime = sqrt(xprime*xprime + (rhoc*R + yprime)*(rhoc*R + yprime));		// [17-36]
		const double AzBprime = atan(xprime /(rhoc*R + yprime));								// [17-37]
	
		double rhoB = rhoBprime;							// [17-38]
		double zB = 2.0 * atan(pow(rhoB / (F*R), 1.0/n));		// [17-39]
		alpha = acos( (pow(tan(0.5*zB), n) + pow(tan(0.5*(DegreeToRadian(104)-zB)), n)) / T );	// [17-40]

		// if |AzB'| is equal to or greater than alpha, go to equation [17-42]. If |AzB'| is less
		// than alpha, calculate

		if (fabs(AzBprime) < alpha)
		{
			rhoB = rhoBprime * cos(alpha-AzBprime);		// [17-41]

//TODO:
//the worked example did not look for this predetermined convergence - TBD
			// and use this value to recalculate equations (17-39), (17-40), and (17-41), repeating
			// until rhoB found in (17-41) changes by less than a predetermined convergence.
		}
		
		const double AzB = AzBA - AzBprime / n;		// [17-42]
		
		// using AzB and the final value of zB,

		phi = asin( sin45 * cos(zB) + cos45 * sin(zB) * cos(AzB) );						// [17-43]
		lambda = lambdaB - atan(sin(AzB) / (cos45/tan(zB) - sin45 * cos(AzB) ));		// [17-44]
	}
	else
	{
//TODO:
//this code path has no test coverage
		const double rhoAprime = sqrt(xprime*xprime + (rhoc*R - yprime)*(rhoc*R - yprime));			// [17-45]
		const double AzAprime  = atan(xprime / (rhoc*R - yprime));									// [17-46]
		
		double rhoA = rhoAprime;																	// [17-47]
		const double zA = 2.0 * atan( pow( (rhoA / (F*R)), 1.0/n ) );								// [17-48]
		alpha = acos( (pow(tan(0.5*zA), n) + pow(tan(0.5*(DegreeToRadian(104)-zA)), n)) / T );		// [17-49]
		
		// if |AzA'| is equal to or greater than alpha, go to equation [17-51]. If |AzA'| is less
		// than alpha, calculate
	
		if (fabs(AzAprime) < alpha)
		{
			rhoA = rhoAprime * cos(alpha + AzAprime);	// [17-50]
		}
		
		// and use this value to recalculate equations (17-48), (17-49), and (17-50), repeating
		// until pA found in equation (17-50) changes by less than a predetermined convergence.

//TODO:
//the worked example did not look for this predetermined convergence - TBD

		const double AzA = AzAB - AzAprime / n;		// [17-51]
		
		// using AzA and the final value of zA,
	
		phi 	= asin( sin(DegreeToRadian(-20)) * cos(zA) + cos(DegreeToRadian(20)) * sin(zA) * cos(AzA) );							// [17-52]
		lambda 	= atan( sin(AzA) / (cos(DegreeToRadian(-20))/tan(zA) - sin(DegreeToRadian(-20)) * cos(AzA))) - DegreeToRadian(110);		// [17-53]
	}

	lat = RadianToDegree( phi );
	lon = RadianToDegree( lambda );
}

//void GIS::LatLonToXY_Cassini( double lat, double lon, double& xm, double& ym, Ellipsoid ellipsoid )
///*
//	Cassini Projection - formulae for ellipsoid
//	
//		x = N [A-TA^3/6-(8-T+8C)TA^5/120]						[13-7]
//		y = M - M0 + N tan(phi) [A^2/2 + (5-T+6C)A^4/24]		[13-8]
//		s = 1 + x^2 cos^2 Az (1-e^2 sin^2 phi)^2/[2a^2(1-e^2)]	[13-9]
//		
//	where:
//	
//		N = a / (1 - e^2 sin^2 phi)^1/2							[4-20]
//		T = tan^2 phi											[8-13]
//		A = (lambda-lambda0) cos phi							[8-15] ( with lambda and lambda0 in radians)
//		C = e^2 cos^2 phi/(1 - e^2)								[8-14]
//		M = a [(1 - e^2/4 - 3e^4/64 - 5e^6/256-... ) phi 
//		       - (3e^2/8 + 3e^4/32 + 45e^6/1024 + ... ) sin 2 phi 
//			   + (15e^4/256 + 45e^6/1024 + ... ) sin 4 phi 
//			   - (35e^6/3072 + ... ) sin 6 phi + ... ]
//*/
//{
//	//double phi = DegreeToRadian( lat );
//	//double lambda = DegreeToRadian( lon );
//	
//	// get geoid properties
//	int geoID = GetEllipsoidIndex( ellipsoid );
//	assert(geoID >= 0);
//
//	//double a = m_Ellipsoid[geoID].a;
//	//double e = m_Ellipsoid[geoID].e;
//	
//	//double sin_phi = sin(phi);
//	//double cos_phi = cos(phi);
//	//double tan_phi = tan(phi);
//	//double e2 = e*e;
//	//double e4 = e2*e2;
//	//double e6 = e2*e4;
//	
//	//double N = a / sqrt(1.0 - e*e*sin_phi*sin_phi);
//	//double T = tan_phi*tan_phi;
//
////TODO:
//	//double A = (lambda-lambda0)*cos_phi;
//	//double C = e2 * cos_phi*cos_phi / (1.0 - e2);
//	//double M = a * (
//						  //(1.0 - e2/4.0 - 3e4/64.0 - 5e6/256.0)*phi
//						//- (3e2/8.0 + 3e4/32.0 + 45e6/1024.0)*sin(2.0*phi)
//						//+ (15e4/256.0 + 45e6/1024.0)*sin(4.0*phi)
//						//- (35e6/3072.0)*sin(6.0*phi)
//					//);
//					
//	//xm = N * (A - T*A^3.0/6.0 - (8.0 - T + 8.0*C)*T*A^5.0/120.0);
//	//ym = M - M0 + N * tan(phi) * (A^2/2.0 + (5.0 - T + 6.0*C)*A^4.0/24.0);
//}
//
//void GIS::XYToLatLon_Cassini( double xm, double ym, double& lat, double& lon, Ellipsoid ellipsoid )
///*
//TODO:
//doc all projections as ellipsoidal or spherical
//
//	Cassini Projection - formulae for ellipsoid
//	
//		phi = phi1 - (N1 tan phi1 / R1 [D^2/2 - (1 + 3T1) D^4/24]			[13-10]
//		lambda = lambda0 + [D-T1D^3/3 + (1 + 3T1) T1D^5/15] / cos phi1		[13-11]
//		
//	where:
//		phi1 = "footprint latitude", latitude at the central meridian which has same y 
//				coordinate as that of (phi, lambda)
//				
//			 = u1 + (3e1/2 - 27e^3/32+...) sin 2u1
//				  + (21e1^2/16 - 55e1^4/32 + ...) sin 4u1
//				  + (151e1^3/96 + ...) sin 6u1
//				  + (1097e1^4/512 - ...) sin 8u1 + ...		 				[3-26]
//				  
//		e1 = [1- (1-e^2)^1/2] / [1 + (1-e2^)^1/2]							[3-24]
//		u1 = Ml / [a(1-e^2/4-3e^4/64-5e^6/256-... )]						[7-19]
//		M1 = M0 + y															[13-12]
//		  
//		T1 = tan^2 phi1														[8-22]
//		N1 = a/(1-e^2 sin^2 phi1)^1/2										[8-23]
//		R1 = a (1-e^2)/(1-e^2 sin^2 phi1)^3/2								[8-24]
//		D = x/N1															[13-13]
//
//	source:
//		Map Projections - a working manual, John Parr Snyder
//*/
//{
//	// get geoid properties
//	int geoID = GetEllipsoidIndex( ellipsoid );
//	assert(geoID >= 0);
//
//	//double a = m_Ellipsoid[geoID].a;
//	//double e = m_Ellipsoid[geoID].e;
//	
//	//double e2 = e*e;
//	
//	//double e1 = (1.0 - sqrt(1.0-e2)) / (1.0 + sqrt(1.0-e2));
//	
////TODO:
//	//double phi1 = u1 + (3e1/2 - 27e^3/32) sin 2u1
//				  //+ (21e1^1/16 - 55e1^4/32) sin 4u1
//				  //+ (151e1^3/96 ) sin 6u1
//				  //+ (1097e1^4/512 ) sin 8u1;
//				  
//	//lat = phi1 - (N1 tan phi1 / R1 [D^2/2 - (1 + 3T1) D^4/24];
//	//lambda = lambda0 + [D-T1D^3/3 + (1 + 3T1) T1D^5/15] / cos phi1;
//}

void GIS::LatLonToXY_EquidistantConic(double lat, double lon, double& xm, double& ym,
									  double stdParallel1 /* = 29.5 */, double stdParallel2 /* = 45.5 */,
									  Ellipsoid ellipsoid /* = Ellipsoid::CLARKE_1866 */)
{
	//	Equidistant Conic projection - formulae for the ellipsoid
	//	
	//		x = rho sin theta							[14-1]
	//		y = rho0 - rho cos theta					[14-2]
	//		k = pn / am									[14-16]
	//		  = (G-M/a)n / m							[16-7]
	//
	//	where:
	//	
	//		rho = a G - M								[16-8]
	//		theta = n(lambda-lambda0)					[14-4]
	//		rho0 = a G - M0								[16-9]
	//		n = a(m1-m2)/(M2-M1)						[16-10]
	//		m = cos(phi) / (1-e^2 sin^2(phi))^1/2		[14-15]
	//		G = m1/n + M1/a								[16-11]
	//
	//		M = a * [(1 - e2/4 - 3e4/64 - 5e6/256 -....)*phi - (3e2/8 + 3e4/32 + 45e6/1024+....)sin(2* phi)		[3-21]
	//			     + (15e4/256 + 45e6/1024 +.....)sin(4*phi) - (35e6/3072 + ....)sin(6*phi) + .....]
	//		
	//	source:
	//		Map Projections - a working manual, John Parr Snyder
	//

	double phi = DegreeToRadian(lat);
	double lambda = DegreeToRadian(lon);

	double phi1 = DegreeToRadian(stdParallel1);
	double phi2 = DegreeToRadian(stdParallel2);

	double phi0 = DegreeToRadian(DEFAULT_PHI0);
	double lambda0 = DegreeToRadian(DEFAULT_LAMBDA0);

	// get geoid properties
	int geoID = GetEllipsoidIndex(ellipsoid);
	assert(geoID >= 0);

	double a = m_Ellipsoid[geoID].a;
	double e = m_Ellipsoid[geoID].e;
	double e2 = e*e;
	double e4 = e2*e2;
	double e6 = e4*e2;

	//double cos_phi = cos(phi);
	double cos_phi1 = cos(phi1);
	double cos_phi2 = cos(phi2);
	//double sin_phi = sin(phi);
	double sin_phi1 = sin(phi1);
	double sin_phi2 = sin(phi2);

	// from equation [14-15]
	//double m  = cos_phi  / sqrt( 1.0 - e2 * sin_phi  * sin_phi  );
	double m1 = cos_phi1 / sqrt(1.0 - e2 * sin_phi1 * sin_phi1);
	double m2 = cos_phi2 / sqrt(1.0 - e2 * sin_phi2 * sin_phi2);

	// from equation [3-21]
	double M = a * (  (1.0 - (e2)/4.0 - (3.0*e4)/64.0 - (5.0*e6)/256.0) * phi 
					- ((3.0*e2)/8.0 + (3.0*e4)/32.0 + (45.0*e6)/1024.0) * sin(2.0*phi)
					+ ((15.0*e4)/256.0 + (45.0*e6)/1024.0) * sin(4.0*phi)
					- ((35.0*e6)/3072.0) * sin(6.0*phi) );

	double M1 = a * ( (1.0 - (e2)/4.0 - (3.0*e4)/64.0 - (5.0*e6)/256.0) * phi1 
					- ((3.0*e2)/8.0 + (3.0*e4)/32.0 + (45.0*e6)/1024.0) * sin(2.0*phi1)
					+ ((15.0*e4)/256.0 + (45.0*e6)/1024.0) * sin(4.0*phi1)
					- ((35.0*e6)/3072.0) * sin(6.0*phi1) );

	double M2 = a * ( (1.0 - (e2)/4.0 - (3.0*e4)/64.0 - (5.0*e6)/256.0) * phi2 
					- ((3.0*e2)/8.0 + (3.0*e4)/32.0 + (45.0*e6)/1024.0) * sin(2.0*phi2)
					+ ((15.0*e4)/256.0 + (45.0*e6)/1024.0) * sin(4.0*phi2)
					- ((35.0*e6)/3072.0) * sin(6.0*phi2) );

	double M0 = a * ( (1.0 - (e2)/4.0 - (3.0*e4)/64.0 - (5.0*e6)/256.0) * phi0 
					- ((3.0*e2)/8.0 + (3.0*e4)/32.0 + (45.0*e6)/1024.0) * sin(2.0*phi0)
					+ ((15.0*e4)/256.0 + (45.0*e6)/1024.0) * sin(4.0*phi0)
					- ((35.0*e6)/3072.0) * sin(6.0*phi0) );

	
	double n = a * (m1 - m2) / (M2 - M1);		// [16-10]
	double G = m1/n + M1/a;						// [16-11]
	double rho0 = a*G - M0;						// [16-9]
	double rho = a*G - M;						// [16-8]
	double theta = n*(lambda-lambda0);			// [14-4]
	
	xm = rho * sin(theta);						// [14-1]
	ym = rho0 - rho * cos(theta);				// [14-2]
}

void GIS::XYToLatLon_EquidistantConic(double xm, double ym,
									  double& lat, double& lon,
									  double stdParallel1 /* = 29.5 */, double stdParallel2 /* = 45.5 */,
									  Ellipsoid ellipsoid /* = Ellipsoid::CLARKE_1866 */)
{
	//	Equidistant Conic projection - formulae for the ellipsoid
	//	
	//	given a, e, phi1, phi2, phi0, lambda0, x, and y, to
	//	find phi and lambda: n, G, and rho0 are calculated from equations [16-10], [16-11], and
	//	[16-9], respectively.
	//	
	//	then
	//	
	//		phi =  ... 		[3-26]
	//
	//	where
	//
	//		e1 		= [1 - (1-e^2)^1/2] / [1 + (1-e^2)^1/2]			[3-24]
	//		mu 		= M / [a(l-e^2/4-3e^4/64-5e^6/256 - ... )]		[7-19]
	//		M 		= a G - rho										[16-12]
	//		rho		= ± [x^2 + (rho0 -y)^1/2, taking the sign of n	[14-10]
	//		lambda 	= lambda0 + theta/n								[14-9]
	//		theta	= arctan [x/(rho0-y)]							[14-11]
	//
	//	source:
	//		Map Projections - a working manual, John Parr Snyder
	//

	double phi1 = DegreeToRadian(stdParallel1);
	double phi2 = DegreeToRadian(stdParallel2);

	double phi0 = DegreeToRadian(DEFAULT_PHI0);
	double lambda0 = DegreeToRadian(DEFAULT_LAMBDA0);

	// get geoid properties
	int geoID = GetEllipsoidIndex(ellipsoid);
	assert(geoID >= 0);

	double a = m_Ellipsoid[geoID].a;
	double e = m_Ellipsoid[geoID].e;
	double e2 = e*e;
	double e4 = e2*e2;
	double e6 = e4*e2;

	double sin_phi1 = sin(phi1);
	double sin_phi2 = sin(phi2);

	double cos_phi1 = cos(phi1);
	double cos_phi2 = cos(phi2);

	// from equation [14-15]
	double m1 = cos_phi1 / sqrt(1.0 - e2 * sin_phi1*sin_phi1);
	double m2 = cos_phi2 / sqrt(1.0 - e2 * sin_phi2*sin_phi2);

	// from equation [3-21]
	double M1 = a * ( (1.0 - e2/4.0 - (3.0*e4)/64.0 - (5.0*e6)/256.0) * phi1 
					- ((3.0*e2)/8.0 + (3.0*e4)/32.0 + (45.0*e6)/1024.0) * sin(2.0*phi1)
					+ ((15.0*e4)/256.0 + (45.0*e6)/1024.0) * sin(4.0*phi1)
					- ((35.0*e6)/3072.0) * sin(6.0*phi1) );

	double M2 = a * ( (1.0 - e2/4.0 - (3.0*e4)/64.0 - (5.0*e6)/256.0) * phi2 
					- ((3.0*e2)/8.0 + (3.0*e4)/32.0 + (45.0*e6)/1024.0) * sin(2.0*phi2)
					+ ((15.0*e4)/256.0 + (45.0*e6)/1024.0) * sin(4.0*phi2)
					- ((35.0*e6)/3072.0) * sin(6.0*phi2) );

	double M0 = a * ( (1.0 - e2/4.0 - (3.0*e4)/64.0 - (5.0*e6)/256.0) * phi0 
					- ((3.0*e2)/8.0 + (3.0*e4)/32.0 + (45.0*e6)/1024.0) * sin(2.0*phi0)
					+ ((15.0*e4)/256.0 + (45.0*e6)/1024.0) * sin(4.0*phi0)
					- ((35.0*e6)/3072.0) * sin(6.0*phi0) );

	
	double n = a * (m1 - m2) / (M2 - M1);		// [16-10]
	double G = m1/n + M1/a;						// [16-11]
	double rho0 = a*G - M0;						// [16-9]

	double rho = sqrt(xm*xm + (rho0 - ym)*(rho0 - ym));		// [14-10]
	// rho takes the sign of n
	if (n < 0.0)
		rho = -rho;

	double theta = atan(xm/(rho0-ym));			// [14-11]
	double M = a*G - rho;						// [16-12]
	double mu = M / (a*(1.0 - e2/4.0 - 3.0*e4/64.0 - 5.0*e6/256.0));		// [7-19]
	
	double sqrt_1e2 = sqrt(1.0 - e2);
	double e1 = (1.0 - sqrt_1e2) / (1.0 + sqrt_1e2);	// [3-24]

	// note: using form of pow() with int for arg2 to hopefully tell it to
	//		 simply multiply arg1 exp times
	double phi = mu + (3.0*e1 / 2.0 - 27.0*pow(e1, 3) / 32.0) * sin(2.0*mu)					// [3-26]
		+ (21.0*pow(e1, 2) / 16.0 - 55.0*pow(e1, 4) / 32.0) * sin(4.0*mu)
		+ (151.0*pow(e1, 3) / 96.0) * sin(6.0*mu)
		+ (1097.0*pow(e1, 4) / 512.0) * sin(8.0*mu);

	
	double lambda = lambda0 + theta / n;			// [14-9]

	// return latitude & longitude
	lat = RadianToDegree(phi);
	lon = RadianToDegree(lambda);
}

//void GIS::LatLonToXY_PolyConic(double lat, double lon, double& xm, double& ym, Ellipsoid ellipsoid)
//{
//	//	Polyconic projection - formulae for the ellipsoid
//	//	
//	//	if phi == 0
//	//	
//	//		x = a(lambda-lambda0)				[7-6]
//	//		y = -M0								[18-11]
//	//		
//	//	if phi != 0
//	//	
//	//		E = (lambda-lambda0) sin(phi)		[18-2]
//	//		x = N cot(phi) sin(E)				[18-12]
//	//		y = M - M0 + N cot(phi) (1-cos(E))	[18-13]
//	//		
//	//	where
//	//	
//	//		M = ... 	 						[3-21]
//	//		N = a / (1-e^2 sin^2(phi))^1/2		[4-20]
//	//
//	//	source:
//	//		Map Projections - a working manual, John Parr Snyder
//	//
//}
//
//void GIS::XYToLatLon_PolyConic(double xm, double ym, double& lat, double& lon, Ellipsoid ellipsoid)
//{
//	//	Polyconic projection - formulae for the ellipsoid
//	//	
//	//	inverse ellipsoidal formulas are given in a Newton-Raphson form, converging to 
//	//	any desired degree of accuracy after several iterations. As before, if |lambda-lambda0| > 90°, 
//	//	this iteration does not converge, but the projection should not be used in that range 
//	//	in any case. The formulas may be calculated in the following order, given a, e, phi0, lambda0, 
//	//	x, and y. First M0 is calculated from equation [3-21], as in the forward case, with phi0
//	//	for phi and M0 for M.
//	//
//	//	if y = -M0, the iteration is not applicable, but
//	//
//	//		phi = 0
//	//		lambda = x/a + lambda0		[7-12]
//	//
//	//	if y != -M0, the calculation is as follows:
//	//
//	//		A = (M0 +y)/a
//	//		B = x^2/a^2 + A^2
//	//
//	//	using an initial value of phin = A, the following calculations are made:
//	//
//	//		C = (1-e^2 sin^2(phin))^1/2 tan(phin)		[18-20]
//	//		
//	//	Then Mn and Mn' are found from equations [3-21] and [18-17], using
//	//	phin for phin, Mn for M, and Mn' for M'. Let Ma = Mn/a.
//	//	
//	//		phin+1 = phin - [A(CMa+1)-Ma-1/2(Ma^2+B)C]/
//	//						[e^2 sin(2phin)(Ma^2 +B-2AMa)/4C+(A-Ma)(CMn'-2/sin(2phin))-Mn'] [18-21]
//	//		
//	//	Each value of phin+1 (phi with a subscript of "n+1") is substituted in place of phin, 
//	//	and C, Mn, Mn', and phin+1 are recalculated from equations [18-20], [3-21], [18-17], 
//	//	and [18-21], respectively.
//	//	
//	//	This process is repeated until phin+1 varies from phin by less than a predetermined
//	//	convergence value. Then phi equals the final phin+1.
//	//	
//	//		lambda = [arcsin (xC/a)] / sin(phi) + lambda0		[18-22]
//	//
//	//	using the C calculated for the last phin from equation [18-20]. If phi = ±90°, 
//	//	lambda is indeterminate, but may be given any value.
//	//
//	//	source:
//	//		Map Projections - a working manual, John Parr Snyder
//
//}

void GIS::LatLonToXY_LambertCC(	double lat, double lon,
								double& xm, double& ym,
								double stdParallel1 /* = 33.0 */, double stdParallel2 /* = 45.0 */,
								Ellipsoid ellipsoid /* = Ellipsoid::CLARKE_1866 */)
{
	//	project latitude/longitude to planar coordinates using Lambert Conic Conformal projection
	//
	//	formulae for the ellipsoid
	//
	//	inputs:
	//		lat, lon = position expressed in fractional degrees
	//		stdParallel1/2 = standard parallels, degrees
	//		ellipsoid = ellipsoid to use for projection
	//
	//	outputs:
	//		xm, ym = plane coordinates, meters
	//
	//	notes:
	//
	//		x = (rho)(sin(phi))						[14-1]
	//		y = rho0 - (rho)(cos(phi))				[14-2]
	//
	//	where:
	//
	//		rho		= a F t^n																			[15-7]
	//		rho0	= a F t0^n																			[15-7a]
	//		theta	= n(lambda-lamda0)																	[14-4]
	//		n		= (ln m1 - ln m2) / (ln t1 - ln t2)													[15-8]
	//		m		= cos(phi) / sqrt(1 - e^2 sin(phi)^2)												[14-15]
	//		t		= tan(PI*0.25-phi*0.5) / pow((1.0 - e_sin_phi) / (1.0 + e_sin_phi), e*0.5)			[15-9]
	//			-or-
	//		t		= [(1-sin(theta))/(1+sin(theta)) / ((1-e*sin(theta))/(1+e*sin(theta)))^e]^1/2		[15-9a]
	//		F		= m1 / (n * pow(t1, n));															[15-10]
	//
	//	source:
	//		Map Projections - a working manual, John Parr Snyder
	//

    double lambda;		// input longitude
    double phi;			// input latitude

    double phi1;		// std parallel 1
    double phi2;		// std parallel 2
    double lambdaF;		// false origin - longitude
    double phiF;		// false origin - latitude

    double e;			// eccentricity
    double ee;			// e squared
    double a;			// equatorial radius, meters
    double tF; 
    double t1; 
    double t2; 
    double F; 
    double rho0; 
    double n; 
    double t; 
    double rho; 
    double m1; 
    double m2; 
    double theta; 
    
	// convert input lat/lon to radians

	phi = DegreeToRadian( lat );
	lambda = DegreeToRadian( lon );

	// get geoid properties
	int geoID = GetEllipsoidIndex( ellipsoid );
	assert(geoID >= 0);

	a = m_Ellipsoid[geoID].a;
	e = m_Ellipsoid[geoID].e;
	ee = e*e;

	// convert standard parallels to radians
	phi1 = DegreeToRadian( stdParallel1 );
	phi2 = DegreeToRadian( stdParallel2 );

	// convert lat/long of false origin to radians
	phiF = DegreeToRadian( DEFAULT_PHI0 );
	lambdaF = DegreeToRadian( DEFAULT_LAMBDA0 );

	// calc m values
	m1 = cos(phi1) / sqrt(1.0 - ee * (sin(phi1)*sin(phi1)));
	m2 = cos(phi2) / sqrt(1.0 - ee * (sin(phi2)*sin(phi2)));

	// calc t values
	tF = tan(PI/4.0-phiF/2.0) / pow((1.0 - e*sin(phiF)) / (1.0 + e*sin(phiF)), e*0.5);
	t1 = tan(PI/4.0-phi1/2.0) / pow((1.0 - e*sin(phi1)) / (1.0 + e*sin(phi1)), e*0.5);
	t2 = tan(PI/4.0-phi2/2.0) / pow((1.0 - e*sin(phi2)) / (1.0 + e*sin(phi2)), e*0.5);

	n = (log(m1) - log(m2)) / (log(t1) - log(t2));

	F = m1 / (n * pow(t1, n));

	if (tF < 0.000001)
		rho0 = 0.0;
	else
		rho0 = (a * F) * pow(tF, n);
	
    // calc variable t value
	double e_sin_phi = e * sin(phi);
    t = tan(PI*0.25 - phi*0.5) / pow((1.0 - e_sin_phi) / (1.0 + e_sin_phi), e*0.5);
 
    rho = (a * F) * pow(t, n);
    
    theta = n * (lambda - lambdaF);
    
    xm = rho * sin(theta);
    ym = rho0 - rho * cos(theta);
}

void GIS::XYToLatLon_LambertCC(	double xm, double ym, double& lat, double& lon,
								double stdParallel1 /* = 33.0 */, double stdParallel2 /* = 45.0 */,
								Ellipsoid ellipsoid /* = Ellipsoid::CLARKE_1866 */)
{
	//	project plane coordinates to latitude/longitude, Lambert Conic Conformal projection
	//	 
	//	formulae for the ellipsoid
	//
	//	inputs:
	//		xm, ym = position, meters
	//		stdParallel1/2 = standard parallels, degrees
	//		ellipsoid = ellipsoid to use for projection
	//
	//	outputs:
	//		lat, lon = latitude & longitude in fractional degrees
	//
	//	notes:
	//
	//		lambda	= theta/n + lambda0												[14-9]
	//		phi		= PI/2 - 2 atan|t[(1 - e sin(phi))/(1 + e sin(phi))]^e/2|		[7-9] - which features phi in terms of phi!
	//
	//	source:
	//		Map Projections - a working manual, John Parr Snyder
	//

    double lambda;		// output longitude
    double phi;			// output latitude

    double phi1;		// std parallel 1
    double phi2;		// std parallel 2
    double lambdaF;		// false origin - longitude
    double phiF;		// false origin - latitude

    double m1;
    double m2;
    double e;			// eccentricity
    double ee;			// e squared
    double tF; 
    double t1;
    double t2;
    double a;			// equatorial radius, meters
    double n;
    double n_inv;		// 1/n
    double F;
    double rho0;
    double t;
    double rho;
    double theta;

	// get geoid properties
	int geoID = GetEllipsoidIndex(ellipsoid);
	assert(geoID >= 0);

	a = m_Ellipsoid[geoID].a;
	e = m_Ellipsoid[geoID].e;
	ee = e*e;

	// convert standard parallels to radians
	phi1 = DegreeToRadian(stdParallel1);
	phi2 = DegreeToRadian(stdParallel2);

	double sinPhi1 = sin(phi1);
	double sinPhi2 = sin(phi2);

	// convert lat/long of false origin to radians
	phiF = DegreeToRadian(DEFAULT_PHI0);
	lambdaF = DegreeToRadian(DEFAULT_LAMBDA0);

	// calc t values
	tF = tan(PI*0.25 - phiF * 0.5) / pow((1.0 - e*sin(phiF)) / (1.0 + e*sin(phiF)), e*0.5);
	t1 = tan(PI*0.25 - phi1 * 0.5) / pow((1.0 - e*sinPhi1) / (1.0 + e*sinPhi1), e*0.5);
	t2 = tan(PI*0.25 - phi2 * 0.5) / pow((1.0 - e*sinPhi2) / (1.0 + e*sinPhi2), e*0.5);

	// calc m values
	m1 = cos(phi1) / pow(1.0 - ee * (sinPhi1 * sinPhi1), 0.5);
	m2 = cos(phi2) / pow(1.0 - ee * (sinPhi2 * sinPhi2), 0.5);

	n = (log(m1) - log(m2)) / (log(t1) - log(t2));
	n_inv = 1.0 / n;

	F = m1 / (n * pow(t1, n));

	if (tF < 0.000001)
		rho0 = 0.0;
	else
		rho0 = (a * F) * pow(tF, n);
	
    theta = atan(xm / (rho0 - ym));
    
    rho = sqrt(xm*xm + (rho0 - ym)*(rho0 - ym));
    
	if (n < 0.0)	// rho takes sign of n
		rho = -rho;
    
	t = pow(rho / (a * F), n_inv);

	// note:
	// eqn 3-5 [parr] may be used instead of iteration, but not sure it's any more accurate
	phi = CalcPhi(e, t);
	lambda = theta * n_inv + lambdaF;

	// return latitude & longitude

	lat = RadianToDegree(phi);
	lon = RadianToDegree(lambda);
}

double GIS::CalcPhi(double e, double t)
{
	//	Calculate value of Phi by numerical approximation
	//
	//	there are multiple situations where phi is calculated by approximation - not all use
	//	the same formula; this one is from [7-9]
	//

	double	phiIn;
	double	phi;
	int		nLastSign;
	double	fReturn = 0.0;

	// phi is latitude so must be between -1.57079633 and 1.57079633

	static double last_e;
	static double last_t;
	static double last_Phi;

	if (last_e != e || last_t != t)		// only calc if input parms change
	{
		double delta = 0.1;

		phiIn = -PI*0.5;
		phi = (PI*0.5) - 2.0*atan(t*(pow((1.0 - e*sin(phiIn)) / (1.0 + e*sin(phiIn)), e*0.5)));

		if (phi < phiIn)
			nLastSign = -1;
		else
			nLastSign = 1;
		while (phiIn < 1.57079633)
		{
			phi = (PI*0.5) - 2.0*atan(t*(pow((1.0 - e*sin(phiIn)) / (1.0 + e*sin(phiIn)), e*0.5)));
			if (nLastSign != sign(phi - phiIn))    // overshot
				delta = -(delta * 0.5);
			if (fabs(phi - phiIn) < 0.000001)
			{
				// all done
				fReturn = phiIn;
				break;
			}
			nLastSign = sign(phi - phiIn);
			phiIn += delta;
		}
		// update the static variables
		last_e = e;
		last_t = t;
		last_Phi = fReturn;
	}
	else
	{
		fReturn = last_Phi;
	}

	return fReturn;
}

void GIS::LatLonToXY_Mercator(double lat, double lon, double& xm, double& ym, Ellipsoid ellipsoid)
{
	//	Mercator Projection - formulae for the ellipsoid
	//
	//	notes:
	//
	//		x = a (lambda - lambda0)																		[7-6]
	//
	//		y = a ln [ tan(PI/4 + phi/2) ( (1-e sin(phi))/(1 + e sin(phi)))^e/2 ]							[7-7]
	//
	//		- OR -
	//
	//		y = (a/2) ln [ ( (1 + sin(phi))/(1 - sin(phi)) ) ( (1-e sin(phi))/(1 + e sin(phi)) )^e ]		[7-7a]
	//
	//	source:
	//		Map Projections - a working manual, John Parr Snyder
	//

	double phi = DegreeToRadian(lat);
	double lambda = DegreeToRadian(lon);

	double lambda0 = DegreeToRadian(-180.0);		// central meridian

	// get geoid properties

	int geoID = GetEllipsoidIndex(ellipsoid);
	assert(geoID >= 0);

	double a = m_Ellipsoid[geoID].a;		// equatorial radius of ellipsoid
	double e = m_Ellipsoid[geoID].e;		// eccentricity of ellipsoid

	xm = a * (lambda - lambda0);
	ym = a * log(tan(PI*0.25 + phi*0.5) * pow((1.0 - e*sin(phi)) / (1.0 + e*sin(phi)), e*0.5));
}

void GIS::XYToLatLon_Mercator(double xm, double ym, double& lat, double& lon, Ellipsoid ellipsoid)
{
	//	Mercator Projection - formulae for the ellipsoid
	//
	//	notes:
	//
	//		lat = phi = PI/2 - 2 atan| t[(1 - e sin(phi))/(1 + e sin(phi))]^e/2 |		[7-9] - which features phi in terms of phi!
	//		lon = lambda = x/a + lambda0												[7-12]
	//
	//	where:
	//
	//		t = ebase ^ (-y/a)			[7-10]
	//		ebase = base of natural logarithms
	//
	//	source:
	//		Map Projections - a working manual, John Parr Snyder
	//

	// get geoid properties

	int geoID = GetEllipsoidIndex(ellipsoid);
	assert(geoID >= 0);

	double a = m_Ellipsoid[geoID].a;		// equatorial radius of ellipsoid
	double e = m_Ellipsoid[geoID].e;		// eccentricity of ellipsoid

	double lambda0 = DegreeToRadian(-180.0);		// central meridian

	//double exp = -ym/a;
	double t = pow(E_LN, -ym / a);

	double phi = CalcPhi(e, t);
	double lambda = xm / a + lambda0;

	lat = RadianToDegree(phi);
	lon = RadianToDegree(lambda);
}

void GIS::LatLonToXY_UTM(double lat, double lon, double& xm, double& ym, Ellipsoid ellipsoid /* = CLARKE_1866 */)
{
	//	Universal Transverse Mercator projection - formulae for the ellipsoid
	//
	//	inputs:
	//		lat/lon = point to project, fractional degrees
	//
	//	outputs:
	//		xm/ym = projected point, meters
	//
	//	notes:
	//	
	//		each "zone" represents a different projection, that's why zone # is required parameter - to explicitly
	//		specify which projection is being used (of course the projection from geo coordinates could derive these 
	//	
	//		E = FE + k0 * v * (A + (1-T+C)*(A^3/6) + (5 - 18T + T^2 + 72C - 58*e_prime_2)*(A^5/120))
	//		N = FN + k0 * {M - M0 + v*tan(phi)*[A2/2 + (5 - T + 9C + 4C^2)*(A^4/24) + (61 - 58T + T^2 + 600C - 330e'2)*(A^6/720)]} 
	//
	//	where:
	//
	//		phi = input latitude
	//		lambda = input longitude
	//
	//		phi0 = latitude of natural origin
	//		lambda0 = longitude of natural origin
	//
	//		T = tan(phi) ^2
	//		C = (e_2/(1.0-e_2)) * (cos(phi)*cos(phi));
	//		e' = e^2 / (1-e^2)
	//
	//		k0 = 0.9996		// scale factor
	//
	//		v =             a					= radius of curvature at latitude f perpendicular to a meridian 
	//			---------------------------
	//			sqrt(1 - e^2 * sin(phi)^2 )
	//
	//		A = (lambda-lambda0) * cos(phi)		// lambda in radians
	//
	//		M = a * [(1 - e2/4 - 3e4/64 - 5e6/256 -....)*phi - (3e2/8 + 3e4/32 + 45e6/1024+....)sin(2* phi)		[3-21]
	//			     + (15e4/256 + 45e6/1024 +.....)sin(4*phi) - (35e6/3072 + ....)sin(6*phi) + .....]
	//
	//	source:
	//		Map Projections - a working manual, John Parr Snyder
	//

	const double k0 = 0.9996;		// scale factor

	int nZone = GetUTMZone((float)lon);

	double phi = DegreeToRadian(lat);
	double lambda = DegreeToRadian(lon);

	// get zone origin, radians

	// lat of false northing
	double phi0 = lat < 0.0 ? UTM_NORTHING_LAT_S : UTM_NORTHING_LAT_N;
	phi0 = DegreeToRadian(phi0);
	// meridian of specified zone
	double lambda0 = GetUTMLongitude(nZone) + 3.0;
	lambda0 = DegreeToRadian(lambda0);

	// get geoid properties

	double a = 0.0;
	double e = 0.0;

	int geoID = GetEllipsoidIndex(ellipsoid);
	assert(geoID >= 0);

	a = m_Ellipsoid[geoID].a;
	e = m_Ellipsoid[geoID].e;

	double e_2 = e*e;
	double e_4 = e*e*e*e;
	double e_6 = e*e*e*e*e*e;

	double T = tan(phi) * tan(phi);								// OK
	double C = (e_2 / (1.0 - e_2)) * (cos(phi)*cos(phi));		// OK
	double A = (lambda - lambda0)* cos(phi);					// OK

	double A_2 = A*A;
	double A_3 = A_2*A;
	double A_4 = A_3*A;
	double A_5 = A_4*A;
	double A_6 = A_5*A;

	double sinPhi = sin(phi);

	// from equation [3-21]
	double M = a * (  (1.0 - (e_2)/4.0 - (3.0*e_4)/64.0 - (5.0*e_6)/256.0) * phi 
					- ((3.0*e_2)/8.0 + (3.0*e_4)/32.0 + (45.0*e_6)/1024.0) * sin(2.0*phi)
					+ ((15.0*e_4)/256.0 + (45.0*e_6)/1024.0) * sin(4.0*phi)
					- ((35.0*e_6)/3072.0) * sin(6.0*phi) );

	double M0 = a * (  (1.0 - (e_2)/4.0 - (3.0*e_4)/64.0 - (5.0*e_6)/256.0) * phi0 
					 - ((3.0*e_2)/8.0 + (3.0*e_4)/32.0 + (45.0*e_6)/1024.0) * sin(2.0*phi0)
					 + ((15.0*e_4)/256.0 + (45.0*e_6)/1024.0) * sin(4.0*phi0)
					 - ((35.0*e_6)/3072.0) * sin(6.0*phi0) );


	double e_prime_2 = e_2 / (1.0-e_2);

	double v = a / sqrt(1.0 - e_2 * (sinPhi*sinPhi)); 

	double E = k0 * v *
				(
					A + (1.0-T+C) * (A_3/6.0) + (5.0 - 18.0*T + T*T + 72.0*C - 58.0*e_prime_2) * (A_5/120.0)
				);
			
	double N = k0 * 
			(
				M - M0 + v*tan(phi) * 
					(
						  A_2/2.0 
						+ (5.0 - T + 9.0*C + 4.0*(C*C)) * (A_4/24.0) 
						+ (61.0 - 58.0*T + T*T + 600.0*C - 330.0*e_prime_2) * (A_6/720.0) 
					)
			);

	xm = E + UTM_EASTING;
	ym = N + UTM_NORTHING;
}

void GIS::XYToLatLon_UTM(int zone, Hemi hemi, double xm, double ym, double& lat, double& lon, Ellipsoid ellipsoid /* = CLARKE_1866 */)
{
	//	Universal Transverse Mercator projection - formulae for the ellipsoid
	//
	//	inputs:
	//		zone = UTM zone #
	//		hemi = northern/southern hemisphere
	//		xm / ym = x/z coordinates, meters
	//
	//	source:
	//		Map Projections - a working manual, John Parr Snyder
	//

	const double k0 = 0.9996;		// scale factor

	// get geoid properties

	int geoID = GetEllipsoidIndex(ellipsoid);
	assert(geoID >= 0);

	double a = m_Ellipsoid[geoID].a;
	double e = m_Ellipsoid[geoID].e;

	double e_2 = e*e;
	double e_4 = e*e*e*e;
	double e_6 = e*e*e*e*e*e;

	// get zone origin, radians

	// lat of false northing
	double phi0 = hemi == Hemi::S ? UTM_NORTHING_LAT_S : UTM_NORTHING_LAT_N;
	phi0 = DegreeToRadian(phi0);
	// meridian of specified zone
	double lambda0 = GetUTMLongitude(zone) + 3.0;
	lambda0 = DegreeToRadian(lambda0);

	double M0 = a * (  (1.0 - (e_2)/4.0 - (3.0*e_4)/64.0 - (5.0*e_6)/256.0) * phi0 
				 	 - ((3.0*e_2)/8.0 + (3.0*e_4)/32.0 + (45.0*e_6)/1024.0) * sin(2.0*phi0)
				 	 + ((15.0*e_4)/256.0 + (45.0*e_6)/1024.0) * sin(4.0*phi0)
					 - ((35.0*e_6)/3072.0) * sin(6.0*phi0) );

	double e1 = ( 1.0 - sqrt(1.0-e_2) ) / ( 1.0 + sqrt(1.0-e_2) );				// OK
	double M1 = M0 + (ym-UTM_NORTHING) / k0;									// OK
	double u1 = M1 / ( a * (1.0 - e_2/4.0 - (3*e_4)/64.0 - (5*e_6)/256.0) );	// OK

	double e1_2 = e1 * e1;
	double e1_3 = e1_2 * e1;
	double e1_4 = e1_3 * e1;

	double phi1 = u1 + ( (3*e1)/2.0 - (27.0*e1_3)/32.0		 ) * sin(2.0*u1)
					 + ( (21.0*e1_2)/16.0 - (55.0*e1_4)/32.0 ) * sin(4.0*u1)
					 + ( (151.0*e1_3)/96.0					 ) * sin(6.0*u1)
					 + ( (1097.0*e1_4)/512.0				 ) * sin(8.0*u1);

	double sin_phi1_2 = sin(phi1)*sin(phi1);
	double cos_phi1 = cos(phi1);

	double v1 = a / sqrt(1.0 - e_2 * sin_phi1_2);

	double rho1 = (a * (1.0 - e_2)) / pow(1.0 - e_2*sin_phi1_2, 1.5);

	double e_prime_2 = e_2 / (1.0-e_2);

	double D = (xm - UTM_EASTING) / (v1 * k0);
	double T1 = tan(phi1)*tan(phi1);
	double C1 = e_prime_2 * (cos_phi1*cos_phi1);

	double D_2 = D*D;		// pre-calculate D^n terms through n = 6
	double D_3 = D_2*D;
	double D_4 = D_3*D;
	double D_5 = D_4*D;
	double D_6 = D_5*D;

	double C1_2 = C1*C1;
	double T1_2 = T1*T1;

	double phi = phi1 - ((v1*tan(phi1))/rho1) *
						( 
							D_2/2.0 - (5.0 + 3.0*T1 + 10.0*C1 - 4.0*C1_2 - 9.0*e_prime_2) * (D_4/24.0)
									+ (61.0 + 90.0*T1 + 298.0*C1 + 45.0*T1_2 - 252.0*e_prime_2 - 3.0*C1_2) * (D_6/720.0)
						);

	double lambda = lambda0 + 
					(
						D - (1.0 + 2.0*T1 + C1) * (D_3/6.0)
						  + (5.0 - 2.0*C1 + 28.0*T1 - 3.0*C1_2 + 8.0*e_prime_2 + 24.0*T1_2)*(D_5/120.0)

					) / cos_phi1;

	lat = RadianToDegree( phi );
	lon = RadianToDegree( lambda );
}

void GIS::XYToLatLon_UTM(int zone, char hemi, double xm, double ym, double& lat, double& lon, Ellipsoid ellipsoid /* = CLARKE_1866 */)
{
	// Convenience overload that take char hemisphere.
	//

	GIS::Hemi gis_hemi = hemi == 'N' ? GIS::Hemi::N : GIS::Hemi::S;
	XYToLatLon_UTM(zone, gis_hemi, xm, ym, lat, lon, ellipsoid);
}

int GIS::GetUTMZone(double lon)
{
	//	return the UTM zone for the specified longitude
	//

	if (lon >= 180.0)
		return 60;
	else if (lon <= -180.0)
		return 1;

	int nReturn = (int)((lon + 180.0) / 6.0 + 1.0);

	return nReturn;
}

double GIS::GetUTMLongitude(int utmZone)
{
	//	return the longitude of the specified UTM zone
	//
	//	outputs:
	//		return = left longitude; meridian and right are simply +3 and +6 degrees
	//

	return (double)((utmZone - 1) * 6 - 180);
}

double GIS::DMSToDD(int deg, int min, double sec, Hemi hemi)
{
	//	convert degrees/minutes/seconds to decimal degrees
	//

	double degrees = (double)deg + ((double)min) / 60.0 + (sec / 3600.0);
	if (hemi == Hemi::S || hemi == Hemi::W)
		degrees = -degrees;

	return degrees;
}

void GIS::DDToDMS(double degrees, int& deg, int& min, double& sec, Hemi& hemi)
{
	//	convert decimal degrees to degrees/minutes/seconds
	//
	//	inputs:
	//		degrees = signed fractional degree value
	//		hemi = 'N' for latitudes, 'E' for longitudes
	//

	if (degrees < 0.0)
	{
		if (hemi == Hemi::N)
			hemi = Hemi::S;
		else
			hemi = Hemi::W;
		degrees = -degrees;	// flip sign
	}

	deg = (int)floor(degrees);
	degrees -= (double)deg;
	min = (int)(degrees * 60.0);
	sec = degrees - (double)min / 60.0;
	sec *= 3600.0;
}

char* GIS::ProjectionStr(GIS::Projection proj)
{
	switch (proj) {
	case Projection::None: return "---";
	case Projection::AlbersEqualAreaConic: return "Albers Equal Area Conic";
	case Projection::BipolarObliqueCC: return "Bipolar Oblique Conformal Conic";
	case Projection::EquidistantConic: return "Equidistant Conic";
	case Projection::Geographic: return "Geographic";
	case Projection::LambertCC: return "Lambert Conformal Conic";
	case Projection::Mercator: return "Mercator";
	case Projection::UTM: return "UTM";
	}

	assert(false);

	return "---";
}

GIS::GEODATA GIS::GetFileType(const char* fileName)
{
	// Return file type based on extension.
	//

	GIS::GEODATA fileType = GIS::GEODATA::None;

	XString str = fileName;

	// raster types
	if (str.EndsWithNoCase(".DEM"))
	{
		if (str.EndsWithNoCase("_10M.DEM"))
			fileType = GEODATA::DEM10;
		else if (str.EndsWithNoCase("_30M.DEM"))
			fileType = GEODATA::DEM30;
	}
	else if (str.EndsWithNoCase(".IMG"))
	{
		// .img files may be pds, erdas imagine,
		// or even disk image files
		FILE* pFile = fopen(fileName, "rb");
		if (pFile)
		{
			char buf[16] = { 0 };
			fread(buf, 1, sizeof(buf), pFile);
			fclose(pFile);

			if (strncmp(buf, "PDS_VERSION_ID", sizeof("PDS_VERSION_ID") - 1) == 0)
				fileType = GIS::GEODATA::PDS;
			else if (strncmp(buf, "EHFA_HEADER_TAG", sizeof("EHFA_HEADER_TAG") - 1) == 0)
				fileType = GIS::GEODATA::HFA;
		}
	}
	else if (str.EndsWithNoCase(".DT0"))
		fileType = GEODATA::DTED0;
	else if (str.EndsWithNoCase(".DT1"))
		fileType = GEODATA::DTED1;
	else if (str.EndsWithNoCase(".DT2"))
		fileType = GEODATA::DTED2;
	else if (str.EndsWithNoCase(".E00"))
		fileType = GEODATA::E00GRID;
	else if (str.EndsWithNoCase(".DAT"))
		fileType = GEODATA::TerrainFile;
	else if (str.EndsWithNoCase(".TIF") || str.EndsWithNoCase(".TIFF"))		// may be geotiff, but can't determine from ext
		fileType = GEODATA::TIFF;
	else if (str.EndsWithNoCase(".bt"))
		fileType = GEODATA::VtpBT;
	else if (str.EndsWithNoCase(".xyz"))
		fileType = GEODATA::XYZ;

	// vector types
	else if (str.EndsWithNoCase(".SHP"))
		fileType = GEODATA::ShapeFile;

	// point cloud types
	else if (str.EndsWithNoCase(".LAS"))
		fileType = GEODATA::LAS;
	else if (str.EndsWithNoCase(".LAZ"))
		fileType = GEODATA::LAZ;

	return fileType;
}

char* GIS::GeodataToString(GIS::GEODATA geoType)
{
	switch (geoType){

	// raster
	case GIS::GEODATA::DEM10:		return "10m DEM";
	case GIS::GEODATA::DEM30:		return "30m DEM";
	case GIS::GEODATA::DTED0:		return "DTED Level 0";
	case GIS::GEODATA::DTED1:		return "DTED Level 1";
	case GIS::GEODATA::DTED2:		return "DTED Level 2";
	case GIS::GEODATA::SDTS_DEM:	return "USGS DEM file, SDTS Format";
	case GIS::GEODATA::E00GRID:		return "ESRI E00 Grid";
	case GIS::GEODATA::NITF:		return "NITF";
	case GIS::GEODATA::TIFF:		return "TIFF";
	case GIS::GEODATA::GeoTIFF:		return "GeoTIFF";
	case GIS::GEODATA::VtpBT:		return "Virtual Terrain Project BT File";
	case GIS::GEODATA::XYZ:			return "ASCII Gridded XYZ File";
	case GIS::GEODATA::TerrainFile:	return "AeroMap Terrain Model";

	// vector
	case GIS::GEODATA::ShapeFile:	return "ESRI Shapefile";
	case GIS::GEODATA::ArcGen:		return "Arc/Info Vector File";
	case GIS::GEODATA::BNA:			return "BNA";
	}

	return "---";
}

int GIS::GetEllipsoidIndex(const char* ellipsoid)
{
	// Given name, return ellipsoid index.
	//

	int nReturn = -1;
	for (unsigned int i = 0; i < sizeof(m_Ellipsoid) / sizeof(EllipsoidType); ++i)
	{
		if (strcmp(m_Ellipsoid[i].Name, ellipsoid) == 0)
		{
			nReturn = i;
			break;
		}
	}

	return nReturn;
}

int GIS::GetEllipsoidIndex(Ellipsoid ellipsoid)
{
	// Given id, return ellipsoid index.
	//

	int nReturn = -1;
	for (unsigned int i = 0; i < sizeof(m_Ellipsoid) / sizeof(EllipsoidType); ++i)
	{
		if (m_Ellipsoid[i].Id == ellipsoid)
		{
			nReturn = i;
			break;
		}
	}

	return nReturn;
}

char* GIS::GetEllipsoidName(unsigned int ellipseIndex)
{
	// Given index, return ellipsoid name.
	//

	char* pReturn = nullptr;
	if (ellipseIndex < sizeof(m_Ellipsoid) / sizeof(EllipsoidType))
		pReturn = m_Ellipsoid[ellipseIndex].Name;

	return pReturn;
}

bool GIS::IsRaster(GEODATA type)
{
	bool isRaster = false;

	switch (type) {
	case GEODATA::DEM10:
	case GEODATA::DEM30:
	case GEODATA::DTED0:
	case GEODATA::DTED1:
	case GEODATA::DTED2:
	case GEODATA::E00GRID:
	case GEODATA::GeoTIFF:
	case GEODATA::HFA:
	case GEODATA::PDS:
	case GEODATA::TIFF:
	case GEODATA::TerrainFile:
	case GEODATA::VtpBT:
	case GEODATA::XYZ:
		isRaster = true;
		break;
	default:
		break;
	}

	return isRaster;
}

bool GIS::IsVector(GEODATA type)
{
	bool isVector = false;

	switch (type) {
	case GEODATA::ShapeFile:
		isVector = true;
		break;
	default:
		break;
	}

	return isVector;
}

bool GIS::IsPointCloud(GEODATA type)
{
	bool isPoint = false;

	switch (type) {
	case GEODATA::LAS:
	case GEODATA::LAZ:
		isPoint = true;
		break;
	default:
		break;
	}

	return isPoint;
}

char* GIS::StateToID(const char* stateName)
{
	char* stateID = nullptr;

	XString name = stateName;
	name.Trim();

	if (name.CompareNoCase( "Alabama"))
		stateID = "AL";
	else if (name.CompareNoCase( "Alaska"))
		stateID = "AK";
	else if (name.CompareNoCase( "Arizona"))
		stateID = "AZ";
	else if (name.CompareNoCase( "Arkansas"))
		stateID = "AR";
	else if (name.CompareNoCase( "California"))
		stateID = "CA";
	else if (name.CompareNoCase( "Colorado"))
		stateID = "CO";
	else if (name.CompareNoCase( "Connecticut"))
		stateID = "CT";
	else if (name.CompareNoCase( "Deleware"))
		stateID = "DE";
	else if (name.CompareNoCase( "Florida"))
		stateID = "FL";
	else if (name.CompareNoCase( "Georgia"))
		stateID = "GA";
	else if (name.CompareNoCase( "Hawaii"))
		stateID = "HI";
	else if (name.CompareNoCase( "Idaho"))
		stateID = "ID";
	else if (name.CompareNoCase( "Illinois"))
		stateID = "IL";
	else if (name.CompareNoCase( "Indiana"))
		stateID = "IN";
	else if (name.CompareNoCase( "Iowa"))
		stateID = "IA";
	else if (name.CompareNoCase( "Kansas"))
		stateID = "KS";
	else if (name.CompareNoCase( "Kentucky"))
		stateID = "KY";
	else if (name.CompareNoCase( "Louisiana"))
		stateID = "LA";
	else if (name.CompareNoCase( "Maine"))
		stateID = "ME";
	else if (name.CompareNoCase( "Maryland"))
		stateID = "MD";
	else if (name.CompareNoCase( "Massachusetts"))
		stateID = "MA";
	else if (name.CompareNoCase( "Michigan"))
		stateID = "MI";
	else if (name.CompareNoCase( "Minnesota"))
		stateID = "MN";
	else if (name.CompareNoCase( "Mississippi"))
		stateID = "MS";
	else if (name.CompareNoCase( "Missouri"))
		stateID = "MO";
	else if (name.CompareNoCase( "Montana"))
		stateID = "MT";
	else if (name.CompareNoCase( "Nebraska"))
		stateID = "NE";
	else if (name.CompareNoCase( "Nevada"))
		stateID = "NV";
	else if (name.CompareNoCase( "New Hampshire"))
		stateID = "NH";
	else if (name.CompareNoCase( "New Jersey"))
		stateID = "NJ";
	else if (name.CompareNoCase( "New Mexico"))
		stateID = "NM";
	else if (name.CompareNoCase( "New York"))
		stateID = "NY";
	else if (name.CompareNoCase( "North Carolina"))
		stateID = "NC";
	else if (name.CompareNoCase( "North Dakota"))
		stateID = "ND";
	else if (name.CompareNoCase( "Ohio"))
		stateID = "OH";
	else if (name.CompareNoCase( "Oklahoma"))
		stateID = "OK";
	else if (name.CompareNoCase( "Oregon"))
		stateID = "OR";
	else if (name.CompareNoCase( "Pennsylvania"))
		stateID = "PA";
	else if (name.CompareNoCase( "Rhode Island"))
		stateID = "RI";
	else if (name.CompareNoCase( "South Carolina"))
		stateID = "SC";
	else if (name.CompareNoCase( "South Dakota"))
		stateID = "SD";
	else if (name.CompareNoCase( "Tennessee"))
		stateID = "TN";
	else if (name.CompareNoCase( "Texas"))
		stateID = "TX";
	else if (name.CompareNoCase( "Utah"))
		stateID = "UT";
	else if (name.CompareNoCase( "Vermont"))
		stateID = "VT";
	else if (name.CompareNoCase( "Virgina"))
		stateID = "VA";
	else if (name.CompareNoCase( "Washington"))
		stateID = "WA";
	else if (name.CompareNoCase( "West Virginia"))
		stateID = "WV";
	else if (name.CompareNoCase( "Wisconsin"))
		stateID = "WI";
	else if (name.CompareNoCase( "Wyoming"))
		stateID = "WY";
	// Unincorporated organized territories
	else if (name.CompareNoCase( "Guam"))
		stateID = "GU";
	else if (name.CompareNoCase( "Puerto Rico"))
		stateID = "PR";
	else if (name.CompareNoCase( "US Virgin Islands"))
		stateID = "VI";

	return stateID;
}

char* GIS::IDToState(const char* stateID)
{
	char* stateName = nullptr;

	XString id = stateID;
	id.Trim();
	id.MakeUpper();

	if (id == "AL")
		stateName = "Alabama";
	else if (id == "AK")
		stateName = "Alaska";
	else if (id == "AZ")
		stateName = "Arizona";
	else if (id == "AR")
		stateName = "Arkansas";
	else if (id == "CA")
		stateName = "California";
	else if (id == "CO")
		stateName = "Colorado";
	else if (id == "CT")
		stateName = "Connecticut";
	else if (id == "DE")
		stateName = "Deleware";
	else if (id == "FL")
		stateName = "Florida";
	else if (id == "GA")
		stateName = "Georgia";
	else if (id == "HI")
		stateName = "Hawaii";
	else if (id == "ID")
		stateName = "Idaho";
	else if (id == "IL")
		stateName = "Illinois";
	else if (id == "IN")
		stateName = "Indiana";
	else if (id == "IA")
		stateName = "Iowa";
	else if (id == "KS")
		stateName = "Kansas";
	else if (id == "KY")
		stateName = "Kentucky";
	else if (id == "LA")
		stateName = "Louisiana";
	else if (id == "ME")
		stateName = "Maine";
	else if (id == "MD")
		stateName = "Maryland";
	else if (id == "MA")
		stateName = "Massachusetts";
	else if (id == "MI")
		stateName = "Michigan";
	else if (id == "MN")
		stateName = "Minnesota";
	else if (id == "MS")
		stateName = "Mississippi";
	else if (id == "MO")
		stateName = "Missouri";
	else if (id == "MT")
		stateName = "Montana";
	else if (id == "NE")
		stateName = "Nebraska";
	else if (id == "NV")
		stateName = "Nevada";
	else if (id == "NH")
		stateName = "New Hampshire";
	else if (id == "NJ")
		stateName = "New Jersey";
	else if (id == "NM")
		stateName = "New Mexico";
	else if (id == "NY")
		stateName = "New York";
	else if (id == "NC")
		stateName = "North Carolina";
	else if (id == "ND")
		stateName = "North Dakota";
	else if (id == "OH")
		stateName = "Ohio";
	else if (id == "OK")
		stateName = "Oklahoma";
	else if (id == "OR")
		stateName = "Oregon";
	else if (id == "PA")
		stateName = "Pennsylvania";
	else if (id == "RI")
		stateName = "Rhode Island";
	else if (id == "SC")
		stateName = "South Carolina";
	else if (id == "SD")
		stateName = "South Dakota";
	else if (id == "TN")
		stateName = "Tennessee";
	else if (id == "TX")
		stateName = "Texas";
	else if (id == "UT")
		stateName = "Utah";
	else if (id == "VT")
		stateName = "Vermont";
	else if (id == "VA")
		stateName = "Virgina";
	else if (id == "WA")
		stateName = "Washington";
	else if (id == "WV")
		stateName = "West Virginia";
	else if (id == "WI")
		stateName = "Wisconsin";
	else if (id == "WY")
		stateName = "Wyoming";
	//Unincorporated organized territories
	else if (id == "GU")
		stateName = "Guam";
	else if (id == "PR")
		stateName = "Puerto Rico";
	else if (id == "VI")
		stateName = "US Virgin Islands";

	return stateName;
}
