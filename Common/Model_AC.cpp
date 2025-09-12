/*
	Model_AC.cpp
	AC3D model manager

	THE AC3D FILE FORMAT (This document last updated 27th March 2003)

	The AC3D file format is ascii text and is very simple to parse.  It's also very easy to generate AC3D 
	files from your own data. This document describes the structure of AC3D files.  Some simple examples are included at the end.
	AC3D filenames usually have a '.ac' suffix. For more information about AC3D, see http://www.ac3d.org.

	an AC3D file has this overall structure:

		(HEADER)
		(Materials - the palette)
		(objects)


	An AC3D file always starts with a header line e.g.:

		AC3Db

	The first four characters are always 'AC3D' the next hex number following
	immediately defines the internal version of the file (0xb == 11).
	If the version is later than the program knows about then it will probably
	may refuse to load it.  As new versions of the AC3D file format evolve, this number
	is used to make the loading code backwards compatible.

	To parse an AC3D file you need something like this:

		read the header (AC3Dx)
		while not end of file
		{
			read a line
			check first token on line
			handle this tokens values (this might involve reading more lines)
		}


	This is the basic structure.  %f indicates a floating point value, %d an
	integer and %s a string (must be surrounded by quotes " if it contains spaces).
	Lines beginning with a '*' indicate an optional section.  The tokens are
	case-sensitive.  The indentation is used to show the structure of the file - a
	real AC3D file has no indentation.

		MATERIAL %s rgb %f %f %f  amb %f %f %f  emis %f %f %f  spec %f %f %f  shi %d  trans %f
		OBJECT %s
		*name %s
		*data %d
		*texture %s
		*texrep %f %f
		*rot %f %f %f  %f %f %f  %f %f %f
		*loc %f %f %f
		*url %s
		*numvert %d
		numvert lines of %f %f %f
		*numsurf %d
		*SURF %d
		*mat %d
		refs %d
		refs lines of %d %f %f
		kids %d
	
	MATERIAL (name) rgb %f %f %f  amb %f %f %f  emis %f %f %f  spec %f %f %f  shi %d  trans %f

		single line describing a material.  These are referenced by the "mat"
		token of a surface.  The first "MATERIAL" in the file will be indexed as
		zero.  The materials are usually all specified at the start of the file,
		imediately after the header.

	OBJECT %s

		Specifies the start of an object. The end of the object section must
		be a 'kids' line which indicates how many children objects (may be zero)
		follow.  The parameter is the object type - one of:  world, poly,
		group.

	*name %s

		Optional - a name for the object

	*data %d

		Optional - object data.  Usually the object-data string for an object.
		The parameter is an interger which specifies the number of characters
		(starting on the next line) to be read.

	*texture %s

		Optional - default is no texture. the path of the texture bitmap file for the texture of the current object.

	*texrep %f %f

		Optional - default 1.0,1.0 .  The texture repeat values for the tiling of a texture
		on an object's surfaces.

	*rot %f %f %f  %f %f %f  %f %f %f

		The 3x3 rotation matrix for this objects vertices.  Note that the rotation is relative
		to the object's parent i.e. it is not a global rotation matrix.  If this token
		is not specified then the default rotation matrix is 1 0 0, 0 1 0, 0 0 1

	*loc %f %f %f

		The translation of the object.  Effectively the definition of the centre of the object.  This is
		relative to the parent - i.e. not a global position.  If this is not found then
		the default centre of the object will be 0, 0, 0.

	*url %s

		The url of an object - default is blank.

	*numvert %d
		numvert lines of %f %f %f

		The number of vertices in an object.  Parameter specifies the number of
		lins that follow.  If this token is read then you MUST read
		that many lines of (%f %f %f) - specifying each vertex point as a local
		coordinate.  Some objects (e.g. groups) may not have a numvert token.

	*numsurf %d

		The number of surfaces that this object contains.  The parameter specifies
		the number of subsections that follow - each one being a different surface

	SURF %d

		The start of a surface.  The parameter specifies the surface type and flags.
		The first 4 bits (flags & 0xF) is the type (0 = polygon, 1 = closedline,
		2 = line).  The next four bits (flags >> 4) specify the shading and
		backface.  bit1 = shaded surface bit2 = twosided.

	*mat %d

		The index to the material that this surface has.

	refs %d

		refs lines of %d %f %f

		The number of vertices in the surface.  This number indicates the number of lines
		following.  Each line contains an index to the vertex and the texture coordinates
		for this surface vertex.

	kids %d

		This is the final token of an object section and it must exist.  If the parameter
		is a number > 0 then more objects are recursively loaded as children of the
		current object.

	Here is an example file - a simple rectangle (white)

		AC3Db
		MATERIAL "" rgb 1 1 1  amb 0.2 0.2 0.2  emis 0 0 0  spec 0.5 0.5 0.5  shi 10  trans 0
		OBJECT world
		kids 1
		OBJECT poly
		name "rect"
		loc 1 0.5 0
		numvert 4
		-1 0.5 0
		1 0.5 0
		1 -0.5 0
		-1 -0.5 0
		numsurf 1
		SURF 0x20
		mat 0
		refs 4
		3 0 0
		2 1 0
		1 1 1
		0 0 1
		kids 0

	Another example - an object with 7 vertices (no surfaces, no materials defined). This is a 
	good way of getting point data into AC3D.  The Vertex->create convex-surface/object can be 
	used on these vertices to 'wrap' a 3d shape around them.

		AC3Db
		OBJECT poly
		numvert 7
		-0.83 -0.235 -0.04
		-0.63 0.185 -0.04
		-0.55 0.235 -0.25
		-0.33 0.235 0.29
		0.09 0.235 -0.29
		0.33 -0.195 -0.04
		0.83 0.005 -0.04
		kids 0

	If you write a loader then you least you need is code to handle the object
	token, and the objects numvert/vertice and numsurf/sufaces - esentially
	the geometry of the model.

	You can ignore any line that starts with a token other than these e,g textures,
	rotation, location etc.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

const int OBJECT_WORLD = 999;
const int OBJECT_NORMAL = 0;
const int OBJECT_GROUP = 1;
const int OBJECT_LIGHT = 2;

const int SURFACE_SHADED = (1 << 4);
const int SURFACE_TWOSIDED = (1 << 5);

const int SURFACE_TYPE_POLYGON = 0;
const int SURFACE_TYPE_CLOSEDLINE = 1;
const int SURFACE_TYPE_LINE = 2;

// texture stuff
const int ALPHA_NONE = (0x0000);			// no alpha info
const int ALPHA_OPAQUE = (0x0001 << 0);		// alpha = 1
const int ALPHA_INVIS = (0x0001 << 1);		// alpha = 0
const int ALPHA_TRANSP = (0x0004 << 2);		// 0 < alpha < 1

#include "Model_AC.h"

Model_AC::Model_AC()
	: Model()
	, m_startMatIndex(0)
	, m_lineNumber(0)
	, m_num_texture(0)
	, m_pACObject(nullptr)
{
	memset(m_buff, 0, sizeof(m_buff));
}

Model_AC::~Model_AC()
{
}

bool Model_AC::Load(const char* fileName)
{
	if (fileName == nullptr)
		return false;

	mv_Mesh.clear();

	delete m_pACObject;
	m_pACObject = nullptr;

	FILE* pFile = fopen(fileName, "r");
	if (pFile == nullptr)
	{
		Logger::Write(__FUNCTION__, "Unable to open %s.\n", fileName);
		return nullptr;
	}
	else
	{
		ms_FileName = fileName;

		read_line(pFile);

		if (strncmp(m_buff, "AC3D", 4))
		{
			Logger::Write(__FUNCTION__, "'%s' is not a valid AC3D file.", fileName);
			fclose(pFile);
			return 0;
		}

		m_pACObject = load_object(pFile, nullptr);
		fclose(pFile);

		CalculateNormals();
		calc_vertex_normals(m_pACObject);
	}

	return m_pACObject != nullptr;
}

Model_AC::ACObject* Model_AC::load_object(FILE* pFile, ACObject* /* pParent */)
{
	// Recursive routine to load objects (meshes).
	//

	ACObject* ob = nullptr;		// return value

	while (!feof(pFile))
	{
		int tokc = 0;
		char* tokv[30] = { 0 };
		char t[20] = { 0 };

		read_line(pFile);

		sscanf(m_buff, "%s", t);

		if (streq(t, "MATERIAL"))
		{
			if (get_tokens(m_buff, &tokc, tokv) != 22)
			{
				Logger::Write(__FUNCTION__, "expected 21 params after \"MATERIAL\" - line %d", m_lineNumber);
			}
			else
			{
				Material mat;

				mat.SetName(tokv[1]);
				mat.SetDiffuse(vec3((float)atof(tokv[3]), (float)atof(tokv[4]), (float)atof(tokv[5])));
				mat.SetAmbient(vec3((float)atof(tokv[7]), (float)atof(tokv[8]), (float)atof(tokv[9])));
				//mat.SetEmissive(vec3((float)atof(tokv[11]), (float)atof(tokv[12]), (float)atof(tokv[13])));
				mat.SetSpecular(vec3((float)atof(tokv[15]), (float)atof(tokv[16]), (float)atof(tokv[17])));
				mat.SetShininess((float)atof(tokv[19]));
				mat.SetTransparency((float)atof(tokv[21]));

				mv_Mat.push_back(mat);
			}
		}
		else if (streq(t, "OBJECT"))
		{
			char type[20];
			char str[20];
			ob = new_object();

			sscanf(m_buff, "%s %s", str, type);

			ob->type = string_to_objecttype(type);
		}
		else if (streq(t, "data"))
		{
			if (get_tokens(m_buff, &tokc, tokv) != 2)
				Logger::Write(__FUNCTION__, "expected 'data <number>' at line %d", m_lineNumber);
			else
			{
				char *str;
				int len;

				len = atoi(tokv[1]);
				if (len > 0)
				{
					str = (char *)myalloc(len + 1);
					fread(str, len, 1, pFile);
					str[len] = 0;
					fscanf(pFile, "\n"); m_lineNumber++;
					ob->data = STRING(str);
					myfree(str);
				}
			}
		}
		else if (streq(t, "name"))
		{
			int numtok = get_tokens(m_buff, &tokc, tokv);
			if (numtok != 2)
			{
				Logger::Write(__FUNCTION__, "expected quoted name at line %d (got %d tokens)", m_lineNumber, numtok);
			}
			else
				ob->name = STRING(tokv[1]);
		}
		else if (streq(t, "texture"))
		{
			if (get_tokens(m_buff, &tokc, tokv) != 2)
			{
				Logger::Write(__FUNCTION__, "expected quoted texture name at line %d", m_lineNumber);
			}
			else
			{
//TODO:
// currently, only support 1 texture / model
//				ob->texture = load_texture(tokv[1]);
				if (m_Texture.IsValid() == false)
				{
					XString strTextureFile = GetTextureFileName(tokv[1]);
					if (!m_Texture.Load(strTextureFile.c_str()))
					{
						Logger::Write(__FUNCTION__, "Unable to load texture: '%s'", strTextureFile.c_str());
					}
				}
			}
		}
		else if (streq(t, "texrep"))
		{
			if (get_tokens(m_buff, &tokc, tokv) != 3)
			{
				Logger::Write(__FUNCTION__, "expected 'texrep <float> <float>' at line %d", m_lineNumber);
			}
			else
			{
				ob->texture_repeat_x = (float)atof(tokv[1]);
				ob->texture_repeat_y = (float)atof(tokv[2]);
			}
		}
		else if (streq(t, "texoff"))
		{
			if (get_tokens(m_buff, &tokc, tokv) != 3)
			{
				Logger::Write(__FUNCTION__, "expected 'texoff <float> <float>' at line %d", m_lineNumber);
			}
			else
			{
				ob->texture_offset_x = (float)atof(tokv[1]);
				ob->texture_offset_y = (float)atof(tokv[2]);
			}
		}
		else if (streq(t, "rot"))
		{
			float r[9];
			char str2[5];
			int n;

			sscanf(m_buff, "%s %f %f %f %f %f %f %f %f %f",
				str2, &r[0], &r[1], &r[2], &r[3], &r[4], &r[5], &r[6], &r[7], &r[8]);

			for (n = 0; n < 9; n++)
				ob->matrix[n] = r[n];
		}
		else if (streq(t, "loc"))
		{
			char str[5];
			sscanf(m_buff, "%s %f %f %f", str,
				&ob->loc.x, &ob->loc.y, &ob->loc.z);
		}
		else if (streq(t, "url"))
		{
			if (get_tokens(m_buff, &tokc, tokv) != 2)
				Logger::Write(__FUNCTION__, "expected one arg to url at line %d (got %s)", m_lineNumber, tokv[0]);
			else
				ob->url = STRING(tokv[1]);
		}
		else if (streq(t, "numvert"))
		{
			int num = 0;
			char str[10];

			sscanf(m_buff, "%s %d", str, &num);

			if (num > 0)
			{
				ob->num_vert = num;
				ob->vertices = (ACVertex *)myalloc(sizeof(ACVertex)*num);

				for (int n = 0; n < num; ++n)
				{
					ACVertex p;
					fscanf(pFile, "%f %f %f\n", &p.x, &p.y, &p.z); m_lineNumber++;
					ob->vertices[n] = p;
				}
			}
		}
		else if (streq(t, "numsurf"))
		{
			int num = 0;
			char str[10];

			sscanf(m_buff, "%s %d", str, &num);
			if (num > 0)
			{
				ob->num_surf = num;
				ob->surfaces = (ACSurface *)myalloc(sizeof(ACSurface) * num);

				for (int n = 0; n < num; ++n)
				{
					ACSurface* news = read_surface(pFile, &ob->surfaces[n], ob);
					if (news ==nullptr)
					{
						Logger::Write(__FUNCTION__, "error whilst reading surface at line: %d", m_lineNumber);
						return nullptr;
					}
				}
			}
		}
		else if (streq(t, "kids"))		// 'kids' is the last token in an object
		{
			int num = 0;

			sscanf(m_buff, "%s %d", t, &num);
			if (num != 0)
			{
				ob->kids = (ACObject **)myalloc(num * sizeof(ACObject *));
				ob->num_kids = num;

				for (int n = 0; n < num; ++n)
				{
					ACObject* k = load_object(pFile, ob);

					if (k == nullptr)
					{
						Logger::Write(__FUNCTION__, "error reading expected child object %d of %d at line: %d", n + 1, num, m_lineNumber);
						return ob;
					}
					else
						ob->kids[n] = k;
				}
			}

			// this appears to be the "normal" return path
			break;
		}
	}

	// load base class structures for editing / rendering

	// need at least 1 surface in order to create a mesh
	if (ob->num_surf > 0)
	{
		// Note: My meshes have to parent/child concept, so that is getting 
		// lost here. Perhaps should be added?
		MeshType mesh;
		strncpy(mesh.Name, ob->name, sizeof(mesh.Name));
		for (int vxIdx = 0; vxIdx < ob->num_vert; ++vxIdx)
		{
			VertexType vx;
			vx.P = VEC3(ob->vertices[vxIdx].x, ob->vertices[vxIdx].y, ob->vertices[vxIdx].z);
			mesh.vVx.push_back(vx);
		}
		// "surfaces" are not equal to triangular faces
		// SURF 0x30
		// mat 0	
		// refs 4		<- 4 vx in surface
		// 5 0 0		<- vx index, tu, tv for vx
		// 2 1 0
		// 1 1 1
		// 6 0 1
		for (int surfIdx = 0; surfIdx < ob->num_surf; ++surfIdx)
		{
			// surfaces can have more than 3 vx, so
			// triangulate them here

			// each vx beyond the first 3 represents a new 
			// triangle that needs to be created
			int faceCount = ob->surfaces[surfIdx].num_vertref - 2;
			for (int faceIdx = 0; faceIdx < faceCount; ++faceIdx)
			{
				// vertref[] is index into vertex array

				FaceType face;
				face.vx0 = ob->surfaces[surfIdx].vertref[0];
				face.vx1 = ob->surfaces[surfIdx].vertref[faceIdx + 1];
				face.vx2 = ob->surfaces[surfIdx].vertref[faceIdx + 2];

				TexCoord tx;
				face.tx0 = static_cast<int>(mesh.vTex.size());
				tx.u = ob->surfaces[surfIdx].uvs[0].u;
				tx.v = ob->surfaces[surfIdx].uvs[0].v;
				mesh.vTex.push_back(tx);
				face.tx1 = static_cast<int>(mesh.vTex.size());
				tx.u = ob->surfaces[surfIdx].uvs[faceIdx + 1].u;
				tx.v = ob->surfaces[surfIdx].uvs[faceIdx + 1].v;
				mesh.vTex.push_back(tx);
				face.tx2 = static_cast<int>(mesh.vTex.size());
				tx.u = ob->surfaces[surfIdx].uvs[faceIdx + 2].u;
				tx.v = ob->surfaces[surfIdx].uvs[faceIdx + 2].v;
				mesh.vTex.push_back(tx);

				mesh.vFace.push_back(face);
			}
		}
		mesh.UpdateExtents();
		mv_Mesh.push_back(mesh);
	}

	return ob;
}

bool Model_AC::read_line(FILE* f)
{
	fgets(m_buff, 255, f);
	m_lineNumber++;
	return true;
}

int Model_AC::get_tokens(char *s, int *argc, char *argv[])
{
	// '\0' chars at the end of tokens and set up the array
	// (tokv) and count (tokc) like argv argc

	char* p = s;
	char c;

	int tc = 0;
	while ((c = *p) != 0)
	{
		char* st;
		if ((c != ' ') && (c != '\t') && (c != '\n') && (c != 13))
		{
			if (c == '"')
			{
				c = *p++;
				st = p;
				while ((c = *p) && ((c != '"') && (c != '\n') && (c != 13)))
				{
					if (c == '\\')
						strcpy(p, p + 1);
					p++;
				}
				*p = 0;
				argv[tc++] = st;
			}
			else
			{
				st = p;
				while ((c = *p) && ((c != ' ') && (c != '\t') && (c != '\n') && (c != 13)))
					p++;
				*p = 0;
				argv[tc++] = st;
			}
		}
		p++;
	}

	*argc = tc;
	return tc;
}

Model_AC::ACObject* Model_AC::new_object()
{
	ACObject* ob = (ACObject *)myalloc(sizeof(ACObject));

	ob->loc.x = ob->loc.y = ob->loc.z = 0.0;
	ob->name = ob->url = nullptr;
	ob->data = nullptr;
	ob->vertices = nullptr;
	ob->num_vert = 0;
	ob->surfaces = nullptr;
	ob->num_surf = 0;
	ob->texture = -1;
	ob->texture_repeat_x = ob->texture_repeat_y = 1.0;
	ob->texture_offset_x = ob->texture_offset_y = 0.0;
	ob->kids = nullptr;
	ob->num_kids = 0;
	ob->matrix[0] = 1;
	ob->matrix[1] = 0;
	ob->matrix[2] = 0;
	ob->matrix[3] = 0;
	ob->matrix[4] = 1;
	ob->matrix[5] = 0;
	ob->matrix[6] = 0;
	ob->matrix[7] = 0;
	ob->matrix[8] = 1;

	return ob;
}

void Model_AC::object_free(ACObject* ob)
{
	for (int n = 0; n < ob->num_kids; n++)
		object_free(ob->kids[n]);

	if (ob->vertices)
		free(ob->vertices);

	for (int s = 0; s < ob->num_surf; s++)
	{
		free(ob->surfaces[s].vertref);
		free(ob->surfaces[s].uvs);
	}

	if (ob->surfaces)
		free(ob->surfaces);

	if (ob->data)
		free(ob->data);

	if (ob->url)
		free(ob->url);

	if (ob->name)
		free(ob->name);

	free(ob);
}

void Model_AC::init_surface(ACSurface* s)
{
	s->vertref = nullptr;
	s->uvs = nullptr;
	s->num_vertref = 0;
	s->flags = 0;
	s->mat = 0;
	s->normal.x = 0.0; s->normal.z = 0.0; s->normal.z = 0.0;
}

void Model_AC::tri_calc_normal(ACPoint* v1, ACPoint* v2, ACPoint* v3, ACPoint* n)
{
	n->x = (v2->y - v1->y)*(v3->z - v1->z) - (v3->y - v1->y)*(v2->z - v1->z);
	n->y = (v2->z - v1->z)*(v3->x - v1->x) - (v3->z - v1->z)*(v2->x - v1->x);
	n->z = (v2->x - v1->x)*(v3->y - v1->y) - (v3->x - v1->x)*(v2->y - v1->y);
	double len = sqrt(n->x*n->x + n->y*n->y + n->z*n->z);

	if (len > 0.0)
	{
		n->x /= (float)len;
		n->y /= (float)len;
		n->z /= (float)len;
	}
}

Model_AC::ACSurface* Model_AC::read_surface(FILE *f, ACSurface *s, ACObject* ob)
{
	int tokc = 0;
	char* tokv[30] = { 0 };
	char t[20] = { 0 };

	init_surface(s);

	while (!feof(f))
	{
		read_line(f);
		sscanf(m_buff, "%s", t);

		if (streq(t, "SURF"))
		{
			int flgs;

			if (get_tokens(m_buff, &tokc, tokv) != 2)
			{
				Logger::Write(__FUNCTION__, "SURF should be followed by one flags argument");
			}
			else
			{
				flgs = strtol(tokv[1], nullptr, 0);
				s->flags = flgs;
			}
		}
		else if (streq(t, "mat"))
		{
			int mindx;

			sscanf(m_buff, "%s %d", t, &mindx);
			s->mat = mindx + m_startMatIndex;
		}
		else if (streq(t, "refs"))
		{
			int num, n;
			int ind;
			float tx, ty;

			sscanf(m_buff, "%s %d", t, &num);

			s->num_vertref = num;
			s->vertref = (int *)malloc(num * sizeof(int));
			s->uvs = (ACUV *)malloc(num * sizeof(ACUV));

			for (n = 0; n < num; n++)
			{
				fscanf(f, "%d %f %f\n", &ind, &tx, &ty); m_lineNumber++;
				s->vertref[n] = ind;
				s->uvs[n].u = tx;
				s->uvs[n].v = ty;
			}

			/** calc surface normal **/
			if (s->num_vertref >= 3)
			{
				tri_calc_normal((ACPoint *)&ob->vertices[s->vertref[0]],
					(ACPoint *)&ob->vertices[s->vertref[1]],
					(ACPoint *)&ob->vertices[s->vertref[2]],
					(ACPoint *)&s->normal);
			}

			return s;
		}
		else
			Logger::Write(__FUNCTION__, "ignoring %s", t);
	}

	return nullptr;
}

void Model_AC::object_calc_vertex_normals(ACObject* ob)
{
	// for each vertex in this object
	for (int v = 0; v < ob->num_vert; v++)
	{
		ACNormal n = { 0, 0, 0 };
		int found = 0;

		// go through each surface
		for (int s = 0; s < ob->num_surf; s++)
		{
			ACSurface *surf = &ob->surfaces[s];

			// check if this vertex is used in this surface
			// if it is, use it to create an average normal
			for (int vr = 0; vr < surf->num_vertref; vr++)
			{
				if (surf->vertref[vr] == v)
				{
					n.x += surf->normal.x;
					n.y += surf->normal.y;
					n.z += surf->normal.z;
					found++;
				}
			}
		}
		if (found > 0)
		{
			n.x /= found;
			n.y /= found;
			n.z /= found;
		}
		ob->vertices[v].normal = n;
	}
}

void Model_AC::calc_vertex_normals(ACObject* ob)
{
	object_calc_vertex_normals(ob);
	if (ob->num_kids)
	{
		for (int n = 0; n < ob->num_kids; n++)
			calc_vertex_normals(ob->kids[n]);
	}
}

int Model_AC::string_to_objecttype(char *s)
{
	if (streq("world", s))
		return(OBJECT_WORLD);
	if (streq("poly", s))
		return(OBJECT_NORMAL);
	if (streq("group", s))
		return(OBJECT_GROUP);
	if (streq("light", s))
		return(OBJECT_LIGHT);
	return(OBJECT_NORMAL);
}

void Model_AC::dump(ACObject* ob)
{
	int n;

	Logger::Write(__FUNCTION__, "OBJECT name %s\nloc %f %f %f\nnum_vert %d\nnum_surf %d",
		ob->name, ob->loc.x, ob->loc.y, ob->loc.z, ob->num_vert, ob->num_surf);

	for (n = 0; n < ob->num_vert; n++)
		Logger::Write(__FUNCTION__, "\tv %f %f %f", ob->vertices[n].x, ob->vertices[n].y, ob->vertices[n].z);

	for (n = 0; n < ob->num_surf; n++)
	{
		ACSurface *s = &ob->surfaces[n];
		Logger::Write(__FUNCTION__, "surface %d, %d refs, mat %d", n, s->num_vertref, s->mat);
	}

	if (ob->num_kids)
	{
		for (n = 0; n < ob->num_kids; n++)
			dump(ob->kids[n]);
	}
}

bool Model_AC::Save(const char* fileName)
{
	// Write model in AC3D format.
	//

	FILE* pFile = fopen(fileName, "wt");
	if (pFile == nullptr)
	{
		Logger::Write(__FUNCTION__, "Unable to open file for writing: '%s'.", fileName);
		return false;
	}

	ms_FileName = fileName;

	// write file marker, version
	fprintf(pFile, "AC3Db\n");

	// write materials
 	for (int i = 0; i < GetMaterialCount(); ++i)
	{
		Material* pMat = GetMaterial(i);
		fprintf(pFile, "MATERIAL \"%s\" rgb %f %f %f  amb %f %f %f  emis %f %f %f  spec %f %f %f  shi %d  trans %f \n",
			pMat->GetName().c_str(),
			pMat->GetDiffuse().r, pMat->GetDiffuse().g, pMat->GetDiffuse().b,
			pMat->GetAmbient().r, pMat->GetAmbient().g, pMat->GetAmbient().b,
			0.0F, 0.0F, 0.0F,
			pMat->GetSpecular().r, pMat->GetSpecular().g, pMat->GetSpecular().b,
			static_cast<int>(pMat->GetShininess()),
			pMat->GetTransparency());
	}

	// AC3D meshes (objects) can be recursive, but that's not
	// yet supported 

	// write the root world object
	fprintf(pFile, "OBJECT world\n");
	fprintf(pFile, "kids 1\n");

	for (int meshIdx = 0; meshIdx < GetMeshCount(); ++meshIdx)
	{
		MeshType* pMesh = &mv_Mesh[meshIdx];

		// write group name (interchangeable with mesh?)

		fprintf(pFile, "OBJECT poly\n");
		
		XString name = pMesh->Name;
		name.Trim();
		if (name.GetLength() == 0)
			name = name.Format("Mesh%d", meshIdx);
		fprintf(pFile, "name \"%s\"\n", name.c_str());
		fprintf(pFile, "\n");

		// write texture for this object
		if (GetMaterialCount() > 0)
		{
			XString strTexture = GetMaterial(0)->GetMapDiffuse();
			if (strTexture.GetLength() > 0)
			{
				// things don't really line up here - materials have the texture, which i 
				// consider a good design, but ac3d writes the texture per object; since
				// i only support 1 texture anyway, i'm just writing the first one
				fprintf(pFile, "texture \"%s\"\n", strTexture.c_str());
			}
		}

		// write object position
		fprintf(pFile, "loc 0 0 0\n");

		// write vertices
		fprintf(pFile, "numvert %d\n", GetVertexCount(meshIdx));
		for (VertexType vx : pMesh->vVx)
		{
			fprintf(pFile, "%0.4f %0.4f %0.4f\n", vx.P.x, vx.P.y, vx.P.z);
		}
		fprintf(pFile, "\n");

		// write "surfaces", which from my models are synonymous with
		// triangular faces

		fprintf(pFile, "numsurf %d\n", GetFaceCount(meshIdx));
		for (FaceType face : pMesh->vFace)
		{
			// Start of surface.
			// Low 4 bits is the type (0 = polygon, 1 = closedline, 2 = line)
			// High 4 bits specify the shading and backface (bit1 = shaded surface bit2 = twosided)
			fprintf(pFile, "SURF 0x30\n");
			fprintf(pFile, "mat 0\n");
			// they are all triangular
			fprintf(pFile, "refs 3\n");
			fprintf(pFile, "%d %0.4f %0.4f\n", face.vx0, pMesh->vTex[face.tx0].u, pMesh->vTex[face.tx0].v);
			fprintf(pFile, "%d %0.4f %0.4f\n", face.vx1, pMesh->vTex[face.tx1].u, pMesh->vTex[face.tx1].v);
			fprintf(pFile, "%d %0.4f %0.4f\n", face.vx2, pMesh->vTex[face.tx2].u, pMesh->vTex[face.tx2].v);
		}

		// end of object
		fprintf(pFile, "kids 0\n");
		fprintf(pFile, "\n");
	}

	fclose(pFile);

	return true;
}
