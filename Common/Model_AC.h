#ifndef MODEL_AC_H
#define MODEL_AC_H

#include <stdio.h>
#include <stdlib.h>

#include "XString.h"
#include "Model.h"

#define STRING(s)  (char *)(strcpy((char *)myalloc(strlen(s)+1), s))
#define streq(a,b)  (!strcmp(a,b))
#define myalloc malloc
#define myfree free

class Model_AC : public Model
{
public:

	Model_AC();
	~Model_AC();

	// Model
	virtual Model::Format GetFormat() override { return Model::Format::AC3D; }
	virtual bool Load(const char* fileName) override;
	virtual bool Save(const char* fileName) override;

private:

	typedef struct ACPoint_t
	{
		float x, y, z;
	} ACPoint;

	typedef struct ACNormal_t
	{
		float x, y, z;
	} ACNormal;

	typedef struct ACVertex_t
	{
		float x, y, z;
		ACNormal normal;
	} ACVertex;

	typedef struct ACUV_t
	{
		float u, v;
	} ACUV;

	typedef struct ACSurface_t
	{
		int* vertref;
		ACUV* uvs;
		int num_vertref;
		ACNormal normal;
		int flags;
		int mat;
	} ACSurface;

	typedef struct ACObject_t
	{
		ACPoint loc;
		char* name;
		char* data;
		char* url;
		ACVertex* vertices;
		int num_vert;

		ACSurface *surfaces;
		int num_surf;
		float texture_repeat_x, texture_repeat_y;
		float texture_offset_x, texture_offset_y;

		int num_kids;
		struct ACObject_t **kids;
		float matrix[9];
		int type;
		int texture;
	} ACObject;

private:

	ACObject* m_pACObject;      // primary model object

	int m_startMatIndex;
	int m_lineNumber;
	char m_buff[255];
	int m_num_texture;

private:

	bool read_line(FILE* pFile);
	int get_tokens(char* s, int* argc, char* argv[]);
	ACObject* new_object();
	void init_surface(ACSurface* s);
	void tri_calc_normal(ACPoint* v1, ACPoint* v2, ACPoint* v3, ACPoint* n);
	ACSurface* read_surface(FILE* pFile, ACSurface* s, ACObject* ob);
	ACObject* load_object(FILE* pFile, ACObject* parent);
	void object_calc_vertex_normals(ACObject* ob);
	void object_free(ACObject* ob);
	void calc_vertex_normals(ACObject* ob);
	int string_to_objecttype(char* s);
	void dump(ACObject* ob);
};

#endif // #ifndef MODEL_AC_H
