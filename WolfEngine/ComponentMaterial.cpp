#include "ComponentMaterial.h"
#include "Application.h"
#include "ModuleTextures.h"
#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include "OpenGL.h"
#include "GameObject.h"
#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_sdl_gl3.h"

ComponentMaterial::ComponentMaterial(GameObject* parent) : Component(Component::Type::MATERIAL, parent)
{
}

ComponentMaterial::~ComponentMaterial()
{
}

void ComponentMaterial::Load(aiMaterial* material, const aiString& folder_path)
{
	aiColor4D ambient;
	aiColor4D diffuse;
	aiColor4D specular;
	material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
	material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
	material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
	if (material->Get(AI_MATKEY_SHININESS, shiness) == AI_SUCCESS)
		shiness *= 128.0f;
	float shine_strength = 1.0f;
	if (material->Get(AI_MATKEY_SHININESS_STRENGTH, shine_strength) == AI_SUCCESS)
		specular *= shine_strength;
	for (int i = 0; i < 3; i++)
	{
		this->ambient[i] = ambient[i];
		this->diffuse[i] = diffuse[i];
		this->specular[i] = specular[i];
	}

	unsigned num_textures = material->GetTextureCount(aiTextureType_DIFFUSE);
	if (num_textures >= 1)
	{
		aiString path;
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
		{
			aiString full_path = aiString(folder_path);
			full_path.Append(path.data);

			LoadTexture(full_path);
		}
	}
}

void ComponentMaterial::LoadTexture(const aiString& texture_path)
{
	texture = App->textures->LoadTexture(texture_path);
}

bool ComponentMaterial::OnUpdate()
{
	return true;
}

bool ComponentMaterial::OnDraw() const
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shiness);

	glBindTexture(GL_TEXTURE_2D, texture);
	
	return true;
}

bool ComponentMaterial::OnEditor()
{
	if (ImGui::CollapsingHeader("Material"))
	{
		ImGui::Checkbox("Active", &enable);

		ImGui::SameLine();

		if (ImGui::Button("Delete"))
			parent->DeleteComponent(this);

		ImGui::DragFloat4("Ambient", (float*)&ambient, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat4("Diffuse", (float*)&diffuse, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat4("Specular", (float*)&specular, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Shiness", (float*)&shiness, 1.0f, 0.0f, 128.0f);
	}

	return ImGui::IsItemClicked();
}
