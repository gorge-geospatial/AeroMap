// This file was programatically generated from '\DevLib\pdal-2.3\pdal\Dimension.json.
// Do not edit directly.

#pragma once

#include <string>
#include <vector>

#include <pdal/DimUtil.hpp>
#include <pdal/pdal_types.hpp>
#include <pdal/util/Utils.hpp>

namespace pdal
{
namespace Dimension
{

enum class Id
{
    Unknown,
    X,
    Y,
    Z,
    W,
    Intensity,
    Amplitude,
    Reflectance,
    ReturnNumber,
    NumberOfReturns,
    ScanDirectionFlag,
    EdgeOfFlightLine,
    Classification,
    ScanAngleRank,
    UserData,
    PointSourceId,
    Red,
    Green,
    Blue,
    GpsTime,
    InternalTime,
    OffsetTime,
    IsPpsLocked,
    StartPulse,
    ReflectedPulse,
    Pdop,
    Pitch,
    Roll,
    PulseWidth,
    Deviation,
    PassiveSignal,
    BackgroundRadiation,
    PassiveX,
    PassiveY,
    PassiveZ,
    XVelocity,
    YVelocity,
    ZVelocity,
    Azimuth,
    WanderAngle,
    XBodyAccel,
    YBodyAccel,
    ZBodyAccel,
    XBodyAngRate,
    YBodyAngRate,
    ZBodyAngRate,
    Flag,
    Mark,
    Alpha,
    EchoRange,
    ScanChannel,
    Infrared,
    HeightAboveGround,
    ClassFlags,
    LvisLfid,
    ShotNumber,
    LongitudeCentroid,
    LatitudeCentroid,
    ElevationCentroid,
    LongitudeLow,
    LatitudeLow,
    ElevationLow,
    LongitudeHigh,
    LatitudeHigh,
    ElevationHigh,
    PointId,
    OriginId,
    NormalX,
    NormalY,
    NormalZ,
    Curvature,
    Density,
    Omit,
    ClusterID,
    NNDistance,
    TextureU,
    TextureV,
    TextureW,
    Linearity,
    Planarity,
    Scattering,
    Verticality,
    Omnivariance,
    Anisotropy,
    Eigenentropy,
    EigenvalueSum,
    SurfaceVariation,
    DemantkeVerticality,
    OptimalKNN,
    OptimalRadius,
    Coplanar,
    LocalReachabilityDistance,
    LocalOutlierFactor,
    Miniball,
    Reciprocity,
    Rank,
    Eigenvalue0,
    Eigenvalue1,
    Eigenvalue2,
    PlaneFit,
    RadialDensity
};
typedef std::vector<Id> IdList;


/// Get a description of a predefined dimension.
/// \param[in] id  Dimension ID.
/// \return  Dimension description.
inline std::string description(Id id)
{
    switch (id)
    {
    case Id::X:
        return "X coordinate";
    case Id::Y:
        return "Y coordinate";
    case Id::Z:
        return "Z coordinate";
    case Id::W:
        return "W coordinate";
    case Id::Intensity:
        return "Representation of the pulse return magnitude";
    case Id::Amplitude:
        return "This is the ratio of the received power to the power "
            "received at the detection limit expressed in dB";
    case Id::Reflectance:
        return "Ratio of the received power to the power that would be "
            "received from a white diffuse target at the same distance "
            "expressed in dB. The reflectance represents a range "
            "independent property of the target.  The surface normal of "
            "this target is assumed to be in parallel to the laser beam "
            "direction.";
    case Id::ReturnNumber:
        return "Pulse return number for a given output pulse. A given "
            "output laser pulse can have many returns, and they must be "
            "marked in order, starting with 1";
    case Id::NumberOfReturns:
        return "Total number of returns for a given pulse.";
    case Id::ScanDirectionFlag:
        return "Direction at which the scanner mirror was traveling at the "
            "time of the output pulse. A value of 1 is a positive scan "
            "direction, and a bit value of 0 is a negative scan direction, "
            "where positive scan direction is a scan moving from the left "
            "side of the in-track direction to the right side and negative "
            "the opposite";
    case Id::EdgeOfFlightLine:
        return "Indicates the end of scanline before a direction change "
            "with a value of 1 - 0 otherwise";
    case Id::Classification:
        return "ASPRS classification.  0 for no classification.  See LAS "
            "specification for details.";
    case Id::ScanAngleRank:
        return "Angle degree at which the laser point was output from the "
            "system, including the roll of the aircraft.  The scan angle is "
            "based on being nadir, and -90 the left side of the aircraft in "
            "the direction of flight";
    case Id::UserData:
        return "Unspecified user data";
    case Id::PointSourceId:
        return "File source ID from which the point originated.  Zero "
            "indicates that the point originated in the current file";
    case Id::Red:
        return "Red image channel value";
    case Id::Green:
        return "Green image channel value";
    case Id::Blue:
        return "Blue image channel value";
    case Id::GpsTime:
        return "GPS time that the point was acquired";
    case Id::InternalTime:
        return "Scanner's internal time when the point was acquired, in "
            "seconds";
    case Id::OffsetTime:
        return "Milliseconds from first acquired point";
    case Id::IsPpsLocked:
        return "The external PPS signal was found to be synchronized at the "
            "time of the current laser shot.";
    case Id::StartPulse:
        return "Relative pulse signal strength";
    case Id::ReflectedPulse:
        return "Relative reflected pulse signal strength";
    case Id::Pdop:
        return "GPS PDOP (dilution of precision)";
    case Id::Pitch:
        return "Pitch in degrees";
    case Id::Roll:
        return "Roll in degrees";
    case Id::PulseWidth:
        return "Laser received pulse width (digitizer samples)";
    case Id::Deviation:
        return "Difference between the shape of the reference pulse and the "
            "return pulse. A larger value for deviation indicates larger "
            "distortion.";
    case Id::PassiveSignal:
        return "Relative passive signal";
    case Id::BackgroundRadiation:
        return "A measure of background radiation.";
    case Id::PassiveX:
        return "Passive X footprint";
    case Id::PassiveY:
        return "Passive Y footprint";
    case Id::PassiveZ:
        return "Passive Z footprint";
    case Id::XVelocity:
        return "X Velocity";
    case Id::YVelocity:
        return "Y Velocity";
    case Id::ZVelocity:
        return "Z Velocity";
    case Id::Azimuth:
        return "Scanner azimuth";
    case Id::WanderAngle:
        return "Wander Angle";
    case Id::XBodyAccel:
        return "X Body Acceleration";
    case Id::YBodyAccel:
        return "Y Body Acceleration";
    case Id::ZBodyAccel:
        return "Z Body Acceleration";
    case Id::XBodyAngRate:
        return "X Body Angle Rate";
    case Id::YBodyAngRate:
        return "Y Body Angle Rate";
    case Id::ZBodyAngRate:
        return "Z Body Angle Rate";
    case Id::Flag:
        return "Flag";
    case Id::Mark:
        return "Mark";
    case Id::Alpha:
        return "Alpha";
    case Id::EchoRange:
        return "Echo Range";
    case Id::ScanChannel:
        return "Scan Channel";
    case Id::Infrared:
        return "Infrared";
    case Id::HeightAboveGround:
        return "Height Above Ground";
    case Id::ClassFlags:
        return "Class Flags";
    case Id::LvisLfid:
        return "LVIS_LFID";
    case Id::ShotNumber:
        return "Shot Number";
    case Id::LongitudeCentroid:
        return "Longitude Centroid";
    case Id::LatitudeCentroid:
        return "Latitude Centroid";
    case Id::ElevationCentroid:
        return "Elevation Centroid";
    case Id::LongitudeLow:
        return "Longitude Low";
    case Id::LatitudeLow:
        return "Latitude Low";
    case Id::ElevationLow:
        return "Elevation Low";
    case Id::LongitudeHigh:
        return "Longitude High";
    case Id::LatitudeHigh:
        return "Latitude High";
    case Id::ElevationHigh:
        return "Elevation High";
    case Id::PointId:
        return "An explicit representation of point ordering within a file, "
            "which allows this usually-implicit information to be preserved "
            "when reordering points.";
    case Id::OriginId:
        return "A file source ID from which the point originated.  This ID "
            "is global to a derivative dataset which may be aggregated from "
            "multiple files.";
    case Id::NormalX:
        return "X component of a vector normal to surface at this point";
    case Id::NormalY:
        return "Y component of a vector normal to surface at this point";
    case Id::NormalZ:
        return "Z component of a vector normal to surface at this point";
    case Id::Curvature:
        return "Curvature of surface at this point";
    case Id::Density:
        return "Estimate of point density";
    case Id::Omit:
        return "Used to shallowly mark a point as being omitted without "
            "removing it";
    case Id::ClusterID:
        return "ID assigned to a point by a point-clustering algorithm.";
    case Id::NNDistance:
        return "Distance metric related to a point's nearest neighbors.";
    case Id::TextureU:
        return "U component of a texture location at this point";
    case Id::TextureV:
        return "V component of a texture location at this point";
    case Id::TextureW:
        return "W component of a texture location at this point";
    case Id::Linearity:
        return "Linearity of a point; larger values indicate more linear "
            "regions.";
    case Id::Planarity:
        return "Planarity of a point; larger values indicate more planar "
            "regions.";
    case Id::Scattering:
        return "Scattering of a point; larger values incidate complex "
            "(scattered) 3D regions.";
    case Id::Verticality:
        return "Verticality of a point; larger values indicate vertical "
            "structure.";
    case Id::Omnivariance:
        return "Omnivariance of a point; cube root of the product of all "
            "eigenvalues.";
    case Id::Anisotropy:
        return "Anisotropy of a point; larger values indicate strong "
            "variance in multiple dimensions.";
    case Id::Eigenentropy:
        return "Eigenentropy of a point; small values indicate more ordered "
            "regions, while large values indicate disorder.";
    case Id::EigenvalueSum:
        return "Sum of computed eigenvalues.";
    case Id::SurfaceVariation:
        return "Surface variation of a point; larger values indicate higher "
            "surface variation.";
    case Id::DemantkeVerticality:
        return "Verticality of a point; larger values indicate vertical "
            "structure (Demantke's variation).";
    case Id::OptimalKNN:
        return "Optimal number of k nearest neighbors, such that "
            "eigenentropy is minimized.";
    case Id::OptimalRadius:
        return "Radius corresponding to optimal k nearest neighbors, such "
            "that eigenentropy is minimized.";
    case Id::Coplanar:
        return "Indicator of whether or not a point is part of a coplanar "
            "neighborhood.";
    case Id::LocalReachabilityDistance:
        return "Reachability metric based on the NNDistance of a point's "
            "nearest neighbors.";
    case Id::LocalOutlierFactor:
        return "Outlier factor based on the LocalReachabilityDistance of a "
            "point's nearest neighbors.";
    case Id::Miniball:
        return "Metric capturing distance from a point to the center of the "
            "smallest enclosing ball encapsulating k-nearest neighbors, "
            "scaled by radius of the ball.";
    case Id::Reciprocity:
        return "Metric capturing percentage of k-nearest neighbors that "
            "also contain the current point in their k-neighborhood.";
    case Id::Rank:
        return "Estimated rank of neighborhood of points.";
    case Id::Eigenvalue0:
        return "Smallest eigenvalue obtained form covariance of XYZ "
            "coordinates in k-neighborhood.";
    case Id::Eigenvalue1:
        return "Middle eigenvalue obtained form covariance of XYZ "
            "coordinates in k-neighborhood.";
    case Id::Eigenvalue2:
        return "Largest eigenvalue obtained form covariance of XYZ "
            "coordinates in k-neighborhood.";
    case Id::PlaneFit:
        return "Metric capturing current point's point to plane distance "
            "compared to those in the same k-neighborhood used to estimate "
            "the plane.";
    case Id::RadialDensity:
        return "Estimate of radial point density";
    case Id::Unknown:
        return "";
    }
    return "";
}

/// Get a predefined dimension ID given a dimension name. Multiple names
/// may map to the same dimension for convenience.  Names are case-insensitive.
/// \param[in] s  Name of dimension.
/// \return  Dimension ID associated with the name.  Id::Unknown is returned
///    if the name doesn't map to a predefined dimension.
inline Id id(std::string s)
{
    s = Utils::toupper(s);

    if (s == "X")
        return Id::X;
    if (s == "Y")
        return Id::Y;
    if (s == "Z")
        return Id::Z;
    if (s == "W")
        return Id::W;
    if (s == "INTENSITY")
        return Id::Intensity;
    if (s == "AMPLITUDE")
        return Id::Amplitude;
    if (s == "REFLECTANCE")
        return Id::Reflectance;
    if (s == "RETURNNUMBER")
        return Id::ReturnNumber;
    if (s == "NUMBEROFRETURNS")
        return Id::NumberOfReturns;
    if (s == "SCANDIRECTIONFLAG")
        return Id::ScanDirectionFlag;
    if (s == "EDGEOFFLIGHTLINE")
        return Id::EdgeOfFlightLine;
    if (s == "CLASSIFICATION")
        return Id::Classification;
    if (s == "SCANANGLERANK")
        return Id::ScanAngleRank;
    if (s == "SCANANGLE")
        return Id::ScanAngleRank;
    if (s == "USERDATA")
        return Id::UserData;
    if (s == "POINTSOURCEID")
        return Id::PointSourceId;
    if (s == "RED")
        return Id::Red;
    if (s == "DIFFUSE_RED")
        return Id::Red;
    if (s == "GREEN")
        return Id::Green;
    if (s == "DIFFUSE_GREEN")
        return Id::Green;
    if (s == "BLUE")
        return Id::Blue;
    if (s == "DIFFUSE_BLUE")
        return Id::Blue;
    if (s == "GPSTIME")
        return Id::GpsTime;
    if (s == "INTERNALTIME")
        return Id::InternalTime;
    if (s == "OFFSETTIME")
        return Id::OffsetTime;
    if (s == "TIME")
        return Id::OffsetTime;
    if (s == "ISPPSLOCKED")
        return Id::IsPpsLocked;
    if (s == "STARTPULSE")
        return Id::StartPulse;
    if (s == "REFLECTEDPULSE")
        return Id::ReflectedPulse;
    if (s == "PDOP")
        return Id::Pdop;
    if (s == "PITCH")
        return Id::Pitch;
    if (s == "ROLL")
        return Id::Roll;
    if (s == "PULSEWIDTH")
        return Id::PulseWidth;
    if (s == "DEVIATION")
        return Id::Deviation;
    if (s == "PASSIVESIGNAL")
        return Id::PassiveSignal;
    if (s == "BACKGROUNDRADIATION")
        return Id::BackgroundRadiation;
    if (s == "PASSIVEX")
        return Id::PassiveX;
    if (s == "PASSIVEY")
        return Id::PassiveY;
    if (s == "PASSIVEZ")
        return Id::PassiveZ;
    if (s == "XVELOCITY")
        return Id::XVelocity;
    if (s == "YVELOCITY")
        return Id::YVelocity;
    if (s == "ZVELOCITY")
        return Id::ZVelocity;
    if (s == "AZIMUTH")
        return Id::Azimuth;
    if (s == "PLATFORMHEADING")
        return Id::Azimuth;
    if (s == "WANDERANGLE")
        return Id::WanderAngle;
    if (s == "XBODYACCEL")
        return Id::XBodyAccel;
    if (s == "YBODYACCEL")
        return Id::YBodyAccel;
    if (s == "ZBODYACCEL")
        return Id::ZBodyAccel;
    if (s == "XBODYANGRATE")
        return Id::XBodyAngRate;
    if (s == "YBODYANGRATE")
        return Id::YBodyAngRate;
    if (s == "ZBODYANGRATE")
        return Id::ZBodyAngRate;
    if (s == "FLAG")
        return Id::Flag;
    if (s == "MARK")
        return Id::Mark;
    if (s == "ALPHA")
        return Id::Alpha;
    if (s == "ECHORANGE")
        return Id::EchoRange;
    if (s == "SCANCHANNEL")
        return Id::ScanChannel;
    if (s == "INFRARED")
        return Id::Infrared;
    if (s == "NEARINFRARED")
        return Id::Infrared;
    if (s == "HEIGHTABOVEGROUND")
        return Id::HeightAboveGround;
    if (s == "CLASSFLAGS")
        return Id::ClassFlags;
    if (s == "LVISLFID")
        return Id::LvisLfid;
    if (s == "LVIS_LFID")
        return Id::LvisLfid;
    if (s == "SHOTNUMBER")
        return Id::ShotNumber;
    if (s == "LONGITUDECENTROID")
        return Id::LongitudeCentroid;
    if (s == "LONGITUDE_CENTROID")
        return Id::LongitudeCentroid;
    if (s == "LATITUDECENTROID")
        return Id::LatitudeCentroid;
    if (s == "LATITUDE_CENTROID")
        return Id::LatitudeCentroid;
    if (s == "ELEVATIONCENTROID")
        return Id::ElevationCentroid;
    if (s == "ELEVATION_CENTROID")
        return Id::ElevationCentroid;
    if (s == "LONGITUDELOW")
        return Id::LongitudeLow;
    if (s == "LONGITUDE_LOW")
        return Id::LongitudeLow;
    if (s == "LATITUDELOW")
        return Id::LatitudeLow;
    if (s == "LATITUDE_LOW")
        return Id::LatitudeLow;
    if (s == "ELEVATIONLOW")
        return Id::ElevationLow;
    if (s == "ELEVATION_LOW")
        return Id::ElevationLow;
    if (s == "LONGITUDEHIGH")
        return Id::LongitudeHigh;
    if (s == "LONGITUDE_HIGH")
        return Id::LongitudeHigh;
    if (s == "LATITUDEHIGH")
        return Id::LatitudeHigh;
    if (s == "LATITUDE_HIGH")
        return Id::LatitudeHigh;
    if (s == "ELEVATIONHIGH")
        return Id::ElevationHigh;
    if (s == "ELEVATION_HIGH")
        return Id::ElevationHigh;
    if (s == "POINTID")
        return Id::PointId;
    if (s == "ORIGINID")
        return Id::OriginId;
    if (s == "NORMALX")
        return Id::NormalX;
    if (s == "NX")
        return Id::NormalX;
    if (s == "NORMAL_X")
        return Id::NormalX;
    if (s == "NORMALX")
        return Id::NormalX;
    if (s == "NORMALY")
        return Id::NormalY;
    if (s == "NY")
        return Id::NormalY;
    if (s == "NORMAL_Y")
        return Id::NormalY;
    if (s == "NORMALY")
        return Id::NormalY;
    if (s == "NORMALZ")
        return Id::NormalZ;
    if (s == "NZ")
        return Id::NormalZ;
    if (s == "NORMAL_Z")
        return Id::NormalZ;
    if (s == "NORMALZ")
        return Id::NormalZ;
    if (s == "CURVATURE")
        return Id::Curvature;
    if (s == "DENSITY")
        return Id::Density;
    if (s == "OMIT")
        return Id::Omit;
    if (s == "CLUSTERID")
        return Id::ClusterID;
    if (s == "NNDISTANCE")
        return Id::NNDistance;
    if (s == "TEXTUREU")
        return Id::TextureU;
    if (s == "TU")
        return Id::TextureU;
    if (s == "TEXTURE_U")
        return Id::TextureU;
    if (s == "TEXTUREU")
        return Id::TextureU;
    if (s == "TEXTUREV")
        return Id::TextureV;
    if (s == "TV")
        return Id::TextureV;
    if (s == "TEXTURE_V")
        return Id::TextureV;
    if (s == "TEXTUREV")
        return Id::TextureV;
    if (s == "TEXTUREW")
        return Id::TextureW;
    if (s == "TW")
        return Id::TextureW;
    if (s == "TEXTURE_W")
        return Id::TextureW;
    if (s == "TEXTUREW")
        return Id::TextureW;
    if (s == "LINEARITY")
        return Id::Linearity;
    if (s == "PLANARITY")
        return Id::Planarity;
    if (s == "SCATTERING")
        return Id::Scattering;
    if (s == "VERTICALITY")
        return Id::Verticality;
    if (s == "OMNIVARIANCE")
        return Id::Omnivariance;
    if (s == "ANISOTROPY")
        return Id::Anisotropy;
    if (s == "EIGENENTROPY")
        return Id::Eigenentropy;
    if (s == "EIGENVALUESUM")
        return Id::EigenvalueSum;
    if (s == "SURFACEVARIATION")
        return Id::SurfaceVariation;
    if (s == "DEMANTKEVERTICALITY")
        return Id::DemantkeVerticality;
    if (s == "OPTIMALKNN")
        return Id::OptimalKNN;
    if (s == "OPTIMALRADIUS")
        return Id::OptimalRadius;
    if (s == "COPLANAR")
        return Id::Coplanar;
    if (s == "LOCALREACHABILITYDISTANCE")
        return Id::LocalReachabilityDistance;
    if (s == "LOCALOUTLIERFACTOR")
        return Id::LocalOutlierFactor;
    if (s == "MINIBALL")
        return Id::Miniball;
    if (s == "RECIPROCITY")
        return Id::Reciprocity;
    if (s == "RANK")
        return Id::Rank;
    if (s == "EIGENVALUE0")
        return Id::Eigenvalue0;
    if (s == "EIGENVALUE1")
        return Id::Eigenvalue1;
    if (s == "EIGENVALUE2")
        return Id::Eigenvalue2;
    if (s == "PLANEFIT")
        return Id::PlaneFit;
    if (s == "RADIALDENSITY")
        return Id::RadialDensity;
    return Id::Unknown;
}

/// Get the name of a predefined dimension.
/// \param[in] id  Dimension ID
/// \return  Dimension name.
inline std::string name(Id id)
{
    switch (id)
    {
    case Id::X:
        return "X";
    case Id::Y:
        return "Y";
    case Id::Z:
        return "Z";
    case Id::W:
        return "W";
    case Id::Intensity:
        return "Intensity";
    case Id::Amplitude:
        return "Amplitude";
    case Id::Reflectance:
        return "Reflectance";
    case Id::ReturnNumber:
        return "ReturnNumber";
    case Id::NumberOfReturns:
        return "NumberOfReturns";
    case Id::ScanDirectionFlag:
        return "ScanDirectionFlag";
    case Id::EdgeOfFlightLine:
        return "EdgeOfFlightLine";
    case Id::Classification:
        return "Classification";
    case Id::ScanAngleRank:
        return "ScanAngleRank";
    case Id::UserData:
        return "UserData";
    case Id::PointSourceId:
        return "PointSourceId";
    case Id::Red:
        return "Red";
    case Id::Green:
        return "Green";
    case Id::Blue:
        return "Blue";
    case Id::GpsTime:
        return "GpsTime";
    case Id::InternalTime:
        return "InternalTime";
    case Id::OffsetTime:
        return "OffsetTime";
    case Id::IsPpsLocked:
        return "IsPpsLocked";
    case Id::StartPulse:
        return "StartPulse";
    case Id::ReflectedPulse:
        return "ReflectedPulse";
    case Id::Pdop:
        return "Pdop";
    case Id::Pitch:
        return "Pitch";
    case Id::Roll:
        return "Roll";
    case Id::PulseWidth:
        return "PulseWidth";
    case Id::Deviation:
        return "Deviation";
    case Id::PassiveSignal:
        return "PassiveSignal";
    case Id::BackgroundRadiation:
        return "BackgroundRadiation";
    case Id::PassiveX:
        return "PassiveX";
    case Id::PassiveY:
        return "PassiveY";
    case Id::PassiveZ:
        return "PassiveZ";
    case Id::XVelocity:
        return "XVelocity";
    case Id::YVelocity:
        return "YVelocity";
    case Id::ZVelocity:
        return "ZVelocity";
    case Id::Azimuth:
        return "Azimuth";
    case Id::WanderAngle:
        return "WanderAngle";
    case Id::XBodyAccel:
        return "XBodyAccel";
    case Id::YBodyAccel:
        return "YBodyAccel";
    case Id::ZBodyAccel:
        return "ZBodyAccel";
    case Id::XBodyAngRate:
        return "XBodyAngRate";
    case Id::YBodyAngRate:
        return "YBodyAngRate";
    case Id::ZBodyAngRate:
        return "ZBodyAngRate";
    case Id::Flag:
        return "Flag";
    case Id::Mark:
        return "Mark";
    case Id::Alpha:
        return "Alpha";
    case Id::EchoRange:
        return "EchoRange";
    case Id::ScanChannel:
        return "ScanChannel";
    case Id::Infrared:
        return "Infrared";
    case Id::HeightAboveGround:
        return "HeightAboveGround";
    case Id::ClassFlags:
        return "ClassFlags";
    case Id::LvisLfid:
        return "LvisLfid";
    case Id::ShotNumber:
        return "ShotNumber";
    case Id::LongitudeCentroid:
        return "LongitudeCentroid";
    case Id::LatitudeCentroid:
        return "LatitudeCentroid";
    case Id::ElevationCentroid:
        return "ElevationCentroid";
    case Id::LongitudeLow:
        return "LongitudeLow";
    case Id::LatitudeLow:
        return "LatitudeLow";
    case Id::ElevationLow:
        return "ElevationLow";
    case Id::LongitudeHigh:
        return "LongitudeHigh";
    case Id::LatitudeHigh:
        return "LatitudeHigh";
    case Id::ElevationHigh:
        return "ElevationHigh";
    case Id::PointId:
        return "PointId";
    case Id::OriginId:
        return "OriginId";
    case Id::NormalX:
        return "NormalX";
    case Id::NormalY:
        return "NormalY";
    case Id::NormalZ:
        return "NormalZ";
    case Id::Curvature:
        return "Curvature";
    case Id::Density:
        return "Density";
    case Id::Omit:
        return "Omit";
    case Id::ClusterID:
        return "ClusterID";
    case Id::NNDistance:
        return "NNDistance";
    case Id::TextureU:
        return "TextureU";
    case Id::TextureV:
        return "TextureV";
    case Id::TextureW:
        return "TextureW";
    case Id::Linearity:
        return "Linearity";
    case Id::Planarity:
        return "Planarity";
    case Id::Scattering:
        return "Scattering";
    case Id::Verticality:
        return "Verticality";
    case Id::Omnivariance:
        return "Omnivariance";
    case Id::Anisotropy:
        return "Anisotropy";
    case Id::Eigenentropy:
        return "Eigenentropy";
    case Id::EigenvalueSum:
        return "EigenvalueSum";
    case Id::SurfaceVariation:
        return "SurfaceVariation";
    case Id::DemantkeVerticality:
        return "DemantkeVerticality";
    case Id::OptimalKNN:
        return "OptimalKNN";
    case Id::OptimalRadius:
        return "OptimalRadius";
    case Id::Coplanar:
        return "Coplanar";
    case Id::LocalReachabilityDistance:
        return "LocalReachabilityDistance";
    case Id::LocalOutlierFactor:
        return "LocalOutlierFactor";
    case Id::Miniball:
        return "Miniball";
    case Id::Reciprocity:
        return "Reciprocity";
    case Id::Rank:
        return "Rank";
    case Id::Eigenvalue0:
        return "Eigenvalue0";
    case Id::Eigenvalue1:
        return "Eigenvalue1";
    case Id::Eigenvalue2:
        return "Eigenvalue2";
    case Id::PlaneFit:
        return "PlaneFit";
    case Id::RadialDensity:
        return "RadialDensity";
    case Id::Unknown:
        return "";
    }
    return "";
}

/// Get the default storage type of a predefined dimension.
/// \param id  ID of the predefined dimension.
/// \return  The dimension's default storage type.  An exception is thrown if
///   the id doesn't represent a predefined dimension.
inline Type defaultType(Id id)
{
    switch (id)
    {
    case Id::X:
        return Type::Double;
    case Id::Y:
        return Type::Double;
    case Id::Z:
        return Type::Double;
    case Id::W:
        return Type::Double;
    case Id::Intensity:
        return Type::Unsigned16;
    case Id::Amplitude:
        return Type::Float;
    case Id::Reflectance:
        return Type::Float;
    case Id::ReturnNumber:
        return Type::Unsigned8;
    case Id::NumberOfReturns:
        return Type::Unsigned8;
    case Id::ScanDirectionFlag:
        return Type::Unsigned8;
    case Id::EdgeOfFlightLine:
        return Type::Unsigned8;
    case Id::Classification:
        return Type::Unsigned8;
    case Id::ScanAngleRank:
        return Type::Float;
    case Id::UserData:
        return Type::Unsigned8;
    case Id::PointSourceId:
        return Type::Unsigned16;
    case Id::Red:
        return Type::Unsigned16;
    case Id::Green:
        return Type::Unsigned16;
    case Id::Blue:
        return Type::Unsigned16;
    case Id::GpsTime:
        return Type::Double;
    case Id::InternalTime:
        return Type::Double;
    case Id::OffsetTime:
        return Type::Unsigned32;
    case Id::IsPpsLocked:
        return Type::Unsigned8;
    case Id::StartPulse:
        return Type::Signed32;
    case Id::ReflectedPulse:
        return Type::Signed32;
    case Id::Pdop:
        return Type::Float;
    case Id::Pitch:
        return Type::Float;
    case Id::Roll:
        return Type::Float;
    case Id::PulseWidth:
        return Type::Float;
    case Id::Deviation:
        return Type::Float;
    case Id::PassiveSignal:
        return Type::Signed32;
    case Id::BackgroundRadiation:
        return Type::Float;
    case Id::PassiveX:
        return Type::Double;
    case Id::PassiveY:
        return Type::Double;
    case Id::PassiveZ:
        return Type::Double;
    case Id::XVelocity:
        return Type::Double;
    case Id::YVelocity:
        return Type::Double;
    case Id::ZVelocity:
        return Type::Double;
    case Id::Azimuth:
        return Type::Double;
    case Id::WanderAngle:
        return Type::Double;
    case Id::XBodyAccel:
        return Type::Double;
    case Id::YBodyAccel:
        return Type::Double;
    case Id::ZBodyAccel:
        return Type::Double;
    case Id::XBodyAngRate:
        return Type::Double;
    case Id::YBodyAngRate:
        return Type::Double;
    case Id::ZBodyAngRate:
        return Type::Double;
    case Id::Flag:
        return Type::Unsigned8;
    case Id::Mark:
        return Type::Unsigned8;
    case Id::Alpha:
        return Type::Unsigned16;
    case Id::EchoRange:
        return Type::Double;
    case Id::ScanChannel:
        return Type::Unsigned8;
    case Id::Infrared:
        return Type::Unsigned16;
    case Id::HeightAboveGround:
        return Type::Double;
    case Id::ClassFlags:
        return Type::Unsigned8;
    case Id::LvisLfid:
        return Type::Unsigned64;
    case Id::ShotNumber:
        return Type::Unsigned64;
    case Id::LongitudeCentroid:
        return Type::Double;
    case Id::LatitudeCentroid:
        return Type::Double;
    case Id::ElevationCentroid:
        return Type::Double;
    case Id::LongitudeLow:
        return Type::Double;
    case Id::LatitudeLow:
        return Type::Double;
    case Id::ElevationLow:
        return Type::Double;
    case Id::LongitudeHigh:
        return Type::Double;
    case Id::LatitudeHigh:
        return Type::Double;
    case Id::ElevationHigh:
        return Type::Double;
    case Id::PointId:
        return Type::Unsigned32;
    case Id::OriginId:
        return Type::Unsigned32;
    case Id::NormalX:
        return Type::Double;
    case Id::NormalY:
        return Type::Double;
    case Id::NormalZ:
        return Type::Double;
    case Id::Curvature:
        return Type::Double;
    case Id::Density:
        return Type::Double;
    case Id::Omit:
        return Type::Unsigned8;
    case Id::ClusterID:
        return Type::Signed64;
    case Id::NNDistance:
        return Type::Double;
    case Id::TextureU:
        return Type::Double;
    case Id::TextureV:
        return Type::Double;
    case Id::TextureW:
        return Type::Double;
    case Id::Linearity:
        return Type::Double;
    case Id::Planarity:
        return Type::Double;
    case Id::Scattering:
        return Type::Double;
    case Id::Verticality:
        return Type::Double;
    case Id::Omnivariance:
        return Type::Double;
    case Id::Anisotropy:
        return Type::Double;
    case Id::Eigenentropy:
        return Type::Double;
    case Id::EigenvalueSum:
        return Type::Double;
    case Id::SurfaceVariation:
        return Type::Double;
    case Id::DemantkeVerticality:
        return Type::Double;
    case Id::OptimalKNN:
        return Type::Unsigned64;
    case Id::OptimalRadius:
        return Type::Double;
    case Id::Coplanar:
        return Type::Unsigned8;
    case Id::LocalReachabilityDistance:
        return Type::Double;
    case Id::LocalOutlierFactor:
        return Type::Double;
    case Id::Miniball:
        return Type::Double;
    case Id::Reciprocity:
        return Type::Double;
    case Id::Rank:
        return Type::Unsigned8;
    case Id::Eigenvalue0:
        return Type::Double;
    case Id::Eigenvalue1:
        return Type::Double;
    case Id::Eigenvalue2:
        return Type::Double;
    case Id::PlaneFit:
        return Type::Double;
    case Id::RadialDensity:
        return Type::Double;
    case Id::Unknown:
        throw pdal_error("No type found for undefined dimension.");
    }
    throw pdal_error("No type found for undefined dimension.");
}

} // namespace Dimension
} // namespace pdal

