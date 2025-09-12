/******************************************************************************
 *
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  Function to register all known OGR drivers.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ****************************************************************************/

#include "ogrsf_frmts.h"

/************************************************************************/
/*                           OGRRegisterAll()                           */
/************************************************************************/

void OGRRegisterAll()
{
    GDALAllRegister();
}

void OGRRegisterAllInternal()
{
    RegisterOGRARCGEN();
    RegisterOGRBNA();
    RegisterOGRGTM();
    RegisterOGROpenFileGDB();       /* Register OpenFileGDB before FGDB as it is more capable for read-only */
    RegisterOGRShape();
    RegisterOGRSDTS();
    RegisterOGRTAB();    // mitab

    // geojson
    RegisterOGRGeoJSON();
    RegisterOGRGeoJSONSeq();
    RegisterOGRESRIJSON();
    RegisterOGRTopoJSON();

#ifdef DB2_ENABLED
    RegisterOGRDB2();
#endif
#ifdef NTF_ENABLED
    RegisterOGRNTF();
#endif
#ifdef LVBAG_ENABLED
    RegisterOGRLVBAG();
#endif
#ifdef S57_ENABLED
    RegisterOGRS57();
#endif
#ifdef DGN_ENABLED
    RegisterOGRDGN();
#endif
#ifdef VRT_ENABLED
    RegisterOGRVRT();
#endif
#ifdef REC_ENABLED
    RegisterOGRREC();
#endif
#ifdef MEM_ENABLED
    RegisterOGRMEM();
#endif
#ifdef CSV_ENABLED
    RegisterOGRCSV();
#endif
#ifdef NAS_ENABLED
    RegisterOGRNAS();
#endif
#ifdef GML_ENABLED
    RegisterOGRGML();
#endif
#ifdef GPX_ENABLED
    RegisterOGRGPX();
#endif
#ifdef LIBKML_ENABLED
    RegisterOGRLIBKML();
#endif
#ifdef KML_ENABLED
    RegisterOGRKML();
#endif
#ifdef ILI_ENABLED
    RegisterOGRILI1();
    RegisterOGRILI2();
#endif
#ifdef GMT_ENABLED
    RegisterOGRGMT();
#endif
#ifdef GPKG_ENABLED
    RegisterOGRGeoPackage();
#endif
#ifdef SQLITE_ENABLED
    RegisterOGRSQLite();
#endif
#ifdef DODS_ENABLED
    RegisterOGRDODS();
#endif
#ifdef ODBC_ENABLED
    RegisterOGRODBC();
#endif
#ifdef WASP_ENABLED
    RegisterOGRWAsP();
#endif

/* Register before PGeo and Geomedia drivers */
/* that don't work well on Linux */
#ifdef MDB_ENABLED
    RegisterOGRMDB();
#endif

#ifdef PGEO_ENABLED
    RegisterOGRPGeo();
#endif
#ifdef MSSQLSPATIAL_ENABLED
    RegisterOGRMSSQLSpatial();
#endif
#ifdef OGDI_ENABLED
    RegisterOGROGDI();
#endif
#ifdef PG_ENABLED
    RegisterOGRPG();
#endif
#ifdef MYSQL_ENABLED
    RegisterOGRMySQL();
#endif
#ifdef OCI_ENABLED
    RegisterOGROCI();
#endif
#ifdef INGRES_ENABLED
    RegisterOGRIngres();
#endif
#ifdef FGDB_ENABLED
    RegisterOGRFileGDB();
#endif
#ifdef XPLANE_ENABLED
    RegisterOGRXPlane();
#endif
#ifdef DWG_ENABLED
    RegisterOGRDWG();
#endif
#ifdef DGNV8_ENABLED
    RegisterOGRDGNV8();
#endif
#ifdef DXF_ENABLED
    RegisterOGRDXF();
#endif
#ifdef CAD_ENABLED
    RegisterOGRCAD();
#endif
#ifdef GRASS_ENABLED
    RegisterOGRGRASS();
#endif
#ifdef FLATGEOBUF_ENABLED
    RegisterOGRFlatGeobuf();
#endif
#ifdef FME_ENABLED
    RegisterOGRFME();
#endif
#ifdef IDB_ENABLED
    RegisterOGRIDB();
#endif
#ifdef GEOCONCEPT_ENABLED
    RegisterOGRGeoconcept();
#endif
#ifdef GEORSS_ENABLED
    RegisterOGRGeoRSS();
#endif
#ifdef VFK_ENABLED
    RegisterOGRVFK();
#endif
#ifdef PGDUMP_ENABLED
    RegisterOGRPGDump();
#endif
#ifdef OSM_ENABLED
    /* Register before GPSBabel, that could recognize .osm file too */
    RegisterOGROSM();
#endif
#ifdef GPSBABEL_ENABLED
    RegisterOGRGPSBabel();
#endif
#ifdef SUA_ENABLED
    RegisterOGRSUA();
#endif
#ifdef OPENAIR_ENABLED
    RegisterOGROpenAir();
#endif
#ifdef PDS_ENABLED
    RegisterOGRPDS();
#endif
#ifdef WFS_ENABLED
    RegisterOGRWFS();
    RegisterOGROAPIF();
#endif
#ifdef SOSI_ENABLED
    RegisterOGRSOSI();
#endif
#ifdef HTF_ENABLED
    RegisterOGRHTF();
#endif
#ifdef AERONAVFAA_ENABLED
    RegisterOGRAeronavFAA();
#endif
#ifdef GEOMEDIA_ENABLED
    RegisterOGRGeomedia();
#endif
#ifdef EDIGEO_ENABLED
    RegisterOGREDIGEO();
#endif
#ifdef SVG_ENABLED
    RegisterOGRSVG();
#endif
#ifdef COUCHDB_ENABLED
    RegisterOGRCouchDB();
#endif
#ifdef CLOUDANT_ENABLED
    RegisterOGRCloudant();
#endif
#ifdef IDRISI_ENABLED
    RegisterOGRIdrisi();
#endif
#ifdef SEGUKOOA_ENABLED
    RegisterOGRSEGUKOOA();
#endif
#ifdef SEGY_ENABLED
    RegisterOGRSEGY();
#endif
#ifdef XLS_ENABLED
    RegisterOGRXLS();
#endif
#ifdef ODS_ENABLED
    RegisterOGRODS();
#endif
#ifdef XLSX_ENABLED
    RegisterOGRXLSX();
#endif
#ifdef ELASTIC_ENABLED
    RegisterOGRElastic();
#endif
#ifdef WALK_ENABLED
    RegisterOGRWalk();
#endif
#ifdef CARTO_ENABLED
    RegisterOGRCarto();
#endif
#ifdef AMIGOCLOUD_ENABLED
    RegisterOGRAmigoCloud();
#endif
#ifdef SXF_ENABLED
    RegisterOGRSXF();
#endif
#ifdef SELAFIN_ENABLED
    RegisterOGRSelafin();
#endif
#ifdef JML_ENABLED
    RegisterOGRJML();
#endif
#ifdef PLSCENES_ENABLED
    RegisterOGRPLSCENES();
#endif
#ifdef CSW_ENABLED
    RegisterOGRCSW();
#endif
#ifdef MONGODBV3_ENABLED
    RegisterOGRMongoDBv3();
#endif
#ifdef MONGODB_ENABLED
    RegisterOGRMongoDB();
#endif
#ifdef VDV_ENABLED
    RegisterOGRVDV();
#endif
#ifdef GMLAS_ENABLED
    RegisterOGRGMLAS();
#endif
#ifdef MVT_ENABLED
    RegisterOGRMVT();
#endif
#ifdef NGW_ENABLED
    RegisterOGRNGW();
#endif // NGW_ENABLED
#ifdef MAPML_ENABLED
    RegisterOGRMapML();
#endif

/* Put TIGER and AVCBIN at end since they need poOpenInfo->GetSiblingFiles() */
#ifdef TIGER_ENABLED
    RegisterOGRTiger();
#endif
#ifdef AVC_ENABLED
    RegisterOGRAVCBin();
    RegisterOGRAVCE00();
#endif


} /* OGRRegisterAll */
