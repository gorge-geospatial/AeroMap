#ifndef ORTHOPHOTO_H
#define ORTHOPHOTO_H

#include "Calc.h"
#include "XString.h"

class Orthophoto
{
public:

	static void generate_png(XString orthophoto_file, XString output_file = "", SizeType outsize = SizeType());
	static void generate_kmz(XString orthophoto_file, XString output_file = "", SizeType outsize = SizeType());
	static void build_overviews(XString orthophoto_file);
	static void post_orthophoto_steps(XString bounds_file_path, XString orthophoto_file, XString orthophoto_tiles_dir);
	static void get_orthophoto_vars(QStringList args);
};

#endif // #ifndef ORTHOPHOTO_H
