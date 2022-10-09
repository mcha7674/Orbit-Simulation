#include "Body.h"

Body::Body()
{
	ID = 1;
	mass = 1.0f;
	a = 1.0f;
	radius = 1.0f;

}

Body::Body(const unsigned int id, float m, float a, float rad)
	:ID(id),mass(m), a(a), radius(rad)
{
}

glm::vec3 Body::UpdateScaleResize(const float& aspectRatio)
{
	glm::vec3 newScale = glm::vec3(1.0f);
	if (radius < 1.0f)
	{
		newScale.x = radius;
		newScale.y = newScale.x * aspectRatio;
	}
	else {
		newScale.y = radius;
		newScale.x = newScale.y * aspectRatio;
	}
	

	return newScale;
}

void Body::setColor(float r, float g, float b, float a)
{
	quad_shader->use();
	quad_shader->SetUniformVec4fv("u_Color", glm::vec4{r,g,b,a});
}


