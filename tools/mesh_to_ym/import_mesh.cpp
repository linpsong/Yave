/*******************************
Copyright (c) 2016-2017 Gr�goire Angerand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**********************************/

#include "import.h"

MeshData import_mesh(aiMesh* mesh, const aiScene* scene) {
	const char* err_msg = "Unable to load mesh.";

	if(!mesh ||
	   mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE ||
	   !mesh->HasNormals() ||
	   !mesh->HasTangentsAndBitangents() ||
	   !mesh->HasTextureCoords(0)) {
		fatal(err_msg);
	}

	usize vertex_count = mesh->mNumVertices;
	auto vertices = vector_with_capacity<Vertex>(vertex_count);
	for(usize i = 0; i != vertex_count; ++i) {
		vertices << Vertex {
				{mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z},
				{mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z},
				{mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z},
				{mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y}
			};
	}

	usize triangle_count = mesh->mNumFaces;
	auto triangles = vector_with_capacity<IndexedTriangle>(triangle_count);
	for(usize i = 0; i != triangle_count; ++i) {
		triangles << IndexedTriangle{mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2]};
	}

	if(mesh->HasBones()) {
		SkeletonData skeleton = import_skeleton(mesh, scene);
		return MeshData::from_parts(std::move(vertices), std::move(triangles), std::move(skeleton.skin), std::move(skeleton.bones));
	}

	return MeshData::from_parts(std::move(vertices), std::move(triangles));
}









