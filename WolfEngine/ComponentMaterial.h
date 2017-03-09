#ifndef COMPONENTMATERIAL_H
#define COMPONENTMATERIAL_H

#include "Component.h"
#include "MathGeoLib/src/Math/float4.h"

struct aiMaterial;
struct aiString;

class ComponentMaterial : public Component
{
public:
	ComponentMaterial(GameObject* parent);
	~ComponentMaterial();

	void Load(aiMaterial* material, const aiString& folder_path);

	bool OnUpdate();
	bool OnDraw() const;
	bool OnEditor(int selection_mask, int id);

private:
	float ambient[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float shiness = 0.0f;
	unsigned texture = 0;
};



#endif // !COMPONENTMATERIAL_H