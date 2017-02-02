#include "Math.h"
#include "ModuleCamera.h"
#include "ModuleInput.h"
#include "Application.h"

ModuleCamera::ModuleCamera() : Module(MODULE_CAMERA)
{
	frustum = new Frustum();
}

ModuleCamera::~ModuleCamera()
{
	RELEASE(frustum);
}

bool ModuleCamera::Start()
{
	frustum = new Frustum();

	return true;
}

update_status ModuleCamera::Update(float dt)
{

	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
	{
		SetPosition(speed * dt * frustum->front);
	}

	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
	{
		SetPosition(-speed * dt * frustum->front);
	}

	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		SetPosition(-speed * dt * frustum->WorldRight());
	}

	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		SetPosition(speed * dt * frustum->WorldRight());
	}

	return UPDATE_CONTINUE;
}

bool ModuleCamera::CleanUp()
{
	RELEASE(frustum);

	return true;
}

void ModuleCamera::SetFOV(float fov)
{
	float r = frustum->AspectRatio();
	frustum->verticalFov = fov;
	SetFOH(fov, r);
}

void ModuleCamera::SetAspectRatio(float r)
{
	float fov = frustum->verticalFov;
	SetFOH(fov, r);
}

void ModuleCamera::SetPlaneDistances(float nearPlaneDistance, float farPlaneDistance)
{
	frustum->nearPlaneDistance = nearPlaneDistance;
	frustum->farPlaneDistance = farPlaneDistance;
}

void ModuleCamera::SetPosition(float3 position)
{
	frustum->pos = position;
}

void ModuleCamera::SetOrientation(float3 rotation)
{
	
}

void ModuleCamera::LookAt(float3 position)
{
	frustum->front = frustum->pos - position;
}

float* ModuleCamera::GetProjectionMatrix()
{
	float* ret = &(frustum->ProjectionMatrix().v[0][0]);
	return ret;
}

float* ModuleCamera::GetViewMatrix()
{
	float* ret = &(frustum->ViewMatrix().v[0][0]);
	return ret;
}

void ModuleCamera::WindowResize(int width, int height)
{
	float r = width / height;
	float fov = frustum->verticalFov;
	SetFOH(fov, r);
}

void ModuleCamera::SetFOH(float fov, float r)
{
	float foh = 2 * Atan(r * tan(fov / 2));
	frustum->horizontalFov = foh;
}


