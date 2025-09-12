// CppUnitLite test harness for Calc library
//
// Supported Tests:
//		CHECK( boolean )
//		LONGS_EQUAL( num1, num2 )
//

#include <stdio.h>
#include <stdlib.h>

#include "CppUnitLite/TestHarness.h"	// CppUnitLite library
#include "Calc.h"						// interface to class under test

int main(int argc, char* argv[])
{
	// output name of executable
	if (argc > 0 && argv[0])
		printf("%s\n", argv[0]);

	TestResult tr;
	TestRegistry::runAllTests(tr);

	// always pause if errors
	int failureCount = tr.GetFailureCount();
	if (failureCount > 0)
		getc(stdin);

	return failureCount;
}

//----------------------------------------------------------------------------
TEST(Calc, Spline)
{
	// Test bi-cubic spline function.
	//

	{
		double x[10] = { 1030, 1346, 1546, 1192, 638, 1007, 1769, 1015, 307, 1130 };
		double y[10] = { 1200, 1075, 1391, 1675, 1191, 641, 916, 2000, 891, 216 };
		double kt[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		int samples = sizeof(x) / sizeof(double);

		// calculate the x & y values independently
		std::vector<double> xx = CubicSpline(x, kt, samples);
		std::vector<double> yy = CubicSpline(y, kt, samples);

		// x values

		// these come out in blocks of 20 per segment because there's a 
		// hard-coded value of 20 interpolated values per segment

		DOUBLES_EQUAL( xx[0], 1030.0, 0.0);
		DOUBLES_EQUAL( xx[1], 1045.742229, 1E-6);
		DOUBLES_EQUAL( xx[2], 1061.485327, 1E-6);
		DOUBLES_EQUAL( xx[3], 1077.230162, 1E-6);
		DOUBLES_EQUAL( xx[4], 1092.977603, 1E-6);
		DOUBLES_EQUAL( xx[5], 1108.728519, 1E-6);
		DOUBLES_EQUAL( xx[6], 1124.483779, 1E-6);
		DOUBLES_EQUAL( xx[7], 1140.244252, 1E-6);
		DOUBLES_EQUAL( xx[8], 1156.010805, 1E-6);
		DOUBLES_EQUAL( xx[9], 1171.784309, 1E-6);
		DOUBLES_EQUAL(xx[10], 1187.565631, 1E-6);
		DOUBLES_EQUAL(xx[11], 1203.355640, 1E-6);
		DOUBLES_EQUAL(xx[12], 1219.155206, 1E-6);
		DOUBLES_EQUAL(xx[13], 1234.965196, 1E-6);
		DOUBLES_EQUAL(xx[14], 1250.786481, 1E-6);
		DOUBLES_EQUAL(xx[15], 1266.619927, 1E-6);
		DOUBLES_EQUAL(xx[16], 1282.466404, 1E-6);
		DOUBLES_EQUAL(xx[17], 1298.326782, 1E-6);
		DOUBLES_EQUAL(xx[18], 1314.201928, 1E-6);
		DOUBLES_EQUAL(xx[19], 1330.092711, 1E-6);

		DOUBLES_EQUAL(xx[20], 1346.000000, 1E-6);
		DOUBLES_EQUAL(xx[21], 1361.909295, 1E-6);
		DOUBLES_EQUAL(xx[22], 1377.744622, 1E-6);
		DOUBLES_EQUAL(xx[23], 1393.414635, 1E-6);
		DOUBLES_EQUAL(xx[24], 1408.827993, 1E-6);
		DOUBLES_EQUAL(xx[25], 1423.893350, 1E-6);
		DOUBLES_EQUAL(xx[26], 1438.519364, 1E-6);
		DOUBLES_EQUAL(xx[27], 1452.614690, 1E-6);
		DOUBLES_EQUAL(xx[28], 1466.087985, 1E-6);
		DOUBLES_EQUAL(xx[29], 1478.847906, 1E-6);
		DOUBLES_EQUAL(xx[30], 1490.803108, 1E-6);
		DOUBLES_EQUAL(xx[31], 1501.862248, 1E-6);
		DOUBLES_EQUAL(xx[32], 1511.933982, 1E-6);
		DOUBLES_EQUAL(xx[33], 1520.926966, 1E-6);
		DOUBLES_EQUAL(xx[34], 1528.749857, 1E-6);
		DOUBLES_EQUAL(xx[35], 1535.311312, 1E-6);
		DOUBLES_EQUAL(xx[36], 1540.519985, 1E-6);
		DOUBLES_EQUAL(xx[37], 1544.284535, 1E-6);
		DOUBLES_EQUAL(xx[38], 1546.513616, 1E-6);
		DOUBLES_EQUAL(xx[39], 1547.115886, 1E-6);

		DOUBLES_EQUAL(xx[40], 1546.000000, 1E-6);
		DOUBLES_EQUAL(xx[41], 1543.095840, 1E-6);
		DOUBLES_EQUAL(xx[42], 1538.418187, 1E-6);
		DOUBLES_EQUAL(xx[43], 1532.003047, 1E-6);
		DOUBLES_EQUAL(xx[44], 1523.886427, 1E-6);
		DOUBLES_EQUAL(xx[45], 1514.104331, 1E-6);
		DOUBLES_EQUAL(xx[46], 1502.692766, 1E-6);
		DOUBLES_EQUAL(xx[47], 1489.687738, 1E-6);
		DOUBLES_EQUAL(xx[48], 1475.125254, 1E-6);
		DOUBLES_EQUAL(xx[49], 1459.041319, 1E-6);
		DOUBLES_EQUAL(xx[50], 1441.471939, 1E-6);
		DOUBLES_EQUAL(xx[51], 1422.453120, 1E-6);
		DOUBLES_EQUAL(xx[52], 1402.020868, 1E-6);
		DOUBLES_EQUAL(xx[53], 1380.211190, 1E-6);
		DOUBLES_EQUAL(xx[54], 1357.060091, 1E-6);
		DOUBLES_EQUAL(xx[55], 1332.603577, 1E-6);
		DOUBLES_EQUAL(xx[56], 1306.877655, 1E-6);
		DOUBLES_EQUAL(xx[57], 1279.918330, 1E-6);
		DOUBLES_EQUAL(xx[58], 1251.761608, 1E-6);
		DOUBLES_EQUAL(xx[59], 1222.443496, 1E-6);

		DOUBLES_EQUAL(xx[60], 1192.000000, 1E-6);		// 60
		DOUBLES_EQUAL(xx[61], 1160.496594, 1E-6);
		DOUBLES_EQUAL(xx[62], 1128.116629, 1E-6);
		DOUBLES_EQUAL(xx[63], 1095.072925, 1E-6);
		DOUBLES_EQUAL(xx[64], 1061.578301, 1E-6);
		DOUBLES_EQUAL(xx[65], 1027.845577, 1E-6);
		DOUBLES_EQUAL(xx[66],  994.087572, 1E-6);
		DOUBLES_EQUAL(xx[67],  960.517106, 1E-6);
		DOUBLES_EQUAL(xx[68],  927.346999, 1E-6);
		DOUBLES_EQUAL(xx[69],  894.790070, 1E-6);
		DOUBLES_EQUAL(xx[70],  863.059138, 1E-6);
		DOUBLES_EQUAL(xx[71],  832.367024, 1E-6);
		DOUBLES_EQUAL(xx[72],  802.926546, 1E-6);
		DOUBLES_EQUAL(xx[73],  774.950525, 1E-6);
		DOUBLES_EQUAL(xx[74],  748.651780, 1E-6);
		DOUBLES_EQUAL(xx[75],  724.243130, 1E-6);
		DOUBLES_EQUAL(xx[76],  701.937396, 1E-6);
		DOUBLES_EQUAL(xx[77],  681.947396, 1E-6);
		DOUBLES_EQUAL(xx[78],  664.485950, 1E-6);
		DOUBLES_EQUAL(xx[79],  649.765878, 1E-6);

		DOUBLES_EQUAL(xx[80], 638.000000, 1E-6);		// 80
		DOUBLES_EQUAL(xx[81], 629.358159, 1E-6);
		DOUBLES_EQUAL(xx[82], 623.838296, 1E-6);
		DOUBLES_EQUAL(xx[83], 621.395378, 1E-6);
		DOUBLES_EQUAL(xx[84], 621.984370, 1E-6);
		DOUBLES_EQUAL(xx[85], 625.560237, 1E-6);
		DOUBLES_EQUAL(xx[86], 632.077946, 1E-6);
		DOUBLES_EQUAL(xx[87], 641.492462, 1E-6);
		DOUBLES_EQUAL(xx[88], 653.758751, 1E-6);
		DOUBLES_EQUAL(xx[89], 668.831778, 1E-6);
		DOUBLES_EQUAL(xx[90], 686.666509, 1E-6);
		DOUBLES_EQUAL(xx[91], 707.217911, 1E-6);
		DOUBLES_EQUAL(xx[92], 730.440947, 1E-6);
		DOUBLES_EQUAL(xx[93], 756.290585, 1E-6);
		DOUBLES_EQUAL(xx[94], 784.721789, 1E-6);
		DOUBLES_EQUAL(xx[95], 815.689527, 1E-6);
		DOUBLES_EQUAL(xx[96], 849.148762, 1E-6);
		DOUBLES_EQUAL(xx[97], 885.054461, 1E-6);
		DOUBLES_EQUAL(xx[98], 923.361590, 1E-6);
		DOUBLES_EQUAL(xx[99], 964.025115, 1E-6);

		DOUBLES_EQUAL(xx[100], 1007.000000, 1E-6);		// 100
		DOUBLES_EQUAL(xx[101], 1052.177021, 1E-6);
		DOUBLES_EQUAL(xx[102], 1099.190186, 1E-6);
		DOUBLES_EQUAL(xx[103], 1147.609313, 1E-6);
		DOUBLES_EQUAL(xx[104], 1197.004219, 1E-6);
		DOUBLES_EQUAL(xx[105], 1246.944723, 1E-6);
		DOUBLES_EQUAL(xx[106], 1297.000643, 1E-6);
		DOUBLES_EQUAL(xx[107], 1346.741795, 1E-6);
		DOUBLES_EQUAL(xx[108], 1395.737997, 1E-6);
		DOUBLES_EQUAL(xx[109], 1443.559068, 1E-6);
		DOUBLES_EQUAL(xx[110], 1489.774824, 1E-6);
		DOUBLES_EQUAL(xx[111], 1533.955084, 1E-6);
		DOUBLES_EQUAL(xx[112], 1575.669665, 1E-6);
		DOUBLES_EQUAL(xx[113], 1614.488385, 1E-6);
		DOUBLES_EQUAL(xx[114], 1649.981062, 1E-6);
		DOUBLES_EQUAL(xx[115], 1681.717513, 1E-6);
		DOUBLES_EQUAL(xx[116], 1709.267556, 1E-6);
		DOUBLES_EQUAL(xx[117], 1732.201008, 1E-6);
		DOUBLES_EQUAL(xx[118], 1750.087688, 1E-6);
		DOUBLES_EQUAL(xx[119], 1762.497413, 1E-6);

		//	1769.000000		// 120
		//	1769.292634
		//	1763.581961
		//	1752.201997
		//	1735.486753
		//	1713.770244
		//	1687.386483
		//	1656.669484
		//	1621.953260
		//	1583.571826
		//	1541.859193
		//	1497.149378
		//	1449.776391
		//	1400.074248
		//	1348.376962
		//	1295.018547
		//	1240.333015
		//	1184.654381
		//	1128.316658
		//	1071.653860

		//	1015.000000		// 140
		//	958.677695
		//	902.963969
		//	848.124451
		//	794.424770
		//	742.130552
		//	691.507426
		//	642.821020
		//	596.336962
		//	552.320880
		//	511.038402
		//	472.755156
		//	437.736770
		//	406.248871
		//	378.557089
		//	354.927051
		//	335.624385
		//	320.914719
		//	311.063680
		//	306.336898

		DOUBLES_EQUAL(xx[160], 307.000000, 1E-6);		// 160
		DOUBLES_EQUAL(xx[161], 313.227213, 1E-6);
		DOUBLES_EQUAL(xx[162], 324.827163, 1E-6);
		DOUBLES_EQUAL(xx[163], 341.517073, 1E-6);
		DOUBLES_EQUAL(xx[164], 363.014169, 1E-6);
		DOUBLES_EQUAL(xx[165], 389.035674, 1E-6);
		DOUBLES_EQUAL(xx[166], 419.298814, 1E-6);
		DOUBLES_EQUAL(xx[167], 453.520812, 1E-6);
		DOUBLES_EQUAL(xx[168], 491.418892, 1E-6);
		DOUBLES_EQUAL(xx[169], 532.710280, 1E-6);
		DOUBLES_EQUAL(xx[170], 577.112199, 1E-6);
		DOUBLES_EQUAL(xx[171], 624.341875, 1E-6);
		DOUBLES_EQUAL(xx[172], 674.116531, 1E-6);
		DOUBLES_EQUAL(xx[173], 726.153391, 1E-6);
		DOUBLES_EQUAL(xx[174], 780.169681, 1E-6);
		DOUBLES_EQUAL(xx[175], 835.882625, 1E-6);
		DOUBLES_EQUAL(xx[176], 893.009446, 1E-6);
		DOUBLES_EQUAL(xx[177], 951.267370, 1E-6);
		DOUBLES_EQUAL(xx[178], 1010.373621, 1E-6);
		DOUBLES_EQUAL(xx[179], 1070.045423, 1E-6);

		// y values

		DOUBLES_EQUAL( yy[0], 1200.000000, 1E-6);
		DOUBLES_EQUAL( yy[1], 1188.431244, 1E-6);
		DOUBLES_EQUAL( yy[2], 1176.942469, 1E-6);
		DOUBLES_EQUAL( yy[3], 1165.613656, 1E-6);
		DOUBLES_EQUAL( yy[4], 1154.524788, 1E-6);
		DOUBLES_EQUAL( yy[5], 1143.755844, 1E-6);
		DOUBLES_EQUAL( yy[6], 1133.386807, 1E-6);
		DOUBLES_EQUAL( yy[7], 1123.497658, 1E-6);
		DOUBLES_EQUAL( yy[8], 1114.168378, 1E-6);
		DOUBLES_EQUAL( yy[9], 1105.478949, 1E-6);
		DOUBLES_EQUAL(yy[10], 1097.509351, 1E-6);
		DOUBLES_EQUAL(yy[11], 1090.339566, 1E-6);
		DOUBLES_EQUAL(yy[12], 1084.049575, 1E-6);
		DOUBLES_EQUAL(yy[13], 1078.719360, 1E-6);
		DOUBLES_EQUAL(yy[14], 1074.428902, 1E-6);
		DOUBLES_EQUAL(yy[15], 1071.258182, 1E-6);
		DOUBLES_EQUAL(yy[16], 1069.287181, 1E-6);
		DOUBLES_EQUAL(yy[17], 1068.595882, 1E-6);
		DOUBLES_EQUAL(yy[18], 1069.264264, 1E-6);
		DOUBLES_EQUAL(yy[19], 1071.372310, 1E-6);

		DOUBLES_EQUAL(yy[20], 1075.000000, 1E-6);		// 20
		DOUBLES_EQUAL(yy[21], 1080.202460, 1E-6);
		DOUBLES_EQUAL(yy[22], 1086.935390, 1E-6);
		DOUBLES_EQUAL(yy[23], 1095.129632, 1E-6);
		DOUBLES_EQUAL(yy[24], 1104.716031, 1E-6);
		DOUBLES_EQUAL(yy[25], 1115.625430, 1E-6);
		DOUBLES_EQUAL(yy[26], 1127.788673, 1E-6);
		DOUBLES_EQUAL(yy[27], 1141.136602, 1E-6);
		DOUBLES_EQUAL(yy[28], 1155.600062, 1E-6);
		DOUBLES_EQUAL(yy[29], 1171.109896, 1E-6);
		DOUBLES_EQUAL(yy[30], 1187.596948, 1E-6);
		DOUBLES_EQUAL(yy[31], 1204.992060, 1E-6);
		DOUBLES_EQUAL(yy[32], 1223.226078, 1E-6);
		DOUBLES_EQUAL(yy[33], 1242.229843, 1E-6);
		DOUBLES_EQUAL(yy[34], 1261.934200, 1E-6);
		DOUBLES_EQUAL(yy[35], 1282.269992, 1E-6);
		DOUBLES_EQUAL(yy[36], 1303.168062, 1E-6);
		DOUBLES_EQUAL(yy[37], 1324.559255, 1E-6);
		DOUBLES_EQUAL(yy[38], 1346.374413, 1E-6);
		DOUBLES_EQUAL(yy[39], 1368.544380, 1E-6);

		DOUBLES_EQUAL(yy[40], 1391.000000, 1E-6);		// 40
		DOUBLES_EQUAL(yy[41], 1413.657291, 1E-6);
		DOUBLES_EQUAL(yy[42], 1436.372973, 1E-6);
		DOUBLES_EQUAL(yy[43], 1458.988941, 1E-6);
		DOUBLES_EQUAL(yy[44], 1481.347088, 1E-6);
		DOUBLES_EQUAL(yy[45], 1503.289310, 1E-6);
		DOUBLES_EQUAL(yy[46], 1524.657502, 1E-6);
		DOUBLES_EQUAL(yy[47], 1545.293558, 1E-6);
		DOUBLES_EQUAL(yy[48], 1565.039373, 1E-6);
		DOUBLES_EQUAL(yy[49], 1583.736841, 1E-6);
		DOUBLES_EQUAL(yy[50], 1601.227858, 1E-6);
		DOUBLES_EQUAL(yy[51], 1617.354317, 1E-6);
		DOUBLES_EQUAL(yy[52], 1631.958114, 1E-6);
		DOUBLES_EQUAL(yy[53], 1644.881143, 1E-6);
		DOUBLES_EQUAL(yy[54], 1655.965299, 1E-6);
		DOUBLES_EQUAL(yy[55], 1665.052476, 1E-6);
		DOUBLES_EQUAL(yy[56], 1671.984570, 1E-6);
		DOUBLES_EQUAL(yy[57], 1676.603475, 1E-6);
		DOUBLES_EQUAL(yy[58], 1678.751085, 1E-6);
		DOUBLES_EQUAL(yy[59], 1678.269295, 1E-6);

		//	1675.000000		// 60
		//	1668.836375
		//	1659.876718
		//	1648.270606
		//	1634.167616
		//	1617.717328
		//	1599.069318
		//	1578.373165
		//	1555.778445
		//	1531.434738
		//	1505.491620
		//	1478.098670
		//	1449.405466
		//	1419.561585
		//	1388.716604
		//	1357.020103
		//	1324.621658
		//	1291.670847
		//	1258.317249
		//	1224.710441

		//	1191.000000		// 80
		//	1157.324958
		//	1123.782155
		//	1090.457887
		//	1057.438446
		//	1024.810128
		//	992.659225
		//	961.072033
		//	930.134846
		//	899.933957
		//	870.555660
		//	842.086251
		//	814.612022
		//	788.219269
		//	762.994284
		//	739.023363
		//	716.392799
		//	695.188887
		//	675.497920
		//	657.406193

		//	641.000000		// 100
		//	626.380168
		//	613.705661
		//	603.149972
		//	594.886599
		//	589.089036
		//	585.930781
		//	585.585327
		//	588.226172
		//	594.026810
		//	603.160738
		//	615.801451
		//	632.122445
		//	652.297216
		//	676.499259
		//	704.902071
		//	737.679146
		//	775.003981
		//	817.050071
		//	863.990912

		//	916.000000		// 120
		//	973.089868
		//	1034.629202
		//	1099.825725
		//	1167.887158
		//	1238.021226
		//	1309.435652
		//	1381.338158
		//	1452.936467
		//	1523.438303
		//	1592.051387
		//	1657.983444
		//	1720.442197
		//	1778.635367
		//	1831.770679
		//	1879.055855
		//	1919.698618
		//	1952.906691
		//	1977.887797
		//	1993.849659

		DOUBLES_EQUAL(yy[140], 2000.000000, 1E-6);		// 140
		DOUBLES_EQUAL(yy[141], 1995.802608, 1E-6);
		DOUBLES_EQUAL(yy[142], 1981.745531, 1E-6);
		DOUBLES_EQUAL(yy[143], 1958.572880, 1E-6);
		DOUBLES_EQUAL(yy[144], 1927.028768, 1E-6);
		DOUBLES_EQUAL(yy[145], 1887.857308, 1E-6);
		DOUBLES_EQUAL(yy[146], 1841.802612, 1E-6);
		DOUBLES_EQUAL(yy[147], 1789.608791, 1E-6);
		DOUBLES_EQUAL(yy[148], 1732.019960, 1E-6);
		DOUBLES_EQUAL(yy[149], 1669.780230, 1E-6);
		DOUBLES_EQUAL(yy[150], 1603.633713, 1E-6);
		DOUBLES_EQUAL(yy[151], 1534.324521, 1E-6);
		DOUBLES_EQUAL(yy[152], 1462.596768, 1E-6);
		DOUBLES_EQUAL(yy[153], 1389.194565, 1E-6);
		DOUBLES_EQUAL(yy[154], 1314.862026, 1E-6);
		DOUBLES_EQUAL(yy[155], 1240.343261, 1E-6);
		DOUBLES_EQUAL(yy[156], 1166.382384, 1E-6);
		DOUBLES_EQUAL(yy[157], 1093.723507, 1E-6);
		DOUBLES_EQUAL(yy[158], 1023.110743, 1E-6);
		DOUBLES_EQUAL(yy[159],  955.288203, 1E-6);

		DOUBLES_EQUAL(yy[160], 891.000000, 1E-6);		// 160
		DOUBLES_EQUAL(yy[161], 830.830574, 1E-6);
		DOUBLES_EQUAL(yy[162], 774.725676, 1E-6);
		DOUBLES_EQUAL(yy[163], 722.471382, 1E-6);
		DOUBLES_EQUAL(yy[164], 673.853770, 1E-6);
		DOUBLES_EQUAL(yy[165], 628.658917, 1E-6);
		DOUBLES_EQUAL(yy[166], 586.672902, 1E-6);
		DOUBLES_EQUAL(yy[167], 547.681801, 1E-6);
		DOUBLES_EQUAL(yy[168], 511.471693, 1E-6);
		DOUBLES_EQUAL(yy[169], 477.828654, 1E-6);
		DOUBLES_EQUAL(yy[170], 446.538762, 1E-6);
		DOUBLES_EQUAL(yy[171], 417.388096, 1E-6);
		DOUBLES_EQUAL(yy[172], 390.162731, 1E-6);
		DOUBLES_EQUAL(yy[173], 364.648746, 1E-6);
		DOUBLES_EQUAL(yy[174], 340.632219, 1E-6);
		DOUBLES_EQUAL(yy[175], 317.899227, 1E-6);
		DOUBLES_EQUAL(yy[176], 296.235846, 1E-6);
		DOUBLES_EQUAL(yy[177], 275.428156, 1E-6);
		DOUBLES_EQUAL(yy[178], 255.262233, 1E-6);
		DOUBLES_EQUAL(yy[179], 235.524155, 1E-6);
	}

	{
	}
}

//----------------------------------------------------------------------------
TEST(Calc, Mat3)
{
	// Test 3x3 matrix.
	//

	{
		// verify defaults to identity

		MAT3 mat;

		DOUBLES_EQUAL(1.0, mat(0, 0), 0.0);	DOUBLES_EQUAL(0.0, mat(0, 1), 0.0);	DOUBLES_EQUAL(0.0, mat(0, 2), 0.0);
		DOUBLES_EQUAL(0.0, mat(1, 0), 0.0);	DOUBLES_EQUAL(1.0, mat(1, 1), 0.0);	DOUBLES_EQUAL(0.0, mat(1, 2), 0.0);
		DOUBLES_EQUAL(0.0, mat(2, 0), 0.0);	DOUBLES_EQUAL(0.0, mat(2, 1), 0.0);	DOUBLES_EQUAL(1.0, mat(2, 2), 0.0);

		CHECK(mat.IsIdentity() == true);
	}

	{
		MAT3 m1(
			10, 20, 10,
			4, 5, 6,
			2, 3, 5);
		MAT3 m2(
			3, 2, 4,
			3, 3, 9,
			4, 4, 2);

		MAT3 m = m1 * m2;
		DOUBLES_EQUAL(130.0, m(0, 0), 0.0);	DOUBLES_EQUAL(120.0, m(0, 1), 0.0);	DOUBLES_EQUAL(240.0, m(0, 2), 0.0);
		DOUBLES_EQUAL( 51.0, m(1, 0), 0.0);	DOUBLES_EQUAL( 47.0, m(1, 1), 0.0);	DOUBLES_EQUAL( 73.0, m(1, 2), 0.0);
		DOUBLES_EQUAL( 35.0, m(2, 0), 0.0);	DOUBLES_EQUAL( 33.0, m(2, 1), 0.0);	DOUBLES_EQUAL( 45.0, m(2, 2), 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(Calc, Mat4)
{
	// Test 4x4 matrix.
	//

	{
		// verify defaults to identity

		MAT4 mat;

		DOUBLES_EQUAL(1.0, mat(0, 0), 0.0);	DOUBLES_EQUAL(0.0, mat(0, 1), 0.0);	DOUBLES_EQUAL(0.0, mat(0, 2), 0.0);	DOUBLES_EQUAL(0.0, mat(0, 3), 0.0);
		DOUBLES_EQUAL(0.0, mat(1, 0), 0.0);	DOUBLES_EQUAL(1.0, mat(1, 1), 0.0);	DOUBLES_EQUAL(0.0, mat(1, 2), 0.0);	DOUBLES_EQUAL(0.0, mat(1, 3), 0.0);
		DOUBLES_EQUAL(0.0, mat(2, 0), 0.0);	DOUBLES_EQUAL(0.0, mat(2, 1), 0.0);	DOUBLES_EQUAL(1.0, mat(2, 2), 0.0);	DOUBLES_EQUAL(0.0, mat(2, 3), 0.0);
		DOUBLES_EQUAL(0.0, mat(3, 0), 0.0);	DOUBLES_EQUAL(0.0, mat(3, 1), 0.0);	DOUBLES_EQUAL(0.0, mat(3, 2), 0.0);	DOUBLES_EQUAL(1.0, mat(3, 3), 0.0);

		CHECK(mat.IsIdentity() == true);
	}

	{
		// multiplication - ident

		MAT4 m1;
		MAT4 m2;

		MAT4 m = m1 * m2;

		DOUBLES_EQUAL(1.0, m(0, 0), 0.0); DOUBLES_EQUAL(0.0, m(0, 1), 0.0); DOUBLES_EQUAL(0.0, m(0, 2), 0.0); DOUBLES_EQUAL(0.0, m(0, 3), 0.0);
		DOUBLES_EQUAL(0.0, m(1, 0), 0.0); DOUBLES_EQUAL(1.0, m(1, 1), 0.0);	DOUBLES_EQUAL(0.0, m(1, 2), 0.0); DOUBLES_EQUAL(0.0, m(1, 3), 0.0);
		DOUBLES_EQUAL(0.0, m(2, 0), 0.0); DOUBLES_EQUAL(0.0, m(2, 1), 0.0); DOUBLES_EQUAL(1.0, m(2, 2), 0.0); DOUBLES_EQUAL(0.0, m(2, 3), 0.0);
		DOUBLES_EQUAL(0.0, m(3, 0), 0.0); DOUBLES_EQUAL(0.0, m(3, 1), 0.0); DOUBLES_EQUAL(0.0, m(3, 2), 0.0); DOUBLES_EQUAL(1.0, m(3, 3), 0.0);

		CHECK(m.IsIdentity() == true);
	}

	{
		// multiplication

		MAT4 m1(
			5, 7, 9, 10,
			2, 3, 3, 8,
			8, 10, 2, 3,
			3, 3, 4, 8);
		MAT4 m2(
			3, 10, 12, 18,
			12, 1, 4, 9,
			9, 10, 12, 2,
			3, 12, 4, 10);

		MAT4 m = m1 * m2;

		DOUBLES_EQUAL(210.0, m(0, 0), 0.0); DOUBLES_EQUAL(267.0, m(0, 1), 0.0); DOUBLES_EQUAL(236.0, m(0, 2), 0.0); DOUBLES_EQUAL(271.0, m(0, 3), 0.0);
		DOUBLES_EQUAL(93.0, m(1, 0), 0.0); DOUBLES_EQUAL(149.0, m(1, 1), 0.0);	DOUBLES_EQUAL(104.0, m(1, 2), 0.0); DOUBLES_EQUAL(149.0, m(1, 3), 0.0);
		DOUBLES_EQUAL(171.0, m(2, 0), 0.0); DOUBLES_EQUAL(146.0, m(2, 1), 0.0); DOUBLES_EQUAL(172.0, m(2, 2), 0.0); DOUBLES_EQUAL(268.0, m(2, 3), 0.0);
		DOUBLES_EQUAL(105.0, m(3, 0), 0.0); DOUBLES_EQUAL(169.0, m(3, 1), 0.0); DOUBLES_EQUAL(128.0, m(3, 2), 0.0); DOUBLES_EQUAL(169.0, m(3, 3), 0.0);
	}

	{
		// addition

		MAT4 m1(
			1.0, 1.0, 1.0, 1.0,
			1.0, 1.0, 1.0, 1.0,
			1.0, 1.0, 1.0, 1.0,
			1.0, 1.0, 1.0, 1.0);
		MAT4 m2(
			 1.0,  2.0,  3.0,  4.0,
			 5.0,  6.0,  7.0,  8.0,
			 9.0, 10.0, 11.0, 12.0,
			13.0, 14.0, 15.0, 16.0);

		MAT4 m = m1 + m2;

		DOUBLES_EQUAL(2.0, m(0, 0), 0.0); DOUBLES_EQUAL(3.0, m(0, 1), 0.0); DOUBLES_EQUAL(4.0, m(0, 2), 0.0); DOUBLES_EQUAL(5.0, m(0, 3), 0.0);
		DOUBLES_EQUAL(6.0, m(1, 0), 0.0); DOUBLES_EQUAL(7.0, m(1, 1), 0.0); DOUBLES_EQUAL(8.0, m(1, 2), 0.0); DOUBLES_EQUAL(9.0, m(1, 3), 0.0);
		DOUBLES_EQUAL(10.0, m(2, 0), 0.0); DOUBLES_EQUAL(11.0, m(2, 1), 0.0); DOUBLES_EQUAL(12.0, m(2, 2), 0.0); DOUBLES_EQUAL(13.0, m(2, 3), 0.0);
		DOUBLES_EQUAL(14.0, m(3, 0), 0.0); DOUBLES_EQUAL(15.0, m(3, 1), 0.0); DOUBLES_EQUAL(16.0, m(3, 2), 0.0); DOUBLES_EQUAL(17.0, m(3, 3), 0.0);
	}

	{
		// subtraction

		MAT4 m1(
			1.0, 1.0, 1.0, 1.0,
			1.0, 1.0, 1.0, 1.0,
			1.0, 1.0, 1.0, 1.0,
			1.0, 1.0, 1.0, 1.0);
		MAT4 m2(
			 1.0,  2.0,  3.0,  4.0,
			 5.0,  6.0,  7.0,  8.0,
			 9.0, 10.0, 11.0, 12.0,
			13.0, 14.0, 15.0, 16.0);

		MAT4 m = m2 - m1;

		DOUBLES_EQUAL(0.0, m(0, 0), 0.0); DOUBLES_EQUAL(1.0, m(0, 1), 0.0); DOUBLES_EQUAL(2.0, m(0, 2), 0.0); DOUBLES_EQUAL(3.0, m(0, 3), 0.0);
		DOUBLES_EQUAL(4.0, m(1, 0), 0.0); DOUBLES_EQUAL(5.0, m(1, 1), 0.0); DOUBLES_EQUAL(6.0, m(1, 2), 0.0); DOUBLES_EQUAL(7.0, m(1, 3), 0.0);
		DOUBLES_EQUAL(8.0, m(2, 0), 0.0); DOUBLES_EQUAL(9.0, m(2, 1), 0.0); DOUBLES_EQUAL(10.0, m(2, 2), 0.0); DOUBLES_EQUAL(11.0, m(2, 3), 0.0);
		DOUBLES_EQUAL(12.0, m(3, 0), 0.0); DOUBLES_EQUAL(13.0, m(3, 1), 0.0); DOUBLES_EQUAL(14.0, m(3, 2), 0.0); DOUBLES_EQUAL(15.0, m(3, 3), 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(Calc, ModLTE)
{
	double result = ModLTE(5.2, 1.0);
	CHECK(result == 5.0);

	result = ModLTE(-5.2, 1.0);
	CHECK(result == -6.0);

	result = ModLTE(3.2, 1.0);
	CHECK(result == 3.0);

	result = ModLTE(5.7, 0.5);
	CHECK(result == 5.5);

	result = ModLTE(-5.2, 0.5);
	CHECK(result == -5.5);

	result = ModLTE(-5.7, 0.5);
	CHECK(result == -6.0);

	result = ModLTE(5.2, 1.0);
	CHECK(result == 5.0);

	result = ModLTE(-5.2, 1.0);
	CHECK(result == -6.0);

	result = ModLTE(0.0, 1.0);
	CHECK(result == 0.0);
}

//----------------------------------------------------------------------------
TEST(Calc, ModGTE)
{
	double result = ModGTE(5.2, 0.5);
	CHECK(result == 5.5);

	result = ModGTE(5.7, 0.5);
	CHECK(result == 6.0);

	result = ModGTE(-5.2, 0.5);
	CHECK(result == -5.0);

	result = ModGTE(-5.7, 0.5);
	CHECK(result == -5.5);

	result = ModGTE(5.2, 1.0);
	CHECK(result == 6.0);

	result = ModGTE(-5.2, 1.0);
	CHECK(result == -5.0);

	result = ModGTE(0.0, 1.0);
	CHECK(result == 0.0);
}


//----------------------------------------------------------------------------
TEST(Calc, UpHeadingToPitchRoll)
{
	VEC3 vUp(0, 1, 0);
	double heading = 0.0;
	double pitch = 0.0;
	double roll = 0.0;
	UpHeadingToPitchRoll(vUp, heading, pitch, roll);

	CHECK(fabs(pitch) < 0.0001);
	CHECK(fabs(roll) < 0.0001);

	{
		// pitched down in y/z plane

		VEC3 vUp(0, 1, 0);
		double x1, y1;
		Rotate2d(DegreeToRadian(45.0), vUp.z, vUp.y, &x1, &y1);     // rotate forward 45 degrees
		vUp.z = x1;
		vUp.y = y1;
		double heading = 0.0;
		double pitch = 0.0;
		double roll = 0.0;
		UpHeadingToPitchRoll(vUp, heading, pitch, roll);
		double pitchDegrees = RadianToDegree(pitch);

		CHECK(fabs(pitchDegrees + 45.0) < 0.0001);      // verify pitched down 45 degrees
		CHECK(fabs(roll) < 0.0001);
	}
	{
		// pitched down in y/z plane & rotated out of y/z plane - pitch s/b same

		VEC3 vUp(0, 1, 0);
		double x1, y1;
		Rotate2d(DegreeToRadian(45.0), vUp.z, vUp.y, &x1, &y1);     // rotate forward 45 degrees
		vUp.z = x1;
		vUp.y = y1;

		double headingDegrees = 30.0;
		Rotate2d(DegreeToRadian(headingDegrees), vUp.x, vUp.z, &x1, &y1);     // rotate to right 30 degrees - should not affect pitch
		vUp.x = x1;
		vUp.z = y1;
		double pitch = 0.0;
		double roll = 0.0;
		UpHeadingToPitchRoll(vUp, DegreeToRadian(headingDegrees), pitch, roll);
		double pitchDegrees = RadianToDegree(pitch);

		CHECK(fabs(pitchDegrees + 45.0) < 0.0001);      // verify pitched down 45 degrees
		CHECK(fabs(roll) < 0.0001);
	}
}

//----------------------------------------------------------------------------
TEST(Calc, DotProduct3)
{
	// Test 3D dot product.

	{
		// normalized inputs - orthogonal

		VEC3 v1(0, 1, 0);
		VEC3 v2(0, 0, 1);

		double dot = DotProduct(v1, v2);
		CHECK(dot == 0.0);
	}
	{
		// non-normalized inputs - orthogonal
		VEC3 v1(0, 2, 0);
		VEC3 v2(0, 0, 2);

		double dot = DotProduct(v1, v2);
		CHECK(dot == 0.0);
	}
	{
		// unequal inputs - orthogonal
		VEC3 v1(0, 2, 0);
		VEC3 v2(0, 0, 7.5);

		double dot = DotProduct(v1, v2);
		CHECK(dot == 0.0);
	}
	{
		// nonzero inputs
		VEC3 v1(1, 2, 3);
		VEC3 v2(4, 5, 6);

		double dot = DotProduct(v1, v2);
		CHECK(dot == 32.0);
	}
}

//----------------------------------------------------------------------------
TEST(Calc, DotProduct2)
{
	// Test 2D dot product.

	{
		// normalized inputs - orthogonal

		VEC2 v1(-6, 8);
		VEC2 v2(5, 12);

		double dot = DotProduct(v1, v2);
		DOUBLES_EQUAL(66.0, dot, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(Calc, CrossProduct)
{
	// Test 3D cross product.

	{
		// normalized inputs

		VEC3 v1(0, 1, 0);
		VEC3 v2(0, 0, 1);

		VEC3 v3 = CrossProduct(v1, v2);
		CHECK(v3.x == 1.0);
		CHECK(v3.y == 0.0);
		CHECK(v3.z == 0.0);

		v3 = CrossProduct(v2, v1);
		CHECK(v3.x == -1.0);
		CHECK(v3.y == 0.0);
		CHECK(v3.z == 0.0);
	}

	{
		VEC3 v1(0, 2, 0);
		VEC3 v2(0, 0, 2);

		VEC3 v3 = CrossProduct(v1, v2);
		CHECK(v3.x == 4.0);
		CHECK(v3.y == 0.0);
		CHECK(v3.z == 0.0);

		v3 = CrossProduct(v2, v1);
		CHECK(v3.x == -4.0);
		CHECK(v3.y == 0.0);
		CHECK(v3.z == 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(Calc, Vector2)
{
	// 2d vector operators
	//

	// default constructor
	{
		VEC2 vecDefault;
		DOUBLES_EQUAL(vecDefault.x, 0.0, 0.0);
		DOUBLES_EQUAL(vecDefault.y, 0.0, 0.0);
	}

	VEC2 v1(0, 1);
	VEC2 v2(0, 1);

	// equality / inequality

	CHECK(v1 == v2);
	CHECK(v1 == VEC2(0, 1));
	CHECK(v1 != VEC2(1, 1));

	// binary arithmetic operators

	CHECK(v1 + v2 == VEC2(0, 2));
	CHECK(v1 - v2 == VEC2(0, 0));
	CHECK(v1 * 5.0 == v2 * 5.0);
	CHECK(v1 / 5.0 == v2 / 5.0);

	// +=

	v1 = VEC2(1.0, 2.0);
	v2 = VEC2(-1.0, -2.0);
	v1 += v2;
	CHECK(v1 == VEC2(0, 0));
	CHECK(v1 == VEC2(0.0, 0.0));

	// -=

	v1 = VEC2(10.0, 10.0);
	v2 = VEC2(1.5, 5.0);
	v1 -= v2;
	CHECK(v1.x == 8.5);
	CHECK(v1.y == 5.0);

	// *=

	v1 = VEC2(1.0, 1.0);
	v1 *= 15.0;
	CHECK(v1.x == 15.0);
	CHECK(v1.y == 15.0);

	// /= 

	v1 = VEC2(15.0, 15.0);
	v1 /= 15.0;
	CHECK(v1.x == 1.0);
	CHECK(v1.y == 1.0);

	// unary operators
	v2 = VEC2(1, -1);
	v1 = +v2;								// VEC2 operator + () const;
	CHECK(v1 == VEC2(1, -1));
	v1 = -v2;								// VEC2 operator - () const;
	CHECK(v1 == VEC2(-1, 1));

	// magnitude
	{
		VEC2 testVec(10.0, 15.0);
		double mag = testVec.Magnitude();
		CHECK(mag == 18.027756377319946);
	}
	{
		VEC2 testVec(-10.0, 15.0);
		double mag = testVec.Magnitude();
		CHECK(mag == 18.027756377319946);
	}
	{
		VEC2 testVec(0.0, 0.0);
		double mag = testVec.Magnitude();
		CHECK(mag == 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(Calc, Vector3)
{
	// 3d vector operators
	//

	// default constructor
	{
		VEC3 vecDefault;
		DOUBLES_EQUAL(vecDefault.x, 0.0, 0.0);
		DOUBLES_EQUAL(vecDefault.y, 0.0, 0.0);
		DOUBLES_EQUAL(vecDefault.z, 0.0, 0.0);
	}

	{
		// 3x3 matrix x 3d vector

		VEC3 vin(2, 1, 3);
		MAT3 matin(
			1, 2, 3,
			4, 5, 6,
			7, 8, 9);

		VEC3 v = Vec3Transform(vin, matin);

		// verified this against multiply examples & calculators
		DOUBLES_EQUAL(13.0, v.x, 0.0);
		DOUBLES_EQUAL(31.0, v.y, 0.0);
		DOUBLES_EQUAL(49.0, v.z, 0.0);
	}

	{
		VEC3 v1(0, 1, 0);
		VEC3 v2(0, 1, 0);

		// equality / inequality

		CHECK(v1 == v2);
		CHECK(v1 == VEC3(0, 1, 0));
		CHECK(v1 != VEC3(1, 1, 1));

		// binary arithmetic operators

		CHECK(v1 + v2 == VEC3(0, 2, 0));
		CHECK(v1 - v2 == VEC3(0, 0, 0));
		CHECK(v1 * 5.0 == v2 * 5.0);
		CHECK(v1 / 5.0 == v2 / 5.0);

		// +=

		v1 = VEC3(1.0, 2.0, 3.0);
		v2 = VEC3(-1.0, -2.0, -3.0);
		v1 += v2;
		CHECK(v1 == VEC3(0, 0, 0));
		CHECK(v1 == VEC3(0.0, 0.0, 0.0));

		// -=

		v1 = VEC3(10.0, 10.0, 10.0);
		v2 = VEC3(1.5, 5.0, 8.0);
		v1 -= v2;
		CHECK(v1.x == 8.5);
		CHECK(v1.y == 5.0);
		CHECK(v1.z == 2.0);

		// *=

		v1 = VEC3(1.0, 1.0, 1.0);
		v1 *= 15.0;
		CHECK(v1.x == 15.0);
		CHECK(v1.y == 15.0);
		CHECK(v1.z == 15.0);

		// /= 

		v1 = VEC3(15.0, 15.0, 15.0);
		v1 /= 15.0;
		CHECK(v1.x == 1.0);
		CHECK(v1.y == 1.0);
		CHECK(v1.z == 1.0);

		// unary operators
		v2 = VEC3(1, -1, 0);
		v1 = +v2;								// VEC3 operator + () const;
		CHECK(v1 == VEC3(1, -1, 0));
		v1 = -v2;								// VEC3 operator - () const;
		CHECK(v1 == VEC3(-1, 1, 0));
	}

	// magnitude
	{
		VEC3 testVec(10.0, 15.0, 20.0);
		double mag = testVec.Magnitude();
		CHECK(mag == 26.925824035672520);
	}
	{
		VEC3 testVec(10.0, -15.0, 20.0);
		double mag = testVec.Magnitude();
		CHECK(mag == 26.925824035672520);
	}
	{
		VEC3 testVec(0.0, 0.0, 0.0);
		double mag = testVec.Magnitude();
		CHECK(mag == 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(Calc, Vector4)
{
	// 4d vector operators
	//

	// default constructor
	{
		VEC4 vecDefault;
		DOUBLES_EQUAL(vecDefault.x, 0.0, 0.0);
		DOUBLES_EQUAL(vecDefault.y, 0.0, 0.0);
		DOUBLES_EQUAL(vecDefault.z, 0.0, 0.0);
		DOUBLES_EQUAL(vecDefault.w, 0.0, 0.0);
	}

	VEC4 v1(0, 1, 0, 0);
	VEC4 v2(0, 1, 0, 0);

	// equality / inequality

	CHECK(v1 == v2);
	CHECK(v1 == VEC4(0, 1, 0, 0));
	CHECK(v1 != VEC4(1, 1, 1, 1));

	// binary arithmetic operators

	CHECK(v1 + v2 == VEC4(0, 2, 0, 0));
	CHECK(v1 - v2 == VEC4(0, 0, 0, 0));
	CHECK(v1 * 5.0 == v2 * 5.0);
	CHECK(v1 / 5.0 == v2 / 5.0);

	// +=

	v1 = VEC4(1.0, 2.0, 3.0, 4.0);
	v2 = VEC4(-1.0, -2.0, -3.0, -4.0);
	v1 += v2;
	CHECK(v1 == VEC4(0, 0, 0, 0));
	CHECK(v1 == VEC4(0.0, 0.0, 0.0, 0.0));

	// -=

	v1 = VEC4(10.0, 10.0, 10.0, 10.0);
	v2 = VEC4(1.5, 5.0, 8.0, 10.0);
	v1 -= v2;
	CHECK(v1.x == 8.5);
	CHECK(v1.y == 5.0);
	CHECK(v1.z == 2.0);
	CHECK(v1.w == 0.0);

	// *=

	v1 = VEC4(1.0, 1.0, 1.0, 1.0);
	v1 *= 15.0;
	CHECK(v1.x == 15.0);
	CHECK(v1.y == 15.0);
	CHECK(v1.z == 15.0);
	CHECK(v1.w == 15.0);

	// /= 

	v1 = VEC4(15.0, 15.0, 15.0, 15.0);
	v1 /= 15.0;
	CHECK(v1.x == 1.0);
	CHECK(v1.y == 1.0);
	CHECK(v1.z == 1.0);
	CHECK(v1.w == 1.0);

	// unary operators
	v2 = VEC4(1, -1, 0, 3.5);
	v1 = +v2;								// VEC4 operator + () const;
	CHECK(v1 == VEC4(1, -1, 0, 3.5));
	v1 = -v2;								// VEC4 operator - () const;
	CHECK(v1 == VEC4(-1, 1, 0, -3.5));
}

//----------------------------------------------------------------------------
TEST(Calc, PointInPoly)
{
	{
		// basic PointInPoly() test

		std::vector<VEC2> vxList;
		vxList.push_back(VEC2(0.0, 0.0));
		vxList.push_back(VEC2(0.0, 10.0));
		vxList.push_back(VEC2(10.0, 10.0));
		vxList.push_back(VEC2(10.0, 0.0));

		bool inside = PointInPoly(5.0, 5.0, vxList);
		CHECK(inside == true);

		inside = PointInPoly(0.001, 0.001, vxList);
		CHECK(inside == true);

		inside = PointInPoly(-3.0, 5.0, vxList);
		CHECK(inside == false);
	}
}

//----------------------------------------------------------------------------
TEST(Calc, RectType)
{
	{
		// B inside A

		RectType rectA = RectType(1, 1, 5, 5);
		RectType rectB = RectType(2, 2, 4, 4);

		bool intersect = rectA.Intersects(rectB);
		CHECK(intersect == true);
	}

	{
		// A inside B

		RectType rectA = RectType(2, 2, 4, 4);
		RectType rectB = RectType(1, 1, 5, 5);

		bool intersect = rectA.Intersects(rectB);
		CHECK(intersect == true);
	}

	{
		// A partially inside B

		RectType rectA = RectType(2, 2, 8, 8);
		RectType rectB = RectType(1, 1, 5, 5);

		bool intersect = rectA.Intersects(rectB);
		CHECK(intersect == true);
	}

	{
		// B partially inside A

		RectType rectA = RectType(1, 1, 5, 5);
		RectType rectB = RectType(2, 2, 8, 8);

		bool intersect = rectA.Intersects(rectB);
		CHECK(intersect == true);
	}

	{
		// A completely above B

		RectType rectA = RectType(9, 1, 15, 5);
		RectType rectB = RectType(8, 8, 10, 10);

		bool intersect = rectA.Intersects(rectB);
		CHECK(intersect == false);
	}

	{
		// A completely below B

		RectType rectA = RectType(9, 11, 15, 15);
		RectType rectB = RectType(8, 8, 10, 10);

		bool intersect = rectA.Intersects(rectB);
		CHECK(intersect == false);
	}

	{
		// A completely to right of B

		RectType rectA = RectType(11, 8, 16, 10);
		RectType rectB = RectType(8, 8, 10, 10);

		bool intersect = rectA.Intersects(rectB);
		CHECK(intersect == false);
	}

	{
		// A completely to left of B

		RectType rectA = RectType(5, 8, 7, 10);
		RectType rectB = RectType(8, 8, 10, 10);

		bool intersect = rectA.Intersects(rectB);
		CHECK(intersect == false);
	}

	{
		// A == B

		RectType rectA = RectType(5, 8, 7, 10);
		RectType rectB = rectA;

		bool intersect = rectA.Intersects(rectB);
		CHECK(intersect == true);
	}
}

//----------------------------------------------------------------------------
TEST(Calc, IntersectRectRect)
{
	{
		// B inside A

		RectD rectA = RectD(1.0, 1.0, 5.0, 5.0);
		RectD rectB = RectD(2.0, 2.0, 4.0, 4.0);

		bool intersect = IntersectRectRect(rectA, rectB);
		CHECK(intersect == true);
	}

	{
		// A inside B

		RectD rectA = RectD(2.0, 2.0, 4.0, 4.0);
		RectD rectB = RectD(1.0, 1.0, 5.0, 5.0);

		bool intersect = IntersectRectRect(rectA, rectB);
		CHECK(intersect == true);
	}

	{
		// A partially inside B

		RectD rectA = RectD(2.0, 2.0, 8.0, 8.0);
		RectD rectB = RectD(1.0, 1.0, 5.0, 5.0);

		bool intersect = IntersectRectRect(rectA, rectB);
		CHECK(intersect == true);
	}

	{
		// B partially inside A

		RectD rectA = RectD(1.0, 1.0, 5.0, 5.0);
		RectD rectB = RectD(2.0, 2.0, 8.0, 8.0);

		bool intersect = IntersectRectRect(rectA, rectB);
		CHECK(intersect == true);
	}

	{
		// A completely above B

		RectD rectA = RectD(9.0, 1.0, 15.0, 5.0);
		RectD rectB = RectD(8.0, 8.0, 10.0, 10.0);

		bool intersect = IntersectRectRect(rectA, rectB);
		CHECK(intersect == false);
	}

	{
		// A completely below B

		RectD rectA = RectD(9.0, 11.0, 15.0, 15.0);
		RectD rectB = RectD(8.0, 8.0, 10.0, 10.0);

		bool intersect = IntersectRectRect(rectA, rectB);
		CHECK(intersect == false);
	}

	{
		// A completely to right of B

		RectD rectA = RectD(11.0, 8.0, 16.0, 10.0);
		RectD rectB = RectD(8.0, 8.0, 10.0, 10.0);

		bool intersect = IntersectRectRect(rectA, rectB);
		CHECK(intersect == false);
	}

	{
		// A completely to left of B

		RectD rectA = RectD(5.0, 8.0, 7.0, 10.0);
		RectD rectB = RectD(8.0, 8.0, 10.0, 10.0);

		bool intersect = IntersectRectRect(rectA, rectB);
		CHECK(intersect == false);
	}

	{
		// A == B

		RectD rectA = RectD(5.0, 8.0, 7.0, 10.0);
		RectD rectB = rectA;

		bool intersect = IntersectRectRect(rectA, rectB);
		CHECK(intersect == true);
	}
}

//----------------------------------------------------------------------------
TEST(Calc, IntersectPolyPoly2D)
{
	{
		// Polygons intersect, but no vertex from either
		// lies inside the other.

		std::vector<VEC2> vxList1;
		vxList1.push_back(VEC2(125, 33));
		vxList1.push_back(VEC2(300, 218));
		vxList1.push_back(VEC2(125, 218));

		std::vector<VEC2> vxList2;
		vxList2.push_back(VEC2(90, 74));
		vxList2.push_back(VEC2(312, 74));
		vxList2.push_back(VEC2(312, 162));
		vxList2.push_back(VEC2(90, 162));

		bool intersect = IntersectPolyPoly2D(vxList1, vxList2);
		CHECK(intersect == true);
	}

	{
		// Polygons intersect.

		std::vector<VEC2> vxList1;
		vxList1.push_back(VEC2(26, 32));
		vxList1.push_back(VEC2(162, 32));
		vxList1.push_back(VEC2(162, 125));
		vxList1.push_back(VEC2(26, 125));

		std::vector<VEC2> vxList2;
		vxList2.push_back(VEC2(90, 74));
		vxList2.push_back(VEC2(312, 74));
		vxList2.push_back(VEC2(312, 162));
		vxList2.push_back(VEC2(90, 162));

		bool intersect = IntersectPolyPoly2D(vxList1, vxList2);
		CHECK(intersect == true);
	}

	{
		// Polygons do not intersect.

		std::vector<VEC2> vxList1;
		vxList1.push_back(VEC2(26, 32));
		vxList1.push_back(VEC2(162, 32));
		vxList1.push_back(VEC2(162, 125));
		vxList1.push_back(VEC2(26, 125));

		std::vector<VEC2> vxList2;
		vxList2.push_back(VEC2(190, 74));
		vxList2.push_back(VEC2(312, 74));
		vxList2.push_back(VEC2(312, 162));
		vxList2.push_back(VEC2(190, 162));

		bool intersect = IntersectPolyPoly2D(vxList1, vxList2);
		CHECK(intersect == false);
	}

	{
		// Polygon 1 completely contained by polygon 2.

		std::vector<VEC2> vxList1;
		vxList1.push_back(VEC2(1.0, 2.0));
		vxList1.push_back(VEC2(1.0, 8.0));
		vxList1.push_back(VEC2(5.0, 8.0));
		vxList1.push_back(VEC2(5.0, 2.0));

		std::vector<VEC2> vxList2;
		vxList2.push_back(VEC2(0.0, 0.0));
		vxList2.push_back(VEC2(0.0, 10.0));
		vxList2.push_back(VEC2(10.0, 10.0));
		vxList2.push_back(VEC2(10.0, 0.0));

		bool intersect = IntersectPolyPoly2D(vxList1, vxList2);
		CHECK(intersect == true);
	}

	{
		// Polygon 2 completely contained by polygon 1.

		std::vector<VEC2> vxList2;
		vxList2.push_back(VEC2(1.0, 2.0));
		vxList2.push_back(VEC2(1.0, 8.0));
		vxList2.push_back(VEC2(5.0, 8.0));
		vxList2.push_back(VEC2(5.0, 2.0));

		std::vector<VEC2> vxList1;
		vxList1.push_back(VEC2(0.0, 0.0));
		vxList1.push_back(VEC2(0.0, 10.0));
		vxList1.push_back(VEC2(10.0, 10.0));
		vxList1.push_back(VEC2(10.0, 0.0));

		bool intersect = IntersectPolyPoly2D(vxList1, vxList2);
		CHECK(intersect == true);
	}
}

//----------------------------------------------------------------------------
TEST(Calc, LeastSquare)
{
	{
		std::vector<PointD> ptList;
		ptList.push_back(PointD(1.0, 10.0));
		ptList.push_back(PointD(2.0, 8.0));
		ptList.push_back(PointD(3.0, 12.0));
		ptList.push_back(PointD(4.0, 10.0));
		ptList.push_back(PointD(5.0, 7.0));
		ptList.push_back(PointD(6.0, 9.5));
		ptList.push_back(PointD(7.0, 11.2));
		ptList.push_back(PointD(8.0, 10.0));

		double b = 0.0;
		double m = 0.0;
		double ls = LeastSquares(ptList, b, m, false);

		DOUBLES_EQUAL(ls, 0.0099873548531938198, 1E-12);
		DOUBLES_EQUAL(b, 9.4178571428571480, 1E-12);
		DOUBLES_EQUAL(m, 0.065476190476189119, 1E-12);
	}
}
