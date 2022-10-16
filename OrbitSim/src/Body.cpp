#include "Body.h"



Body::Body(const unsigned int id, float m, float rad, unsigned int numSides)
	:ID(id),mass(m), radius(rad)
{
	NumberOfSides = numSides;
	// Create The Circle vertices
	InitCircle(NumberOfSides, radius);
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
	circleColor = glm::vec4(r, g, b, a);
	Circle_shader->use();
	Circle_shader->SetUniformVec4fv("u_Color", circleColor);
}

void Body::setAlpha(const float alphaVal)
{
	circleColor.w = alphaVal;
	Circle_shader->use();
	Circle_shader->SetUniformVec4fv("u_Color", circleColor);
}

//void Body::setColor(float r, float g, float b, float a)
//{
//	quad_shader->use();
//	quad_shader->SetUniformVec4fv("u_Color", glm::vec4{ r,g,b,a });
//}


