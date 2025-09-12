// TestProj.cpp
// CppUnitLite test harness for proj library.
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>

#include "CppUnitLite/TestHarness.h"	// CppUnitLite library
#include "../Common/UnitTest.h"			// unit test helpers

#include "proj.h"
#include "proj_internal.h"
#include <proj_api.h>

#include "proj.h"
#include "proj_constants.h"
#include "proj_experimental.h"

#include "proj/common.hpp"
#include "proj/coordinateoperation.hpp"
#include "proj/coordinatesystem.hpp"
#include "proj/crs.hpp"
#include "proj/datum.hpp"
#include "proj/io.hpp"
#include "proj/metadata.hpp"
#include "proj/util.hpp"

using namespace osgeo::proj::common;
using namespace osgeo::proj::crs;
using namespace osgeo::proj::cs;
using namespace osgeo::proj::datum;
using namespace osgeo::proj::io;
using namespace osgeo::proj::metadata;
using namespace osgeo::proj::operation;
using namespace osgeo::proj::util;

PJ_CONTEXT* m_ctx = nullptr;		// single context that may be reused

// intermediate replacements for google test macros
#define EXPECT_EQ(parm1, parm2) { CHECK(parm1 == parm2); }
#define ASSERT_EQ(parm1, parm2) { CHECK(parm1 == parm2); }
#define EXPECT_NE(parm1, parm2) { CHECK(parm1 != parm2); }
#define ASSERT_NE(parm1, parm2) { CHECK(parm1 != parm2); }
#define EXPECT_FALSE(parm1) { CHECK(parm1 == false); }

int main(int argc, char* argv[])
{
	// output name of executable
	if (argc > 0 && argv[0])
		printf("%s\n", argv[0]);

	//proj google tests did this - should i?
	// Use a potentially non-C locale to make sure we are robust
	//setlocale(LC_ALL, "");

	PJ_CONTEXT* m_ctx = proj_context_create();

	TestResult tr;
	TestRegistry::runAllTests(tr);

	proj_context_destroy(m_ctx);

	// always pause if errors
	int failureCount = tr.GetFailureCount();
	if (failureCount > 0)
		getc(stdin);

	return failureCount;
}

TEST(PROJ4, C_API)
{
	// proj.db must be accessible for these to work, usually
	// specified in PROJ_LIB environment variable

	{
		// proj_create

		proj_destroy(nullptr);

		CHECK(proj_create(m_ctx, "invalid") == nullptr);

		PJ* obj =
			proj_create(m_ctx, GeographicCRS::EPSG_4326
				->exportToWKT(WKTFormatter::create().get())
				.c_str());

		CHECK(obj != nullptr);

		// Check that functions that operate on 'non-C++' PJ don't crash
		PJ_COORD coord;
		coord.xyzt.x = 0;
		coord.xyzt.y = 0;
		coord.xyzt.z = 0;
		coord.xyzt.t = 0;
		DOUBLES_EQUAL(proj_trans(obj, PJ_FWD, coord).xyzt.x, std::numeric_limits<double>::infinity(), 0.0);
		DOUBLES_EQUAL(proj_geod(obj, coord, coord).xyzt.x, std::numeric_limits<double>::infinity(), 0.0);
		DOUBLES_EQUAL(proj_lp_dist(obj, coord, coord), std::numeric_limits<double>::infinity(), 0.0);

		auto info = proj_pj_info(obj);
		CHECK(info.id == nullptr);
		CHECK(info.description != nullptr);
		CHECK(info.description == std::string("WGS 84"));
		CHECK(info.definition != nullptr);
		CHECK(info.definition == std::string(""));
		proj_destroy(obj);

		{
			PJ* obj = proj_create(m_ctx, "EPSG:4326");
			CHECK(obj != nullptr);
			proj_destroy(obj);
		}
	}

	{
		// proj_create_from_wkt

		{
			CHECK(proj_create_from_wkt(m_ctx, "invalid", nullptr, nullptr, nullptr) == nullptr);
		}
		{
			PROJ_STRING_LIST warningList = nullptr;
			PROJ_STRING_LIST errorList = nullptr;
			PJ_CONTEXT* ctx = proj_context_create();
			CHECK(proj_create_from_wkt(ctx, "invalid", nullptr, &warningList, &errorList) == nullptr);
			CHECK(warningList == nullptr);
			proj_string_list_destroy(warningList);
			CHECK(errorList != nullptr);
			proj_string_list_destroy(errorList);
			proj_context_destroy(ctx);
		}
		{
			PJ* pj = proj_create_from_wkt(m_ctx, GeographicCRS::EPSG_4326->exportToWKT(WKTFormatter::create().get()).c_str(), nullptr, nullptr, nullptr);
			CHECK(pj != nullptr);
			proj_destroy(pj);
		}
		{
			//ERROR
			//PJ* pj = proj_create_from_wkt(
			//	m_ctx,
			//	"GEOGCS[\"WGS 84\",\n"
			//	"    DATUM[\"WGS_1984\",\n"
			//	"        SPHEROID[\"WGS 84\",6378137,298.257223563,\"unused\"]],\n"
			//	"    PRIMEM[\"Greenwich\",0],\n"
			//	"    UNIT[\"degree\",0.0174532925199433]]",
			//	nullptr, nullptr, nullptr);
			//CHECK(pj == nullptr);
			//proj_destroy(pj);
		}
		{
			PROJ_STRING_LIST warningList = nullptr;
			PROJ_STRING_LIST errorList = nullptr;
			PJ* pj = proj_create_from_wkt(
				m_ctx,
				"GEOGCS[\"WGS 84\",\n"
				"    DATUM[\"WGS_1984\",\n"
				"        SPHEROID[\"WGS 84\",6378137,298.257223563,\"unused\"]],\n"
				"    PRIMEM[\"Greenwich\",0],\n"
				"    UNIT[\"degree\",0.0174532925199433]]",
				nullptr, &warningList, &errorList);
			CHECK(pj != nullptr);
			CHECK(warningList == nullptr);
			proj_string_list_destroy(warningList);
			CHECK(errorList != nullptr);
			proj_string_list_destroy(errorList);
			proj_destroy(pj);
		}
		{
			PROJ_STRING_LIST warningList = nullptr;
			PROJ_STRING_LIST errorList = nullptr;
			const char* const options[] = { "STRICT=NO", nullptr };
			PJ* pj = proj_create_from_wkt(
				m_ctx,
				"GEOGCS[\"WGS 84\",\n"
				"    DATUM[\"WGS_1984\",\n"
				"        SPHEROID[\"WGS 84\",6378137,298.257223563,\"unused\"]],\n"
				"    PRIMEM[\"Greenwich\",0],\n"
				"    UNIT[\"degree\",0.0174532925199433]]",
				options, &warningList, &errorList);
			CHECK(pj != nullptr);
			CHECK(warningList == nullptr);
			proj_string_list_destroy(warningList);
			CHECK(errorList != nullptr);
			proj_string_list_destroy(errorList);
			proj_destroy(pj);
		}
		{
			PROJ_STRING_LIST warningList = nullptr;
			PROJ_STRING_LIST errorList = nullptr;
			PJ* pj = proj_create_from_wkt(
				m_ctx,
				GeographicCRS::EPSG_4326->exportToWKT(WKTFormatter::create().get())
				.c_str(),
				nullptr, &warningList, &errorList);
			CHECK(pj != nullptr);
			CHECK(warningList == nullptr);
			CHECK(errorList == nullptr);
			proj_destroy(pj);
		}
		// Warnings: missing projection parameters
		{
			PROJ_STRING_LIST warningList = nullptr;
			PROJ_STRING_LIST errorList = nullptr;
			PJ* pj = proj_create_from_wkt(
				m_ctx, 
				"PROJCS[\"test\",\n"
				"  GEOGCS[\"WGS 84\",\n"
				"    DATUM[\"WGS_1984\",\n"
				"        SPHEROID[\"WGS 84\",6378137,298.257223563]],\n"
				"    PRIMEM[\"Greenwich\",0],\n"
				"    UNIT[\"degree\",0.0174532925199433]],\n"
				"  PROJECTION[\"Transverse_Mercator\"],\n"
				"  PARAMETER[\"latitude_of_origin\",31],\n"
				"  UNIT[\"metre\",1]]",
				nullptr, &warningList, &errorList);
			CHECK(pj != nullptr);
			CHECK(warningList != nullptr);
			proj_string_list_destroy(warningList);
			CHECK(errorList == nullptr);
			proj_string_list_destroy(errorList);
			proj_destroy(pj);
		}
		{
			PJ* pj = proj_create_from_wkt(
				m_ctx, 
				"PROJCS[\"test\",\n"
				"  GEOGCS[\"WGS 84\",\n"
				"    DATUM[\"WGS_1984\",\n"
				"        SPHEROID[\"WGS 84\",6378137,298.257223563]],\n"
				"    PRIMEM[\"Greenwich\",0],\n"
				"    UNIT[\"degree\",0.0174532925199433]],\n"
				"  PROJECTION[\"Transverse_Mercator\"],\n"
				"  PARAMETER[\"latitude_of_origin\",31],\n"
				"  UNIT[\"metre\",1]]",
				nullptr, nullptr, nullptr);
			CHECK(pj != nullptr);
			proj_destroy(pj);
		}
	}

	{
		// proj_as_wkt

		auto obj = proj_create_from_wkt(
			m_ctx,
			GeographicCRS::EPSG_4326->exportToWKT(WKTFormatter::create().get())
			.c_str(),
			nullptr, nullptr, nullptr);
		CHECK(obj != nullptr);

		{
			const char* wkt = proj_as_wkt(m_ctx, obj, PJ_WKT2_2018, nullptr);
			CHECK(wkt != nullptr);
			CHECK(std::string(wkt).find("GEOGCRS[") == 0);
		}

		{
			const char* wkt = proj_as_wkt(m_ctx, obj, PJ_WKT2_2018_SIMPLIFIED, nullptr);
			CHECK(wkt != nullptr);
			CHECK(std::string(wkt).find("GEOGCRS[") == 0);
			CHECK(std::string(wkt).find("ANGULARUNIT[") == std::string::npos);
		}

		{
			const char* wkt = proj_as_wkt(m_ctx, obj, PJ_WKT2_2015, nullptr);
			CHECK(wkt != nullptr);
			CHECK(std::string(wkt).find("GEODCRS[") == 0);
		}

		{
			const char* wkt = proj_as_wkt(m_ctx, obj, PJ_WKT2_2015_SIMPLIFIED, nullptr);
			CHECK(wkt != nullptr);
			CHECK(std::string(wkt).find("GEODCRS[") == 0);
			CHECK(std::string(wkt).find("ANGULARUNIT[") == std::string::npos);
		}

		{
			const char* wkt = proj_as_wkt(m_ctx, obj, PJ_WKT1_GDAL, nullptr);
			CHECK(wkt != nullptr);
			CHECK(std::string(wkt).find("GEOGCS[\"WGS 84\"") == 0);
		}

		{
			const char* wkt = proj_as_wkt(m_ctx, obj, PJ_WKT1_ESRI, nullptr);
			CHECK(wkt != nullptr);
			CHECK(std::string(wkt).find("GEOGCS[\"GCS_WGS_1984\"") == 0);
		}

		{
			// MULTILINE=NO
			const char* const options[] = { "MULTILINE=NO", nullptr };
			const char* wkt = proj_as_wkt(m_ctx, obj, PJ_WKT1_GDAL, options);
			CHECK(wkt != nullptr);
			CHECK(std::string(wkt).find("\n") == std::string::npos);
		}

		{
			// INDENTATION_WIDTH=2
			const char* const options[] = { "INDENTATION_WIDTH=2", nullptr };
			const char* wkt = proj_as_wkt(m_ctx, obj, PJ_WKT1_GDAL, options);
			CHECK(wkt != nullptr);
			CHECK(std::string(wkt).find("\n  DATUM") != std::string::npos);
		}

		{
			// OUTPUT_AXIS=NO
			const char* const options[] = { "OUTPUT_AXIS=NO", nullptr };
			const char* wkt = proj_as_wkt(m_ctx, obj, PJ_WKT1_GDAL, options);
			CHECK(wkt != nullptr);
			CHECK(std::string(wkt).find("AXIS") == std::string::npos);
		}

		{
			// OUTPUT_AXIS=AUTO
			const char* const options[] = { "OUTPUT_AXIS=AUTO", nullptr };
			const char* wkt = proj_as_wkt(m_ctx, obj, PJ_WKT1_GDAL, options);
			CHECK(wkt != nullptr);
			CHECK(std::string(wkt).find("AXIS") == std::string::npos);
		}

		{
			// OUTPUT_AXIS=YES
			const char* const options[] = { "OUTPUT_AXIS=YES", nullptr };
			const char* wkt = proj_as_wkt(m_ctx, obj, PJ_WKT1_GDAL, options);
			CHECK(wkt != nullptr);
			CHECK(std::string(wkt).find("AXIS") != std::string::npos);
		}

		PJ* crs4979 = proj_create_from_wkt(
			m_ctx,
			GeographicCRS::EPSG_4979->exportToWKT(WKTFormatter::create().get())
			.c_str(),
			nullptr, nullptr, nullptr);
		CHECK(crs4979 != nullptr);

		{
			// STRICT=NO
			CHECK(proj_as_wkt(m_ctx, crs4979, PJ_WKT1_GDAL, nullptr) == nullptr);

			const char* const options[] = { "STRICT=NO", nullptr };
			const char* wkt = proj_as_wkt(m_ctx, crs4979, PJ_WKT1_GDAL, options);
			CHECK(wkt != nullptr);
			CHECK(std::string(wkt).find("GEOGCS[\"WGS 84\"") == 0);
		}

		{
			// unsupported option
			const char* const options[] = { "unsupported=yes", nullptr };
			const char* wkt = proj_as_wkt(m_ctx, obj, PJ_WKT2_2018, options);
			CHECK(wkt == nullptr);
		}

		proj_destroy(crs4979);
		proj_destroy(obj);
	}

	{
		// proj_as_proj_string

		PJ* pj = proj_create_from_wkt(
			m_ctx,
			GeographicCRS::EPSG_4326->exportToWKT(WKTFormatter::create().get())
			.c_str(),
			nullptr, nullptr, nullptr);

		CHECK(pj != nullptr);

		{
			auto proj_5 = proj_as_proj_string(m_ctx, pj, PJ_PROJ_5, nullptr);
			CHECK(proj_5 != nullptr);
			CHECK(std::string(proj_5) == "+proj=longlat +datum=WGS84 +no_defs +type=crs");
		}
		{
			auto proj_4 = proj_as_proj_string(m_ctx, pj, PJ_PROJ_4, nullptr);
			CHECK(proj_4 != nullptr);
			CHECK(std::string(proj_4) == "+proj=longlat +datum=WGS84 +no_defs +type=crs");
		}
		proj_destroy(pj);
	}

	{
		// proj_as_proj_string_approx_tmerc_option_yes

		PJ* pj = proj_create(m_ctx, "+proj=tmerc +type=crs");

		const char* options[] = { "USE_APPROX_TMERC=YES", nullptr };
		auto str = proj_as_proj_string(m_ctx, pj, PJ_PROJ_4, options);
		CHECK(str != nullptr);
		CHECK(str ==
			std::string("+proj=tmerc +approx +lat_0=0 +lon_0=0 +k=1 +x_0=0 "
						"+y_0=0 +datum=WGS84 +units=m +no_defs +type=crs"));
		proj_destroy(pj);
	}

	{
		// proj_crs_create_bound_crs_to_WGS84

		auto crs = proj_create_from_database(m_ctx, "EPSG", "3844", PJ_CATEGORY_CRS, false, nullptr);
		CHECK(crs != nullptr);

		auto res = proj_crs_create_bound_crs_to_WGS84(m_ctx, crs, nullptr);
		ASSERT_NE(res, nullptr);

		auto proj_4 = proj_as_proj_string(m_ctx, res, PJ_PROJ_4, nullptr);
		CHECK(proj_4 != nullptr);
		CHECK(std::string(proj_4) ==
			"+proj=sterea +lat_0=46 +lon_0=25 +k=0.99975 +x_0=500000 "
			"+y_0=500000 +ellps=krass "
			"+towgs84=2.329,-147.042,-92.08,-0.309,0.325,0.497,5.69 "
			"+units=m +no_defs +type=crs");

		auto base_crs = proj_get_source_crs(m_ctx, res);
		CHECK(base_crs != nullptr);

		auto hub_crs = proj_get_target_crs(m_ctx, res);
		CHECK(hub_crs != nullptr);

		auto transf = proj_crs_get_coordoperation(m_ctx, res);
		CHECK(transf != nullptr);

		std::vector<double> values(7, 0);
		CHECK(proj_coordoperation_get_towgs84_values(m_ctx, transf, values.data(), 7, true));
		auto expected = std::vector<double>{ 2.329, -147.042, -92.08, -0.309, 0.325, 0.497, 5.69 };
		CHECK(values == expected);

		auto res2 = proj_crs_create_bound_crs(m_ctx, base_crs, hub_crs, transf);
		CHECK(res2 != nullptr);

		CHECK(proj_is_equivalent_to(res, res2, PJ_COMP_STRICT));

		proj_destroy(base_crs);
		proj_destroy(hub_crs);
		proj_destroy(transf);
		proj_destroy(res2);
		proj_destroy(res);
		proj_destroy(crs);
	}

	{
		// proj_get_name
		
		auto obj = proj_create_from_wkt(
			m_ctx,
			GeographicCRS::EPSG_4326->exportToWKT(WKTFormatter::create().get()).c_str(),
			nullptr, nullptr, nullptr);
		CHECK(obj != nullptr);
		auto name = proj_get_name(obj);
		CHECK(name != nullptr);
		CHECK(name == std::string("WGS 84"));
		CHECK(name == proj_get_name(obj));
		proj_destroy(obj);
	}

	{
		// proj_get_id_auth_name

		PJ* obj = proj_create_from_wkt(
			m_ctx,
			GeographicCRS::EPSG_4326->exportToWKT(WKTFormatter::create().get()).c_str(),
			nullptr, nullptr, nullptr);
		CHECK(obj != nullptr);
		auto auth = proj_get_id_auth_name(obj, 0);
		CHECK(auth != nullptr);
		CHECK(auth == std::string("EPSG"));
		CHECK(auth == proj_get_id_auth_name(obj, 0));
		CHECK(proj_get_id_auth_name(obj, -1) == nullptr);
		CHECK(proj_get_id_auth_name(obj, 1) == nullptr);
		proj_destroy(obj);
	}

	{
		// proj_get_id_code

		PJ* obj = proj_create_from_wkt(
			m_ctx,
			GeographicCRS::EPSG_4326->exportToWKT(WKTFormatter::create().get()).c_str(),
			nullptr, nullptr, nullptr);

		CHECK(obj != nullptr);
		auto code = proj_get_id_code(obj, 0);
		CHECK(code != nullptr);
		CHECK(code == std::string("4326"));
		CHECK(code == proj_get_id_code(obj, 0));
		CHECK(proj_get_id_code(obj, -1) == nullptr);
		CHECK(proj_get_id_code(obj, 1) == nullptr);

		proj_destroy(obj);
	}

	{
		// proj_get_type

		{
			PJ* pj = proj_create_from_wkt(
				m_ctx,
				GeographicCRS::EPSG_4326->exportToWKT(WKTFormatter::create().get()).c_str(),
				nullptr, nullptr, nullptr);
			CHECK(pj != nullptr);
			EXPECT_EQ(proj_get_type(pj), PJ_TYPE_GEOGRAPHIC_2D_CRS);
			proj_destroy(pj);
		}
		{
			PJ* pj = proj_create_from_wkt(
				m_ctx,
				GeographicCRS::EPSG_4979->exportToWKT(WKTFormatter::create().get()).c_str(),
				nullptr, nullptr, nullptr);
			CHECK(pj != nullptr);
			EXPECT_EQ(proj_get_type(pj), PJ_TYPE_GEOGRAPHIC_3D_CRS);
			proj_destroy(pj);
		}
		{
			PJ* pj = proj_create_from_wkt(
				m_ctx,
				GeographicCRS::EPSG_4978->exportToWKT(WKTFormatter::create().get()).c_str(),
				nullptr, nullptr, nullptr);
			CHECK(pj != nullptr);
			EXPECT_EQ(proj_get_type(pj), PJ_TYPE_GEOCENTRIC_CRS);
			proj_destroy(pj);
		}
		{
			PJ* pj = proj_create_from_wkt(
				m_ctx, GeographicCRS::EPSG_4326->datum()
				->exportToWKT(WKTFormatter::create().get()).c_str(),
				nullptr, nullptr, nullptr);
			CHECK(pj != nullptr);
			EXPECT_EQ(proj_get_type(pj), PJ_TYPE_GEODETIC_REFERENCE_FRAME);
			proj_destroy(pj);
		}
		{
			PJ* pj = proj_create_from_wkt(
				m_ctx, 
				GeographicCRS::EPSG_4326->ellipsoid()->exportToWKT(WKTFormatter::create().get()).c_str(),
				nullptr, nullptr, nullptr);
			CHECK(pj != nullptr);
			EXPECT_EQ(proj_get_type(pj), PJ_TYPE_ELLIPSOID);
			proj_destroy(pj);
		}
		{
			//proj_create_from_wkt: Parsing error : syntax error, unexpected $undefined.Error occurred around :
			//  AUTHORITY["EPSG", 4326]
			//  ^
			auto obj = proj_create_from_wkt(m_ctx, "AUTHORITY[\"EPSG\", 4326]",
				nullptr, nullptr, nullptr);
			ASSERT_EQ(obj, nullptr);
			proj_destroy(obj);
		}
	}

	{
		// conversion

		PJ* crs = proj_create_from_database(m_ctx, "EPSG", "32631", PJ_CATEGORY_CRS, false, nullptr);
		CHECK(crs != nullptr);

		// invalid object type
		EXPECT_FALSE(proj_coordoperation_get_method_info(m_ctx, crs, nullptr, nullptr, nullptr));

		{
			auto conv = proj_crs_get_coordoperation(m_ctx, crs);
			ASSERT_NE(conv, nullptr);
			ASSERT_EQ(proj_crs_get_coordoperation(m_ctx, conv), nullptr);
			proj_destroy(conv);
		}

		auto conv = proj_crs_get_coordoperation(m_ctx, crs);
		ASSERT_NE(conv, nullptr);

		CHECK(proj_coordoperation_get_method_info(m_ctx, conv, nullptr,	nullptr, nullptr));

		const char* methodName = nullptr;
		const char* methodAuthorityName = nullptr;
		const char* methodCode = nullptr;
		CHECK(proj_coordoperation_get_method_info(m_ctx, conv, &methodName, &methodAuthorityName, &methodCode));

		ASSERT_NE(methodName, nullptr);
		ASSERT_NE(methodAuthorityName, nullptr);
		ASSERT_NE(methodCode, nullptr);
		EXPECT_EQ(methodName, std::string("Transverse Mercator"));
		EXPECT_EQ(methodAuthorityName, std::string("EPSG"));
		EXPECT_EQ(methodCode, std::string("9807"));

		EXPECT_EQ(proj_coordoperation_get_param_count(m_ctx, conv), 5);
		EXPECT_EQ(proj_coordoperation_get_param_index(m_ctx, conv, "foo"), -1);
		EXPECT_EQ(proj_coordoperation_get_param_index(m_ctx, conv, "False easting"), 3);

		EXPECT_FALSE(proj_coordoperation_get_param(
				m_ctx, conv, -1, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
				nullptr, nullptr, nullptr, nullptr));
		EXPECT_FALSE(proj_coordoperation_get_param(
				m_ctx, conv, 5, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
				nullptr, nullptr, nullptr, nullptr));

		const char* name = nullptr;
		const char* nameAuthorityName = nullptr;
		const char* nameCode = nullptr;
		double value = 0;
		const char* valueString = nullptr;
		double valueUnitConvFactor = 0;
		const char* valueUnitName = nullptr;
		const char* unitAuthName = nullptr;
		const char* unitCode = nullptr;
		const char* unitCategory = nullptr;
		CHECK(proj_coordoperation_get_param(
				m_ctx, conv, 3, &name, &nameAuthorityName, &nameCode, &value,
				&valueString, &valueUnitConvFactor, &valueUnitName, &unitAuthName,
				&unitCode, &unitCategory));
		ASSERT_NE(name, nullptr);
		ASSERT_NE(nameAuthorityName, nullptr);
		ASSERT_NE(nameCode, nullptr);
		EXPECT_EQ(valueString, nullptr);
		ASSERT_NE(valueUnitName, nullptr);
		ASSERT_NE(unitAuthName, nullptr);
		ASSERT_NE(unitCategory, nullptr);
		ASSERT_NE(unitCategory, nullptr);
		EXPECT_EQ(name, std::string("False easting"));
		EXPECT_EQ(nameAuthorityName, std::string("EPSG"));
		EXPECT_EQ(nameCode, std::string("8806"));
		EXPECT_EQ(value, 500000.0);
		EXPECT_EQ(valueUnitConvFactor, 1.0);
		EXPECT_EQ(valueUnitName, std::string("metre"));
		EXPECT_EQ(unitAuthName, std::string("EPSG"));
		EXPECT_EQ(unitCode, std::string("9001"));
		EXPECT_EQ(unitCategory, std::string("linear"));
		
		proj_destroy(conv);
		proj_destroy(crs);
	}

	{
		// proj_coordoperation_get_grid_used

		auto op = proj_create_from_database(m_ctx, "EPSG", "1312",
			PJ_CATEGORY_COORDINATE_OPERATION, true,
			nullptr);
		CHECK(op != nullptr);

		EXPECT_EQ(proj_coordoperation_get_grid_used_count(m_ctx, op), 1);
		const char* shortName = nullptr;
		const char* fullName = nullptr;
		const char* packageName = nullptr;
		const char* url = nullptr;
		int directDownload = 0;
		int openLicense = 0;
		int available = 0;
		EXPECT_EQ(proj_coordoperation_get_grid_used(m_ctx, op, -1, nullptr,
			nullptr, nullptr, nullptr,
			nullptr, nullptr, nullptr),
			0);
		EXPECT_EQ(proj_coordoperation_get_grid_used(m_ctx, op, 1, nullptr, nullptr,
			nullptr, nullptr, nullptr,
			nullptr, nullptr),
			0);
		EXPECT_EQ(proj_coordoperation_get_grid_used(
			m_ctx, op, 0, &shortName, &fullName, &packageName, &url,
			&directDownload, &openLicense, &available),
			1);
		ASSERT_NE(shortName, nullptr);
		ASSERT_NE(fullName, nullptr);
		ASSERT_NE(packageName, nullptr);
		ASSERT_NE(url, nullptr);
		//EXPECT_EQ(shortName, std::string("ntv1_can.dat"));
		// EXPECT_EQ(fullName, std::string(""));
		//EXPECT_EQ(packageName, std::string("proj-datumgrid"));
		//CHECK(std::string(url).find("https://download.osgeo.org/proj/proj-datumgrid-") == 0);
		EXPECT_EQ(directDownload, 1);
		EXPECT_EQ(openLicense, 1);

		proj_destroy(op);
	}

	{
		// proj_coordoperation_get_accuracy
		{
			auto crs = proj_create_from_database(m_ctx, "EPSG", "4326",	PJ_CATEGORY_CRS, false, nullptr);
			CHECK(crs != nullptr);
			EXPECT_EQ(proj_coordoperation_get_accuracy(m_ctx, crs), -1.0);
			proj_destroy(crs);
		}
		{
			auto obj = proj_create_from_database(m_ctx, "EPSG", "1170",	PJ_CATEGORY_COORDINATE_OPERATION, false, nullptr);
			CHECK(obj != nullptr);
			EXPECT_EQ(proj_coordoperation_get_accuracy(m_ctx, obj), 16.0);
			proj_destroy(obj);
		}
		{
			auto obj = proj_create(m_ctx, "+proj=helmert");
			CHECK(obj != nullptr);
			EXPECT_EQ(proj_coordoperation_get_accuracy(m_ctx, obj), -1.0);
			proj_destroy(obj);
		}
	}

	{
		// proj_create_geographic_crs

		auto cs = proj_create_ellipsoidal_2D_cs(m_ctx, PJ_ELLPS2D_LATITUDE_LONGITUDE, nullptr, 0);
		CHECK(cs != nullptr);

		{
			auto obj = proj_create_geographic_crs(
				m_ctx, "WGS 84", "World Geodetic System 1984", "WGS 84", 6378137,
				298.257223563, "Greenwich", 0.0, "Degree", 0.0174532925199433, cs);
			CHECK(obj != nullptr);

			auto objRef =
				proj_create(m_ctx, GeographicCRS::EPSG_4326
					->exportToWKT(WKTFormatter::create().get())
					.c_str());
			EXPECT_NE(objRef, nullptr);

			CHECK(proj_is_equivalent_to(obj, objRef, PJ_COMP_EQUIVALENT));

			auto datum = proj_crs_get_datum(m_ctx, obj);
			CHECK(datum != nullptr);

			auto obj2 =
				proj_create_geographic_crs_from_datum(m_ctx, "WGS 84", datum, cs);
			ASSERT_NE(obj2, nullptr);

			CHECK(proj_is_equivalent_to(obj, obj2, PJ_COMP_STRICT));

			proj_destroy(obj);
			proj_destroy(objRef);
			proj_destroy(datum);
			proj_destroy(obj2);
		}
		{
			auto obj =
				proj_create_geographic_crs(m_ctx, nullptr, nullptr, nullptr, 1.0,
					0.0, nullptr, 0.0, nullptr, 0.0, cs);
			CHECK(obj != nullptr);
			proj_destroy(obj);
		}

		proj_destroy(cs);
	}

	{
		// proj_create_geocentric_crs
		{
			auto obj = proj_create_geocentric_crs(
				m_ctx, "WGS 84", "World Geodetic System 1984", "WGS 84", 6378137,
				298.257223563, "Greenwich", 0.0, "Degree", 0.0174532925199433,
				"Metre", 1.0);
			CHECK(obj != nullptr);

			auto objRef =
				proj_create(m_ctx, GeographicCRS::EPSG_4978
					->exportToWKT(WKTFormatter::create().get())
					.c_str());
			EXPECT_NE(objRef, nullptr);

			CHECK(proj_is_equivalent_to(obj, objRef, PJ_COMP_EQUIVALENT));

			auto datum = proj_crs_get_datum(m_ctx, obj);
			CHECK(datum != nullptr);

			auto obj2 = proj_create_geocentric_crs_from_datum(m_ctx, "WGS 84",
				datum, "Metre", 1.0);
			ASSERT_NE(obj2, nullptr);

			CHECK(proj_is_equivalent_to(obj, obj2, PJ_COMP_STRICT));

			proj_destroy(obj);
			proj_destroy(objRef);
			proj_destroy(datum);
			proj_destroy(obj2);
		}
		{
			auto obj = proj_create_geocentric_crs(m_ctx, nullptr, nullptr, nullptr,
				1.0, 0.0, nullptr, 0.0, nullptr,
				0.0, nullptr, 0.0);
			CHECK(obj != nullptr);
			proj_destroy(obj);
		}
	}

	{
		// check_coord_op_obj_can_be_used_with_proj_trans

		{
			auto projCRS = proj_create_conversion_utm(m_ctx, 31, true);
			ASSERT_NE(projCRS, nullptr);

			PJ_COORD coord;
			coord.xyzt.x = proj_torad(3.0);
			coord.xyzt.y = 0;
			coord.xyzt.z = 0;
			coord.xyzt.t = 0;
			DOUBLES_EQUAL(proj_trans(projCRS, PJ_FWD, coord).xyzt.x, 500000.0, 1e-9);

			proj_destroy(projCRS);
		}
	}

	{
		// proj_cs_get_axis_info
		
		{
			auto crs = proj_create_from_database(m_ctx, "EPSG", "4326", PJ_CATEGORY_CRS, false, nullptr);
			CHECK(crs != nullptr);

			auto cs = proj_crs_get_coordinate_system(m_ctx, crs);
			CHECK(cs != nullptr);

			EXPECT_EQ(proj_cs_get_type(m_ctx, cs), PJ_CS_TYPE_ELLIPSOIDAL);

			EXPECT_EQ(proj_cs_get_axis_count(m_ctx, cs), 2);

			EXPECT_FALSE(proj_cs_get_axis_info(m_ctx, cs, -1, nullptr, nullptr,
				nullptr, nullptr, nullptr, nullptr,
				nullptr));

			EXPECT_FALSE(proj_cs_get_axis_info(m_ctx, cs, 2, nullptr, nullptr,
				nullptr, nullptr, nullptr, nullptr,
				nullptr));

			CHECK(proj_cs_get_axis_info(m_ctx, cs, 0, nullptr, nullptr,
				nullptr, nullptr, nullptr, nullptr,
				nullptr));

			const char* name = nullptr;
			const char* abbrev = nullptr;
			const char* direction = nullptr;
			double unitConvFactor = 0.0;
			const char* unitName = nullptr;
			const char* unitAuthority = nullptr;
			const char* unitCode = nullptr;

			CHECK(proj_cs_get_axis_info(
				m_ctx, cs, 0, &name, &abbrev, &direction, &unitConvFactor,
				&unitName, &unitAuthority, &unitCode));
			ASSERT_NE(name, nullptr);
			ASSERT_NE(abbrev, nullptr);
			ASSERT_NE(direction, nullptr);
			ASSERT_NE(unitName, nullptr);
			ASSERT_NE(unitAuthority, nullptr);
			ASSERT_NE(unitCode, nullptr);
			EXPECT_EQ(std::string(name), "Geodetic latitude");
			EXPECT_EQ(std::string(abbrev), "Lat");
			EXPECT_EQ(std::string(direction), "north");
			EXPECT_EQ(unitConvFactor, 0.017453292519943295);
			EXPECT_EQ(std::string(unitName), "degree");
			EXPECT_EQ(std::string(unitAuthority), "EPSG");
			EXPECT_EQ(std::string(unitCode), "9122");

			proj_destroy(cs);
			proj_destroy(crs);
		}

		{
			// Non CRS object
			auto obj = proj_create_from_database(m_ctx, "EPSG", "1170",
				PJ_CATEGORY_COORDINATE_OPERATION,
				false, nullptr);
			CHECK(obj != nullptr);

			EXPECT_EQ(proj_crs_get_coordinate_system(m_ctx, obj), nullptr);
			EXPECT_EQ(proj_cs_get_type(m_ctx, obj), PJ_CS_TYPE_UNKNOWN);
			EXPECT_EQ(proj_cs_get_axis_count(m_ctx, obj), -1);
			EXPECT_FALSE(proj_cs_get_axis_info(m_ctx, obj, 0, nullptr, nullptr,
				nullptr, nullptr, nullptr, nullptr,
				nullptr));

			proj_destroy(obj);
		}
	}

	{
		// ObjectKeeper undefined

		// proj_get_non_deprecated

		//	auto crs = proj_create_from_database(m_ctx, "EPSG", "4226",
		//		PJ_CATEGORY_CRS, false, nullptr);
		//	ObjectKeeper keeper(crs);
		//	CHECK(crs != nullptr);

		//	auto list = proj_get_non_deprecated(m_ctx, crs);
		//	ASSERT_NE(list, nullptr);
		//	ObjListKeeper keeper_list(list);
		//	EXPECT_EQ(proj_list_get_count(list), 2);
	}

	{
		// ObjectKeeper undefined

		// proj_query_geodetic_crs_from_datum

		//	{
		//		auto list = proj_query_geodetic_crs_from_datum(m_ctx, nullptr, "EPSG",
		//			"6326", nullptr);
		//		ASSERT_NE(list, nullptr);
		//		ObjListKeeper keeper_list(list);
		//		EXPECT_GE(proj_list_get_count(list), 3);
		//	}
		//	{
		//		auto list = proj_query_geodetic_crs_from_datum(m_ctx, "EPSG", "EPSG",
		//			"6326", "geographic 2D");
		//		ASSERT_NE(list, nullptr);
		//		ObjListKeeper keeper_list(list);
		//		EXPECT_EQ(proj_list_get_count(list), 1);
		//	}
	}

	{
		// proj_create_cartesian_2D_cs

		{
			auto cs = proj_create_cartesian_2D_cs(m_ctx, PJ_CART2D_EASTING_NORTHING, nullptr, 0);
			CHECK(cs != nullptr);
			proj_destroy(cs);
		}
		{
			auto cs = proj_create_cartesian_2D_cs(m_ctx, PJ_CART2D_NORTHING_EASTING, nullptr, 0);
			CHECK(cs != nullptr);
			proj_destroy(cs);
		}
		{
			auto cs = proj_create_cartesian_2D_cs(m_ctx, PJ_CART2D_NORTH_POLE_EASTING_SOUTH_NORTHING_SOUTH, nullptr, 0);
			CHECK(cs != nullptr);
			proj_destroy(cs);
		}
		{
			auto cs = proj_create_cartesian_2D_cs(m_ctx, PJ_CART2D_SOUTH_POLE_EASTING_NORTH_NORTHING_NORTH, nullptr, 0);
			CHECK(cs != nullptr);
			proj_destroy(cs);
		}
		{
			auto cs = proj_create_cartesian_2D_cs(m_ctx, PJ_CART2D_WESTING_SOUTHING, nullptr, 0);
			CHECK(cs != nullptr);
			proj_destroy(cs);
		}
	}

	{
		// proj_normalize_for_visualization

		{
			auto P = proj_create(m_ctx, "+proj=utm +zone=31 +ellps=WGS84");
			ASSERT_NE(P, nullptr);
			auto Pnormalized = proj_normalize_for_visualization(m_ctx, P);
			EXPECT_EQ(Pnormalized, nullptr);

			proj_destroy(P);
			proj_destroy(Pnormalized);
		}

		auto P = proj_create_crs_to_crs(m_ctx, "EPSG:4326", "EPSG:32631", nullptr);
		ASSERT_NE(P, nullptr);
		auto Pnormalized = proj_normalize_for_visualization(m_ctx, P);
		ASSERT_NE(Pnormalized, nullptr);
		auto projstr = proj_as_proj_string(m_ctx, Pnormalized, PJ_PROJ_5, nullptr);
		ASSERT_NE(projstr, nullptr);
		EXPECT_EQ(std::string(projstr),
			"+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad "
			"+step +proj=utm +zone=31 +ellps=WGS84");

		proj_destroy(P);
		proj_destroy(Pnormalized);
	}
}

TEST(PROJ4, Common)
{
	{
		// unit_of_measure

		EXPECT_EQ(UnitOfMeasure::METRE.name(), "metre");

		EXPECT_EQ(UnitOfMeasure::METRE.conversionToSI(), 1.0);
		EXPECT_EQ(UnitOfMeasure::METRE.type(), UnitOfMeasure::Type::LINEAR);

		EXPECT_EQ(UnitOfMeasure::DEGREE.name(), "degree");
		EXPECT_EQ(UnitOfMeasure::DEGREE.conversionToSI(), 0.017453292519943295);
		EXPECT_EQ(UnitOfMeasure::DEGREE.type(), UnitOfMeasure::Type::ANGULAR);

		EXPECT_EQ(UnitOfMeasure::RADIAN.name(), "radian");
		EXPECT_EQ(UnitOfMeasure::RADIAN.conversionToSI(), 1.0);
		EXPECT_EQ(UnitOfMeasure::RADIAN.type(), UnitOfMeasure::Type::ANGULAR);

		EXPECT_EQ(Length(2.0, UnitOfMeasure("km", 1000.0))
			.convertToUnit(UnitOfMeasure::METRE),
			2000.0);

		EXPECT_EQ(
			Angle(2.0, UnitOfMeasure::DEGREE).convertToUnit(UnitOfMeasure::RADIAN),
			2 * 0.017453292519943295);

		EXPECT_EQ(Angle(2.5969213, UnitOfMeasure::GRAD)
			.convertToUnit(UnitOfMeasure::DEGREE),
			2.5969213 / 100.0 * 90.0);
	}

	{
		// measure

		//DOUBLES_EQUAL(Measure(1.0), Measure(1.0), 0.0);
	}

	{
		// identifiedobject_empty

		PropertyMap properties;
		auto obj = OperationParameter::create(properties);
		CHECK(obj->name()->code().empty());
		CHECK(obj->identifiers().empty());
		CHECK(obj->aliases().empty());
		CHECK(obj->remarks().empty());
		CHECK(!obj->isDeprecated());
		CHECK(obj->alias().empty());
	}

	{
		// identifiedobject

		PropertyMap properties;
		properties.set(IdentifiedObject::NAME_KEY, "name");
		properties.set(IdentifiedObject::IDENTIFIERS_KEY, Identifier::create("identifier_code"));
		properties.set(IdentifiedObject::ALIAS_KEY, "alias");
		properties.set(IdentifiedObject::REMARKS_KEY, "remarks");
		properties.set(IdentifiedObject::DEPRECATED_KEY, true);
		auto obj = OperationParameter::create(properties);
		EXPECT_EQ(*(obj->name()->description()), "name");
		ASSERT_EQ(obj->identifiers().size(), 1U);
		EXPECT_EQ(obj->identifiers()[0]->code(), "identifier_code");
		ASSERT_EQ(obj->aliases().size(), 1U);
		EXPECT_EQ(obj->aliases()[0]->toString(), "alias");
		EXPECT_EQ(obj->remarks(), "remarks");
		CHECK(obj->isDeprecated());
	}

	{
		// identifiedobject_identifier_array_of_identifier

		PropertyMap properties;
		auto array = ArrayOfBaseObject::create();
		array->add(Identifier::create("identifier_code1"));
		array->add(Identifier::create("identifier_code2"));
		properties.set(IdentifiedObject::IDENTIFIERS_KEY, array);
		auto obj = OperationParameter::create(properties);
		ASSERT_EQ(obj->identifiers().size(), 2U);
		EXPECT_EQ(obj->identifiers()[0]->code(), "identifier_code1");
		EXPECT_EQ(obj->identifiers()[1]->code(), "identifier_code2");
	}

	{
		// identifiedobject_alias_array_of_string

		PropertyMap properties;
		properties.set(IdentifiedObject::ALIAS_KEY, std::vector<std::string>{"alias1", "alias2"});
		auto obj = OperationParameter::create(properties);
		ASSERT_EQ(obj->aliases().size(), 2U);
		EXPECT_EQ(obj->aliases()[0]->toString(), "alias1");
		EXPECT_EQ(obj->aliases()[1]->toString(), "alias2");
	}

	{
		// DataEpoch

		DataEpoch epochSrc(Measure(2010.5, UnitOfMeasure::YEAR));
		DataEpoch epoch(epochSrc);
		EXPECT_EQ(epoch.coordinateEpoch().value(), 2010.5);
		EXPECT_EQ(epoch.coordinateEpoch().unit(), UnitOfMeasure::YEAR);
	}
}

TEST(PROJ4, crs)
{
	{
		// EPSG_4326_get_components

		auto crs = GeographicCRS::EPSG_4326;
		ASSERT_EQ(crs->identifiers().size(), 1U);
		EXPECT_EQ(crs->identifiers()[0]->code(), "4326");
		EXPECT_EQ(*(crs->identifiers()[0]->codeSpace()), "EPSG");
		EXPECT_EQ(crs->nameStr(), "WGS 84");

		auto datum = crs->datum();
		ASSERT_EQ(datum->identifiers().size(), 1U);
		EXPECT_EQ(datum->identifiers()[0]->code(), "6326");
		EXPECT_EQ(*(datum->identifiers()[0]->codeSpace()), "EPSG");
		EXPECT_EQ(datum->nameStr(), "World Geodetic System 1984");

		auto ellipsoid = datum->ellipsoid();
		EXPECT_EQ(ellipsoid->semiMajorAxis().value(), 6378137.0);
		EXPECT_EQ(ellipsoid->semiMajorAxis().unit(), UnitOfMeasure::METRE);
		EXPECT_EQ(ellipsoid->inverseFlattening()->value(), 298.257223563);
		ASSERT_EQ(ellipsoid->identifiers().size(), 1U);
		EXPECT_EQ(ellipsoid->identifiers()[0]->code(), "7030");
		EXPECT_EQ(*(ellipsoid->identifiers()[0]->codeSpace()), "EPSG");
		EXPECT_EQ(ellipsoid->nameStr(), "WGS 84");
	}

	{
		// GeographicCRS_isEquivalentTo

		auto crs = GeographicCRS::EPSG_4326;
		CHECK(crs->isEquivalentTo(crs.get()));
		CHECK(crs->isEquivalentTo(crs.get(), IComparable::Criterion::EQUIVALENT));
		CHECK(crs->isEquivalentTo(
			crs.get(),
			IComparable::Criterion::EQUIVALENT_EXCEPT_AXIS_ORDER_GEOGCRS));
		CHECK(crs->shallowClone()->isEquivalentTo(crs.get()));

		//EXPECT_FALSE(crs->isEquivalentTo(createUnrelatedObject().get()));
		EXPECT_FALSE(crs->isEquivalentTo(GeographicCRS::EPSG_4979.get()));
		EXPECT_FALSE(crs->isEquivalentTo(GeographicCRS::EPSG_4979.get(),
			IComparable::Criterion::EQUIVALENT));

		EXPECT_FALSE(crs->isEquivalentTo(GeographicCRS::OGC_CRS84.get(),
			IComparable::Criterion::EQUIVALENT));
		CHECK(crs->isEquivalentTo(
			GeographicCRS::OGC_CRS84.get(),
			IComparable::Criterion::EQUIVALENT_EXCEPT_AXIS_ORDER_GEOGCRS));
		CHECK(GeographicCRS::OGC_CRS84->isEquivalentTo(
			crs.get(),
			IComparable::Criterion::EQUIVALENT_EXCEPT_AXIS_ORDER_GEOGCRS));

		EXPECT_FALSE(
			GeographicCRS::create(
				PropertyMap(), GeodeticReferenceFrame::EPSG_6326,
				EllipsoidalCS::createLatitudeLongitude(UnitOfMeasure::DEGREE))
			->isEquivalentTo(crs.get()));

		EXPECT_FALSE(
			GeographicCRS::create(
				PropertyMap(), GeodeticReferenceFrame::EPSG_6326,
				EllipsoidalCS::createLatitudeLongitude(UnitOfMeasure::DEGREE))
			->isEquivalentTo(
				GeographicCRS::create(PropertyMap(),
					GeodeticReferenceFrame::create(
						PropertyMap(), Ellipsoid::WGS84,
						optional<std::string>(),
						PrimeMeridian::GREENWICH),
					EllipsoidalCS::createLatitudeLongitude(
						UnitOfMeasure::DEGREE))
				.get()));
	}

	{
		// GeographicCRS_datum_ensemble

		auto ensemble_vdatum = DatumEnsemble::create(
			PropertyMap(),
			std::vector<DatumNNPtr>{GeodeticReferenceFrame::EPSG_6326,
			GeodeticReferenceFrame::EPSG_6326},
			PositionalAccuracy::create("100"));
		{
			auto crs = GeographicCRS::create(
				PropertyMap()
				.set(IdentifiedObject::NAME_KEY, "unnamed")
				.set(Identifier::CODESPACE_KEY, "MY_CODESPACE")
				.set(Identifier::CODE_KEY, "MY_ID"),
				nullptr, ensemble_vdatum,
				EllipsoidalCS::createLatitudeLongitude(UnitOfMeasure::DEGREE));
			WKTFormatterNNPtr f(
				WKTFormatter::create(WKTFormatter::Convention::WKT2_2018));
			crs->exportToWKT(f.get());
			auto expected =
				"GEOGCRS[\"unnamed\",\n"
				"    ENSEMBLE[\"unnamed\",\n"
				"        MEMBER[\"World Geodetic System 1984\"],\n"
				"        MEMBER[\"World Geodetic System 1984\"],\n"
				"        ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
				"            LENGTHUNIT[\"metre\",1]],\n"
				"        ENSEMBLEACCURACY[100]],\n"
				"    PRIMEM[\"Greenwich\",0,\n"
				"        ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
				"    CS[ellipsoidal,2],\n"
				"        AXIS[\"latitude\",north,\n"
				"            ORDER[1],\n"
				"            ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
				"        AXIS[\"longitude\",east,\n"
				"            ORDER[2],\n"
				"            ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
				"    ID[\"MY_CODESPACE\",\"MY_ID\"]]";

			EXPECT_EQ(f->toString(), expected);
		}

		{
			auto crs = GeographicCRS::create(
				PropertyMap().set(IdentifiedObject::NAME_KEY, "unnamed"), nullptr,
				ensemble_vdatum,
				EllipsoidalCS::createLatitudeLongitude(UnitOfMeasure::DEGREE));
			WKTFormatterNNPtr f(
				WKTFormatter::create(WKTFormatter::Convention::WKT2_2018));
			crs->exportToWKT(f.get());
			auto expected = "GEOGCRS[\"unnamed\",\n"
				"    ENSEMBLE[\"unnamed\",\n"
				"        MEMBER[\"World Geodetic System 1984\",\n"
				"            ID[\"EPSG\",6326]],\n"
				"        MEMBER[\"World Geodetic System 1984\",\n"
				"            ID[\"EPSG\",6326]],\n"
				"        ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
				"            LENGTHUNIT[\"metre\",1],\n"
				"            ID[\"EPSG\",7030]],\n"
				"        ENSEMBLEACCURACY[100]],\n"
				"    PRIMEM[\"Greenwich\",0,\n"
				"        ANGLEUNIT[\"degree\",0.0174532925199433],\n"
				"        ID[\"EPSG\",8901]],\n"
				"    CS[ellipsoidal,2],\n"
				"        AXIS[\"latitude\",north,\n"
				"            ORDER[1],\n"
				"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
				"                ID[\"EPSG\",9122]]],\n"
				"        AXIS[\"longitude\",east,\n"
				"            ORDER[2],\n"
				"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
				"                ID[\"EPSG\",9122]]]]";

			EXPECT_EQ(f->toString(), expected);
		}
	}

	{
		// EPSG_4326_as_WKT2

		auto crs = GeographicCRS::EPSG_4326;
		WKTFormatterNNPtr f(WKTFormatter::create());
		crs->exportToWKT(f.get());
		EXPECT_EQ(f->toString(),
			"GEODCRS[\"WGS 84\",\n"
			"    DATUM[\"World Geodetic System 1984\",\n"
			"        ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"            LENGTHUNIT[\"metre\",1]]],\n"
			"    PRIMEM[\"Greenwich\",0,\n"
			"        ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"    CS[ellipsoidal,2],\n"
			"        AXIS[\"latitude\",north,\n"
			"            ORDER[1],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"        AXIS[\"longitude\",east,\n"
			"            ORDER[2],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"    ID[\"EPSG\",4326]]");
	}

	{
		// EPSG_4326_as_WKT2_2018

		auto crs = GeographicCRS::EPSG_4326;
		WKTFormatterNNPtr f(
			WKTFormatter::create(WKTFormatter::Convention::WKT2_2018));
		crs->exportToWKT(f.get());
		EXPECT_EQ(f->toString(),
			"GEOGCRS[\"WGS 84\",\n"
			"    DATUM[\"World Geodetic System 1984\",\n"
			"        ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"            LENGTHUNIT[\"metre\",1]]],\n"
			"    PRIMEM[\"Greenwich\",0,\n"
			"        ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"    CS[ellipsoidal,2],\n"
			"        AXIS[\"latitude\",north,\n"
			"            ORDER[1],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"        AXIS[\"longitude\",east,\n"
			"            ORDER[2],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"    ID[\"EPSG\",4326]]");
	}

	{
		// EPSG_4326_as_WKT2_SIMPLIFIED

		auto crs = GeographicCRS::EPSG_4326;
		WKTFormatterNNPtr f(
			WKTFormatter::create(WKTFormatter::Convention::WKT2_SIMPLIFIED));
		crs->exportToWKT(f.get());
		EXPECT_EQ(f->toString(),
			"GEODCRS[\"WGS 84\",\n"
			"    DATUM[\"World Geodetic System 1984\",\n"
			"        ELLIPSOID[\"WGS 84\",6378137,298.257223563]],\n"
			"    CS[ellipsoidal,2],\n"
			"        AXIS[\"latitude\",north],\n"
			"        AXIS[\"longitude\",east],\n"
			"        UNIT[\"degree\",0.0174532925199433],\n"
			"    ID[\"EPSG\",4326]]");
	}

	{
		// EPSG_4326_as_WKT2_SIMPLIFIED_single_line

		auto crs = GeographicCRS::EPSG_4326;
		WKTFormatterNNPtr f(
			WKTFormatter::create(WKTFormatter::Convention::WKT2_SIMPLIFIED));
		f->setMultiLine(false);
		crs->exportToWKT(f.get());
		EXPECT_EQ(
			f->toString(),
			"GEODCRS[\"WGS 84\",DATUM[\"World Geodetic System "
			"1984\",ELLIPSOID[\"WGS "
			"84\",6378137,298.257223563]],"
			"CS[ellipsoidal,2],AXIS[\"latitude\",north],AXIS[\"longitude\",east],"
			"UNIT[\"degree\",0.0174532925199433],ID[\"EPSG\",4326]]");
	}

	{
		// EPSG_4326_as_PROJ_string

		auto crs = GeographicCRS::EPSG_4326;
		EXPECT_EQ(crs->exportToPROJString(PROJStringFormatter::create().get()),
			"+proj=longlat +datum=WGS84 +no_defs +type=crs");
	}

	{
		// EPSG_32661_projected_north_pole_north_east

		auto dbContext = DatabaseContext::create();
		auto factory = AuthorityFactory::create(dbContext, "EPSG");
		auto crs = factory->createCoordinateReferenceSystem("32661");
		auto proj_crs = nn_dynamic_pointer_cast<ProjectedCRS>(crs);
		CHECK(proj_crs != nullptr);

		auto proj_string =
			"+proj=pipeline +step +proj=axisswap +order=2,1 +step "
			"+proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=stere "
			"+lat_0=90 +lon_0=0 +k=0.994 +x_0=2000000 +y_0=2000000 "
			"+ellps=WGS84 +step +proj=axisswap +order=2,1";

		auto op = CoordinateOperationFactory::create()->createOperation(GeographicCRS::EPSG_4326, NN_NO_CHECK(proj_crs));
		CHECK(op != nullptr);
		EXPECT_EQ(op->exportToPROJString(PROJStringFormatter::create().get()), proj_string);
	}

	{
		// EPSG_5041_projected_north_pole_east_north

		auto dbContext = DatabaseContext::create();
		auto factory = AuthorityFactory::create(dbContext, "EPSG");
		auto crs = factory->createCoordinateReferenceSystem("5041");
		auto proj_crs = nn_dynamic_pointer_cast<ProjectedCRS>(crs);
		CHECK(proj_crs != nullptr);
		auto proj_string =
			"+proj=pipeline +step +proj=axisswap +order=2,1 +step "
			"+proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=stere "
			"+lat_0=90 +lon_0=0 +k=0.994 +x_0=2000000 +y_0=2000000 "
			"+ellps=WGS84";

		auto op = CoordinateOperationFactory::create()->createOperation(GeographicCRS::EPSG_4326, NN_NO_CHECK(proj_crs));
		CHECK(op != nullptr);
		EXPECT_EQ(op->exportToPROJString(PROJStringFormatter::create().get()), proj_string);
	}

	{
		// EPSG_32761_projected_south_pole_north_east

		auto dbContext = DatabaseContext::create();
		auto factory = AuthorityFactory::create(dbContext, "EPSG");
		auto crs = factory->createCoordinateReferenceSystem("32761");
		auto proj_crs = nn_dynamic_pointer_cast<ProjectedCRS>(crs);
		CHECK(proj_crs != nullptr);
		auto proj_string =
			"+proj=pipeline +step +proj=axisswap +order=2,1 +step "
			"+proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=stere "
			"+lat_0=-90 +lon_0=0 +k=0.994 +x_0=2000000 +y_0=2000000 "
			"+ellps=WGS84 +step +proj=axisswap +order=2,1";

		auto op = CoordinateOperationFactory::create()->createOperation(GeographicCRS::EPSG_4326, NN_NO_CHECK(proj_crs));
		CHECK(op != nullptr);
		EXPECT_EQ(op->exportToPROJString(PROJStringFormatter::create().get()), proj_string);
	}

	{
		// EPSG_5042_projected_south_pole_east_north

		auto dbContext = DatabaseContext::create();
		auto factory = AuthorityFactory::create(dbContext, "EPSG");
		auto crs = factory->createCoordinateReferenceSystem("5042");
		auto proj_crs = nn_dynamic_pointer_cast<ProjectedCRS>(crs);
		CHECK(proj_crs != nullptr);
		auto proj_string =
			"+proj=pipeline +step +proj=axisswap +order=2,1 +step "
			"+proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=stere "
			"+lat_0=-90 +lon_0=0 +k=0.994 +x_0=2000000 +y_0=2000000 "
			"+ellps=WGS84";

		auto op = CoordinateOperationFactory::create()->createOperation(GeographicCRS::EPSG_4326, NN_NO_CHECK(proj_crs));
		CHECK(op != nullptr);
		EXPECT_EQ(op->exportToPROJString(PROJStringFormatter::create().get()), proj_string);
	}

	{
		// projectedCRS_from_WKT1_ESRI_as_WKT1_ESRI

		auto dbContext = DatabaseContext::create();
		// Comes literally from the text_definition column of
		// projected_crs table
		auto esri_wkt =
			"PROJCS[\"WGS_1984_Web_Mercator\","
			"GEOGCS[\"GCS_WGS_1984_Major_Auxiliary_Sphere\","
			"DATUM[\"D_WGS_1984_Major_Auxiliary_Sphere\","
			"SPHEROID[\"WGS_1984_Major_Auxiliary_Sphere\",6378137.0,0.0]],"
			"PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION[\"Mercator\"],PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",0.0],"
			"PARAMETER[\"Standard_Parallel_1\",0.0],UNIT[\"Meter\",1.0]]";

		auto obj =
			WKTParser().attachDatabaseContext(dbContext).createFromWKT(esri_wkt);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(
			crs->exportToWKT(
				WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI, dbContext)
				.get()),
			esri_wkt);
	}

	{
		// projectedCRS_Krovak_with_approximate_alpha_as_PROJ_string

		// 30deg 17' 17.303''   = 30.288139722222223 as used in GDAL WKT1
		auto obj = PROJStringParser().createFromPROJString(
			"+proj=krovak +lat_0=49.5 +lon_0=42.5 +alpha=30.28813972222222 "
			"+k=0.9999 +x_0=0 +y_0=0 +ellps=bessel +pm=ferro +units=m +no_defs "
			"+type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		auto op = CoordinateOperationFactory::create()->createOperation(crs->baseCRS(), NN_NO_CHECK(crs));
		CHECK(op != nullptr);
		EXPECT_EQ(op->exportToPROJString(PROJStringFormatter::create().get()),
			"+proj=pipeline "
			"+step +proj=unitconvert +xy_in=deg +xy_out=rad "
			"+step +inv +proj=longlat +ellps=bessel +pm=ferro "
			"+step +proj=krovak +lat_0=49.5 +lon_0=42.5 "
			"+alpha=30.2881397222222 +k=0.9999 +x_0=0 +y_0=0 "
			"+ellps=bessel +pm=ferro");
	}

	{
		// mercator_1SP_as_WKT1_ESRI

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=merc +lon_0=110 +k=0.997 +x_0=3900000 +y_0=900000 "
			"+ellps=bessel +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		
		auto expected = "PROJCS[\"unknown\",GEOGCS[\"GCS_unknown\","
			"DATUM[\"D_Unknown_based_on_Bessel_1841_ellipsoid\","
			"SPHEROID[\"Bessel_1841\",6377397.155,299.1528128]],"
			"PRIMEM[\"Greenwich\",0.0],"
			"UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION[\"Mercator\"],"
			"PARAMETER[\"False_Easting\",3900000.0],"
			"PARAMETER[\"False_Northing\",900000.0],"
			"PARAMETER[\"Central_Meridian\",110.0],"
			"PARAMETER[\"Standard_Parallel_1\",4.45405154589748],"
			"UNIT[\"Meter\",1.0]]";
		EXPECT_EQ(crs->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI,
				DatabaseContext::create())
			.get()),
			expected);
	}

	{
		// Plate_Carree_as_WKT1_ESRI

		auto obj = PROJStringParser().createFromPROJString(
			"+title=my Plate carree +proj=eqc +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		
		auto expected = "PROJCS[\"my_Plate_carree\",GEOGCS[\"GCS_unknown\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\","
			"6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],"
			"UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION[\"Plate_Carree\"],"
			"PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Central_Meridian\",0.0],"
			"UNIT[\"Meter\",1.0]]";
			EXPECT_EQ(crs->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI,
				DatabaseContext::create())
			.get()),
			expected);
	}

	{
		// Equidistant_Cylindrical_as_WKT1_ESRI

		auto obj = PROJStringParser().createFromPROJString("+proj=eqc +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		
		auto expected = "PROJCS[\"unknown\",GEOGCS[\"GCS_unknown\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\","
			"6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],"
			"UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION[\"Equidistant_Cylindrical\"],"
			"PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Central_Meridian\",0.0],"
			"PARAMETER[\"Standard_Parallel_1\",0.0],"
			"UNIT[\"Meter\",1.0]]";
		EXPECT_EQ(crs->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI, DatabaseContext::create()).get()),
			expected);
	}

	{
		// Hotine_Oblique_Mercator_Azimuth_Natural_Origin_as_WKT1_ESRI

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=omerc +no_uoff +gamma=295 +alpha=295 +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		
		auto expected = "PROJCS[\"unknown\",GEOGCS[\"GCS_unknown\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\","
			"6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],"
			"UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION["
			"\"Hotine_Oblique_Mercator_Azimuth_Natural_Origin\"],"
			"PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Scale_Factor\",1.0],"
			// we renormalize angles to [-180,180]
			"PARAMETER[\"Azimuth\",-65.0],"
			"PARAMETER[\"Longitude_Of_Center\",0.0],"
			"PARAMETER[\"Latitude_Of_Center\",0.0],"
			"UNIT[\"Meter\",1.0]]";
		EXPECT_EQ(crs->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI,
				DatabaseContext::create())
			.get()),
			expected);
	}

	{
		// Rectified_Skew_Orthomorphic_Natural_Origin_as_WKT1_ESRI

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=omerc +no_uoff +gamma=3 +alpha=2 +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		
		auto expected = "PROJCS[\"unknown\",GEOGCS[\"GCS_unknown\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\","
			"6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],"
			"UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION["
			"\"Rectified_Skew_Orthomorphic_Natural_Origin\"],"
			"PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Scale_Factor\",1.0],"
			"PARAMETER[\"Azimuth\",2.0],"
			"PARAMETER[\"Longitude_Of_Center\",0.0],"
			"PARAMETER[\"Latitude_Of_Center\",0.0],"
			"PARAMETER[\"XY_Plane_Rotation\",3.0],"
			"UNIT[\"Meter\",1.0]]";
		EXPECT_EQ(crs->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI,
				DatabaseContext::create())
			.get()),
			expected);
	}

	{
		// Hotine_Oblique_Mercator_Azimuth_Center_as_WKT1_ESRI

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=omerc +gamma=2 +alpha=2 +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		
		auto expected = "PROJCS[\"unknown\",GEOGCS[\"GCS_unknown\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\","
			"6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],"
			"UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION["
			"\"Hotine_Oblique_Mercator_Azimuth_Center\"],"
			"PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Scale_Factor\",1.0],"
			"PARAMETER[\"Azimuth\",2.0],"
			"PARAMETER[\"Longitude_Of_Center\",0.0],"
			"PARAMETER[\"Latitude_Of_Center\",0.0],"
			"UNIT[\"Meter\",1.0]]";
		EXPECT_EQ(crs->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI,
				DatabaseContext::create())
			.get()),
			expected);
	}

	{
		// Rectified_Skew_Orthomorphic_Center_as_WKT1_ESRI

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=omerc +gamma=3 +alpha=2 +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		auto expected = "PROJCS[\"unknown\",GEOGCS[\"GCS_unknown\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\","
			"6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],"
			"UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION["
			"\"Rectified_Skew_Orthomorphic_Center\"],"
			"PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Scale_Factor\",1.0],"
			"PARAMETER[\"Azimuth\",2.0],"
			"PARAMETER[\"Longitude_Of_Center\",0.0],"
			"PARAMETER[\"Latitude_Of_Center\",0.0],"
			"PARAMETER[\"XY_Plane_Rotation\",3.0],"
			"UNIT[\"Meter\",1.0]]";
		EXPECT_EQ(crs->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI,
			DatabaseContext::create()).get()),
			expected);
	}

	{
		// Gauss_Kruger_as_WKT1_ESRI

		auto obj = PROJStringParser().createFromPROJString(
			"+title=my Gauss Kruger +proj=tmerc +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		auto expected = "PROJCS[\"my_Gauss_Kruger\",GEOGCS[\"GCS_unknown\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,"
			"298.257223563]],PRIMEM[\"Greenwich\",0.0],"
			"UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION[\"Gauss_Kruger\"],"
			"PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Central_Meridian\",0.0],"
			"PARAMETER[\"Scale_Factor\",1.0],"
			"PARAMETER[\"Latitude_Of_Origin\",0.0],"
			"UNIT[\"Meter\",1.0]]";
		EXPECT_EQ(crs->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI, DatabaseContext::create()).get()), expected);
	}

	{
		// Stereographic_North_Pole_as_WKT1_ESRI

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=stere +lat_0=90 +lat_ts=70 +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		auto expected = "PROJCS[\"unknown\",GEOGCS[\"GCS_unknown\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\","
			"6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],"
			"UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION[\"Stereographic_North_Pole\"],"
			"PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Central_Meridian\",0.0],"
			"PARAMETER[\"Standard_Parallel_1\",70.0],"
			"UNIT[\"Meter\",1.0]]";
		EXPECT_EQ(crs->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI,
			DatabaseContext::create()).get()),
			expected);
	}

	{
		// Stereographic_South_Pole_as_WKT1_ESRI

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=stere +lat_0=-90 +lat_ts=-70 +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		auto expected = "PROJCS[\"unknown\",GEOGCS[\"GCS_unknown\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\","
			"6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],"
			"UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION[\"Stereographic_South_Pole\"],"
			"PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Central_Meridian\",0.0],"
			"PARAMETER[\"Standard_Parallel_1\",-70.0],"
			"UNIT[\"Meter\",1.0]]";
		EXPECT_EQ(crs->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI,
			DatabaseContext::create()).get()),
			expected);
	}

	{
		// Krovak_North_Orientated_as_WKT1_ESRI
		
		auto obj =
			PROJStringParser().createFromPROJString("+proj=krovak +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		
		auto expected = "PROJCS[\"unknown\",GEOGCS[\"GCS_unknown\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\","
			"6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],"
			"UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION[\"Krovak\"],"
			"PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Pseudo_Standard_Parallel_1\",78.5],"
			"PARAMETER[\"Scale_Factor\",1.0],"
			"PARAMETER[\"Azimuth\",30.2881397527778],"
			"PARAMETER[\"Longitude_Of_Center\",0.0],"
			"PARAMETER[\"Latitude_Of_Center\",0.0],"
			"PARAMETER[\"X_Scale\",-1.0],"
			"PARAMETER[\"Y_Scale\",1.0],"
			"PARAMETER[\"XY_Plane_Rotation\",90.0],"
			"UNIT[\"Meter\",1.0]]";

		//ERROR
		//EXPECT_EQ(crs->exportToWKT(
			//WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI,
			//DatabaseContext::create()).get()),
			//expected);
	}

	{
		// Krovak_as_WKT1_ESRI

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=krovak +axis=swu +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		auto expected = "PROJCS[\"unknown\",GEOGCS[\"GCS_unknown\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\","
			"6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],"
			"UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION[\"Krovak\"],"
			"PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Pseudo_Standard_Parallel_1\",78.5],"
			"PARAMETER[\"Scale_Factor\",1.0],"
			"PARAMETER[\"Azimuth\",30.2881397527778],"
			"PARAMETER[\"Longitude_Of_Center\",0.0],"
			"PARAMETER[\"Latitude_Of_Center\",0.0],"
			"PARAMETER[\"X_Scale\",1.0],"
			"PARAMETER[\"Y_Scale\",1.0],"
			"PARAMETER[\"XY_Plane_Rotation\",0.0],"
			"UNIT[\"Meter\",1.0]]";

		//ERROR
		//EXPECT_EQ(crs->exportToWKT(
			//WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI,
			//DatabaseContext::create()).get()),
			//expected);
	}

	{
		// LCC_1SP_as_WKT1_ESRI

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=lcc +lat_1=1 +lat_0=1 +k=0.9 +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		auto expected = "PROJCS[\"unknown\",GEOGCS[\"GCS_unknown\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\","
			"6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],"
			"UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION[\"Lambert_Conformal_Conic\"],"
			"PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Central_Meridian\",0.0],"
			"PARAMETER[\"Standard_Parallel_1\",1.0],"
			"PARAMETER[\"Scale_Factor\",0.9],"
			"PARAMETER[\"Latitude_Of_Origin\",1.0],"
			"UNIT[\"Meter\",1.0]]";
		EXPECT_EQ(crs->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI,
			DatabaseContext::create()).get()),
			expected);
	}

	{
		// LCC_2SP_as_WKT1_ESRI

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=lcc +lat_0=1.5 +lat_1=1 +lat_2=2 +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		auto expected = "PROJCS[\"unknown\",GEOGCS[\"GCS_unknown\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\","
			"6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],"
			"UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION[\"Lambert_Conformal_Conic\"],"
			"PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Central_Meridian\",0.0],"
			"PARAMETER[\"Standard_Parallel_1\",1.0],"
			"PARAMETER[\"Standard_Parallel_2\",2.0],"
			"PARAMETER[\"Latitude_Of_Origin\",1.5],"
			"UNIT[\"Meter\",1.0]]";
		EXPECT_EQ(crs->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI,
			DatabaseContext::create()).get()),
			expected);
	}

	{
		// ESRI_WKT1_to_ESRI_WKT1

		auto in_wkt =
			"PROJCS[\"NAD_1983_CORS96_StatePlane_North_Carolina_FIPS_3200_Ft_US\","
			"GEOGCS[\"GCS_NAD_1983_CORS96\",DATUM[\"D_NAD_1983_CORS96\","
			"SPHEROID[\"GRS_1980\",6378137.0,298.257222101]],"
			"PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION[\"Lambert_Conformal_Conic\"],"
			"PARAMETER[\"False_Easting\",2000000.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Central_Meridian\",-79.0],"
			"PARAMETER[\"Standard_Parallel_1\",34.33333333333334],"
			"PARAMETER[\"Standard_Parallel_2\",36.16666666666666],"
			"PARAMETER[\"Latitude_Of_Origin\",33.75],"
			"UNIT[\"Foot_US\",0.3048006096012192]]";

		auto obj = WKTParser().createFromWKT(in_wkt);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		auto expected =
			"PROJCS[\"NAD_1983_CORS96_StatePlane_North_Carolina_FIPS_3200_Ft_US\","
			"GEOGCS[\"GCS_NAD_1983_CORS96\",DATUM[\"D_NAD_1983_CORS96\","
			"SPHEROID[\"GRS_1980\",6378137.0,298.257222101]],"
			"PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION[\"Lambert_Conformal_Conic\"],"
			"PARAMETER[\"False_Easting\",2000000.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Central_Meridian\",-79.0],"
			"PARAMETER[\"Standard_Parallel_1\",34.3333333333333],"
			"PARAMETER[\"Standard_Parallel_2\",36.1666666666667],"
			"PARAMETER[\"Latitude_Of_Origin\",33.75],"
			"UNIT[\"Foot_US\",0.304800609601219]]";

		EXPECT_EQ(crs->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI,
				DatabaseContext::create())
			.get()),
			expected);
	}
}

TEST(PROJ4, Transform)
{
	{
		// longlat_to_longlat

		auto src = pj_init_plus("+proj=longlat +datum=WGS84");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84");
		double x = 2.0 * DEG_TO_RAD;
		double y = 49.0 * DEG_TO_RAD;
		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, nullptr));
		DOUBLES_EQUAL(2.0 * DEG_TO_RAD, x, 0.0);
		DOUBLES_EQUAL(49.0 * DEG_TO_RAD, y, 0.0);

		x = 182.0 * DEG_TO_RAD;
		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, nullptr));
		DOUBLES_EQUAL(182.0 * DEG_TO_RAD, x, 0.0);
		DOUBLES_EQUAL(49.0 * DEG_TO_RAD, y, 0.0);

		pj_free(src);
		pj_free(dst);
	}

	{
		// longlat_to_proj

		auto src = pj_init_plus("+proj=longlat +datum=WGS84");
		auto dst = pj_init_plus("+proj=utm +zone=31 +datum=WGS84");
		double x = 3 * DEG_TO_RAD;
		double y = 0 * DEG_TO_RAD;
		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, nullptr));
		DOUBLES_EQUAL(500000, x, 1e-8);
		DOUBLES_EQUAL(0, y, 1e-8);
		pj_free(src);
		pj_free(dst);
	}

	{
		// longlat_to_proj_tometer

		auto src = pj_init_plus("+proj=longlat +datum=WGS84");
		auto dst = pj_init_plus("+proj=utm +zone=31 +datum=WGS84 +to_meter=1000");
		double x = 3 * DEG_TO_RAD;
		double y = 0 * DEG_TO_RAD;
		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, nullptr));
		DOUBLES_EQUAL(500, x, 1e-8);
		DOUBLES_EQUAL(0, y, 1e-8);
		pj_free(src);
		pj_free(dst);
	}

	{
		// proj_to_longlat

		auto src = pj_init_plus("+proj=utm +zone=31 +datum=WGS84");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84");
		double x = 500000;
		double y = 0;
		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, nullptr));
		DOUBLES_EQUAL(3 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(0 * DEG_TO_RAD, y, 1e-12);
		pj_free(src);
		pj_free(dst);
	}

	{
		// proj_to_proj

		auto src = pj_init_plus("+proj=utm +zone=31 +datum=WGS84");
		auto dst = pj_init_plus("+proj=utm +zone=31 +datum=WGS84");
		double x = 500000;
		double y = 0;
		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, nullptr));
		DOUBLES_EQUAL(500000, x, 1e-8);
		DOUBLES_EQUAL(0, y, 1e-8);
		pj_free(src);
		pj_free(dst);
	}

	{
		// longlat_to_geocent

		auto src = pj_init_plus("+proj=longlat +R=2");
		auto dst = pj_init_plus("+proj=geocent +R=2");
		double x = 0;
		double y = 0;
		double z = 0;
		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(2, x, 1e-8);
		DOUBLES_EQUAL(0, y, 1e-8);
		DOUBLES_EQUAL(0, z, 1e-8);

		x = 90 * DEG_TO_RAD;
		y = 0;
		z = 0;
		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(0, x, 1e-8);
		DOUBLES_EQUAL(2, y, 1e-8);
		DOUBLES_EQUAL(0, z, 1e-8);

		x = 0;
		y = 90 * DEG_TO_RAD;
		z = 0.1;
		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(0, x, 1e-8);
		DOUBLES_EQUAL(0, y, 1e-8);
		DOUBLES_EQUAL(2 + 0.1, z, 1e-8);

		pj_free(src);
		pj_free(dst);
	}

	{
		// longlat_to_geocent_to_meter

		auto src = pj_init_plus("+proj=longlat +R=2");
		auto dst = pj_init_plus("+proj=geocent +R=2 +to_meter=1000");
		double x = 0;
		double y = 0;
		double z = 0;
		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(2e-3, x, 1e-8);
		DOUBLES_EQUAL(0, y, 1e-8);
		DOUBLES_EQUAL(0, z, 1e-8);
		pj_free(src);
		pj_free(dst);
	}

	{
		// geocent_to_longlat

		auto src = pj_init_plus("+proj=geocent +R=2");
		auto dst = pj_init_plus("+proj=longlat +R=2");

		double x = 0;
		double y = 2;
		double z = 0;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(90 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(0, y, 1e-12);
		DOUBLES_EQUAL(0, z, 1e-12);

		pj_free(src);
		pj_free(dst);
	}

	{
		// geocent_to_meter_to_longlat

		auto src = pj_init_plus("+proj=geocent +to_meter=1000 +R=2");
		auto dst = pj_init_plus("+proj=longlat +R=2");

		double x = 0;
		double y = 2e-3;
		double z = 0;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(90 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(0, y, 1e-12);
		DOUBLES_EQUAL(0, z, 1e-12);

		pj_free(src);
		pj_free(dst);
	}

	{
		// pm

		auto src = pj_init_plus("+proj=longlat +pm=3 +datum=WGS84");
		auto dst = pj_init_plus("+proj=longlat +pm=1 +datum=WGS84");
		double x = 2 * DEG_TO_RAD;
		double y = 49 * DEG_TO_RAD;
		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, nullptr));
		DOUBLES_EQUAL((2 + 3 - 1) * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(y, 49 * DEG_TO_RAD, 0.0);
		pj_free(src);
		pj_free(dst);
	}

	{
		// longlat_geoc_to_longlat

		auto src = pj_init_plus("+proj=longlat +geoc +datum=WGS84");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84");

		double x = 2 * DEG_TO_RAD;
		double y = 49 * DEG_TO_RAD;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, nullptr));
		DOUBLES_EQUAL(2 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(48.809360314691766 * DEG_TO_RAD, y, 1e-12);

		pj_free(src);
		pj_free(dst);
	}

	{
		// longlat_to_longlat_geoc

		auto src = pj_init_plus("+proj=longlat +datum=WGS84");
		auto dst = pj_init_plus("+proj=longlat +geoc +datum=WGS84");

		double x = 2 * DEG_TO_RAD;
		double y = 48.809360314691766 * DEG_TO_RAD;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, nullptr));
		DOUBLES_EQUAL(2 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(49 * DEG_TO_RAD, y, 1e-12);

		pj_free(src);
		pj_free(dst);
	}

	{
		// ellps_to_ellps_noop

		auto src = pj_init_plus("+proj=longlat +ellps=clrk66");
		auto dst = pj_init_plus("+proj=longlat +ellps=WGS84");

		double x = 2 * DEG_TO_RAD;
		double y = 49 * DEG_TO_RAD;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, nullptr));
		DOUBLES_EQUAL(2 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(49 * DEG_TO_RAD, y, 1e-12);

		pj_free(src);
		pj_free(dst);
	}

	{
		// towgs84_3param_noop

		auto src = pj_init_plus("+proj=longlat +ellps=WGS84 +towgs84=1,2,3");
		auto dst = pj_init_plus("+proj=longlat +ellps=WGS84 +towgs84=1,2,3");

		double x = 2 * DEG_TO_RAD;
		double y = 49 * DEG_TO_RAD;
		double z = 10;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(2 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(49 * DEG_TO_RAD, y, 1e-12);
		DOUBLES_EQUAL(10, z, 1e-8);

		pj_free(src);
		pj_free(dst);
	}

	{
		// towgs84_7param_noop

		auto src = pj_init_plus("+proj=longlat +ellps=WGS84 +towgs84=1,2,3,4,5,6,7");
		auto dst = pj_init_plus("+proj=longlat +ellps=WGS84 +towgs84=1,2,3,4,5,6,7");

		double x = 2 * DEG_TO_RAD;
		double y = 49 * DEG_TO_RAD;
		double z = 10;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(2 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(49 * DEG_TO_RAD, y, 1e-12);
		DOUBLES_EQUAL(10, z, 1e-8);

		pj_free(src);
		pj_free(dst);
	}

	{
		// longlat_towgs84_3param_to_datum

		auto src = pj_init_plus("+proj=longlat +ellps=WGS84 +towgs84=0,1,0");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84");

		double x = 90 * DEG_TO_RAD;
		double y = 0 * DEG_TO_RAD;
		double z = 10;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(90 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(0 * DEG_TO_RAD, y, 1e-12);
		DOUBLES_EQUAL(10 + 1, z, 1e-8);

		pj_free(src);
		pj_free(dst);
	}

	{
		// longlat_towgs84_3param_to_datum_no_z

		auto src = pj_init_plus("+proj=longlat +ellps=WGS84 +towgs84=0,1,0");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84");
		double x = 90 * DEG_TO_RAD;
		double y = 0 * DEG_TO_RAD;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, nullptr));
		DOUBLES_EQUAL(90 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(0 * DEG_TO_RAD, y, 1e-12);

		pj_free(src);
		pj_free(dst);
	}

	{
		// longlat_towgs84_7param_to_datum

		auto src = pj_init_plus("+proj=longlat +ellps=WGS84 +towgs84=0,1,0,0,0,0,0.5");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84");

		double x = 90 * DEG_TO_RAD;
		double y = 0 * DEG_TO_RAD;
		double z = 10;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(90 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(0 * DEG_TO_RAD, y, 1e-12);
		DOUBLES_EQUAL(14.189073500223458, z, 1e-8);

		pj_free(src);
		pj_free(dst);
	}

	{
		// datum_to_longlat_towgs84_3param

		auto src = pj_init_plus("+proj=longlat +datum=WGS84");
		auto dst = pj_init_plus("+proj=longlat +ellps=WGS84 +towgs84=0,1,0");

		double x = 90 * DEG_TO_RAD;
		double y = 0 * DEG_TO_RAD;
		double z = 10 + 1;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(90 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(0 * DEG_TO_RAD, y, 1e-12);
		DOUBLES_EQUAL(10, z, 1e-8);

		pj_free(src);
		pj_free(dst);
	}

	{
		// datum_to_longlat_towgs84_7param

		auto src = pj_init_plus("+proj=longlat +datum=WGS84");
		auto dst = pj_init_plus("+proj=longlat +ellps=WGS84 +towgs84=0,1,0,0,0,0,0.5");

		double x = 90 * DEG_TO_RAD;
		double y = 0 * DEG_TO_RAD;
		double z = 14.189073500223458;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(90 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(0 * DEG_TO_RAD, y, 1e-12);
		DOUBLES_EQUAL(10, z, 1e-8);

		pj_free(src);
		pj_free(dst);
	}

	{
		// ellps_grs80_towgs84_to_datum_wgs84

		auto src = pj_init_plus("+proj=longlat +ellps=GRS80 +towgs84=0,0,0");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84");

		double x = 2 * DEG_TO_RAD;
		double y = 49 * DEG_TO_RAD;
		double z = 10;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(2 * DEG_TO_RAD, x, 1e-15);
		DOUBLES_EQUAL(49 * DEG_TO_RAD, y, 1e-15);
		DOUBLES_EQUAL(10, z, 1e-8);

		pj_free(src);
		pj_free(dst);
	}

	{
		// longlat_nadgrids_to_datum

		auto src = pj_init_plus("+proj=longlat +ellps=clrk66 +nadgrids=conus");
		auto dst = pj_init_plus("+proj=longlat +datum=NAD83");

		double x = -100 * DEG_TO_RAD;
		double y = 40 * DEG_TO_RAD;
		double z = 10;

		int ret = pj_transform(src, dst, 1, 0, &x, &y, &z);
		CHECK(ret == 0 || ret == PJD_ERR_FAILED_TO_LOAD_GRID);
		if (ret == 0)
		{
			DOUBLES_EQUAL(-100.00040583667015 * DEG_TO_RAD, x, 1e-12);
			DOUBLES_EQUAL(40.000005895651363 * DEG_TO_RAD, y, 1e-12);
			DOUBLES_EQUAL(10.000043224543333, z, 1e-8);;
		}
		pj_free(src);
		pj_free(dst);
	}

	{
		// nadgrids_noop

		auto src = pj_init_plus("+proj=longlat +ellps=clrk66 +nadgrids=conus");
		auto dst = pj_init_plus("+proj=longlat +ellps=clrk66 +nadgrids=conus");

		double x = -100 * DEG_TO_RAD;
		double y = 40 * DEG_TO_RAD;
		double z = 10;

		int ret = pj_transform(src, dst, 1, 0, &x, &y, &z);
		CHECK(ret == 0);
		if (ret == 0)
		{
			DOUBLES_EQUAL(-100 * DEG_TO_RAD, x, 1e-15);
			DOUBLES_EQUAL(40 * DEG_TO_RAD, y, 1e-15);
			DOUBLES_EQUAL(10, z, 1e-8);
		}

		pj_free(src);
		pj_free(dst);
	}

	{
		// datum_to_longlat_nadgrids

		auto src = pj_init_plus("+proj=longlat +datum=NAD83");
		auto dst = pj_init_plus("+proj=longlat +ellps=clrk66 +nadgrids=conus");

		double x = -100.00040583667015 * DEG_TO_RAD;
		double y = 40.000005895651363 * DEG_TO_RAD;
		double z = 10.000043224543333;

		int ret = pj_transform(src, dst, 1, 0, &x, &y, &z);

		CHECK(ret == 0 || ret == PJD_ERR_FAILED_TO_LOAD_GRID);
		if (ret == 0)
		{
			DOUBLES_EQUAL(-100 * DEG_TO_RAD, x, 1e-12);
			DOUBLES_EQUAL(40 * DEG_TO_RAD, y, 1e-12);
			DOUBLES_EQUAL(10, z, 1e-8);
		}

		pj_free(src);
		pj_free(dst);
	}

	{
		// long_wrap

		auto src = pj_init_plus("+proj=longlat +datum=WGS84");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84 +lon_wrap=180");

		double x = -1 * DEG_TO_RAD;
		double y = 0 * DEG_TO_RAD;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, nullptr));
		DOUBLES_EQUAL(359 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(0 * DEG_TO_RAD, y, 1e-12);

		pj_free(src);
		pj_free(dst);
	}

	{
		// src_vto_meter

		auto src = pj_init_plus("+proj=longlat +datum=WGS84 +vto_meter=1000");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84");

		double x = 2 * DEG_TO_RAD;
		double y = 49 * DEG_TO_RAD;
		double z = 1;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(2 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(49 * DEG_TO_RAD, y, 1e-12);
		DOUBLES_EQUAL(1000, z, 1e-8);

		pj_free(src);
		pj_free(dst);
	}

	{
		// dest_vto_meter

		auto src = pj_init_plus("+proj=longlat +datum=WGS84");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84 +vto_meter=1000");

		double x = 2 * DEG_TO_RAD;
		double y = 49 * DEG_TO_RAD;
		double z = 1000;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(2 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(49 * DEG_TO_RAD, y, 1e-12);
		DOUBLES_EQUAL(1, z, 1e-8);

		pj_free(src);
		pj_free(dst);
	}

	{
		// src_axis_neu_with_z

		auto src = pj_init_plus("+proj=longlat +datum=WGS84 +axis=neu");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84");

		double x = 49 * DEG_TO_RAD;
		double y = 2 * DEG_TO_RAD;
		double z = 1;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(2 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(49 * DEG_TO_RAD, y, 1e-12);
		DOUBLES_EQUAL(1, z, 1e-8);

		pj_free(src);
		pj_free(dst);
	}

	{
		// src_axis_neu_without_z

		auto src = pj_init_plus("+proj=longlat +datum=WGS84 +axis=neu");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84");

		double x = 49 * DEG_TO_RAD;
		double y = 2 * DEG_TO_RAD;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, nullptr));
		DOUBLES_EQUAL(x, 2 * DEG_TO_RAD, 1e-12);
		DOUBLES_EQUAL(y, 49 * DEG_TO_RAD, 1e-12);

		pj_free(src);
		pj_free(dst);
	}

	{
		// src_axis_swd

		auto src = pj_init_plus("+proj=longlat +datum=WGS84 +axis=swd");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84");

		double x = 49 * DEG_TO_RAD;
		double y = 2 * DEG_TO_RAD;
		double z = -1;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(-2 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(-49 * DEG_TO_RAD, y, 1e-12);
		DOUBLES_EQUAL(1, z, 1e-8);

		pj_free(src);
		pj_free(dst);
	}

	{
		// dst_axis_neu

		auto src = pj_init_plus("+proj=longlat +datum=WGS84");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84 +axis=neu");

		double x = 2 * DEG_TO_RAD;
		double y = 49 * DEG_TO_RAD;
		double z = 1;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(49 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(2 * DEG_TO_RAD, y, 1e-12);
		DOUBLES_EQUAL(1, z, 1e-8);

		pj_free(src);
		pj_free(dst);
	}

	{
		// dst_axis_swd

		auto src = pj_init_plus("+proj=longlat +datum=WGS84");
		auto dst = pj_init_plus("+proj=longlat +datum=WGS84 +axis=swd");

		double x = 2 * DEG_TO_RAD;
		double y = 49 * DEG_TO_RAD;
		double z = 1;

		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, &z));
		DOUBLES_EQUAL(-49 * DEG_TO_RAD, x, 1e-12);
		DOUBLES_EQUAL(-2 * DEG_TO_RAD, y, 1e-12);
		DOUBLES_EQUAL(-1, z, 1e-8);

		pj_free(src);
		pj_free(dst);
	}

	{
		// init_epsg

		auto src = pj_init_plus("+init=epsg:4326");
		CHECK(src != nullptr);
		auto dst = pj_init_plus("+init=epsg:32631");
		CHECK(dst != nullptr);
		double x = 3 * DEG_TO_RAD;
		double y = 0 * DEG_TO_RAD;
		LONGS_EQUAL(0, pj_transform(src, dst, 1, 0, &x, &y, nullptr));
		DOUBLES_EQUAL(500000, x, 1e-8);
		DOUBLES_EQUAL(0, y, 1e-8);
		pj_free(src);
		pj_free(dst);
	}

	{
		// proj_api_h pj_set_searchpath

		const char* path = "/i_do/not/exist";
		pj_set_searchpath(1, &path);
		{
			auto info = proj_info();
			CHECK(1U == info.path_count);
			CHECK(info.paths != nullptr);
			CHECK(info.paths[0] != nullptr);
			CHECK(std::string(info.paths[0]) == path);
		}

		pj_set_searchpath(0, nullptr);
		{
			auto info = proj_info();
			CHECK(info.path_count == 0U);
			CHECK(info.paths == nullptr);
		}
	}
}

TEST(PROJ4, AngularUnits)
{
	{
		// Basic test

		PJ_CONTEXT* ctx = proj_context_create();

		// Instantiate an object from a WKT string, PROJ string or object code
		// (like "EPSG:4326", "urn:ogc:def:crs:EPSG::4326", "urn:ogc:def:coordinateOperation:EPSG::1671").
		//
		// This function calls osgeo::proj::io::createFromUserInput()
		//
		// The returned object must be unreferenced with proj_destroy() after use.
		// It should be used by at most one thread at a time.
		//
		// @param ctx PROJ context, or NULL for default context
		// @param text String(must not be NULL)
		// @return Object that must be unreferenced with proj_destroy(), or NULL in
		// case of error.

		PJ* P = proj_create(ctx, "proj=latlong");

		// Returns 1 if the operator P expects angular input coordinates when
		// operating in direction dir, 0 otherwise.
		// dir: {PJ_FWD, PJ_INV}
		int n = proj_angular_input(P, PJ_FWD);
		LONGS_EQUAL(1, n);

		//	Returns 1 if the operator P provides angular output coordinates when
		//		operating in direction dir, 0 otherwise.
		//	dir: {PJ_FWD, PJ_INV}

		n = proj_angular_output(P, PJ_FWD);
		LONGS_EQUAL(1, n);
		n = proj_angular_input(P, PJ_INV);
		LONGS_EQUAL(1, n);
		n = proj_angular_output(P, PJ_INV);
		LONGS_EQUAL(1, n);

		proj_destroy(P);
		proj_context_destroy(ctx);
	}

	{
		// Pipelines test #1

		auto ctx = proj_context_create();
		auto P =
			proj_create(ctx, "proj=pipeline +step +proj=axisswap +order=2,1 +step "
				"+proj=latlong +step +proj=axisswap +order=2,1");

		CHECK(proj_angular_input(P, PJ_FWD));
		CHECK(proj_angular_output(P, PJ_FWD));
		CHECK(proj_angular_input(P, PJ_INV));
		CHECK(proj_angular_output(P, PJ_INV));

		proj_destroy(P);
		proj_context_destroy(ctx);
	}

	{
		// Pipelines test #2

		auto ctx = proj_context_create();
		auto P = proj_create(
			ctx,
			"+proj=pipeline "
			"+step +proj=axisswap +order=2,1 "
			"+step +proj=unitconvert +xy_in=deg +xy_out=rad "
			"+step +proj=tmerc +lat_0=0 +lon_0=-81 +k=0.9996 +x_0=500000.001016002 "
			"+y_0=0 +ellps=WGS84 "
			"+step +proj=axisswap +order=2,1 "
			"+step +proj=unitconvert +xy_in=m +z_in=m +xy_out=us-ft +z_out=us-ft");

		EXPECT_FALSE(proj_angular_input(P, PJ_FWD));
		EXPECT_FALSE(proj_angular_output(P, PJ_FWD));

		proj_destroy(P);
		proj_context_destroy(ctx);
	}

	{
		// Pipelines test #3

		auto ctx = proj_context_create();
		auto P = proj_create(
			ctx,
			"+proj=pipeline "
			"+step +proj=axisswap +order=2,1 "
			"+step +proj=tmerc +lat_0=0 +lon_0=-81 +k=0.9996 +x_0=500000.001016002 "
			"+y_0=0 +ellps=WGS84 "
			"+step +proj=axisswap +order=2,1 "
			"+step +proj=unitconvert +xy_in=m +z_in=m +xy_out=us-ft +z_out=us-ft");

		CHECK(proj_angular_input(P, PJ_FWD));
		EXPECT_FALSE(proj_angular_output(P, PJ_FWD));

		proj_destroy(P);
		proj_context_destroy(ctx);
	}
}

TEST(PROJ4, datum)
{
	{
		// ellipsoid_from_sphere

		auto ellipsoid = Ellipsoid::createSphere(PropertyMap(), Length(6378137));
		EXPECT_FALSE(ellipsoid->inverseFlattening().has_value());
		EXPECT_FALSE(ellipsoid->semiMinorAxis().has_value());
		EXPECT_FALSE(ellipsoid->semiMedianAxis().has_value());
		CHECK(ellipsoid->isSphere());
		EXPECT_EQ(ellipsoid->semiMajorAxis(), Length(6378137));
		EXPECT_EQ(ellipsoid->celestialBody(), "Earth");

		EXPECT_EQ(ellipsoid->computeSemiMinorAxis(), Length(6378137));
		EXPECT_EQ(ellipsoid->computedInverseFlattening(), 0);

		EXPECT_EQ(ellipsoid->exportToPROJString(PROJStringFormatter::create().get()), "+R=6378137");

		CHECK(ellipsoid->isEquivalentTo(ellipsoid.get()));
		//EXPECT_FALSE(ellipsoid->isEquivalentTo(createUnrelatedObject().get()));
	}

	{
		// ellipsoid_non_earth

		auto ellipsoid = Ellipsoid::createSphere(PropertyMap(), Length(1), "Unity sphere");
		EXPECT_EQ(ellipsoid->celestialBody(), "Unity sphere");
	}

	{
		// ellipsoid_from_inverse_flattening
		
		auto ellipsoid = Ellipsoid::createFlattenedSphere(
			PropertyMap(), Length(6378137), Scale(298.257223563));
		CHECK(ellipsoid->inverseFlattening().has_value());
		EXPECT_FALSE(ellipsoid->semiMinorAxis().has_value());
		EXPECT_FALSE(ellipsoid->semiMedianAxis().has_value());
		EXPECT_FALSE(ellipsoid->isSphere());
		EXPECT_EQ(ellipsoid->semiMajorAxis(), Length(6378137));
		EXPECT_EQ(*ellipsoid->inverseFlattening(), Scale(298.257223563));
		
		EXPECT_EQ(ellipsoid->computeSemiMinorAxis().unit(), ellipsoid->semiMajorAxis().unit());
		DOUBLES_EQUAL(ellipsoid->computeSemiMinorAxis().value(), Length(6356752.31424518).value(), 1e-9);
		EXPECT_EQ(ellipsoid->computedInverseFlattening(), 298.257223563);

		EXPECT_EQ(ellipsoid->exportToPROJString(PROJStringFormatter::create().get()), "+ellps=WGS84");

		CHECK(ellipsoid->isEquivalentTo(ellipsoid.get()));
		EXPECT_FALSE(ellipsoid->isEquivalentTo(Ellipsoid::createTwoAxis(PropertyMap(), Length(6378137), Length(6356752.31424518)).get()));
		CHECK(ellipsoid->isEquivalentTo(Ellipsoid::createTwoAxis(PropertyMap(), Length(6378137), Length(6356752.31424518)).get(), IComparable::Criterion::EQUIVALENT));

		EXPECT_FALSE(Ellipsoid::WGS84->isEquivalentTo(Ellipsoid::GRS1980.get(), IComparable::Criterion::EQUIVALENT));
	}

	{
		// ellipsoid_from_null_inverse_flattening

		auto ellipsoid = Ellipsoid::createFlattenedSphere(PropertyMap(), Length(6378137), Scale(0));
		EXPECT_FALSE(ellipsoid->inverseFlattening().has_value());
		EXPECT_FALSE(ellipsoid->semiMinorAxis().has_value());
		EXPECT_FALSE(ellipsoid->semiMedianAxis().has_value());
		CHECK(ellipsoid->isSphere());
	}

	{
		// ellipsoid_from_semi_minor_axis
		
		auto ellipsoid = Ellipsoid::createTwoAxis(PropertyMap(), Length(6378137), Length(6356752.31424518));
		EXPECT_FALSE(ellipsoid->inverseFlattening().has_value());
		CHECK(ellipsoid->semiMinorAxis().has_value());
		EXPECT_FALSE(ellipsoid->semiMedianAxis().has_value());
		EXPECT_FALSE(ellipsoid->isSphere());
		EXPECT_EQ(ellipsoid->semiMajorAxis(), Length(6378137));
		EXPECT_EQ(*ellipsoid->semiMinorAxis(), Length(6356752.31424518));

		EXPECT_EQ(ellipsoid->computeSemiMinorAxis(), Length(6356752.31424518));
		DOUBLES_EQUAL(ellipsoid->computedInverseFlattening(), 298.257223563, 1e-10);

		EXPECT_EQ(ellipsoid->exportToPROJString(PROJStringFormatter::create().get()), "+ellps=WGS84");

		CHECK(ellipsoid->isEquivalentTo(ellipsoid.get()));
		EXPECT_FALSE(ellipsoid->isEquivalentTo(Ellipsoid::createFlattenedSphere(PropertyMap(), Length(6378137), Scale(298.257223563)).get()));
		CHECK(ellipsoid->isEquivalentTo(Ellipsoid::createFlattenedSphere(PropertyMap(), Length(6378137), Scale(298.257223563)).get(), IComparable::Criterion::EQUIVALENT));
	}

	{
		// prime_meridian_to_PROJString

		EXPECT_EQ(PrimeMeridian::GREENWICH->exportToPROJString(
			PROJStringFormatter::create().get()),
			"+proj=noop");
		
		EXPECT_EQ(PrimeMeridian::PARIS->exportToPROJString(
			PROJStringFormatter::create().get()),
			"+pm=paris");
		
		EXPECT_EQ(PrimeMeridian::create(PropertyMap(), Angle(3.5))
			->exportToPROJString(PROJStringFormatter::create().get()),
			"+pm=3.5");
		
		EXPECT_EQ(
			PrimeMeridian::create(PropertyMap(), Angle(100, UnitOfMeasure::GRAD))
			->exportToPROJString(PROJStringFormatter::create().get()),
			"+pm=90");
		
		EXPECT_EQ(
			PrimeMeridian::create(
				PropertyMap().set(IdentifiedObject::NAME_KEY, "Origin meridian"),
				Angle(0))
			->exportToPROJString(PROJStringFormatter::create().get()),
			"+proj=noop");
		
		CHECK(PrimeMeridian::GREENWICH->isEquivalentTo(
			PrimeMeridian::GREENWICH.get()));
		//EXPECT_FALSE(PrimeMeridian::GREENWICH->isEquivalentTo(createUnrelatedObject().get()));
	}

	{
		// datum_with_ANCHOR

		auto datum = GeodeticReferenceFrame::create(
			PropertyMap().set(IdentifiedObject::NAME_KEY, "WGS_1984 with anchor"),
			Ellipsoid::WGS84, optional<std::string>("My anchor"),
			PrimeMeridian::GREENWICH);

		auto expected = "DATUM[\"WGS_1984 with anchor\",\n"
			"    ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"        LENGTHUNIT[\"metre\",1],\n"
			"        ID[\"EPSG\",7030]],\n"
			"    ANCHOR[\"My anchor\"]]";

		EXPECT_EQ(datum->exportToWKT(WKTFormatter::create().get()), expected);
	}

	{
		// dynamic_geodetic_reference_frame

		auto drf = DynamicGeodeticReferenceFrame::create(
			PropertyMap().set(IdentifiedObject::NAME_KEY, "test"), Ellipsoid::WGS84,
			optional<std::string>("My anchor"), PrimeMeridian::GREENWICH,
			Measure(2018.5, UnitOfMeasure::YEAR),
			optional<std::string>("My model"));
		
		auto expected = "DATUM[\"test\",\n"
			"    ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"        LENGTHUNIT[\"metre\",1],\n"
			"        ID[\"EPSG\",7030]],\n"
			"    ANCHOR[\"My anchor\"]]";
		
		EXPECT_EQ(drf->exportToWKT(WKTFormatter::create().get()), expected);
		
		auto expected_wtk2_2018 =
			"DYNAMIC[\n"
			"    FRAMEEPOCH[2018.5],\n"
			"    MODEL[\"My model\"]],\n"
			"DATUM[\"test\",\n"
			"    ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"        LENGTHUNIT[\"metre\",1],\n"
			"        ID[\"EPSG\",7030]],\n"
			"    ANCHOR[\"My anchor\"]]";
		EXPECT_EQ(
			drf->exportToWKT(
				WKTFormatter::create(WKTFormatter::Convention::WKT2_2018).get()),
			expected_wtk2_2018);
	}

	{
		// ellipsoid_to_PROJString
		
		EXPECT_EQ(Ellipsoid::WGS84->exportToPROJString(
			PROJStringFormatter::create().get()),
			"+ellps=WGS84");
		
		EXPECT_EQ(Ellipsoid::GRS1980->exportToPROJString(
			PROJStringFormatter::create().get()),
			"+ellps=GRS80");
		
		EXPECT_EQ(
			Ellipsoid::createFlattenedSphere(
				PropertyMap(), Length(10, UnitOfMeasure("km", 1000)), Scale(0.5))
			->exportToPROJString(PROJStringFormatter::create().get()),
			"+a=10000 +rf=0.5");
		
		EXPECT_EQ(Ellipsoid::createTwoAxis(PropertyMap(),
			Length(10, UnitOfMeasure("km", 1000)),
			Length(5, UnitOfMeasure("km", 1000)))
			->exportToPROJString(PROJStringFormatter::create().get()),
			"+a=10000 +b=5000");
	}

	{
		// temporal_datum_time_origin_non_ISO8601

		auto datum = TemporalDatum::create(
			PropertyMap().set(IdentifiedObject::NAME_KEY, "Gregorian calendar"),
			DateTime::create("0001 January 1st"),
			TemporalDatum::CALENDAR_PROLEPTIC_GREGORIAN);
		
		auto expected = 
			"TDATUM[\"Gregorian calendar\",\n"
			"    TIMEORIGIN[\"0001 January 1st\"]]";
		
		EXPECT_EQ(datum->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT2).get()),
			expected);
	}

	{
		// temporal_datum_WKT2_2018

		auto datum = TemporalDatum::create(
			PropertyMap().set(IdentifiedObject::NAME_KEY, "Gregorian calendar"),
			DateTime::create("0000-01-01"),
			TemporalDatum::CALENDAR_PROLEPTIC_GREGORIAN);
		
		auto expected = "TDATUM[\"Gregorian calendar\",\n"
			"    CALENDAR[\"proleptic Gregorian\"],\n"
			"    TIMEORIGIN[0000-01-01]]";
		
		EXPECT_EQ(
			datum->exportToWKT(
				WKTFormatter::create(WKTFormatter::Convention::WKT2_2018).get()),
			expected);
	}

	{
		// dynamic_vertical_reference_frame

		auto drf = DynamicVerticalReferenceFrame::create(
			PropertyMap().set(IdentifiedObject::NAME_KEY, "test"),
			optional<std::string>("My anchor"), optional<RealizationMethod>(),
			Measure(2018.5, UnitOfMeasure::YEAR),
			optional<std::string>("My model"));
		
		auto expected = "VDATUM[\"test\",\n"
			"    ANCHOR[\"My anchor\"]]";
		
		EXPECT_EQ(drf->exportToWKT(WKTFormatter::create().get()), expected);
		
		auto expected_wtk2_2018 = "DYNAMIC[\n"
			"    FRAMEEPOCH[2018.5],\n"
			"    MODEL[\"My model\"]],\n"
			"VDATUM[\"test\",\n"
			"    ANCHOR[\"My anchor\"]]";
		EXPECT_EQ(
			drf->exportToWKT(
				WKTFormatter::create(WKTFormatter::Convention::WKT2_2018).get()),
			expected_wtk2_2018);
	}

	{
		// datum_ensemble_vertical

		auto ensemble = DatumEnsemble::create(
			PropertyMap(),
			std::vector<DatumNNPtr>{
			VerticalReferenceFrame::create(
				PropertyMap().set(IdentifiedObject::NAME_KEY, "vdatum1")),
				VerticalReferenceFrame::create(
					PropertyMap().set(IdentifiedObject::NAME_KEY, "vdatum2"))},
			PositionalAccuracy::create("100"));
		EXPECT_EQ(
			ensemble->exportToWKT(
				WKTFormatter::create(WKTFormatter::Convention::WKT2_2018).get()),
			"ENSEMBLE[\"unnamed\",\n"
			"    MEMBER[\"vdatum1\"],\n"
			"    MEMBER[\"vdatum2\"],\n"
			"    ENSEMBLEACCURACY[100]]");
	}

	{
		// edatum

		auto datum = EngineeringDatum::create(
			PropertyMap().set(IdentifiedObject::NAME_KEY, "Engineering datum"),
			optional<std::string>("my anchor"));
		
		auto expected = "EDATUM[\"Engineering datum\",\n"
			"    ANCHOR[\"my anchor\"]]";
		
		EXPECT_EQ(datum->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT2).get()),
			expected);
	}

	{
		// pdatum

		auto datum = ParametricDatum::create(
			PropertyMap().set(IdentifiedObject::NAME_KEY, "Parametric datum"),
			optional<std::string>("my anchor"));
		
		auto expected = "PDATUM[\"Parametric datum\",\n"
			"    ANCHOR[\"my anchor\"]]";
		
		EXPECT_EQ(datum->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT2).get()),
			expected);
	}
}

TEST(PROJ4, io)
{
	// wkt_parsing

	{
		auto n = WKTNode::createFrom("MYNODE[]");
		EXPECT_EQ(n->value(), "MYNODE");
		CHECK(n->children().empty());
		EXPECT_EQ(n->toString(), "MYNODE");
	}
	{
		auto n = WKTNode::createFrom("  MYNODE  [  ]  ");
		EXPECT_EQ(n->value(), "MYNODE");
		CHECK(n->children().empty());
	}
		
	{
		auto n = WKTNode::createFrom("MYNODE[\"x\"]");
		EXPECT_EQ(n->value(), "MYNODE");
		ASSERT_EQ(n->children().size(), 1U);
		EXPECT_EQ(n->children()[0]->value(), "\"x\"");
		EXPECT_EQ(n->toString(), "MYNODE[\"x\"]");
	}
		
	{
		auto n = WKTNode::createFrom("MYNODE[  \"x\"   ]");
		EXPECT_EQ(n->value(), "MYNODE");
		ASSERT_EQ(n->children().size(), 1U);
		EXPECT_EQ(n->children()[0]->value(), "\"x\"");
	}
		
	{
		auto n = WKTNode::createFrom("MYNODE[\"x[\",1]");
		EXPECT_EQ(n->value(), "MYNODE");
		ASSERT_EQ(n->children().size(), 2U);
		EXPECT_EQ(n->children()[0]->value(), "\"x[\"");
		EXPECT_EQ(n->children()[1]->value(), "1");
		EXPECT_EQ(n->toString(), "MYNODE[\"x[\",1]");
	}
		
	{
		auto n = WKTNode::createFrom("A[B[y]]");
		EXPECT_EQ(n->value(), "A");
		ASSERT_EQ(n->children().size(), 1U);
		EXPECT_EQ(n->children()[0]->value(), "B");
		ASSERT_EQ(n->children()[0]->children().size(), 1U);
		EXPECT_EQ(n->children()[0]->children()[0]->value(), "y");
		EXPECT_EQ(n->toString(), "A[B[y]]");
	}

	{
		auto wkt = "A[\"a\",B[\"b\",C[\"c\"]],D[\"d\"]]";
		EXPECT_EQ(WKTNode::createFrom(wkt)->toString(), wkt);
	}

	{
		//	wkt_parsing_with_parenthesis
		
		auto n = WKTNode::createFrom("A(\"x\",B(\"y\"))");
		EXPECT_EQ(n->toString(), "A[\"x\",B[\"y\"]]");
	}

	{
		//	wkt_parsing_with_double_quotes_inside

		auto n = WKTNode::createFrom("A[\"xy\"\"z\"]");
		EXPECT_EQ(n->children()[0]->value(), "\"xy\"z\"");
		EXPECT_EQ(n->toString(), "A[\"xy\"\"z\"]");
	}

	{
		//	wkt_parsing_with_printed_quotes

		static const std::string startPrintedQuote("\xE2\x80\x9C");
		static const std::string endPrintedQuote("\xE2\x80\x9D");
		
		auto n = WKTNode::createFrom("A[" + startPrintedQuote + "x" + endPrintedQuote + "]");
		EXPECT_EQ(n->children()[0]->value(), "\"x\"");
		EXPECT_EQ(n->toString(), "A[\"x\"]");
	}

	{
		// sphere

		auto obj = WKTParser().createFromWKT(
			"ELLIPSOID[\"Sphere\",6378137,0,LENGTHUNIT[\"metre\",1]]");
		auto ellipsoid = nn_dynamic_pointer_cast<Ellipsoid>(obj);
		CHECK(ellipsoid != nullptr);
		CHECK(ellipsoid->isSphere());
	}

	{
		// datum_with_ANCHOR

		auto obj = WKTParser().createFromWKT(
			"DATUM[\"WGS_1984 with anchor\",\n"
			"    ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"        LENGTHUNIT[\"metre\",1,\n"
			"            ID[\"EPSG\",9001]],\n"
			"        ID[\"EPSG\",7030]],\n"
			"    ANCHOR[\"My anchor\"]]");
		auto datum = nn_dynamic_pointer_cast<GeodeticReferenceFrame>(obj);
		CHECK(datum != nullptr);
		EXPECT_EQ(datum->ellipsoid()->celestialBody(), "Earth");
		EXPECT_EQ(datum->primeMeridian()->nameStr(), "Greenwich");
		auto anchor = datum->anchorDefinition();
		CHECK(anchor.has_value());
		EXPECT_EQ(*anchor, "My anchor");
	}

	{
		//  datum_no_pm_not_earth

		auto obj = WKTParser().createFromWKT("DATUM[\"unnamed\",\n"
			"    ELLIPSOID[\"unnamed\",1,0,\n"
			"        LENGTHUNIT[\"metre\",1]]]");
		auto datum = nn_dynamic_pointer_cast<GeodeticReferenceFrame>(obj);
		CHECK(datum != nullptr);
		EXPECT_EQ(datum->ellipsoid()->celestialBody(), "Non-Earth body");
		EXPECT_EQ(datum->primeMeridian()->nameStr(), "Reference meridian");
	}

	{
		// wkt1_projected_with_PROJ4_extension

		auto wkt = "PROJCS[\"unnamed\",\n"
			"    GEOGCS[\"WGS 84\",\n"
			"        DATUM[\"unknown\",\n"
			"            SPHEROID[\"WGS84\",6378137,298.257223563]],\n"
			"        PRIMEM[\"Greenwich\",0],\n"
			"        UNIT[\"degree\",0.0174532925199433]],\n"
			"    PROJECTION[\"Mercator_1SP\"],\n"
			"    PARAMETER[\"central_meridian\",0],\n"
			"    PARAMETER[\"scale_factor\",1],\n"
			"    PARAMETER[\"false_easting\",0],\n"
			"    PARAMETER[\"false_northing\",0],\n"
			"    UNIT[\"Meter\",1],\n"
			"    AXIS[\"Easting\",EAST],\n"
			"    AXIS[\"Northing\",NORTH],\n"
			"    EXTENSION[\"PROJ4\",\"+proj=merc +wktext\"]]";
		auto obj = WKTParser().createFromWKT(wkt);

		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		EXPECT_EQ(
			crs->exportToWKT(
				WKTFormatter::create(WKTFormatter::Convention::WKT1_GDAL).get()),
			wkt);

		EXPECT_EQ(
			crs->exportToPROJString(
				PROJStringFormatter::create(PROJStringFormatter::Convention::PROJ_4)
				.get()),
			"+proj=merc +wktext +type=crs");

		CHECK(
			crs->exportToWKT(WKTFormatter::create().get()).find("EXTENSION") ==
			std::string::npos);

		CHECK(
			crs->exportToWKT(
				WKTFormatter::create(WKTFormatter::Convention::WKT1_ESRI).get())
			.find("EXTENSION") == std::string::npos);
	}

	{
		// wkt1_Mercator_1SP_with_latitude_origin_0

		auto wkt = "PROJCS[\"unnamed\",\n"
			"    GEOGCS[\"WGS 84\",\n"
			"        DATUM[\"unknown\",\n"
			"            SPHEROID[\"WGS84\",6378137,298.257223563]],\n"
			"        PRIMEM[\"Greenwich\",0],\n"
			"        UNIT[\"degree\",0.0174532925199433]],\n"
			"    PROJECTION[\"Mercator_1SP\"],\n"
			"    PARAMETER[\"latitude_of_origin\",0],\n"
			"    PARAMETER[\"central_meridian\",0],\n"
			"    PARAMETER[\"scale_factor\",1],\n"
			"    PARAMETER[\"false_easting\",0],\n"
			"    PARAMETER[\"false_northing\",0],\n"
			"    UNIT[\"Meter\",1],\n"
			"    AXIS[\"Easting\",EAST],\n"
			"    AXIS[\"Northing\",NORTH]]";
		auto obj = WKTParser().createFromWKT(wkt);

		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		auto got_wkt = crs->exportToWKT(
			WKTFormatter::create(WKTFormatter::Convention::WKT1_GDAL).get());
		CHECK(got_wkt.find("Mercator_1SP") != std::string::npos);
	}

	{
		// wkt1_krovak_south_west

		auto wkt =
			"PROJCS[\"S-JTSK / Krovak\","
			"    GEOGCS[\"S-JTSK\","
			"        DATUM[\"System_Jednotne_Trigonometricke_Site_Katastralni\","
			"            SPHEROID[\"Bessel 1841\",6377397.155,299.1528128,"
			"                AUTHORITY[\"EPSG\",\"7004\"]],"
			"            AUTHORITY[\"EPSG\",\"6156\"]],"
			"        PRIMEM[\"Greenwich\",0,"
			"            AUTHORITY[\"EPSG\",\"8901\"]],"
			"        UNIT[\"degree\",0.0174532925199433,"
			"            AUTHORITY[\"EPSG\",\"9122\"]],"
			"        AUTHORITY[\"EPSG\",\"4156\"]],"
			"    PROJECTION[\"Krovak\"],"
			"    PARAMETER[\"latitude_of_center\",49.5],"
			"    PARAMETER[\"longitude_of_center\",24.83333333333333],"
			"    PARAMETER[\"azimuth\",30.28813972222222],"
			"    PARAMETER[\"pseudo_standard_parallel_1\",78.5],"
			"    PARAMETER[\"scale_factor\",0.9999],"
			"    PARAMETER[\"false_easting\",0],"
			"    PARAMETER[\"false_northing\",0],"
			"    UNIT[\"metre\",1,"
			"        AUTHORITY[\"EPSG\",\"9001\"]],"
			"    AXIS[\"X\",SOUTH],"
			"    AXIS[\"Y\",WEST],"
			"    AUTHORITY[\"EPSG\",\"5513\"]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->derivingConversion()->method()->nameStr(), "Krovak");

		auto expected_wkt2 =
			"PROJCRS[\"S-JTSK / Krovak\",\n"
			"    BASEGEODCRS[\"S-JTSK\",\n"
			"        DATUM[\"System_Jednotne_Trigonometricke_Site_Katastralni\",\n"
			"            ELLIPSOID[\"Bessel 1841\",6377397.155,299.1528128,\n"
			"                LENGTHUNIT[\"metre\",1]]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"    CONVERSION[\"unnamed\",\n"
			"        METHOD[\"Krovak\",\n"
			"            ID[\"EPSG\",9819]],\n"
			"        PARAMETER[\"Latitude of projection centre\",49.5,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8811]],\n"
			"        PARAMETER[\"Longitude of origin\",24.8333333333333,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8833]],\n"
			"        PARAMETER[\"Co-latitude of cone axis\",30.2881397222222,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",1036]],\n"
			"        PARAMETER[\"Latitude of pseudo standard parallel\",78.5,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8818]],\n"
			"        PARAMETER[\"Scale factor on pseudo standard "
			"parallel\",0.9999,\n"
			"            SCALEUNIT[\"unity\",1],\n"
			"            ID[\"EPSG\",8819]],\n"
			"        PARAMETER[\"False easting\",0,\n"
			"            LENGTHUNIT[\"metre\",1],\n"
			"            ID[\"EPSG\",8806]],\n"
			"        PARAMETER[\"False northing\",0,\n"
			"            LENGTHUNIT[\"metre\",1],\n"
			"            ID[\"EPSG\",8807]]],\n"
			"    CS[Cartesian,2],\n"
			"        AXIS[\"x\",south,\n"
			"            ORDER[1],\n"
			"            LENGTHUNIT[\"metre\",1]],\n"
			"        AXIS[\"y\",west,\n"
			"            ORDER[2],\n"
			"            LENGTHUNIT[\"metre\",1]],\n"
			"    ID[\"EPSG\",5513]]";

		EXPECT_EQ(crs->exportToWKT(WKTFormatter::create().get()), expected_wkt2);

		auto projString =
			crs->exportToPROJString(PROJStringFormatter::create().get());
		auto expectedPROJString = "+proj=krovak +axis=swu +lat_0=49.5 "
			"+lon_0=24.8333333333333 +alpha=30.2881397222222 "
			"+k=0.9999 +x_0=0 +y_0=0 +ellps=bessel +units=m "
			"+no_defs +type=crs";
		EXPECT_EQ(projString, expectedPROJString);

		obj = PROJStringParser().createFromPROJString(projString);
		crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		auto wkt2 = crs->exportToWKT(WKTFormatter::create().get());
		CHECK(wkt2.find("METHOD[\"Krovak\"") != std::string::npos);
		CHECK(
			wkt2.find("PARAMETER[\"Latitude of pseudo standard parallel\",78.5,") !=
			std::string::npos);
		CHECK(
			wkt2.find("PARAMETER[\"Co-latitude of cone axis\",30.2881397222222,") !=
			std::string::npos);
		EXPECT_EQ(crs->exportToPROJString(PROJStringFormatter::create().get()),
			expectedPROJString);

		obj = PROJStringParser().createFromPROJString(
			"+type=crs +proj=pipeline +step +proj=unitconvert +xy_in=deg "
			"+xy_out=rad "
			"+step +proj=krovak +lat_0=49.5 "
			"+lon_0=24.8333333333333 +alpha=30.2881397222222 "
			"+k=0.9999 +x_0=0 +y_0=0 +ellps=bessel "
			"+step +proj=axisswap +order=-2,-1");
		crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		wkt2 = crs->exportToWKT(WKTFormatter::create().get());
		CHECK(wkt2.find("METHOD[\"Krovak\"") != std::string::npos);
	}

	{
		// wkt1_krovak_north_oriented

		auto wkt =
			"PROJCS[\"S-JTSK / Krovak East North\","
			"    GEOGCS[\"S-JTSK\","
			"        DATUM[\"System_Jednotne_Trigonometricke_Site_Katastralni\","
			"            SPHEROID[\"Bessel 1841\",6377397.155,299.1528128,"
			"                AUTHORITY[\"EPSG\",\"7004\"]],"
			"            AUTHORITY[\"EPSG\",\"6156\"]],"
			"        PRIMEM[\"Greenwich\",0,"
			"            AUTHORITY[\"EPSG\",\"8901\"]],"
			"        UNIT[\"degree\",0.0174532925199433,"
			"            AUTHORITY[\"EPSG\",\"9122\"]],"
			"        AUTHORITY[\"EPSG\",\"4156\"]],"
			"    PROJECTION[\"Krovak\"],"
			"    PARAMETER[\"latitude_of_center\",49.5],"
			"    PARAMETER[\"longitude_of_center\",24.83333333333333],"
			"    PARAMETER[\"azimuth\",30.28813972222222],"
			"    PARAMETER[\"pseudo_standard_parallel_1\",78.5],"
			"    PARAMETER[\"scale_factor\",0.9999],"
			"    PARAMETER[\"false_easting\",0],"
			"    PARAMETER[\"false_northing\",0],"
			"    UNIT[\"metre\",1,"
			"        AUTHORITY[\"EPSG\",\"9001\"]],"
			"    AXIS[\"X\",EAST],"
			"    AXIS[\"Y\",NORTH],"
			"    AUTHORITY[\"EPSG\",\"5514\"]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->derivingConversion()->method()->nameStr(),
			"Krovak (North Orientated)");

		EXPECT_EQ(
			crs->exportToWKT(WKTFormatter::create().get()),
			"PROJCRS[\"S-JTSK / Krovak East North\",\n"
			"    BASEGEODCRS[\"S-JTSK\",\n"
			"        DATUM[\"System_Jednotne_Trigonometricke_Site_Katastralni\",\n"
			"            ELLIPSOID[\"Bessel 1841\",6377397.155,299.1528128,\n"
			"                LENGTHUNIT[\"metre\",1]]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"    CONVERSION[\"unnamed\",\n"
			"        METHOD[\"Krovak (North Orientated)\",\n"
			"            ID[\"EPSG\",1041]],\n"
			"        PARAMETER[\"Latitude of projection centre\",49.5,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8811]],\n"
			"        PARAMETER[\"Longitude of origin\",24.8333333333333,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8833]],\n"
			"        PARAMETER[\"Co-latitude of cone axis\",30.2881397222222,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",1036]],\n"
			"        PARAMETER[\"Latitude of pseudo standard parallel\",78.5,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8818]],\n"
			"        PARAMETER[\"Scale factor on pseudo standard "
			"parallel\",0.9999,\n"
			"            SCALEUNIT[\"unity\",1],\n"
			"            ID[\"EPSG\",8819]],\n"
			"        PARAMETER[\"False easting\",0,\n"
			"            LENGTHUNIT[\"metre\",1],\n"
			"            ID[\"EPSG\",8806]],\n"
			"        PARAMETER[\"False northing\",0,\n"
			"            LENGTHUNIT[\"metre\",1],\n"
			"            ID[\"EPSG\",8807]]],\n"
			"    CS[Cartesian,2],\n"
			"        AXIS[\"x\",east,\n"
			"            ORDER[1],\n"
			"            LENGTHUNIT[\"metre\",1]],\n"
			"        AXIS[\"y\",north,\n"
			"            ORDER[2],\n"
			"            LENGTHUNIT[\"metre\",1]],\n"
			"    ID[\"EPSG\",5514]]");

		EXPECT_EQ(crs->exportToPROJString(PROJStringFormatter::create().get()),
			"+proj=krovak +lat_0=49.5 +lon_0=24.8333333333333 "
			"+alpha=30.2881397222222 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel "
			"+units=m +no_defs +type=crs");
	}

	{
		// wkt1_polar_stereographic_latitude_of_origin_70

		auto wkt = "PROJCS[\"unknown\",\n"
			"    GEOGCS[\"unknown\",\n"
			"        DATUM[\"WGS_1984\",\n"
			"            SPHEROID[\"WGS 84\",6378137,298.257223563,\n"
			"                AUTHORITY[\"EPSG\",\"7030\"]],\n"
			"            AUTHORITY[\"EPSG\",\"6326\"]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            AUTHORITY[\"EPSG\",\"8901\"]],\n"
			"        UNIT[\"degree\",0.0174532925199433,\n"
			"            AUTHORITY[\"EPSG\",\"9122\"]]],\n"
			"    PROJECTION[\"Polar_Stereographic\"],\n"
			"    PARAMETER[\"latitude_of_origin\",70],\n"
			"    PARAMETER[\"central_meridian\",2],\n"
			"    PARAMETER[\"false_easting\",3],\n"
			"    PARAMETER[\"false_northing\",4],\n"
			"    UNIT[\"metre\",1,\n"
			"        AUTHORITY[\"EPSG\",\"9001\"]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		auto projString = crs->exportToPROJString(
			PROJStringFormatter::create(PROJStringFormatter::Convention::PROJ_4)
			.get());
		auto expectedPROJString =
			"+proj=stere +lat_0=90 +lat_ts=70 +lon_0=2 "
			"+x_0=3 +y_0=4 +datum=WGS84 +units=m +no_defs +type=crs";
		EXPECT_EQ(projString, expectedPROJString);

		EXPECT_EQ(crs->coordinateSystem()->axisList()[0]->nameStr(), "Easting");
		EXPECT_EQ(crs->coordinateSystem()->axisList()[0]->direction(),
			AxisDirection::SOUTH);
		EXPECT_EQ(crs->coordinateSystem()->axisList()[1]->nameStr(), "Northing");
		EXPECT_EQ(crs->coordinateSystem()->axisList()[1]->direction(),
			AxisDirection::SOUTH);
	}

	{
		// wkt1_polar_stereographic_latitude_of_origin_minus_70

		auto wkt = "PROJCS[\"unknown\",\n"
			"    GEOGCS[\"unknown\",\n"
			"        DATUM[\"WGS_1984\",\n"
			"            SPHEROID[\"WGS 84\",6378137,298.257223563,\n"
			"                AUTHORITY[\"EPSG\",\"7030\"]],\n"
			"            AUTHORITY[\"EPSG\",\"6326\"]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            AUTHORITY[\"EPSG\",\"8901\"]],\n"
			"        UNIT[\"degree\",0.0174532925199433,\n"
			"            AUTHORITY[\"EPSG\",\"9122\"]]],\n"
			"    PROJECTION[\"Polar_Stereographic\"],\n"
			"    PARAMETER[\"latitude_of_origin\",-70],\n"
			"    PARAMETER[\"central_meridian\",2],\n"
			"    PARAMETER[\"false_easting\",3],\n"
			"    PARAMETER[\"false_northing\",4],\n"
			"    UNIT[\"metre\",1,\n"
			"        AUTHORITY[\"EPSG\",\"9001\"]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->coordinateSystem()->axisList()[0]->nameStr(), "Easting");
		EXPECT_EQ(crs->coordinateSystem()->axisList()[0]->direction(), AxisDirection::NORTH);
		EXPECT_EQ(crs->coordinateSystem()->axisList()[1]->nameStr(), "Northing");
		EXPECT_EQ(crs->coordinateSystem()->axisList()[1]->direction(), AxisDirection::NORTH);
	}

	{
		// wkt1_polar_stereographic_latitude_of_origin_90

		auto wkt = "PROJCS[\"unknown\",\n"
			"    GEOGCS[\"unknown\",\n"
			"        DATUM[\"WGS_1984\",\n"
			"            SPHEROID[\"WGS 84\",6378137,298.257223563,\n"
			"                AUTHORITY[\"EPSG\",\"7030\"]],\n"
			"            AUTHORITY[\"EPSG\",\"6326\"]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            AUTHORITY[\"EPSG\",\"8901\"]],\n"
			"        UNIT[\"degree\",0.0174532925199433,\n"
			"            AUTHORITY[\"EPSG\",\"9122\"]]],\n"
			"    PROJECTION[\"Polar_Stereographic\"],\n"
			"    PARAMETER[\"latitude_of_origin\",90],\n"
			"    PARAMETER[\"central_meridian\",2],\n"
			"    PARAMETER[\"false_easting\",3],\n"
			"    PARAMETER[\"false_northing\",4],\n"
			"    UNIT[\"metre\",1,\n"
			"        AUTHORITY[\"EPSG\",\"9001\"]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		auto projString = crs->exportToPROJString(
			PROJStringFormatter::create(PROJStringFormatter::Convention::PROJ_4)
			.get());
		auto expectedPROJString =
			"+proj=stere +lat_0=90 +lat_ts=90 +lon_0=2 "
			"+x_0=3 +y_0=4 +datum=WGS84 +units=m +no_defs +type=crs";
		EXPECT_EQ(projString, expectedPROJString);
	}

	{
		// wkt1_polar_stereographic_latitude_of_origin_90_scale_factor_1

		auto wkt = "PROJCS[\"unknown\",\n"
			"    GEOGCS[\"unknown\",\n"
			"        DATUM[\"WGS_1984\",\n"
			"            SPHEROID[\"WGS 84\",6378137,298.257223563,\n"
			"                AUTHORITY[\"EPSG\",\"7030\"]],\n"
			"            AUTHORITY[\"EPSG\",\"6326\"]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            AUTHORITY[\"EPSG\",\"8901\"]],\n"
			"        UNIT[\"degree\",0.0174532925199433,\n"
			"            AUTHORITY[\"EPSG\",\"9122\"]]],\n"
			"    PROJECTION[\"Polar_Stereographic\"],\n"
			"    PARAMETER[\"latitude_of_origin\",90],\n"
			"    PARAMETER[\"central_meridian\",2],\n"
			"    PARAMETER[\"scale_factor\",1],\n"
			"    PARAMETER[\"false_easting\",3],\n"
			"    PARAMETER[\"false_northing\",4],\n"
			"    UNIT[\"metre\",1,\n"
			"        AUTHORITY[\"EPSG\",\"9001\"]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		auto projString = crs->exportToPROJString(
			PROJStringFormatter::create(PROJStringFormatter::Convention::PROJ_4)
			.get());
		auto expectedPROJString =
			"+proj=stere +lat_0=90 +lat_ts=90 +lon_0=2 "
			"+x_0=3 +y_0=4 +datum=WGS84 +units=m +no_defs +type=crs";
		EXPECT_EQ(projString, expectedPROJString);
	}

	{
		// wkt1_polar_stereographic_scale_factor

		auto wkt =
			"PROJCS[\"unknown\",\n"
			"    GEOGCS[\"unknown\",\n"
			"        DATUM[\"WGS_1984\",\n"
			"            SPHEROID[\"WGS 84\",6378137,298.257223563,\n"
			"                AUTHORITY[\"EPSG\",\"7030\"]],\n"
			"            AUTHORITY[\"EPSG\",\"6326\"]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            AUTHORITY[\"EPSG\",\"8901\"]],\n"
			"        UNIT[\"degree\",0.0174532925199433,\n"
			"            AUTHORITY[\"EPSG\",\"9122\"]]],\n"
			"    PROJECTION[\"Polar_Stereographic\"],\n"
			"    PARAMETER[\"latitude_of_origin\",90],\n"
			"    PARAMETER[\"central_meridian\",2],\n"
			"    PARAMETER[\"scale_factor\",0.99],\n"
			"    PARAMETER[\"false_easting\",3],\n"
			"    PARAMETER[\"false_northing\",4],\n"
			"    UNIT[\"metre\",1,\n"
			"        AUTHORITY[\"EPSG\",\"9001\"]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		auto projString = crs->exportToPROJString(
			PROJStringFormatter::create(PROJStringFormatter::Convention::PROJ_4)
			.get());
		auto expectedPROJString = "+proj=stere +lat_0=90 +lon_0=2 +k=0.99 +x_0=3 "
			"+y_0=4 +datum=WGS84 +units=m +no_defs +type=crs";
		EXPECT_EQ(projString, expectedPROJString);
	}

	{
		// wkt1_Spherical_Cross_Track_Height

		auto wkt =
			"PROJCS[\"unknown\",\n"
			"    GEOGCS[\"unknown\",\n"
			"        DATUM[\"WGS_1984\",\n"
			"            SPHEROID[\"WGS 84\",6378137,298.257223563,\n"
			"                AUTHORITY[\"EPSG\",\"7030\"]],\n"
			"            AUTHORITY[\"EPSG\",\"6326\"]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            AUTHORITY[\"EPSG\",\"8901\"]],\n"
			"        UNIT[\"degree\",0.0174532925199433,\n"
			"            AUTHORITY[\"EPSG\",\"9122\"]]],\n"
			"    PROJECTION[\"Spherical_Cross_Track_Height\"],\n"
			"    PARAMETER[\"peg_point_latitude\",1],\n"
			"    PARAMETER[\"peg_point_longitude\",2],\n"
			"    PARAMETER[\"peg_point_heading\",3],\n"
			"    PARAMETER[\"peg_point_height\",4],\n"
			"    UNIT[\"metre\",1,\n"
			"        AUTHORITY[\"EPSG\",\"9001\"]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		auto projString = crs->exportToPROJString(
			PROJStringFormatter::create(PROJStringFormatter::Convention::PROJ_4)
			.get());
		auto expectedPROJString = "+proj=sch +plat_0=1 +plon_0=2 +phdg_0=3 +h_0=4 "
			"+datum=WGS84 +units=m +no_defs +type=crs";
		EXPECT_EQ(projString, expectedPROJString);
	}

	{
		// vertcrs_VRF_WKT2

		auto wkt =
			"VERTCRS[\"ODN height\",\n"
			"    VRF[\"Ordnance Datum Newlyn\"],\n"
			"    CS[vertical,1],\n"
			"        AXIS[\"gravity-related height (H)\",up,\n"
			"            LENGTHUNIT[\"metre\",1]],\n"
			"    ID[\"EPSG\",5701]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<VerticalCRS>(obj);
		CHECK(crs != nullptr);

		//proj_destroy(obj) ???
	}

	{
		// vertcrs_with_GEOIDMODEL

		auto wkt =
			"VERTCRS[\"CGVD2013\","
			"    VRF[\"Canadian Geodetic Vertical Datum of 2013\"],"
			"    CS[vertical,1],"
			"        AXIS[\"gravity-related height (H)\",up],"
			"        LENGTHUNIT[\"metre\",1.0],"
			"    GEOIDMODEL[\"CGG2013\",ID[\"EPSG\",6648]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<VerticalCRS>(obj);
		CHECK(crs != nullptr);

		//proj_destroy(obj) ???
	}

	{
		// vertcrs_WKT1_GDAL

		auto wkt = "VERT_CS[\"ODN height\",\n"
			"    VERT_DATUM[\"Ordnance Datum Newlyn\",2005,\n"
			"        AUTHORITY[\"EPSG\",\"5101\"]],\n"
			"    UNIT[\"metre\",1,\n"
			"        AUTHORITY[\"EPSG\",\"9001\"]],\n"
			"    AXIS[\"gravity-related height\",UP],\n"
			"    AUTHORITY[\"EPSG\",\"5701\"]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<VerticalCRS>(obj);
		CHECK(crs != nullptr);
		EXPECT_EQ(crs->nameStr(), "ODN height");
		ASSERT_EQ(crs->identifiers().size(), 1U);
		EXPECT_EQ(crs->identifiers()[0]->code(), "5701");
		EXPECT_EQ(*(crs->identifiers()[0]->codeSpace()), "EPSG");

		auto datum = crs->datum();
		EXPECT_EQ(datum->nameStr(), "Ordnance Datum Newlyn");
		ASSERT_EQ(datum->identifiers().size(), 1U);
		EXPECT_EQ(datum->identifiers()[0]->code(), "5101");
		EXPECT_EQ(*(datum->identifiers()[0]->codeSpace()), "EPSG");

		auto cs = crs->coordinateSystem();
		ASSERT_EQ(cs->axisList().size(), 1U);
		EXPECT_EQ(cs->axisList()[0]->nameStr(), "Gravity-related height");
		EXPECT_EQ(cs->axisList()[0]->abbreviation(), ""); // "H" in WKT2
		EXPECT_EQ(cs->axisList()[0]->direction(), AxisDirection::UP);
	}

	{
		// vertcrs_WKT1_GDAL_minimum

		auto wkt = "VERT_CS[\"ODN height\",\n"
			"    VERT_DATUM[\"Ordnance Datum Newlyn\",2005],\n"
			"    UNIT[\"metre\",1]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<VerticalCRS>(obj);
		EXPECT_EQ(crs->nameStr(), "ODN height");

		auto datum = crs->datum();
		EXPECT_EQ(datum->nameStr(), "Ordnance Datum Newlyn");

		auto cs = crs->coordinateSystem();
		ASSERT_EQ(cs->axisList().size(), 1U);
		EXPECT_EQ(cs->axisList()[0]->nameStr(), "Gravity-related height");
		EXPECT_EQ(cs->axisList()[0]->direction(), AxisDirection::UP);
	}

	{
		// dynamic_vertical_reference_frame

		auto obj = WKTParser().createFromWKT(
			"VERTCRS[\"RH2000\","
			"  DYNAMIC[FRAMEEPOCH[2000.0],MODEL[\"NKG2016LU\"]],"
			"  VDATUM[\"Rikets Hojdsystem 2000\",ANCHOR[\"my anchor\"]],"
			"  CS[vertical,1],"
			"    AXIS[\"gravity-related height (H)\",up],"
			"    LENGTHUNIT[\"metre\",1.0]"
			"]");
		auto crs = nn_dynamic_pointer_cast<VerticalCRS>(obj);
		CHECK(crs != nullptr);
		auto dgrf =
			std::dynamic_pointer_cast<DynamicVerticalReferenceFrame>(crs->datum());
		CHECK(dgrf != nullptr);
		auto anchor = dgrf->anchorDefinition();
		CHECK(anchor.has_value());
		EXPECT_EQ(*anchor, "my anchor");
		CHECK(dgrf->frameReferenceEpoch() ==
			Measure(2000.0, UnitOfMeasure::YEAR));
		auto model = dgrf->deformationModelName();
		CHECK(model.has_value());
		EXPECT_EQ(*model, "NKG2016LU");
	}

	{
		// vertcrs_with_ensemble

		auto obj = WKTParser().createFromWKT(
			"VERTCRS[\"unnamed\",\n"
			"    ENSEMBLE[\"unnamed\",\n"
			"        MEMBER[\"vdatum1\"],\n"
			"        MEMBER[\"vdatum2\"],\n"
			"        ENSEMBLEACCURACY[100]],\n"
			"    CS[vertical,1],\n"
			"        AXIS[\"gravity-related height (H)\",up,\n"
			"            LENGTHUNIT[\"metre\",1]]]");
		auto crs = nn_dynamic_pointer_cast<VerticalCRS>(obj);
		CHECK(crs != nullptr);
		CHECK(crs->datum() == nullptr);
		CHECK(crs->datumEnsemble() != nullptr);
		EXPECT_EQ(crs->datumEnsemble()->datums().size(), 2U);
	}

	{
		// vdatum_with_ANCHOR

		auto obj = WKTParser().createFromWKT("VDATUM[\"Ordnance Datum Newlyn\",\n"
			"    ANCHOR[\"my anchor\"],\n"
			"    ID[\"EPSG\",5101]]");
		auto datum = nn_dynamic_pointer_cast<VerticalReferenceFrame>(obj);
		CHECK(datum != nullptr);
		auto anchor = datum->anchorDefinition();
		CHECK(anchor.has_value());
		EXPECT_EQ(*anchor, "my anchor");
	}

	{
		// COMPOUNDCRS

		auto obj = WKTParser().createFromWKT(
			"COMPOUNDCRS[\"horizontal + vertical\",\n"
			"    PROJCRS[\"WGS 84 / UTM zone 31N\",\n"
			"        BASEGEODCRS[\"WGS 84\",\n"
			"            DATUM[\"World Geodetic System 1984\",\n"
			"                ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                    LENGTHUNIT[\"metre\",1]]],\n"
			"            PRIMEM[\"Greenwich\",0,\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"        CONVERSION[\"UTM zone 31N\",\n"
			"            METHOD[\"Transverse Mercator\",\n"
			"                ID[\"EPSG\",9807]],\n"
			"            PARAMETER[\"Latitude of natural origin\",0,\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"                ID[\"EPSG\",8801]],\n"
			"            PARAMETER[\"Longitude of natural origin\",3,\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"                ID[\"EPSG\",8802]],\n"
			"            PARAMETER[\"Scale factor at natural origin\",0.9996,\n"
			"                SCALEUNIT[\"unity\",1],\n"
			"                ID[\"EPSG\",8805]],\n"
			"            PARAMETER[\"False easting\",500000,\n"
			"                LENGTHUNIT[\"metre\",1],\n"
			"                ID[\"EPSG\",8806]],\n"
			"            PARAMETER[\"False northing\",0,\n"
			"                LENGTHUNIT[\"metre\",1],\n"
			"                ID[\"EPSG\",8807]]],\n"
			"        CS[Cartesian,2],\n"
			"            AXIS[\"(E)\",east,\n"
			"                ORDER[1],\n"
			"                LENGTHUNIT[\"metre\",1]],\n"
			"            AXIS[\"(N)\",north,\n"
			"                ORDER[2],\n"
			"                LENGTHUNIT[\"metre\",1]]],\n"
			"    VERTCRS[\"ODN height\",\n"
			"        VDATUM[\"Ordnance Datum Newlyn\"],\n"
			"        CS[vertical,1],\n"
			"            AXIS[\"gravity-related height (H)\",up,\n"
			"                LENGTHUNIT[\"metre\",1]]],\n"
			"    ID[\"codespace\",\"code\"]]");
		auto crs = nn_dynamic_pointer_cast<CompoundCRS>(obj);
		CHECK(crs != nullptr);
		EXPECT_EQ(crs->nameStr(), "horizontal + vertical");
		EXPECT_EQ(crs->componentReferenceSystems().size(), 2U);
		ASSERT_EQ(crs->identifiers().size(), 1U);
		EXPECT_EQ(crs->identifiers()[0]->code(), "code");
		EXPECT_EQ(*(crs->identifiers()[0]->codeSpace()), "codespace");
	}

	{
		// COMPOUNDCRS_spatio_parametric_2015

		auto obj = WKTParser().createFromWKT(
			"COMPOUNDCRS[\"ICAO layer 0\",\n"
			"    GEODETICCRS[\"WGS 84\",\n"
			"        DATUM[\"World Geodetic System 1984\",\n"
			"            ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                LENGTHUNIT[\"metre\",1]]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8901]],\n"
			"        CS[ellipsoidal,2],\n"
			"            AXIS[\"latitude\",north,\n"
			"                ORDER[1],\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"            AXIS[\"longitude\",east,\n"
			"                ORDER[2],\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"    PARAMETRICCRS[\"WMO standard atmosphere\",\n"
			"        PARAMETRICDATUM[\"Mean Sea Level\",\n"
			"            ANCHOR[\"Mean Sea Level = 1013.25 hPa\"]],\n"
			"        CS[parametric,1],\n"
			"            AXIS[\"pressure (P)\",unspecified,\n"
			"                PARAMETRICUNIT[\"HectoPascal\",100]]]]");
		auto crs = nn_dynamic_pointer_cast<CompoundCRS>(obj);
		CHECK(crs != nullptr);
	}

	{
		// COMPOUNDCRS_spatio_parametric_2018

		auto obj = WKTParser().createFromWKT(
			"COMPOUNDCRS[\"ICAO layer 0\",\n"
			"    GEOGRAPHICCRS[\"WGS 84\",\n"
			"    DYNAMIC[FRAMEEPOCH[2005]],\n"
			"        DATUM[\"World Geodetic System 1984\",\n"
			"            ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                LENGTHUNIT[\"metre\",1]]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8901]],\n"
			"        CS[ellipsoidal,2],\n"
			"            AXIS[\"latitude\",north,\n"
			"                ORDER[1],\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"            AXIS[\"longitude\",east,\n"
			"                ORDER[2],\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"    PARAMETRICCRS[\"WMO standard atmosphere\",\n"
			"        PARAMETRICDATUM[\"Mean Sea Level\",\n"
			"            ANCHOR[\"Mean Sea Level = 1013.25 hPa\"]],\n"
			"        CS[parametric,1],\n"
			"            AXIS[\"pressure (P)\",unspecified,\n"
			"                PARAMETRICUNIT[\"HectoPascal\",100]]]]");
		auto crs = nn_dynamic_pointer_cast<CompoundCRS>(obj);
		CHECK(crs != nullptr);
	}

	{
		// COMPOUNDCRS_spatio_temporal_2015

		auto obj = WKTParser().createFromWKT(
			"COMPOUNDCRS[\"GPS position and time\",\n"
			"    GEODCRS[\"WGS 84 (G1762)\",\n"
			"        DATUM[\"World Geodetic System 1984 (G1762)\",\n"
			"            ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                LENGTHUNIT[\"metre\",1,\n"
			"                    ID[\"EPSG\",9001]]]],\n"
			"        CS[ellipsoidal,2],\n"
			"            AXIS[\"latitude\",north,\n"
			"                ORDER[1],\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"            AXIS[\"longitude\",east,\n"
			"                ORDER[2],\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"    TIMECRS[\"GPS Time\",\n"
			"        TIMEDATUM[\"Time origin\",TIMEORIGIN[1980-01-01]],\n"
			"        CS[temporal,1],\n"
			"            AXIS[\"time (T)\",future]]]");
		auto crs = nn_dynamic_pointer_cast<CompoundCRS>(obj);
		CHECK(crs != nullptr);
	}

	{
		// COMPOUNDCRS_spatio_temporal_2018

		auto obj = WKTParser().createFromWKT(
			"COMPOUNDCRS[\"2D GPS position with civil time in ISO 8601 format\",\n"
			"    GEOGCRS[\"WGS 84 (G1762)\",\n"
			"        DATUM[\"World Geodetic System 1984 (G1762)\",\n"
			"            ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                LENGTHUNIT[\"metre\",1,\n"
			"                    ID[\"EPSG\",9001]]]],\n"
			"        CS[ellipsoidal,2],\n"
			"            AXIS[\"latitude\",north,\n"
			"                ORDER[1],\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"            AXIS[\"longitude\",east,\n"
			"                ORDER[2],\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"    TIMECRS[\"DateTime\",\n"
			"        TDATUM[\"Gregorian Calendar\"],\n"
			"        CS[TemporalDateTime,1],\n"
			"            AXIS[\"time (T)\",future]]]");
		auto crs = nn_dynamic_pointer_cast<CompoundCRS>(obj);
		CHECK(crs != nullptr);
	}

	{
		// CONCATENATEDOPERATION_with_conversion_coordinateoperation_to_inverse_conversion

		auto wkt =
			"CONCATENATEDOPERATION[\"Inverse of UTM zone 11N + NAD27 to WGS 84 "
			"(79) + UTM zone 11N\",\n"
			"    SOURCECRS[\n"
			"        PROJCRS[\"WGS 84 / UTM zone 11N\",\n"
			"            BASEGEOGCRS[\"WGS 84\",\n"
			"                DATUM[\"World Geodetic System 1984\",\n"
			"                    ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                        LENGTHUNIT[\"metre\",1]]],\n"
			"                PRIMEM[\"Greenwich\",0,\n"
			"                    ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"            CONVERSION[\"UTM zone 11N\",\n"
			"                METHOD[\"Transverse Mercator\",\n"
			"                    ID[\"EPSG\",9807]],\n"
			"                PARAMETER[\"Latitude of natural origin\",0,\n"
			"                    ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"                    ID[\"EPSG\",8801]],\n"
			"                PARAMETER[\"Longitude of natural origin\",-117,\n"
			"                    ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"                    ID[\"EPSG\",8802]],\n"
			"                PARAMETER[\"Scale factor at natural origin\",0.9996,\n"
			"                    SCALEUNIT[\"unity\",1],\n"
			"                    ID[\"EPSG\",8805]],\n"
			"                PARAMETER[\"False easting\",500000,\n"
			"                    LENGTHUNIT[\"metre\",1],\n"
			"                    ID[\"EPSG\",8806]],\n"
			"                PARAMETER[\"False northing\",0,\n"
			"                    LENGTHUNIT[\"metre\",1],\n"
			"                    ID[\"EPSG\",8807]]],\n"
			"            CS[Cartesian,2],\n"
			"                AXIS[\"(E)\",east,\n"
			"                    ORDER[1],\n"
			"                    LENGTHUNIT[\"metre\",1]],\n"
			"                AXIS[\"(N)\",north,\n"
			"                    ORDER[2],\n"
			"                    LENGTHUNIT[\"metre\",1]],\n"
			"            ID[\"EPSG\",32611]]],\n"
			"    TARGETCRS[\n"
			"        PROJCRS[\"NAD27 / UTM zone 11N\",\n"
			"            BASEGEOGCRS[\"NAD27\",\n"
			"                DATUM[\"North American Datum 1927\",\n"
			"                    ELLIPSOID[\"Clarke "
			"1866\",6378206.4,294.978698213898,\n"
			"                        LENGTHUNIT[\"metre\",1]]],\n"
			"                PRIMEM[\"Greenwich\",0,\n"
			"                    ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"            CONVERSION[\"UTM zone 11N\",\n"
			"                METHOD[\"Transverse Mercator\",\n"
			"                    ID[\"EPSG\",9807]],\n"
			"                PARAMETER[\"Latitude of natural origin\",0,\n"
			"                    ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"                    ID[\"EPSG\",8801]],\n"
			"                PARAMETER[\"Longitude of natural origin\",-117,\n"
			"                    ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"                    ID[\"EPSG\",8802]],\n"
			"                PARAMETER[\"Scale factor at natural origin\",0.9996,\n"
			"                    SCALEUNIT[\"unity\",1],\n"
			"                    ID[\"EPSG\",8805]],\n"
			"                PARAMETER[\"False easting\",500000,\n"
			"                    LENGTHUNIT[\"metre\",1],\n"
			"                    ID[\"EPSG\",8806]],\n"
			"                PARAMETER[\"False northing\",0,\n"
			"                    LENGTHUNIT[\"metre\",1],\n"
			"                    ID[\"EPSG\",8807]]],\n"
			"            CS[Cartesian,2],\n"
			"                AXIS[\"(E)\",east,\n"
			"                    ORDER[1],\n"
			"                    LENGTHUNIT[\"metre\",1]],\n"
			"                AXIS[\"(N)\",north,\n"
			"                    ORDER[2],\n"
			"                    LENGTHUNIT[\"metre\",1]],\n"
			"            ID[\"EPSG\",26711]]],\n"
			"    STEP[\n"
			"        CONVERSION[\"Inverse of UTM zone 11N\",\n"
			"            METHOD[\"Inverse of Transverse Mercator\",\n"
			"                ID[\"INVERSE(EPSG)\",9807]],\n"
			"            PARAMETER[\"Latitude of natural origin\",0,\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"                ID[\"EPSG\",8801]],\n"
			"            PARAMETER[\"Longitude of natural origin\",-117,\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"                ID[\"EPSG\",8802]],\n"
			"            PARAMETER[\"Scale factor at natural origin\",0.9996,\n"
			"                SCALEUNIT[\"unity\",1],\n"
			"                ID[\"EPSG\",8805]],\n"
			"            PARAMETER[\"False easting\",500000,\n"
			"                LENGTHUNIT[\"metre\",1],\n"
			"                ID[\"EPSG\",8806]],\n"
			"            PARAMETER[\"False northing\",0,\n"
			"                LENGTHUNIT[\"metre\",1],\n"
			"                ID[\"EPSG\",8807]],\n"
			"            ID[\"INVERSE(EPSG)\",16011]]],\n"
			"    STEP[\n"
			"        COORDINATEOPERATION[\"NAD27 to WGS 84 (79)\",\n"
			"            SOURCECRS[\n"
			"                GEOGCRS[\"NAD27\",\n"
			"                    DATUM[\"North American Datum 1927\",\n"
			"                        ELLIPSOID[\"Clarke "
			"1866\",6378206.4,294.978698213898,\n"
			"                            LENGTHUNIT[\"metre\",1]]],\n"
			"                    PRIMEM[\"Greenwich\",0,\n"
			"                        ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"                    CS[ellipsoidal,2],\n"
			"                        AXIS[\"geodetic latitude (Lat)\",north,\n"
			"                            ORDER[1],\n"
			"                            "
			"ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"                        AXIS[\"geodetic longitude (Lon)\",east,\n"
			"                            ORDER[2],\n"
			"                            "
			"ANGLEUNIT[\"degree\",0.0174532925199433]]]],\n"
			"            TARGETCRS[\n"
			"                GEOGCRS[\"WGS 84\",\n"
			"                    DATUM[\"World Geodetic System 1984\",\n"
			"                        ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                            LENGTHUNIT[\"metre\",1]]],\n"
			"                    PRIMEM[\"Greenwich\",0,\n"
			"                        ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"                    CS[ellipsoidal,2],\n"
			"                        AXIS[\"geodetic latitude (Lat)\",north,\n"
			"                            ORDER[1],\n"
			"                            "
			"ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"                        AXIS[\"geodetic longitude (Lon)\",east,\n"
			"                            ORDER[2],\n"
			"                            "
			"ANGLEUNIT[\"degree\",0.0174532925199433]]]],\n"
			"            METHOD[\"CTABLE2\"],\n"
			"            PARAMETERFILE[\"Latitude and longitude difference "
			"file\",\"conus\"],\n"
			"            ID[\"DERIVED_FROM(EPSG)\",15851]]],\n"
			"    STEP[\n"
			"        CONVERSION[\"UTM zone 11N\",\n"
			"            METHOD[\"Transverse Mercator\",\n"
			"                ID[\"EPSG\",9807]],\n"
			"            PARAMETER[\"Latitude of natural origin\",0,\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"                ID[\"EPSG\",8801]],\n"
			"            PARAMETER[\"Longitude of natural origin\",-117,\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"                ID[\"EPSG\",8802]],\n"
			"            PARAMETER[\"Scale factor at natural origin\",0.9996,\n"
			"                SCALEUNIT[\"unity\",1],\n"
			"                ID[\"EPSG\",8805]],\n"
			"            PARAMETER[\"False easting\",500000,\n"
			"                LENGTHUNIT[\"metre\",1],\n"
			"                ID[\"EPSG\",8806]],\n"
			"            PARAMETER[\"False northing\",0,\n"
			"                LENGTHUNIT[\"metre\",1],\n"
			"                ID[\"EPSG\",8807]],\n"
			"            ID[\"EPSG\",16011]]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto concat = nn_dynamic_pointer_cast<ConcatenatedOperation>(obj);
		CHECK(concat != nullptr);

		EXPECT_EQ(concat->exportToPROJString(PROJStringFormatter::create().get()),
			"+proj=pipeline +step +inv +proj=utm +zone=11 +ellps=WGS84 "
			"+step +inv +proj=hgridshift +grids=conus +step "
			"+proj=utm +zone=11 +ellps=clrk66");
	}

	{
		// BOUNDCRS_transformation_from_names

		auto projcrs = ProjectedCRS::create(
			PropertyMap().set(IdentifiedObject::NAME_KEY, "my PROJCRS"),
			GeographicCRS::create(
				PropertyMap().set(IdentifiedObject::NAME_KEY, "my GEOGCRS"),
				GeodeticReferenceFrame::EPSG_6326,
				EllipsoidalCS::createLatitudeLongitude(UnitOfMeasure::DEGREE)),
			Conversion::createUTM(PropertyMap(), 31, true),
			CartesianCS::createEastingNorthing(UnitOfMeasure::METRE));

		auto wkt =
			"BOUNDCRS[SOURCECRS[" +
			projcrs->exportToWKT(WKTFormatter::create().get()) + "],\n" +
			"TARGETCRS[" +
			GeographicCRS::EPSG_4326->exportToWKT(WKTFormatter::create().get()) +
			"],\n"
			"    ABRIDGEDTRANSFORMATION[\"Transformation to WGS84\",\n"
			"        METHOD[\"Coordinate Frame\"],\n"
			"        PARAMETER[\"X-axis translation\",1],\n"
			"        PARAMETER[\"Y-axis translation\",2],\n"
			"        PARAMETER[\"Z-axis translation\",3],\n"
			"        PARAMETER[\"X-axis rotation\",-4],\n"
			"        PARAMETER[\"Y-axis rotation\",-5],\n"
			"        PARAMETER[\"Z-axis rotation\",-6],\n"
			"        PARAMETER[\"Scale difference\",1.000007]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<BoundCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->baseCRS()->nameStr(), projcrs->nameStr());

		EXPECT_EQ(crs->hubCRS()->nameStr(), GeographicCRS::EPSG_4326->nameStr());

		CHECK(crs->transformation()->sourceCRS() != nullptr);
		EXPECT_EQ(crs->transformation()->sourceCRS()->nameStr(), projcrs->baseCRS()->nameStr());

		auto params = crs->transformation()->getTOWGS84Parameters();
		auto expected = std::vector<double>{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
		ASSERT_EQ(params.size(), expected.size());
		for (int i = 0; i < 7; i++)
		{
			DOUBLES_EQUAL(params[i], expected[i], 1e-10);
		}
	}

	{
		// BOUNDCRS_transformation_from_codes

		auto projcrs = ProjectedCRS::create(
			PropertyMap().set(IdentifiedObject::NAME_KEY, "my PROJCRS"),
			GeographicCRS::create(
				PropertyMap().set(IdentifiedObject::NAME_KEY, "my GEOGCRS"),
				GeodeticReferenceFrame::EPSG_6326,
				EllipsoidalCS::createLatitudeLongitude(UnitOfMeasure::DEGREE)),
			Conversion::createUTM(PropertyMap(), 31, true),
			CartesianCS::createEastingNorthing(UnitOfMeasure::METRE));

		auto wkt =
			"BOUNDCRS[SOURCECRS[" +
			projcrs->exportToWKT(WKTFormatter::create().get()) + "],\n" +
			"TARGETCRS[" +
			GeographicCRS::EPSG_4326->exportToWKT(WKTFormatter::create().get()) +
			"],\n"
			"    ABRIDGEDTRANSFORMATION[\"Transformation to WGS84\",\n"
			"        METHOD[\"bla\",ID[\"EPSG\",1032]],\n"
			"        PARAMETER[\"tx\",1,ID[\"EPSG\",8605]],\n"
			"        PARAMETER[\"ty\",2,ID[\"EPSG\",8606]],\n"
			"        PARAMETER[\"tz\",3,ID[\"EPSG\",8607]],\n"
			"        PARAMETER[\"rotx\",-4,ID[\"EPSG\",8608]],\n"
			"        PARAMETER[\"roty\",-5,ID[\"EPSG\",8609]],\n"
			"        PARAMETER[\"rotz\",-6,ID[\"EPSG\",8610]],\n"
			"        PARAMETER[\"scale\",1.000007,ID[\"EPSG\",8611]]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<BoundCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->baseCRS()->nameStr(), projcrs->nameStr());

		EXPECT_EQ(crs->hubCRS()->nameStr(), GeographicCRS::EPSG_4326->nameStr());

		CHECK(crs->transformation()->sourceCRS() != nullptr);
		EXPECT_EQ(crs->transformation()->sourceCRS()->nameStr(), projcrs->baseCRS()->nameStr());

		auto params = crs->transformation()->getTOWGS84Parameters();
		auto expected = std::vector<double>{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
		ASSERT_EQ(params.size(), expected.size());
		for (int i = 0; i < 7; i++)
		{
			DOUBLES_EQUAL(params[i], expected[i], 1e-10);
		}
	}

	{
		// wkt_parse, boundcrs_of_verticalcrs_to_geog3Dcrs

		auto wkt =
			"BOUNDCRS[\n"
			"    SOURCECRS[\n"
			"        VERTCRS[\"my_height\",\n"
			"            VDATUM[\"my_height\"],\n"
			"            CS[vertical,1],\n"
			"                AXIS[\"up\",up,\n"
			"                    LENGTHUNIT[\"metre\",1,\n"
			"                        ID[\"EPSG\",9001]]]]],\n"
			"    TARGETCRS[\n"
			"        GEODCRS[\"WGS 84\",\n"
			"            DATUM[\"World Geodetic System 1984\",\n"
			"                ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                    LENGTHUNIT[\"metre\",1]]],\n"
			"            PRIMEM[\"Greenwich\",0,\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"            CS[ellipsoidal,3],\n"
			"                AXIS[\"latitude\",north,\n"
			"                    ORDER[1],\n"
			"                    ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"                AXIS[\"longitude\",east,\n"
			"                    ORDER[2],\n"
			"                    ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"                AXIS[\"ellipsoidal height\",up,\n"
			"                    ORDER[3],\n"
			"                    LENGTHUNIT[\"metre\",1]],\n"
			"            ID[\"EPSG\",4979]]],\n"
			"    ABRIDGEDTRANSFORMATION[\"my_height height to WGS84 ellipsoidal "
			"height\",\n"
			"        METHOD[\"GravityRelatedHeight to Geographic3D\"],\n"
			"        PARAMETERFILE[\"Geoid (height correction) model file\","
			"                      \"./tmp/fake.gtx\",\n"
			"            ID[\"EPSG\",8666]]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<BoundCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->baseCRS()->nameStr(), "my_height");

		EXPECT_EQ(crs->hubCRS()->nameStr(), GeographicCRS::EPSG_4979->nameStr());
	}

	{
		// geogcs_TOWGS84_3terms

		auto wkt =
			"GEOGCS[\"my GEOGCRS\",\n"
			"    DATUM[\"WGS_1984\",\n"
			"        SPHEROID[\"WGS 84\",6378137,298.257223563],\n"
			"        TOWGS84[1,2,3]],\n"
			"    PRIMEM[\"Greenwich\",0],\n"
			"    UNIT[\"degree\",0.0174532925199433]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<BoundCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->baseCRS()->nameStr(), "my GEOGCRS");

		EXPECT_EQ(crs->hubCRS()->nameStr(), GeographicCRS::EPSG_4326->nameStr());

		CHECK(crs->transformation()->sourceCRS() != nullptr);
		EXPECT_EQ(crs->transformation()->sourceCRS()->nameStr(), "my GEOGCRS");

		auto params = crs->transformation()->getTOWGS84Parameters();
		auto expected = std::vector<double>{ 1.0, 2.0, 3.0, 0.0, 0.0, 0.0, 0.0 };
		ASSERT_EQ(params.size(), expected.size());
		for (int i = 0; i < 7; i++)
		{
			DOUBLES_EQUAL(params[i], expected[i], 1e-10);
		}
	}

	{
		// projcs_TOWGS84_7terms

		auto wkt =
			"PROJCS[\"my PROJCRS\",\n"
			"    GEOGCS[\"my GEOGCRS\",\n"
			"        DATUM[\"WGS_1984\",\n"
			"            SPHEROID[\"WGS 84\",6378137,298.257223563,\n"
			"                AUTHORITY[\"EPSG\",\"7030\"]],\n"
			"            TOWGS84[1,2,3,4,5,6,7],\n"
			"            AUTHORITY[\"EPSG\",\"6326\"]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            AUTHORITY[\"EPSG\",\"8901\"]],\n"
			"        UNIT[\"degree\",0.0174532925199433,\n"
			"            AUTHORITY[\"EPSG\",\"9122\"]],\n"
			"        AXIS[\"Latitude\",NORTH],\n"
			"        AXIS[\"Longitude\",EAST]],\n"
			"    PROJECTION[\"Transverse_Mercator\"],\n"
			"    PARAMETER[\"latitude_of_origin\",0],\n"
			"    PARAMETER[\"central_meridian\",3],\n"
			"    PARAMETER[\"scale_factor\",0.9996],\n"
			"    PARAMETER[\"false_easting\",500000],\n"
			"    PARAMETER[\"false_northing\",0],\n"
			"    UNIT[\"metre\",1,\n"
			"        AUTHORITY[\"EPSG\",\"9001\"]],\n"
			"    AXIS[\"Easting\",EAST],\n"
			"    AXIS[\"Northing\",NORTH]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<BoundCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->baseCRS()->nameStr(), "my PROJCRS");

		EXPECT_EQ(crs->hubCRS()->nameStr(), GeographicCRS::EPSG_4326->nameStr());

		CHECK(crs->transformation()->sourceCRS() != nullptr);
		EXPECT_EQ(crs->transformation()->sourceCRS()->nameStr(), "my GEOGCRS");

		auto params = crs->transformation()->getTOWGS84Parameters();
		auto expected = std::vector<double>{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
		ASSERT_EQ(params.size(), expected.size());
		for (int i = 0; i < 7; i++)
		{
			DOUBLES_EQUAL(params[i], expected[i], 1e-10);
		}
	}

	{
		// WKT1_VERT_DATUM_EXTENSION

		auto wkt = "VERT_CS[\"EGM2008 geoid height\",\n"
			"    VERT_DATUM[\"EGM2008 geoid\",2005,\n"
			"        EXTENSION[\"PROJ4_GRIDS\",\"egm08_25.gtx\"],\n"
			"        AUTHORITY[\"EPSG\",\"1027\"]],\n"
			"    UNIT[\"metre\",1,\n"
			"        AUTHORITY[\"EPSG\",\"9001\"]],\n"
			"    AXIS[\"Up\",UP],\n"
			"    AUTHORITY[\"EPSG\",\"3855\"]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<BoundCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->baseCRS()->nameStr(), "EGM2008 geoid height");

		EXPECT_EQ(crs->hubCRS()->nameStr(), GeographicCRS::EPSG_4979->nameStr());

		CHECK(crs->transformation()->sourceCRS() != nullptr);
		EXPECT_EQ(crs->transformation()->sourceCRS()->nameStr(),
			crs->baseCRS()->nameStr());

		CHECK(crs->transformation()->targetCRS() != nullptr);
		EXPECT_EQ(crs->transformation()->targetCRS()->nameStr(), crs->hubCRS()->nameStr());

		EXPECT_EQ(crs->transformation()->nameStr(),
			"EGM2008 geoid height to WGS84 ellipsoidal height");
		EXPECT_EQ(crs->transformation()->method()->nameStr(),
			"GravityRelatedHeight to Geographic3D");
		ASSERT_EQ(crs->transformation()->parameterValues().size(), 1U);
		{
			const auto &opParamvalue =
				nn_dynamic_pointer_cast<OperationParameterValue>(
					crs->transformation()->parameterValues()[0]);
			CHECK(opParamvalue);
			const auto &paramName = opParamvalue->parameter()->nameStr();
			const auto &parameterValue = opParamvalue->parameterValue();
			CHECK(opParamvalue->parameter()->getEPSGCode() == 8666);
			EXPECT_EQ(paramName, "Geoid (height correction) model file");
			EXPECT_EQ(parameterValue->type(), ParameterValue::Type::FILENAME);
			EXPECT_EQ(parameterValue->valueFile(), "egm08_25.gtx");
		}
	}

	{
		// WKT1_DATUM_EXTENSION

		auto wkt =
			"PROJCS[\"unnamed\",\n"
			"    GEOGCS[\"International 1909 (Hayford)\",\n"
			"        DATUM[\"unknown\",\n"
			"            SPHEROID[\"intl\",6378388,297],\n"
			"            EXTENSION[\"PROJ4_GRIDS\",\"nzgd2kgrid0005.gsb\"]],\n"
			"        PRIMEM[\"Greenwich\",0],\n"
			"        UNIT[\"degree\",0.0174532925199433]],\n"
			"    PROJECTION[\"New_Zealand_Map_Grid\"],\n"
			"    PARAMETER[\"latitude_of_origin\",-41],\n"
			"    PARAMETER[\"central_meridian\",173],\n"
			"    PARAMETER[\"false_easting\",2510000],\n"
			"    PARAMETER[\"false_northing\",6023150],\n"
			"    UNIT[\"Meter\",1]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<BoundCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->baseCRS()->nameStr(), "unnamed");

		EXPECT_EQ(crs->hubCRS()->nameStr(), GeographicCRS::EPSG_4326->nameStr());

		CHECK(crs->transformation()->sourceCRS() != nullptr);
		EXPECT_EQ(crs->transformation()->sourceCRS()->nameStr(),
			"International 1909 (Hayford)");

		CHECK(crs->transformation()->targetCRS() != nullptr);
		EXPECT_EQ(crs->transformation()->targetCRS()->nameStr(),
			crs->hubCRS()->nameStr());

		EXPECT_EQ(crs->transformation()->nameStr(),
			"International 1909 (Hayford) to WGS84");
		EXPECT_EQ(crs->transformation()->method()->nameStr(), "NTv2");
		ASSERT_EQ(crs->transformation()->parameterValues().size(), 1U);
		{
			const auto &opParamvalue =
				nn_dynamic_pointer_cast<OperationParameterValue>(
					crs->transformation()->parameterValues()[0]);
			CHECK(opParamvalue);
			const auto &paramName = opParamvalue->parameter()->nameStr();
			const auto &parameterValue = opParamvalue->parameterValue();
			CHECK(opParamvalue->parameter()->getEPSGCode() == 8656);
			EXPECT_EQ(paramName, "Latitude and longitude difference file");
			EXPECT_EQ(parameterValue->type(), ParameterValue::Type::FILENAME);
			EXPECT_EQ(parameterValue->valueFile(), "nzgd2kgrid0005.gsb");
		}
	}

	{
		// DerivedGeographicCRS_WKT2

		auto wkt = "GEODCRS[\"WMO Atlantic Pole\",\n"
			"    BASEGEODCRS[\"WGS 84\",\n"
			"        DATUM[\"World Geodetic System 1984\",\n"
			"            ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                LENGTHUNIT[\"metre\",1]]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"    DERIVINGCONVERSION[\"Atlantic pole\",\n"
			"        METHOD[\"Pole rotation\"],\n"
			"        PARAMETER[\"Latitude of rotated pole\",52,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]],\n"
			"        PARAMETER[\"Longitude of rotated pole\",-30,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]],\n"
			"        PARAMETER[\"Axis rotation\",-25,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]]],\n"
			"    CS[ellipsoidal,2],\n"
			"        AXIS[\"latitude\",north,\n"
			"            ORDER[1],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]],\n"
			"        AXIS[\"longitude\",east,\n"
			"            ORDER[2],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<DerivedGeographicCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->nameStr(), "WMO Atlantic Pole");

		EXPECT_EQ(crs->baseCRS()->nameStr(), "WGS 84");
		CHECK(nn_dynamic_pointer_cast<GeographicCRS>(crs->baseCRS()) !=
			nullptr);

		EXPECT_EQ(crs->derivingConversion()->nameStr(), "Atlantic pole");

		CHECK(nn_dynamic_pointer_cast<EllipsoidalCS>(
			crs->coordinateSystem()) != nullptr);
	}

	{
		// DerivedGeographicCRS_WKT2_2018

		auto wkt = "GEOGCRS[\"WMO Atlantic Pole\",\n"
			"    BASEGEOGCRS[\"WGS 84\",\n"
			"        DATUM[\"World Geodetic System 1984\",\n"
			"            ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                LENGTHUNIT[\"metre\",1]]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"    DERIVINGCONVERSION[\"Atlantic pole\",\n"
			"        METHOD[\"Pole rotation\"],\n"
			"        PARAMETER[\"Latitude of rotated pole\",52,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]],\n"
			"        PARAMETER[\"Longitude of rotated pole\",-30,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]],\n"
			"        PARAMETER[\"Axis rotation\",-25,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]]],\n"
			"    CS[ellipsoidal,2],\n"
			"        AXIS[\"latitude\",north,\n"
			"            ORDER[1],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]],\n"
			"        AXIS[\"longitude\",east,\n"
			"            ORDER[2],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<DerivedGeographicCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->nameStr(), "WMO Atlantic Pole");

		EXPECT_EQ(crs->baseCRS()->nameStr(), "WGS 84");
		CHECK(nn_dynamic_pointer_cast<GeographicCRS>(crs->baseCRS()) != nullptr);

		EXPECT_EQ(crs->derivingConversion()->nameStr(), "Atlantic pole");

		CHECK(nn_dynamic_pointer_cast<EllipsoidalCS>(crs->coordinateSystem()) != nullptr);
	}

	{
		// DerivedGeodeticCRS

		auto wkt =
			"GEODCRS[\"Derived geodetic CRS\",\n"
			"    BASEGEODCRS[\"WGS 84\",\n"
			"        DATUM[\"World Geodetic System 1984\",\n"
			"            ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                LENGTHUNIT[\"metre\",1]]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"    DERIVINGCONVERSION[\"Some conversion\",\n"
			"        METHOD[\"Some method\"],\n"
			"        PARAMETER[\"foo\",1.0,UNIT[\"metre\",1]]],\n"
			"    CS[Cartesian,3],\n"
			"        AXIS[\"(X)\",geocentricX,\n"
			"            ORDER[1],\n"
			"            LENGTHUNIT[\"metre\",1,\n"
			"                ID[\"EPSG\",9001]]],\n"
			"        AXIS[\"(Y)\",geocentricY,\n"
			"            ORDER[2],\n"
			"            LENGTHUNIT[\"metre\",1,\n"
			"                ID[\"EPSG\",9001]]],\n"
			"        AXIS[\"(Z)\",geocentricZ,\n"
			"            ORDER[3],\n"
			"            LENGTHUNIT[\"metre\",1,\n"
			"                ID[\"EPSG\",9001]]]]";
		;

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<DerivedGeodeticCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->nameStr(), "Derived geodetic CRS");

		EXPECT_EQ(crs->baseCRS()->nameStr(), "WGS 84");
		CHECK(nn_dynamic_pointer_cast<GeographicCRS>(crs->baseCRS()) != nullptr);

		EXPECT_EQ(crs->derivingConversion()->nameStr(), "Some conversion");

		CHECK(nn_dynamic_pointer_cast<CartesianCS>(crs->coordinateSystem()) != nullptr);
	}

	{
		// DerivedProjectedCRS

		auto wkt =
			"DERIVEDPROJCRS[\"derived projectedCRS\",\n"
			"    BASEPROJCRS[\"WGS 84 / UTM zone 31N\",\n"
			"        BASEGEOGCRS[\"WGS 84\",\n"
			"            DATUM[\"World Geodetic System 1984\",\n"
			"                ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                    LENGTHUNIT[\"metre\",1]]],\n"
			"            PRIMEM[\"Greenwich\",0,\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"        CONVERSION[\"UTM zone 31N\",\n"
			"            METHOD[\"Transverse Mercator\",\n"
			"                ID[\"EPSG\",9807]],\n"
			"            PARAMETER[\"Latitude of natural origin\",0,\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"                ID[\"EPSG\",8801]],\n"
			"            PARAMETER[\"Longitude of natural origin\",3,\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"                ID[\"EPSG\",8802]],\n"
			"            PARAMETER[\"Scale factor at natural origin\",0.9996,\n"
			"                SCALEUNIT[\"unity\",1],\n"
			"                ID[\"EPSG\",8805]],\n"
			"            PARAMETER[\"False easting\",500000,\n"
			"                LENGTHUNIT[\"metre\",1],\n"
			"                ID[\"EPSG\",8806]],\n"
			"            PARAMETER[\"False northing\",0,\n"
			"                LENGTHUNIT[\"metre\",1],\n"
			"                ID[\"EPSG\",8807]]]],\n"
			"    DERIVINGCONVERSION[\"unnamed\",\n"
			"        METHOD[\"PROJ unimplemented\"],\n"
			"        PARAMETER[\"foo\",1.0,UNIT[\"metre\",1]]],\n"
			"    CS[Cartesian,2],\n"
			"        AXIS[\"(E)\",east,\n"
			"            ORDER[1],\n"
			"            LENGTHUNIT[\"metre\",1,\n"
			"                ID[\"EPSG\",9001]]],\n"
			"        AXIS[\"(N)\",north,\n"
			"            ORDER[2],\n"
			"            LENGTHUNIT[\"metre\",1,\n"
			"                ID[\"EPSG\",9001]]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<DerivedProjectedCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->nameStr(), "derived projectedCRS");

		EXPECT_EQ(crs->baseCRS()->nameStr(), "WGS 84 / UTM zone 31N");
		CHECK(nn_dynamic_pointer_cast<ProjectedCRS>(crs->baseCRS()) !=
			nullptr);

		EXPECT_EQ(crs->derivingConversion()->nameStr(), "unnamed");

		CHECK(nn_dynamic_pointer_cast<CartesianCS>(crs->coordinateSystem()) != nullptr);
	}

	{
		// DerivedProjectedCRS_ordinal

		auto wkt =
			"DERIVEDPROJCRS[\"derived projectedCRS\",\n"
			"    BASEPROJCRS[\"BASEPROJCRS\",\n"
			"        BASEGEOGCRS[\"WGS 84\",\n"
			"            DATUM[\"World Geodetic System 1984\",\n"
			"                ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                    LENGTHUNIT[\"metre\",1]]],\n"
			"            PRIMEM[\"Greenwich\",0,\n"
			"                ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"                ID[\"EPSG\",8901]]],\n"
			"        CONVERSION[\"unnamed\",\n"
			"            METHOD[\"PROJ unimplemented\"],\n"
			"            PARAMETER[\"foo\",1,\n"
			"                LENGTHUNIT[\"metre\",1,\n"
			"                    ID[\"EPSG\",9001]]]]],\n"
			"    DERIVINGCONVERSION[\"unnamed\",\n"
			"        METHOD[\"PROJ unimplemented\"],\n"
			"        PARAMETER[\"foo\",1,\n"
			"            LENGTHUNIT[\"metre\",1,\n"
			"                ID[\"EPSG\",9001]]]],\n"
			"    CS[ordinal,2],\n"
			"        AXIS[\"inline (I)\",northNorthWest,\n"
			"            ORDER[1]],\n"
			"        AXIS[\"crossline (J)\",westSouthWest,\n"
			"            ORDER[2]]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<DerivedProjectedCRS>(obj);
		CHECK(crs != nullptr);
		CHECK(nn_dynamic_pointer_cast<OrdinalCS>(crs->coordinateSystem()) != nullptr);

		EXPECT_EQ(
			crs->exportToWKT(
				WKTFormatter::create(WKTFormatter::Convention::WKT2_2018).get()),
			wkt);
	}

	{
		// TemporalDatum

		auto wkt =
			"TDATUM[\"Gregorian calendar\",\n"
			"    CALENDAR[\"my calendar\"],\n"
			"    TIMEORIGIN[0000-01-01]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto tdatum = nn_dynamic_pointer_cast<TemporalDatum>(obj);
		CHECK(tdatum != nullptr);

		EXPECT_EQ(tdatum->nameStr(), "Gregorian calendar");
		EXPECT_EQ(tdatum->temporalOrigin().toString(), "0000-01-01");
		EXPECT_EQ(tdatum->calendar(), "my calendar");
	}

	{
		// wkt1_esri_case_insensitive_names

		auto wkt =
			"PROJCS[\"WGS_1984_UTM_Zone_31N\",GEOGCS[\"GCS_WGS_1984\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,"
			"298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\","
			"0.0174532925199433]],PROJECTION[\"transverse_mercator\"],"
			"PARAMETER[\"false_easting\",500000.0],"
			"PARAMETER[\"false_northing\",0.0],"
			"PARAMETER[\"central_meridian\",3.0],"
			"PARAMETER[\"scale_factor\",0.9996],"
			"PARAMETER[\"latitude_of_origin\",0.0],UNIT[\"Meter\",1.0]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		int zone = 0;
		bool north = false;
		CHECK(crs->derivingConversion()->isUTM(zone, north));
		EXPECT_EQ(zone, 31);
		CHECK(north);
	}

	{
		// wkt1_esri_non_expected_param_name

		// We try to be lax on parameter names.
		auto wkt =
			"PROJCS[\"WGS_1984_UTM_Zone_31N\",GEOGCS[\"GCS_WGS_1984\","
			"DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,"
			"298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\","
			"0.0174532925199433]],PROJECTION[\"transverse_mercator\"],"
			"PARAMETER[\"false_easting\",500000.0],"
			"PARAMETER[\"false_northing\",0.0],"
			"PARAMETER[\"longitude_of_center\",3.0]," // should be Central_Meridian
			"PARAMETER[\"scale_factor\",0.9996],"
			"PARAMETER[\"latitude_of_origin\",0.0],UNIT[\"Meter\",1.0]]";

		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		int zone = 0;
		bool north = false;
		CHECK(crs->derivingConversion()->isUTM(zone, north));
		EXPECT_EQ(zone, 31);
		CHECK(north);
	}

	{
		// wkt1_esri_krovak_south_west

		auto wkt = "PROJCS[\"S-JTSK_Krovak\",GEOGCS[\"GCS_S_JTSK\","
			"DATUM[\"D_S_JTSK\","
			"SPHEROID[\"Bessel_1841\",6377397.155,299.1528128]],"
			"PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],"
			"PROJECTION[\"Krovak\"],PARAMETER[\"False_Easting\",0.0],"
			"PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Pseudo_Standard_Parallel_1\",78.5],"
			"PARAMETER[\"Scale_Factor\",0.9999],"
			"PARAMETER[\"Azimuth\",30.28813975277778],"
			"PARAMETER[\"Longitude_Of_Center\",24.83333333333333],"
			"PARAMETER[\"Latitude_Of_Center\",49.5],"
			"PARAMETER[\"X_Scale\",1.0],"
			"PARAMETER[\"Y_Scale\",1.0],"
			"PARAMETER[\"XY_Plane_Rotation\",0.0],UNIT[\"Meter\",1.0]]";

		auto obj = WKTParser()
			.attachDatabaseContext(DatabaseContext::create())
			.createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);

		EXPECT_EQ(crs->derivingConversion()->method()->nameStr(), "Krovak");

		auto expected_wkt2 =
			"PROJCRS[\"S-JTSK / Krovak\",\n"
			"    BASEGEODCRS[\"S-JTSK\",\n"
			"        DATUM[\"System of the Unified Trigonometrical Cadastral Network\",\n"
			"            ELLIPSOID[\"Bessel 1841\",6377397.155,299.1528128,\n"
			"                LENGTHUNIT[\"metre\",1]],\n"
			"            ID[\"EPSG\",6156]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            ANGLEUNIT[\"Degree\",0.0174532925199433]]],\n"
			"    CONVERSION[\"unnamed\",\n"
			"        METHOD[\"Krovak\",\n"
			"            ID[\"EPSG\",9819]],\n"
			"        PARAMETER[\"Latitude of projection centre\",49.5,\n"
			"            ANGLEUNIT[\"Degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8811]],\n"
			"        PARAMETER[\"Longitude of origin\",24.8333333333333,\n"
			"            ANGLEUNIT[\"Degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8833]],\n"
			"        PARAMETER[\"Co-latitude of cone axis\",30.2881397527778,\n"
			"            ANGLEUNIT[\"Degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",1036]],\n"
			"        PARAMETER[\"Latitude of pseudo standard parallel\",78.5,\n"
			"            ANGLEUNIT[\"Degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8818]],\n"
			"        PARAMETER[\"Scale factor on pseudo standard parallel\",0.9999,\n"
			"            SCALEUNIT[\"unity\",1],\n"
			"            ID[\"EPSG\",8819]],\n"
			"        PARAMETER[\"False easting\",0,\n"
			"            LENGTHUNIT[\"metre\",1],\n"
			"            ID[\"EPSG\",8806]],\n"
			"        PARAMETER[\"False northing\",0,\n"
			"            LENGTHUNIT[\"metre\",1],\n"
			"            ID[\"EPSG\",8807]]],\n"
			"    CS[Cartesian,2],\n"
			"        AXIS[\"southing\",south,\n"
			"            ORDER[1],\n"
			"            LENGTHUNIT[\"metre\",1,\n"
			"                ID[\"EPSG\",9001]]],\n"
			"        AXIS[\"westing\",west,\n"
			"            ORDER[2],\n"
			"            LENGTHUNIT[\"metre\",1,\n"
			"                ID[\"EPSG\",9001]]]]";

		EXPECT_EQ(crs->exportToWKT(WKTFormatter::create().get()), expected_wkt2);
	}

	{
		//	projstringformatter

		{
			auto fmt = PROJStringFormatter::create();
			fmt->addStep("my_proj");
			EXPECT_EQ(fmt->toString(), "+proj=my_proj");
		}
		
		{
			auto fmt = PROJStringFormatter::create();
			fmt->addStep("my_proj");
			fmt->setCurrentStepInverted(true);
			EXPECT_EQ(fmt->toString(), "+proj=pipeline +step +inv +proj=my_proj");
		}
		
		{
			auto fmt = PROJStringFormatter::create();
			fmt->addStep("my_proj1");
			fmt->addStep("my_proj2");
			EXPECT_EQ(fmt->toString(),
				"+proj=pipeline +step +proj=my_proj1 +step +proj=my_proj2");
		}
		
		{
			auto fmt = PROJStringFormatter::create();
			fmt->addStep("my_proj1");
			fmt->setCurrentStepInverted(true);
			fmt->addStep("my_proj2");
			EXPECT_EQ(
				fmt->toString(),
				"+proj=pipeline +step +inv +proj=my_proj1 +step +proj=my_proj2");
		}

		{
			auto fmt = PROJStringFormatter::create();
			fmt->startInversion();
			fmt->addStep("my_proj1");
			fmt->setCurrentStepInverted(true);
			fmt->addStep("my_proj2");
			fmt->stopInversion();
			EXPECT_EQ(
				fmt->toString(),
				"+proj=pipeline +step +inv +proj=my_proj2 +step +proj=my_proj1");
		}

		{
			auto fmt = PROJStringFormatter::create();
			fmt->startInversion();
			fmt->addStep("my_proj1");
			fmt->setCurrentStepInverted(true);
			fmt->startInversion();
			fmt->addStep("my_proj2");
			fmt->stopInversion();
			fmt->stopInversion();
			EXPECT_EQ(fmt->toString(),
				"+proj=pipeline +step +proj=my_proj2 +step +proj=my_proj1");
		}
	}

	{
		//	projstringformatter_helmert_3_param_noop

		auto fmt = PROJStringFormatter::create();
		fmt->addStep("helmert");
		fmt->addParam("x", 0);
		fmt->addParam("y", 0);
		fmt->addParam("z", 0);
		EXPECT_EQ(fmt->toString(), "+proj=noop");
	}

	{
		// projstringformatter_helmert_7_param_noop

		auto fmt = PROJStringFormatter::create();
		fmt->addStep("helmert");
		fmt->addParam("x", 0);
		fmt->addParam("y", 0);
		fmt->addParam("z", 0);
		fmt->addParam("rx", 0);
		fmt->addParam("ry", 0);
		fmt->addParam("rz", 0);
		fmt->addParam("s", 0);
		fmt->addParam("convention", "position_vector");
		EXPECT_EQ(fmt->toString(), "+proj=noop");
	}

	{
		//	projstringformatter_merge_consecutive_helmert_3_param

		auto fmt = PROJStringFormatter::create();
		fmt->addStep("helmert");
		fmt->addParam("x", 10);
		fmt->addParam("y", 20);
		fmt->addParam("z", 30);
		fmt->addStep("helmert");
		fmt->addParam("x", -1);
		fmt->addParam("y", -2);
		fmt->addParam("z", -3);
		EXPECT_EQ(fmt->toString(), "+proj=helmert +x=9 +y=18 +z=27");
	}

	{
		//	projstringformatter_merge_consecutive_helmert_3_param_noop

		auto fmt = PROJStringFormatter::create();
		fmt->addStep("helmert");
		fmt->addParam("x", 10);
		fmt->addParam("y", 20);
		fmt->addParam("z", 30);
		fmt->addStep("helmert");
		fmt->addParam("x", -10);
		fmt->addParam("y", -20);
		fmt->addParam("z", -30);
		EXPECT_EQ(fmt->toString(), "+proj=noop");
	}

	{
		//	projstringformatter_cart_grs80_wgs84

		auto fmt = PROJStringFormatter::create();
		fmt->addStep("cart");
		fmt->addParam("ellps", "WGS84");
		fmt->addStep("cart");
		fmt->setCurrentStepInverted(true);
		fmt->addParam("ellps", "GRS80");
		EXPECT_EQ(fmt->toString(), "+proj=noop");
	}

	{
		//	projstringformatter_axisswap_unitconvert_axisswap

		auto fmt = PROJStringFormatter::create();
		fmt->addStep("axisswap");
		fmt->addParam("order", "2,1");
		fmt->addStep("unitconvert");
		fmt->addParam("xy_in", "rad");
		fmt->addParam("xy_out", "deg");
		fmt->addStep("axisswap");
		fmt->addParam("order", "2,1");
		EXPECT_EQ(fmt->toString(), "+proj=unitconvert +xy_in=rad +xy_out=deg");
	}

	{
		//	projparse_longlat
		
		auto expected = "GEODCRS[\"unknown\",\n"
			"    DATUM[\"World Geodetic System 1984\",\n"
			"        ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"            LENGTHUNIT[\"metre\",1]],\n"
			"        ID[\"EPSG\",6326]],\n"
			"    PRIMEM[\"Greenwich\",0,\n"
			"        ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"        ID[\"EPSG\",8901]],\n"
			"    CS[ellipsoidal,2],\n"
			"        AXIS[\"longitude\",east,\n"
			"            ORDER[1],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]],\n"
			"        AXIS[\"latitude\",north,\n"
			"            ORDER[2],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]]]";
		{
			auto obj =
				PROJStringParser().createFromPROJString("+proj=longlat +type=crs");
			auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
			CHECK(crs != nullptr);
			WKTFormatterNNPtr f(WKTFormatter::create());
			crs->exportToWKT(f.get());
			EXPECT_EQ(f->toString(), expected);
		}
		
		{
			auto obj = PROJStringParser().createFromPROJString(
				"+proj=longlat +datum=WGS84 +type=crs");
			auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
			CHECK(crs != nullptr);
			WKTFormatterNNPtr f(WKTFormatter::create());
			crs->exportToWKT(f.get());
			EXPECT_EQ(f->toString(), expected);
		}
	}

	{
		//	projparse_longlat_datum_NAD83

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=longlat +datum=NAD83 +type=crs");
		auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		crs->exportToWKT(f.get());
		EXPECT_EQ(f->toString(),
			"GEODCRS[\"unknown\",\n"
			"    DATUM[\"North American Datum 1983\",\n"
			"        ELLIPSOID[\"GRS 1980\",6378137,298.257222101,\n"
			"            LENGTHUNIT[\"metre\",1]],\n"
			"        ID[\"EPSG\",6269]],\n"
			"    PRIMEM[\"Greenwich\",0,\n"
			"        ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"        ID[\"EPSG\",8901]],\n"
			"    CS[ellipsoidal,2],\n"
			"        AXIS[\"longitude\",east,\n"
			"            ORDER[1],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]],\n"
			"        AXIS[\"latitude\",north,\n"
			"            ORDER[2],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]]]");
	}

	{
		//	projparse_longlat_datum_NAD27

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=longlat +datum=NAD27 +type=crs");
		auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		crs->exportToWKT(f.get());
		EXPECT_EQ(f->toString(),
			"GEODCRS[\"unknown\",\n"
			"    DATUM[\"North American Datum 1927\",\n"
			"        ELLIPSOID[\"Clarke 1866\",6378206.4,294.978698213898,\n"
			"            LENGTHUNIT[\"metre\",1]],\n"
			"        ID[\"EPSG\",6267]],\n"
			"    PRIMEM[\"Greenwich\",0,\n"
			"        ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"        ID[\"EPSG\",8901]],\n"
			"    CS[ellipsoidal,2],\n"
			"        AXIS[\"longitude\",east,\n"
			"            ORDER[1],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]],\n"
			"        AXIS[\"latitude\",north,\n"
			"            ORDER[2],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]]]");
	}

	{
		//	projparse_longlat_datum_other

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=longlat +datum=carthage +type=crs");
		auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		crs->exportToWKT(f.get());
		EXPECT_EQ(f->toString(),
			"GEODCRS[\"unknown\",\n"
			"    DATUM[\"Carthage\",\n"
			"        ELLIPSOID[\"Clarke 1880 (IGN)\",6378249.2,293.4660213,\n"
			"            LENGTHUNIT[\"metre\",1]],\n"
			"        ID[\"EPSG\",6223]],\n"
			"    PRIMEM[\"Greenwich\",0,\n"
			"        ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"        ID[\"EPSG\",8901]],\n"
			"    CS[ellipsoidal,2],\n"
			"        AXIS[\"longitude\",east,\n"
			"            ORDER[1],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]],\n"
			"        AXIS[\"latitude\",north,\n"
			"            ORDER[2],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433,\n"
			"                ID[\"EPSG\",9122]]]]");
	}

	{
		//	projparse_longlat_ellps_WGS84

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=longlat +ellps=WGS84 +type=crs");
		auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		f->simulCurNodeHasId();
		crs->exportToWKT(f.get());
		auto expected = "GEODCRS[\"unknown\",\n"
			"    DATUM[\"Unknown based on WGS84 ellipsoid\",\n"
			"        ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"            LENGTHUNIT[\"metre\",1]]],\n"
			"    PRIMEM[\"Greenwich\",0,\n"
			"        ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"    CS[ellipsoidal,2],\n"
			"        AXIS[\"longitude\",east,\n"
			"            ORDER[1],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"        AXIS[\"latitude\",north,\n"
			"            ORDER[2],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]]]";
		EXPECT_EQ(f->toString(), expected);
	}

	{
		//	projparse_longlat_ellps_GRS80

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=longlat +ellps=GRS80 +type=crs");
		auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		f->simulCurNodeHasId();
		crs->exportToWKT(f.get());
		auto expected = "GEODCRS[\"unknown\",\n"
			"    DATUM[\"Unknown based on GRS80 ellipsoid\",\n"
			"        ELLIPSOID[\"GRS 1980\",6378137,298.257222101,\n"
			"            LENGTHUNIT[\"metre\",1]]],\n"
			"    PRIMEM[\"Greenwich\",0,\n"
			"        ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"    CS[ellipsoidal,2],\n"
			"        AXIS[\"longitude\",east,\n"
			"            ORDER[1],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"        AXIS[\"latitude\",north,\n"
			"            ORDER[2],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]]]";
		EXPECT_EQ(f->toString(), expected);
	}

	{
		//	projparse_longlat_a_b

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=longlat +a=2 +b=1.5 +type=crs");
		auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		f->simulCurNodeHasId();
		crs->exportToWKT(f.get());
		auto expected = "GEODCRS[\"unknown\",\n"
			"    DATUM[\"unknown\",\n"
			"        ELLIPSOID[\"unknown\",2,4,\n"
			"            LENGTHUNIT[\"metre\",1]]],\n"
			"    PRIMEM[\"Reference meridian\",0,\n"
			"        ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"    CS[ellipsoidal,2],\n"
			"        AXIS[\"longitude\",east,\n"
			"            ORDER[1],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"        AXIS[\"latitude\",north,\n"
			"            ORDER[2],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]]]";
		EXPECT_EQ(f->toString(), expected);
		EXPECT_EQ(crs->ellipsoid()->celestialBody(), "Non-Earth body");
	}

	{
		//	projparse_longlat_a_rf_WGS84

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=longlat +a=6378137 +rf=298.257223563 +type=crs");
		auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		f->simulCurNodeHasId();
		crs->exportToWKT(f.get());
		auto expected = "GEODCRS[\"unknown\",\n"
			"    DATUM[\"unknown\",\n"
			"        ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"            LENGTHUNIT[\"metre\",1]]],\n"
			"    PRIMEM[\"Greenwich\",0,\n"
			"        ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"    CS[ellipsoidal,2],\n"
			"        AXIS[\"longitude\",east,\n"
			"            ORDER[1],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
			"        AXIS[\"latitude\",north,\n"
			"            ORDER[2],\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]]]";
		EXPECT_EQ(f->toString(), expected);
		EXPECT_EQ(crs->ellipsoid()->celestialBody(), Ellipsoid::EARTH);
	}

	{
		//	projparse_vunits

		auto obj = PROJStringParser().createFromPROJString("+vunits=ft +type=crs");
		auto crs = nn_dynamic_pointer_cast<VerticalCRS>(obj);
		CHECK(crs != nullptr);
		EXPECT_EQ(crs->exportToPROJString(PROJStringFormatter::create().get()),
			"+vunits=ft +no_defs +type=crs");
	}

	{
		//	projparse_vto_meter

		auto obj =
			PROJStringParser().createFromPROJString("+vto_meter=2 +type=crs");
		auto crs = nn_dynamic_pointer_cast<VerticalCRS>(obj);
		CHECK(crs != nullptr);
		EXPECT_EQ(crs->exportToPROJString(PROJStringFormatter::create().get()),
			"+vto_meter=2 +no_defs +type=crs");
	}

	{
		//	projparse_longlat_axis_enu

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=longlat +ellps=GRS80 +axis=enu +type=crs");
		auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		f->simulCurNodeHasId();
		f->setMultiLine(false);
		crs->exportToWKT(f.get());
		
		auto wkt = f->toString();
		CHECK(wkt.find("AXIS[\"longitude\",east,ORDER[1]") != std::string::npos);
		CHECK(wkt.find("AXIS[\"latitude\",north,ORDER[2]") != std::string::npos);
		
		auto op = CoordinateOperationFactory::create()->createOperation(
			GeographicCRS::EPSG_4326, NN_NO_CHECK(crs));
		CHECK(op != nullptr);
		EXPECT_EQ(op->exportToPROJString(PROJStringFormatter::create().get()),
			"+proj=axisswap +order=2,1");
	}

	{
		//	projparse_longlat_axis_neu

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=longlat +ellps=GRS80 +axis=neu +type=crs");
		auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		f->simulCurNodeHasId();
		f->setMultiLine(false);
		crs->exportToWKT(f.get());
		
		auto wkt = f->toString();
		CHECK(wkt.find("AXIS[\"latitude\",north,ORDER[1]") != std::string::npos);
		CHECK(wkt.find("AXIS[\"longitude\",east,ORDER[2]") != std::string::npos);
		
		auto op = CoordinateOperationFactory::create()->createOperation(
			GeographicCRS::EPSG_4326, NN_NO_CHECK(crs));
		CHECK(op != nullptr);
		EXPECT_EQ(op->exportToPROJString(PROJStringFormatter::create().get()),
			"+proj=noop");
	}

	{
		//	projparse_longlat_axis_swu

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=longlat +ellps=GRS80 +axis=swu +type=crs");
		auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		f->simulCurNodeHasId();
		f->setMultiLine(false);
		crs->exportToWKT(f.get());
		
		auto wkt = f->toString();
		CHECK(wkt.find("AXIS[\"latitude\",south,ORDER[1]") != std::string::npos);
		CHECK(wkt.find("AXIS[\"longitude\",west,ORDER[2]") != std::string::npos);

		auto op = CoordinateOperationFactory::create()->createOperation(
			GeographicCRS::EPSG_4326, NN_NO_CHECK(crs));
		CHECK(op != nullptr);
		EXPECT_EQ(op->exportToPROJString(PROJStringFormatter::create().get()),
			"+proj=pipeline +step +proj=axisswap +order=2,1 +step "
			"+proj=axisswap +order=-2,-1");
	}

	{
		//	projparse_longlat_unitconvert_deg

		auto obj = PROJStringParser().createFromPROJString(
			"+type=crs +proj=pipeline +step +proj=longlat +ellps=GRS80 +step "
			"+proj=unitconvert +xy_in=rad +xy_out=deg");
		auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
		CHECK(crs != nullptr);
		
		auto op = CoordinateOperationFactory::create()->createOperation(
			GeographicCRS::EPSG_4326, NN_NO_CHECK(crs));
		CHECK(op != nullptr);
		EXPECT_EQ(op->exportToPROJString(PROJStringFormatter::create().get()),
			"+proj=axisswap +order=2,1");
	}

	{
		//	projparse_longlat_unitconvert_grad

		auto obj = PROJStringParser().createFromPROJString(
			"+type=crs +proj=pipeline +step +proj=longlat +ellps=GRS80 +step "
			"+proj=unitconvert +xy_in=rad +xy_out=grad");
		auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
		CHECK(crs != nullptr);
		
		auto op = CoordinateOperationFactory::create()->createOperation(
			GeographicCRS::EPSG_4326, NN_NO_CHECK(crs));
		CHECK(op != nullptr);
		EXPECT_EQ(op->exportToPROJString(PROJStringFormatter::create().get()),
			"+proj=pipeline +step +proj=axisswap +order=2,1 +step "
			"+proj=unitconvert +xy_in=deg +xy_out=rad +step "
			"+proj=unitconvert +xy_in=rad +xy_out=grad");
	}

	{
		//	projparse_longlat_unitconvert_rad

		auto obj = PROJStringParser().createFromPROJString(
			"+type=crs +proj=pipeline +step +proj=longlat +ellps=GRS80 +step "
			"+proj=unitconvert +xy_in=rad +xy_out=rad");
		auto crs = nn_dynamic_pointer_cast<GeographicCRS>(obj);
		CHECK(crs != nullptr);
		
		auto op = CoordinateOperationFactory::create()->createOperation(
			GeographicCRS::EPSG_4326, NN_NO_CHECK(crs));
		CHECK(op != nullptr);
		EXPECT_EQ(op->exportToPROJString(PROJStringFormatter::create().get()),
			"+proj=pipeline +step +proj=axisswap +order=2,1 +step "
			"+proj=unitconvert +xy_in=deg +xy_out=rad");
	}

	{
		//	projparse_tmerc

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=tmerc +x_0=1 +lat_0=1 +k_0=2 +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		f->simulCurNodeHasId();
		crs->exportToWKT(f.get());
		auto expected = "PROJCRS[\"unknown\",\n"
			"    BASEGEODCRS[\"unknown\",\n"
			"        DATUM[\"World Geodetic System 1984\",\n"
			"            ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                LENGTHUNIT[\"metre\",1]]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"    CONVERSION[\"unknown\",\n"
			"        METHOD[\"Transverse Mercator\",\n"
			"            ID[\"EPSG\",9807]],\n"
			"        PARAMETER[\"Latitude of natural origin\",1,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8801]],\n"
			"        PARAMETER[\"Longitude of natural origin\",0,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8802]],\n"
			"        PARAMETER[\"Scale factor at natural origin\",2,\n"
			"            SCALEUNIT[\"unity\",1],\n"
			"            ID[\"EPSG\",8805]],\n"
			"        PARAMETER[\"False easting\",1,\n"
			"            LENGTHUNIT[\"metre\",1],\n"
			"            ID[\"EPSG\",8806]],\n"
			"        PARAMETER[\"False northing\",0,\n"
			"            LENGTHUNIT[\"metre\",1],\n"
			"            ID[\"EPSG\",8807]]],\n"
			"    CS[Cartesian,2],\n"
			"        AXIS[\"(E)\",east,\n"
			"            ORDER[1],\n"
			"            LENGTHUNIT[\"metre\",1]],\n"
			"        AXIS[\"(N)\",north,\n"
			"            ORDER[2],\n"
			"            LENGTHUNIT[\"metre\",1]]]";
		
		EXPECT_EQ(f->toString(), expected);
	}

	{
		//	projparse_tmerc_south_oriented

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=tmerc +axis=wsu +x_0=1 +lat_0=1 +k_0=2 +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		f->simulCurNodeHasId();
		crs->exportToWKT(f.get());
		auto expected =
			"PROJCRS[\"unknown\",\n"
			"    BASEGEODCRS[\"unknown\",\n"
			"        DATUM[\"World Geodetic System 1984\",\n"
			"            ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
			"                LENGTHUNIT[\"metre\",1]]],\n"
			"        PRIMEM[\"Greenwich\",0,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433]]],\n"
			"    CONVERSION[\"unknown\",\n"
			"        METHOD[\"Transverse Mercator (South Orientated)\",\n"
			"            ID[\"EPSG\",9808]],\n"
			"        PARAMETER[\"Latitude of natural origin\",1,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8801]],\n"
			"        PARAMETER[\"Longitude of natural origin\",0,\n"
			"            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
			"            ID[\"EPSG\",8802]],\n"
			"        PARAMETER[\"Scale factor at natural origin\",2,\n"
			"            SCALEUNIT[\"unity\",1],\n"
			"            ID[\"EPSG\",8805]],\n"
			"        PARAMETER[\"False easting\",1,\n"
			"            LENGTHUNIT[\"metre\",1],\n"
			"            ID[\"EPSG\",8806]],\n"
			"        PARAMETER[\"False northing\",0,\n"
			"            LENGTHUNIT[\"metre\",1],\n"
			"            ID[\"EPSG\",8807]]],\n"
			"    CS[Cartesian,2],\n"
			"        AXIS[\"westing\",west,\n"
			"            ORDER[1],\n"
			"            LENGTHUNIT[\"metre\",1]],\n"
			"        AXIS[\"southing\",south,\n"
			"            ORDER[2],\n"
			"            LENGTHUNIT[\"metre\",1]]]";
		
		EXPECT_EQ(f->toString(), expected);
		
		obj = PROJStringParser().createFromPROJString(
			"+type=crs +proj=pipeline +step +proj=tmerc +x_0=1 +lat_0=1 +k_0=2 "
			"+step "
			"+proj=axisswap +order=-1,-2");
		crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		EXPECT_EQ(crs->derivingConversion()->method()->nameStr(),
			"Transverse Mercator (South Orientated)");
	}

	{
		//	projparse_lcc_as_lcc1sp

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=lcc +lat_0=45 +lat_1=45 +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		f->simulCurNodeHasId();
		f->setMultiLine(false);
		crs->exportToWKT(f.get());
		auto wkt = f->toString();
		CHECK(wkt.find("Lambert Conic Conformal (1SP)") != std::string::npos);
	}

	{
		// projparse_lcc_as_lcc2sp)

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=lcc +lat_0=45 +lat_1=46 +type=crs");
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		f->simulCurNodeHasId();
		f->setMultiLine(false);
		crs->exportToWKT(f.get());
		auto wkt = f->toString();
		CHECK(wkt.find("Lambert Conic Conformal (2SP)") != std::string::npos);
	}

	{
		// projparse_projected_vunits

		//ERROR
		//auto obj = PROJStringParser().createFromPROJString(
		//	"+proj=tmerc +vunits=ft +type=crs");
		//auto crs = nn_dynamic_pointer_cast<CompoundCRS>(obj);
		//CHECK(crs != nullptr);
		//WKTFormatterNNPtr f(WKTFormatter::create());
		//f->simulCurNodeHasId();
		//f->setMultiLine(false);
		//crs->exportToWKT(f.get());
		//auto wkt = f->toString();
		//CHECK(wkt.find("CS[Cartesian,2]") != std::string::npos);
		//CHECK(wkt.find("CS[vertical,1],AXIS[\"gravity-related height "
		//	"(H)\",up,LENGTHUNIT[\"foot\",0.3048]") !=
		//	std::string::npos);
	}

	{
		// projparse_geocent

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=geocent +ellps=WGS84 +type=crs");
		auto crs = nn_dynamic_pointer_cast<GeodeticCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		f->simulCurNodeHasId();
		f->setMultiLine(false);
		crs->exportToWKT(f.get());
		auto wkt = f->toString();
		EXPECT_EQ(wkt, "GEODCRS[\"unknown\",DATUM[\"Unknown based on WGS84 "
			"ellipsoid\",ELLIPSOID[\"WGS "
			"84\",6378137,298.257223563,LENGTHUNIT[\"metre\",1]]],"
			"PRIMEM[\"Greenwich\",0,ANGLEUNIT[\"degree\",0."
			"0174532925199433]],CS[Cartesian,3],AXIS[\"(X)\","
			"geocentricX,ORDER[1],LENGTHUNIT[\"metre\",1]],AXIS[\"(Y)\","
			"geocentricY,ORDER[2],LENGTHUNIT[\"metre\",1]],AXIS[\"(Z)\","
			"geocentricZ,ORDER[3],LENGTHUNIT[\"metre\",1]]]");
	}

	{
		// projparse_geocent_towgs84

		auto obj = PROJStringParser().createFromPROJString(
			"+proj=geocent +ellps=WGS84 +towgs84=1,2,3 +type=crs");
		auto crs = nn_dynamic_pointer_cast<BoundCRS>(obj);
		CHECK(crs != nullptr);
		WKTFormatterNNPtr f(WKTFormatter::create());
		f->simulCurNodeHasId();
		f->setMultiLine(false);
		crs->exportToWKT(f.get());
		auto wkt = f->toString();
		CHECK(
			wkt.find("METHOD[\"Geocentric translations (geocentric domain)") !=
			std::string::npos);
		CHECK(wkt.find("PARAMETER[\"X-axis translation\",1") !=
			std::string::npos);
		CHECK(wkt.find("PARAMETER[\"Y-axis translation\",2") != std::string::npos);
		CHECK(wkt.find("PARAMETER[\"Z-axis translation\",3") !=	std::string::npos);
	
		EXPECT_EQ(
			crs->exportToPROJString(
				PROJStringFormatter::create(PROJStringFormatter::Convention::PROJ_4)
				.get()),
			"+proj=geocent +ellps=WGS84 +towgs84=1,2,3,0,0,0,0 +units=m +no_defs "
			"+type=crs");
	}

	{
		// projparse_cart_unit

		std::string input(
			"+proj=pipeline +step +proj=cart +ellps=WGS84 +step "
			"+proj=unitconvert +xy_in=m +z_in=m +xy_out=km +z_out=km");
		auto obj = PROJStringParser().createFromPROJString(input);
		auto crs = nn_dynamic_pointer_cast<GeodeticCRS>(obj);
		CHECK(crs != nullptr);
		auto op = CoordinateOperationFactory::create()->createOperation(
			GeographicCRS::EPSG_4326, NN_NO_CHECK(crs));
		CHECK(op != nullptr);
		EXPECT_EQ(op->exportToPROJString(PROJStringFormatter::create().get()),
			"+proj=pipeline +step +proj=axisswap +order=2,1 +step "
			"+proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=cart "
			"+ellps=WGS84 +step +proj=unitconvert +xy_in=m +z_in=m "
			"+xy_out=km +z_out=km");
	}

	{
		// projparse_cart_unit_numeric

		std::string input(
			"+proj=pipeline +step +proj=cart +ellps=WGS84 +step "
			"+proj=unitconvert +xy_in=m +z_in=m +xy_out=500 +z_out=500");
		auto obj = PROJStringParser().createFromPROJString(input);
		auto crs = nn_dynamic_pointer_cast<GeodeticCRS>(obj);
		CHECK(crs != nullptr);
		auto op = CoordinateOperationFactory::create()->createOperation(
			GeographicCRS::EPSG_4326, NN_NO_CHECK(crs));
		CHECK(op != nullptr);
		EXPECT_EQ(op->exportToPROJString(PROJStringFormatter::create().get()),
			"+proj=pipeline +step +proj=axisswap +order=2,1 +step "
			"+proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=cart "
			"+ellps=WGS84 +step +proj=unitconvert +xy_in=m +z_in=m "
			"+xy_out=500 +z_out=500");
	}

	{
		// projparse_longlat_wktext

		std::string input("+proj=longlat +foo +wktext +type=crs");
		auto obj = PROJStringParser().createFromPROJString(input);
		auto crs = nn_dynamic_pointer_cast<GeodeticCRS>(obj);
		CHECK(crs != nullptr);
		EXPECT_EQ(
			crs->exportToPROJString(
				PROJStringFormatter::create(PROJStringFormatter::Convention::PROJ_4)
				.get()),
			"+proj=longlat +datum=WGS84 +no_defs +type=crs");
	}
	
	{
		// projparse_geocent_wktext

		std::string input("+proj=geocent +foo +wktext +type=crs");
		auto obj = PROJStringParser().createFromPROJString(input);
		auto crs = nn_dynamic_pointer_cast<GeodeticCRS>(obj);
		CHECK(crs != nullptr);
		EXPECT_EQ(
			crs->exportToPROJString(
				PROJStringFormatter::create(PROJStringFormatter::Convention::PROJ_4)
				.get()),
			"+proj=geocent +datum=WGS84 +units=m +no_defs +type=crs");
	}

	{
		// projparse_projected_wktext
	
			std::string input("+proj=merc +foo +wktext +type=crs");
		auto obj = PROJStringParser().createFromPROJString(input);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
		EXPECT_EQ(
			crs->exportToPROJString(
				PROJStringFormatter::create(PROJStringFormatter::Convention::PROJ_4)
				.get()),
			"+proj=merc +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +no_defs "
			"+type=crs");
	}

	{
		// Avoid division by zero
	
		// invalid_angular_unit

		auto wkt = "PROJCS[\"WGS 84 / UTM zone 31N\",\n"
			"    GEOGCS[\"WGS 84\",\n"
			"        DATUM[\"WGS_1984\",\n"
			"            SPHEROID[\"WGS 84\",6378137,298.257223563]],\n"
			"        PRIMEM[\"Greenwich\",0],\n"
			"        UNIT[\"foo\",0]],\n"
			"    PROJECTION[\"Transverse_Mercator\"],\n"
			"    PARAMETER[\"latitude_of_origin\",0],\n"
			"    PARAMETER[\"central_meridian\",3],\n"
			"    PARAMETER[\"scale_factor\",0.9996],\n"
			"    PARAMETER[\"false_easting\",500000],\n"
			"    PARAMETER[\"false_northing\",0],\n"
			"    UNIT[\"meter\",1]]";
	
		auto obj = WKTParser().createFromWKT(wkt);
		auto crs = nn_dynamic_pointer_cast<ProjectedCRS>(obj);
		CHECK(crs != nullptr);
	}
}

static bool equals(ExtentNNPtr extent1, ExtentNNPtr extent2) {
	return extent1->contains(extent2) && extent2->contains(extent1);
}

static bool equals(GeographicExtentNNPtr extent1,
	GeographicExtentNNPtr extent2) {
	return extent1->contains(extent2) && extent2->contains(extent1);
}

static GeographicExtentNNPtr getBBox(ExtentNNPtr extent) {
	assert(extent->geographicElements().size() == 1);
	return extent->geographicElements()[0];
}

TEST(PROJ4, metadata)
{
	{
		// citation

		Citation c("my citation");
		Citation c2(c);
		CHECK(c2.title().has_value());
		ASSERT_EQ(*(c2.title()), "my citation");
	}

	{
		// extent

		Extent::create(
			optional<std::string>(), std::vector<GeographicExtentNNPtr>(),
			std::vector<VerticalExtentNNPtr>(), std::vector<TemporalExtentNNPtr>());
	
		auto world = Extent::createFromBBOX(-180, -90, 180, 90);
		CHECK(world->isEquivalentTo(world.get()));
		CHECK(world->contains(world));
	
		auto west_hemisphere = Extent::createFromBBOX(-180, -90, 0, 90);
		CHECK(!world->isEquivalentTo(west_hemisphere.get()));
		CHECK(world->contains(west_hemisphere));
		CHECK(!west_hemisphere->contains(world));
	
		auto world_inter_world = world->intersection(world);
		CHECK(world_inter_world != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(world_inter_world), world));
	
		auto france = Extent::createFromBBOX(-5, 40, 12, 51);
		CHECK(france->contains(france));
		CHECK(world->contains(france));
		CHECK(!france->contains(
			world)); // We are only speaking about geography here ;-)
		CHECK(world->intersects(france));
		CHECK(france->intersects(world));
	
		auto france_inter_france = france->intersection(france);
		CHECK(france_inter_france != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(france_inter_france), france));
	
		auto france_inter_world = france->intersection(world);
		CHECK(france_inter_world != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(france_inter_world), france));
	
		auto world_inter_france = world->intersection(france);
		CHECK(world_inter_france != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(world_inter_france), france));
	
		auto france_shifted =
			Extent::createFromBBOX(-5 + 5, 40 + 5, 12 + 5, 51 + 5);
		CHECK(france->intersects(france_shifted));
		CHECK(france_shifted->intersects(france));
		CHECK(!france->contains(france_shifted));
		CHECK(!france_shifted->contains(france));
	
		auto europe = Extent::createFromBBOX(-30, 25, 30, 70);
		CHECK(europe->contains(france));
		CHECK(!france->contains(europe));
	
		auto france_inter_europe = france->intersection(europe);
		CHECK(france_inter_europe != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(france_inter_europe), france));
	
		auto europe_intersects_france = europe->intersection(france);
		CHECK(europe_intersects_france != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(europe_intersects_france), france));
	
		auto nz = Extent::createFromBBOX(155.0, -60.0, -170.0, -25.0);
		CHECK(nz->contains(nz));
		CHECK(world->contains(nz));
		CHECK(nz->intersects(world));
		CHECK(world->intersects(nz));
		CHECK(!nz->contains(world));
		CHECK(!nz->contains(france));
		CHECK(!france->contains(nz));
		CHECK(!nz->intersects(france));
		CHECK(!france->intersects(nz));
	
		{
			auto nz_inter_world = nz->intersection(world);
			CHECK(nz_inter_world != nullptr);
			CHECK(equals(NN_CHECK_ASSERT(nz_inter_world), nz));
		}
	
		{
			auto nz_inter_world = getBBox(nz)->intersection(getBBox(world));
			CHECK(nz_inter_world != nullptr);
			CHECK(equals(NN_CHECK_ASSERT(nz_inter_world), getBBox(nz)));
		}
	
		{
			auto world_inter_nz = world->intersection(nz);
			CHECK(world_inter_nz != nullptr);
			CHECK(equals(NN_CHECK_ASSERT(world_inter_nz), nz));
		}
	
		{
			auto world_inter_nz = getBBox(world)->intersection(getBBox(nz));
			CHECK(world_inter_nz != nullptr);
			CHECK(equals(NN_CHECK_ASSERT(world_inter_nz), getBBox(nz)));
		}
	
		CHECK(nz->intersection(france) == nullptr);
		CHECK(france->intersection(nz) == nullptr);
	
		auto bbox_antimeridian_north =
			Extent::createFromBBOX(155.0, 10.0, -170.0, 30.0);
		CHECK(!nz->contains(bbox_antimeridian_north));
		CHECK(!bbox_antimeridian_north->contains(nz));
		CHECK(!nz->intersects(bbox_antimeridian_north));
		CHECK(!bbox_antimeridian_north->intersects(nz));
		CHECK(!nz->intersection(bbox_antimeridian_north));
		CHECK(!bbox_antimeridian_north->intersection(nz));
	
		auto nz_pos_long = Extent::createFromBBOX(155.0, -60.0, 180.0, -25.0);
		CHECK(nz->contains(nz_pos_long));
		CHECK(!nz_pos_long->contains(nz));
		CHECK(nz->intersects(nz_pos_long));
		CHECK(nz_pos_long->intersects(nz));
		auto nz_inter_nz_pos_long = nz->intersection(nz_pos_long);
		CHECK(nz_inter_nz_pos_long != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(nz_inter_nz_pos_long), nz_pos_long));
		auto nz_pos_long_inter_nz = nz_pos_long->intersection(nz);
		CHECK(nz_pos_long_inter_nz != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(nz_pos_long_inter_nz), nz_pos_long));
	
		auto nz_neg_long = Extent::createFromBBOX(-180.0, -60.0, -170.0, -25.0);
		CHECK(nz->contains(nz_neg_long));
		CHECK(!nz_neg_long->contains(nz));
		CHECK(nz->intersects(nz_neg_long));
		CHECK(nz_neg_long->intersects(nz));
		auto nz_inter_nz_neg_long = nz->intersection(nz_neg_long);
		CHECK(nz_inter_nz_neg_long != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(nz_inter_nz_neg_long), nz_neg_long));
		auto nz_neg_long_inter_nz = nz_neg_long->intersection(nz);
		CHECK(nz_neg_long_inter_nz != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(nz_neg_long_inter_nz), nz_neg_long));
	
		auto nz_smaller = Extent::createFromBBOX(160, -55.0, -175.0, -30.0);
		CHECK(nz->contains(nz_smaller));
		CHECK(!nz_smaller->contains(nz));
	
		auto nz_pos_long_shifted_west =
			Extent::createFromBBOX(150.0, -60.0, 175.0, -25.0);
		CHECK(!nz->contains(nz_pos_long_shifted_west));
		CHECK(!nz_pos_long_shifted_west->contains(nz));
		CHECK(nz->intersects(nz_pos_long_shifted_west));
		CHECK(nz_pos_long_shifted_west->intersects(nz));
	
		auto nz_smaller_shifted = Extent::createFromBBOX(165, -60.0, -170.0, -25.0);
		CHECK(!nz_smaller->contains(nz_smaller_shifted));
		CHECK(!nz_smaller_shifted->contains(nz_smaller));
		CHECK(nz_smaller->intersects(nz_smaller_shifted));
		CHECK(nz_smaller_shifted->intersects(nz_smaller));
	
		auto nz_shifted = Extent::createFromBBOX(165.0, -60.0, -160.0, -25.0);
		auto nz_intersect_nz_shifted = nz->intersection(nz_shifted);
		CHECK(nz_intersect_nz_shifted != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(nz_intersect_nz_shifted),
			Extent::createFromBBOX(165, -60.0, -170.0, -25.0)));
	
		auto nz_inter_nz_smaller = nz->intersection(nz_smaller);
		CHECK(nz_inter_nz_smaller != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(nz_inter_nz_smaller), nz_smaller));
	
		auto nz_smaller_inter_nz = nz_smaller->intersection(nz);
		CHECK(nz_smaller_inter_nz != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(nz_smaller_inter_nz), nz_smaller));
	
		auto world_smaller = Extent::createFromBBOX(-179, -90, 179, 90);
		CHECK(!world_smaller->contains(nz));
		CHECK(!nz->contains(world_smaller));
	
		auto nz_inter_world_smaller = nz->intersection(world_smaller);
		CHECK(nz_inter_world_smaller != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(nz_inter_world_smaller),
			Extent::createFromBBOX(155, -60, 179, -25)));
	
		auto world_smaller_inter_nz = world_smaller->intersection(nz);
		CHECK(world_smaller_inter_nz != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(world_smaller_inter_nz),
			Extent::createFromBBOX(155, -60, 179, -25)));
	
		auto world_smaller_east = Extent::createFromBBOX(-179, -90, 150, 90);
		CHECK(!world_smaller_east->contains(nz));
		CHECK(!nz->contains(world_smaller_east));
	
		auto nz_inter_world_smaller_east = nz->intersection(world_smaller_east);
		CHECK(nz_inter_world_smaller_east != nullptr);
		EXPECT_EQ(nn_dynamic_pointer_cast<GeographicBoundingBox>(
			nz_inter_world_smaller_east->geographicElements()[0])
			->westBoundLongitude(),
			-179);
		EXPECT_EQ(nn_dynamic_pointer_cast<GeographicBoundingBox>(
			nz_inter_world_smaller_east->geographicElements()[0])
			->eastBoundLongitude(),
			-170);
		CHECK(equals(NN_CHECK_ASSERT(nz_inter_world_smaller_east),
			Extent::createFromBBOX(-179, -60, -170, -25)));
	
		auto world_smaller_east_inter_nz = world_smaller_east->intersection(nz);
		CHECK(world_smaller_east_inter_nz != nullptr);
		EXPECT_EQ(nn_dynamic_pointer_cast<GeographicBoundingBox>(
			world_smaller_east_inter_nz->geographicElements()[0])
			->westBoundLongitude(),
			-179);
		EXPECT_EQ(nn_dynamic_pointer_cast<GeographicBoundingBox>(
			world_smaller_east_inter_nz->geographicElements()[0])
			->eastBoundLongitude(),
			-170);
		CHECK(equals(NN_CHECK_ASSERT(world_smaller_east_inter_nz),
			Extent::createFromBBOX(-179, -60, -170, -25)));
	
		auto east_hemisphere = Extent::createFromBBOX(0, -90, 180, 90);
		auto east_hemisphere_inter_nz = east_hemisphere->intersection(nz);
		CHECK(east_hemisphere_inter_nz != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(east_hemisphere_inter_nz),
			Extent::createFromBBOX(155.0, -60.0, 180.0, -25.0)));
	
		auto minus_180_to_156 = Extent::createFromBBOX(-180, -90, 156, 90);
		auto minus_180_to_156_inter_nz = minus_180_to_156->intersection(nz);
		CHECK(minus_180_to_156_inter_nz != nullptr);
		CHECK(equals(NN_CHECK_ASSERT(minus_180_to_156_inter_nz),
			Extent::createFromBBOX(-180.0, -60.0, -170.0, -25.0)));
	}

	{
		// identifier_empty

		auto id(Identifier::create());
		Identifier id2(*id);
		CHECK(!id2.authority().has_value());
		CHECK(id2.code().empty());
		CHECK(!id2.codeSpace().has_value());
		CHECK(!id2.version().has_value());
		CHECK(!id2.description().has_value());
	}

	{
		// identifier_properties

		PropertyMap properties;
		properties.set(Identifier::AUTHORITY_KEY, "authority");
		properties.set(Identifier::CODESPACE_KEY, "codespace");
		properties.set(Identifier::VERSION_KEY, "version");
		properties.set(Identifier::DESCRIPTION_KEY, "description");
		auto id(Identifier::create("my code", properties));
		Identifier id2(*id);
		CHECK(id2.authority().has_value());
		ASSERT_EQ(*(id2.authority()->title()), "authority");
		ASSERT_EQ(id2.code(), "my code");
		CHECK(id2.codeSpace().has_value());
		ASSERT_EQ(*(id2.codeSpace()), "codespace");
		CHECK(id2.version().has_value());
		ASSERT_EQ(*(id2.version()), "version");
		CHECK(id2.description().has_value());
		ASSERT_EQ(*(id2.description()), "description");
	}

	{
		// identifier_code_integer

		PropertyMap properties;
		properties.set(Identifier::CODE_KEY, 1234);
		auto id(Identifier::create(std::string(), properties));
		ASSERT_EQ(id->code(), "1234");
	}

	{
		// identifier_code_string

		PropertyMap properties;
		properties.set(Identifier::CODE_KEY, "1234");
		auto id(Identifier::create(std::string(), properties));
		ASSERT_EQ(id->code(), "1234");
	}
	
	{
		// identifier_authority_citation
	
		PropertyMap properties;
		properties.set(Identifier::AUTHORITY_KEY,
			nn_make_shared<Citation>("authority"));
		auto id(Identifier::create(std::string(), properties));
		CHECK(id->authority().has_value());
		ASSERT_EQ(*(id->authority()->title()), "authority");
	}
	
	{
		// id
	
		auto in_wkt = "ID[\"EPSG\",4946,1.5,\n"
			"    CITATION[\"my citation\"],\n"
			"    URI[\"urn:ogc:def:crs:EPSG::4946\"]]";
		auto id =
			nn_dynamic_pointer_cast<Identifier>(WKTParser().createFromWKT(in_wkt));
		CHECK(id != nullptr);

		CHECK(id->authority().has_value());
		EXPECT_EQ(*(id->authority()->title()), "my citation");
		EXPECT_EQ(*(id->codeSpace()), "EPSG");
		EXPECT_EQ(id->code(), "4946");
		CHECK(id->version().has_value());
		EXPECT_EQ(*(id->version()), "1.5");
		CHECK(id->uri().has_value());
		EXPECT_EQ(*(id->uri()), "urn:ogc:def:crs:EPSG::4946");

		auto got_wkt = id->exportToWKT(WKTFormatter::create().get());
		EXPECT_EQ(got_wkt, in_wkt);
	}

	{
		// Identifier_isEquivalentName
	
		CHECK(Identifier::isEquivalentName("Central_Meridian", "Central_- ()/Meridian"));
		CHECK(Identifier::isEquivalentName("\xc3\xa1", "a"));
		CHECK(Identifier::isEquivalentName("a", "\xc3\xa1"));
		CHECK(Identifier::isEquivalentName("\xc3\xa4", "\xc3\xa1"));
	}
}

TEST(PROJ4, util)
{
	{
		// NameFactory

		LocalNameNNPtr localname(NameFactory::createLocalName(nullptr, "foo"));
		auto ns = localname->scope();
		CHECK(ns->isGlobal() == true);
		CHECK(ns->name()->toFullyQualifiedName()->toString() == "global");
		CHECK(localname->toFullyQualifiedName()->toString() == "foo");
	}

	{
		// NameFactory2

		PropertyMap map;
		map.set("separator", "/");
		NameSpaceNNPtr nsIn(NameFactory::createNameSpace(NameFactory::createLocalName(nullptr, std::string("bar")), map));
		LocalNameNNPtr localname(NameFactory::createLocalName(nsIn, std::string("foo")));
		auto ns = localname->scope();
		CHECK(ns->isGlobal() == false);
		auto fullyqualifiedNS = ns->name()->toFullyQualifiedName();
		CHECK(fullyqualifiedNS->toString() == "bar");
		CHECK(fullyqualifiedNS->scope()->isGlobal() == true);
		CHECK(fullyqualifiedNS->scope()->name()->scope()->isGlobal() == true);
		CHECK(localname->toFullyQualifiedName()->toString() == "bar/foo");
	}
}

TEST(PjPhi2Test, Basic)
{
	{
		projCtx ctx = pj_get_default_ctx();

		DOUBLES_EQUAL(M_PI_2, pj_phi2(ctx, 0.0, 0.0), 0.0);

		DOUBLES_EQUAL(0.0, pj_phi2(ctx, 1.0, 0.0), 1e-16);
		DOUBLES_EQUAL(M_PI_2, pj_phi2(ctx, 0.0, 1.0), 0.0);
		DOUBLES_EQUAL(M_PI, pj_phi2(ctx, -1.0, 0.0), 0.0);
		DOUBLES_EQUAL(M_PI_2, pj_phi2(ctx, 0.0, -1.0), 0.0);

		DOUBLES_EQUAL(0.0, pj_phi2(ctx, 1.0, 1.0), 1e-16);
		DOUBLES_EQUAL(M_PI, pj_phi2(ctx, -1.0, -1.0), 0.0);

		// TODO(schwehr): M_PI_4, M_PI_2, M_PI, M_E
		// https://www.gnu.org/software/libc/manual/html_node/Mathematical-Constants.html

		DOUBLES_EQUAL(-0.95445818456292697, pj_phi2(ctx, M_PI, 0.0), 1e-12);
		CHECK(std::isnan(pj_phi2(ctx, 0.0, M_PI)));
		DOUBLES_EQUAL(4.0960508381527205, pj_phi2(ctx, -M_PI, 0.0), 1e-12);
		CHECK(std::isnan(pj_phi2(ctx, 0.0, -M_PI)));

		CHECK(std::isnan(pj_phi2(ctx, M_PI, M_PI)));
		CHECK(std::isnan(pj_phi2(ctx, -M_PI, -M_PI)));
	}

	{
		// AvoidUndefinedBehavior)

		auto ctx = pj_get_default_ctx();

		const auto nan = std::numeric_limits<double>::quiet_NaN();
		CHECK(std::isnan(pj_phi2(ctx, nan, 0.0)));
		CHECK(std::isnan(pj_phi2(ctx, 0.0, nan)));
		CHECK(std::isnan(pj_phi2(ctx, nan, nan)));

		// We do not really care about the values that follow.
		const auto inf = std::numeric_limits<double>::infinity();

		DOUBLES_EQUAL(-M_PI_2, pj_phi2(ctx, inf, 0.0), 0.0);
		CHECK(std::isnan(pj_phi2(ctx, 0.0, inf)));

		DOUBLES_EQUAL(4.7123889803846897, pj_phi2(ctx, -inf, 0.0), 0.0);
		CHECK(std::isnan(pj_phi2(ctx, 0.0, -inf)));

		CHECK(std::isnan(pj_phi2(ctx, inf, inf)));
		CHECK(std::isnan(pj_phi2(ctx, -inf, -inf)));
	}
}
