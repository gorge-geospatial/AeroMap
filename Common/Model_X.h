#ifndef MODEL_X_H
#define MODEL_X_H

#include <stdio.h>			// FILE, etc
#include <stdlib.h>			// atoi(), etc
#include <ctype.h>			// tolower(), etc

#include "Model.h"			// base class

class Model_X : public Model
{
public:

	// construction / destruction

	Model_X();
	virtual ~Model_X();

	// Model

	virtual Model::Format GetFormat() override { return Model::Format::X; }

	// file i/o

	virtual bool Load(const char* fileName) override;
	virtual bool Save(const char* fileName = nullptr) override;

private:

	int GetXFileToken(FILE* pFile, char* token);
};

#endif // MODEL_X_H
