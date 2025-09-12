#ifndef MODEL_OBJ_H
#define MODEL_OBJ_H

#include "Model.h"

class Model_OBJ : public Model
{
public:

	Model_OBJ();
	virtual ~Model_OBJ();

	// Model
	virtual Model::Format GetFormat() override { return Model::Format::OBJ; }
	virtual bool Load(const char* fileName) override;
	virtual bool Save(const char* fileName) override;

private:

	void LoadMaterials(const char* matFileName);
	void SaveMaterials(const char* matFileName);
};

#endif // #ifndef MODEL_OBJ_H
