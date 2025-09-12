// TestModel_AC.cpp
// CppUnitLite test harness for Model_AC class
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "CppUnitLite/TestHarness.h"	// CppUnitLite library
#include "../Common/UnitTest.h"			// unit test helpers
#include "Model_AC.h"					// interface to class under test

int main(int argc, char* argv[])
{
	// output name of executable
	if (argc > 0 && argv[0])
		printf("%s\n", argv[0]);

	SetDataPath(argv[0]);

	TestResult tr;
	TestRegistry::runAllTests(tr);

	// always pause if errors
	int failureCount = tr.GetFailureCount();
	if (failureCount > 0)
		getc(stdin);

	return failureCount;
}

//----------------------------------------------------------------------------
TEST(Model_AC, Test_Create)
{
	// Creating an AC3D file programmatically.
	//

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "temp.ac");

	{
		Model_AC* pModel = new Model_AC();

		// add mesh

		LONGS_EQUAL(0, pModel->GetMeshCount());

		int meshIndex = pModel->AddMesh();
		Model::MeshType* pMesh = pModel->GetMesh(meshIndex);

		LONGS_EQUAL(0, meshIndex);
		LONGS_EQUAL(1, pModel->GetMeshCount());

		// add vertices

		for (int i = 0; i < 8; ++i)
		{
			int vertexIndex = pModel->AddVertex(meshIndex);
			LONGS_EQUAL(i, vertexIndex);
		}
		Model::VertexType* pVx = pModel->GetVertex(0, 0);
		pVx->P = VEC3(-1.0, -1.0, -1.0);
		pVx = pModel->GetVertex(0, 1);
		pVx->P = VEC3(1.0, -1.0, -1.0);
		pVx = pModel->GetVertex(0, 2);
		pVx->P = VEC3(-1.0, 1.0, -1.0);
		pVx = pModel->GetVertex(0, 3);
		pVx->P = VEC3(1.0, 1.0, -1.0);
		pVx = pModel->GetVertex(0, 4);
		pVx->P = VEC3(-1.0, -1.0, 1.0);
		pVx = pModel->GetVertex(0, 5);
		pVx->P = VEC3(1.0, -1.0, 1.0);
		pVx = pModel->GetVertex(0, 6);
		pVx->P = VEC3(-1.0, 1.0, 1.0);
		pVx = pModel->GetVertex(0, 7);
		pVx->P = VEC3(1.0, 1.0, 1.0);

		// add faces

		for (int i = 0; i < 12; ++i)
		{
			int faceIndex = pModel->AddFace(meshIndex);
			LONGS_EQUAL(i, faceIndex);
		}
		Model::FaceType* pFace = pModel->GetFace(0, 0);
		pFace->vx0 = 1; pFace->vx1 = 2; pFace->vx2 = 3;
		pFace = pModel->GetFace(0, 1);
		pFace->vx0 = 2; pFace->vx1 = 1; pFace->vx2 = 0;
		pFace = pModel->GetFace(0, 2);
		pFace->vx0 = 4; pFace->vx1 = 5; pFace->vx2 = 6;
		pFace = pModel->GetFace(0, 3);
		pFace->vx0 = 6; pFace->vx1 = 5; pFace->vx2 = 7;
		pFace = pModel->GetFace(0, 4);
		pFace->vx0 = 3; pFace->vx1 = 2; pFace->vx2 = 6;
		pFace = pModel->GetFace(0, 5);
		pFace->vx0 = 3; pFace->vx1 = 6; pFace->vx2 = 7;
		pFace = pModel->GetFace(0, 6);
		pFace->vx0 = 0; pFace->vx1 = 1; pFace->vx2 = 4;
		pFace = pModel->GetFace(0, 7);
		pFace->vx0 = 4; pFace->vx1 = 1; pFace->vx2 = 5;
		pFace = pModel->GetFace(0, 8);
		pFace->vx0 = 2; pFace->vx1 = 0; pFace->vx2 = 4;
		pFace = pModel->GetFace(0, 9);
		pFace->vx0 = 2; pFace->vx1 = 4; pFace->vx2 = 6;
		pFace = pModel->GetFace(0, 10);
		pFace->vx0 = 1; pFace->vx1 = 3; pFace->vx2 = 5;
		pFace = pModel->GetFace(0, 11);
		pFace->vx0 = 5; pFace->vx1 = 3; pFace->vx2 = 7;

		// add texture coordinates

		for (int i = 0; i < 8; ++i)
		{
			int tc = pModel->AddTexCoord(meshIndex);
			LONGS_EQUAL(i, tc);
		}
		Model::TexCoord* pTex = pModel->GetTexCoord(0, 0);
		pTex->u = 0.0; pTex->v = 0.0;
		pTex = pModel->GetTexCoord(0, 1);
		pTex->u = 0.0; pTex->v = 1.0;
		pTex = pModel->GetTexCoord(0, 2);
		pTex->u = 1.0; pTex->v = 1.0;
		pTex = pModel->GetTexCoord(0, 3);
		pTex->u = 1.0; pTex->v = 0.0;
		pTex = pModel->GetTexCoord(0, 4);
		pTex->u = 0.0; pTex->v = 0.0;
		pTex = pModel->GetTexCoord(0, 5);
		pTex->u = 0.0; pTex->v = 1.0;
		pTex = pModel->GetTexCoord(0, 6);
		pTex->u = 1.0; pTex->v = 1.0;
		pTex = pModel->GetTexCoord(0, 7);
		pTex->u = 1.0; pTex->v = 0.0;

		// save it

		bool status = pModel->Save(fileName.c_str());
		CHECK(status);

		delete pModel;
	}

	{
		// read back

		Model_AC* pModel = new Model_AC();

		bool status = pModel->Load(fileName.c_str());
		CHECK(status);

		LONGS_EQUAL(1, pModel->GetMeshCount());
		LONGS_EQUAL(8, pModel->GetVertexCount(0));
		LONGS_EQUAL(12, pModel->GetFaceCount(0));
		LONGS_EQUAL(36, pModel->GetTexCoordCount(0));

		delete pModel;
	}
}

//----------------------------------------------------------------------------
TEST(Model_AC, Test_ReadWrite)
{
	// Test writing and reading back an AC3D file.
	//

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "test_cube.ac");
	XString outFileName = XString::CombinePath(gs_DataPath.c_str(), "temp_cube.ac");

	{
		// write

		Model_AC* pModel = new Model_AC();
		bool status = pModel->Load(fileName.c_str());
		CHECK(status);

		CHECK(Model::Format::AC3D == pModel->GetFormat());

		LONGS_EQUAL(12, pModel->GetFaceCount());
		LONGS_EQUAL(1, pModel->GetMeshCount());
		LONGS_EQUAL(8, pModel->GetVertexCount());

		status = pModel->Save(outFileName.c_str());
		CHECK(status);

		delete pModel;
	}

	{
		// read back

		Model_AC* pModel = new Model_AC();
		bool status = pModel->Load(outFileName.c_str());
		CHECK(status);

		CHECK(Model::Format::AC3D == pModel->GetFormat());

		LONGS_EQUAL(12, pModel->GetFaceCount());
		LONGS_EQUAL(1, pModel->GetMeshCount());
		LONGS_EQUAL(8, pModel->GetVertexCount());

		delete pModel;
	}
}

//----------------------------------------------------------------------------
TEST(ModelAC, Test_Read)
{
	// Test reading a simple AC3D file.
	//

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "test_cube.ac");

	{
		Model_AC* pModel = new Model_AC();
		bool status = pModel->Load(fileName.c_str());
		CHECK(status);

		CHECK(Model::Format::AC3D == pModel->GetFormat());

		LONGS_EQUAL(12, pModel->GetFaceCount());
		LONGS_EQUAL(1, pModel->GetMeshCount());
		LONGS_EQUAL(8, pModel->GetVertexCount());

		int meshIndex = pModel->GetMeshIndex("box");
		LONGS_EQUAL(0, meshIndex);

		delete pModel;
	}
}
