#include "ComponentMesh.h"
#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include "OpenGL.h"
#include "GameObject.h"
#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_sdl_gl3.h"

ComponentMesh::ComponentMesh(GameObject* parent) : Component(Component::Type::MESH, parent)
{
}

ComponentMesh::~ComponentMesh()
{
	RELEASE_ARRAY(vertices);
	RELEASE_ARRAY(vertices_bind);
	RELEASE_ARRAY(normals);
	RELEASE_ARRAY(tex_coords);
	RELEASE_ARRAY(indices);

	if (has_bones)
	{
		for (std::vector<Bone*>::iterator it = bones.begin(); it != bones.end(); ++it)
		{
			RELEASE_ARRAY((*it)->weights);
			RELEASE(*it);
		}
		bones.clear();
	}
}

void ComponentMesh::Load(aiMesh* mesh)
{
	num_vertices = mesh->mNumVertices;
	vertices = new float[3 * num_vertices];
	vertices_bind = new float[3 * num_vertices];
	unsigned c = 0;
	for (size_t i = 0; i < num_vertices; ++i)
		for (size_t j = 0; j < 3; ++j)
			vertices[c++] = mesh->mVertices[i][j];
	if (c != 3 * mesh->mNumVertices)
		LOG("Error loading meshes: Incorrect number of vertices");

	memcpy(vertices_bind, vertices, 3 * num_vertices * sizeof(float));

	//Creating BoundingBox from vertices points
	parent->bbox.SetNegativeInfinity();
	parent->bbox.Enclose((float3*)vertices, num_vertices);

	has_normals = mesh->HasNormals();
	if (has_normals) 
	{
		normals = new float[3 * num_vertices];
		c = 0;
		for (size_t i = 0; i < num_vertices; ++i)
			for (size_t j = 0; j < 3; ++j)
				normals[c++] = mesh->mNormals[i][j];
		if (c != 3 * mesh->mNumVertices)
			LOG("Error loading meshes: Incorrect number of normals");
	}

	has_tex_coords = mesh->HasTextureCoords(0);
	if (has_tex_coords) 
	{
		tex_coords = new float[2 * num_vertices];
		c = 0;
		for (size_t i = 0; i < num_vertices; ++i)
			for (size_t j = 0; j < 2; ++j)
				tex_coords[c++] = mesh->mTextureCoords[0][i][j];
		if (c != 2 * mesh->mNumVertices)
			LOG("Error loading meshes: Incorrect number of texture coordinates");
	}

	num_indices = 3 * mesh->mNumFaces;
	indices = new unsigned[num_indices];

	c = 0;
	for (size_t j = 0; j < mesh->mNumFaces; ++j)
		for (size_t k = 0; k < 3; ++k)
			indices[c++] = mesh->mFaces[j].mIndices[k];
	if (c != 3 * mesh->mNumFaces)
		LOG("Error loading meshes: Incorrect number of indices");

	if (mesh->HasBones())
	{
		has_bones = true;
		for (int i = 0; i < mesh->mNumBones; i++)
		{
			aiBone* scene_bone = mesh->mBones[i];
			Bone* bone = new Bone;
			bone->name = scene_bone->mName;

			aiVector3D ai_scaling;
			aiVector3D ai_position;
			aiQuaternion ai_rotation;
			scene_bone->mOffsetMatrix.Decompose(ai_scaling, ai_rotation, ai_position);
			float3 position = float3(ai_position.x, ai_position.y, ai_position.z);
			float3 scaling = float3(ai_scaling.x, ai_scaling.y, ai_scaling.z);
			Quat rotation = Quat(ai_rotation.x, ai_rotation.y, ai_rotation.z, ai_rotation.w);
			bone->bind = float4x4(rotation, position).Transposed();
			bone->num_weights = scene_bone->mNumWeights;
			bone->weights = new Weight[bone->num_weights];
			for (int j = 0; j < bone->num_weights; j++)
			{
				bone->weights[j].weight = scene_bone->mWeights[j].mWeight;
				bone->weights[j].vertex = scene_bone->mWeights[j].mVertexId;
			}
			bones.push_back(bone);
		}
	}
}

void ComponentMesh::LoadBones()
{
	if (has_bones)
	{
		for (std::vector<Bone*>::iterator it = bones.begin(); it != bones.end(); ++it)
		{
			(*it)->bone_object = parent->root->FindByName((*it)->name.data);
		}
	}
}

bool ComponentMesh::OnUpdate()
{
	//if (has_bones)
	//{
	//	for (std::vector<Bone*>::iterator it = bones.begin(); it != bones.end(); ++it)
	//	{
	//		float4x4 animation_transform = (*it)->bone_object->GetGlobalBoneTransformMatrix();
	//		for (int i = 0; i < (*it)->num_weights; i++)
	//		{
	//			unsigned index = (*it)->weights[i].vertex;
	//			float4 vertex_bind = float3(vertices_bind[3 * index], vertices_bind[3 * index + 1], vertices_bind[3 * index + 2]).ToPos4();
	//			//float3 vertex_end = (*it)->weights[i].weight * (animation_transform * (*it)->bind * vertex_bind).Float3Part();
	//			float4 vertex_end = (*it)->weights[i].weight * (vertex_bind * (*it)->bind * animation_transform);
	//			//float3 vertex_end = ((*it)->bind.Mul(animation_transform).Mul(vertex_bind).Mul((*it)->weights[i].weight)).Float3Part();
	//			vertices[3 * index] = vertex_end.x;
	//			vertices[3 * index + 1] = vertex_end.y;
	//			vertices[3 * index + 2] = vertex_end.z;
	//		}
	//	}
	//}

	return true;
}

bool ComponentMesh::OnDraw() const
{
	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, vertices);

	if (has_normals)
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, normals);
	}
		
	if (has_tex_coords)
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
	}
		
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, indices);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	return true;
}

bool ComponentMesh::OnEditor()
{
	if (ImGui::CollapsingHeader("Mesh"))
	{
		ImGui::Checkbox("Active", &enable);

		ImGui::SameLine();

		if(ImGui::Button("Delete"))
			this->~ComponentMesh();

		ImGui::Text(folder_path.C_Str());
	}

	return ImGui::IsItemClicked();
}
