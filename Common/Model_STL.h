#ifndef MODEL_STL_H
#define MODEL_STL_H

#include "Model.h"

class Model_STL : public Model
{
public:

	Model_STL();
	virtual ~Model_STL();

	bool StlFileHasASCIIFormat(const char* filename);

	// Model
	virtual Model::Format GetFormat() override { return Model::Format::STL; }
	virtual bool Load(const char* fileName) override;

private:

	// a coordinate triple with an additional index. The index is required
	// for RemoveDoubles, so that triangles can be reindexed properly.
	//template <typename number_t, typename index_t>
	struct CoordWithIndex
	{
		double data[3];
		int index;

		bool operator == (const CoordWithIndex& c) const
		{
			return (c[0] == data[0]) && (c[1] == data[1]) && (c[2] == data[2]);
		}

		bool operator != (const CoordWithIndex& c) const
		{
			return (c[0] != data[0]) || (c[1] != data[1]) || (c[2] != data[2]);
		}

		bool operator < (const CoordWithIndex& c) const
		{
			return (data[0] < c[0])
				|| (data[0] == c[0] && data[1] < c[1])
				|| (data[0] == c[0] && data[1] == c[1] && data[2] < c[2]);
		}

		inline double& operator [] (const size_t i) { return data[i]; }
		inline double operator [] (const size_t i) const { return data[i]; }
	};

private:

	bool ReadStlFile_ASCII(const char* filename);
	bool ReadStlFile_BINARY(const char* fileName);

	void RemoveDoubles(std::vector<VEC3>& uniqueCoordsOut,
		std::vector<int>& trisInOut,
		std::vector <CoordWithIndex> &coordsWithIndexInOut);

};

#endif // #ifndef MODEL_STL_H
