#include "Simulation.h"
#include "Strand.h"
#include "Hair.h"
#include "gtx/transform.hpp"
#include "gtx/extended_min_max.hpp"
#include <iostream>

#define GRAVITY -9.8f
#define TIMESTEP 0.02f
#define GRID_WIDTH 0.1f
#define COLLISIONS true

Simulation::Simulation(Mesh* mesh)
{
	mTime = 0.0f;
	mMesh = mesh;
	mTransform = glm::mat4(1.0f);
	windDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	windStrength = 0.0f;
	stiffness = 0.0f;
	dampening = 1.0f;
}

void Simulation::Update(float time)
{
	mTime = time;
}

void Simulation::Simulate(Hair* hair)
{
	Move(hair);
	CalculateExternalForces(hair);
	ParticleSimulation(hair);
	UpdateHair(hair);
}

void Simulation::UpdateRotation(Hair* hair, float angle, glm::vec3 axis)
{
	UpdateHair(hair);
	mTransform = glm::rotate(mTransform, angle, axis);
}

void Simulation::UpdatePosition(Hair* hair, glm::vec3 transform)
{
	UpdateHair(hair);
	mTransform = glm::translate(mTransform, transform);
}

void Simulation::UpdateHair(Hair* hair)
{
	for (auto& guide : hair->GetGuideHairs())
	{
		for (auto& vertex : guide->vertices)
		{
			vertex->prevPosition = glm::vec3(mTransform * glm::vec4(vertex->startPosition, 1.0f));
		}
	}
}

glm::mat4 Simulation::GetTransform() const
{
	return mTransform;
}

void Simulation::ResetPosition()
{
	/*shake = false;
	nod = false;*/
	mTransform = glm::mat4(1.0f);
}

void Simulation::Move(Hair* hair)
{
	UpdateHair(hair);
	/*if (shake || nod)
	{
		mTransform = glm::rotate((float)sin(mTime), glm::vec3(nod, shake, 0));
	}
	else
	{
		mTransform = glm::rotate(0.0f, glm::vec3(0, 1, 0));
	}*/

}

void Simulation::CalculateExternalForces(Hair* hair)
{
	for (auto& guide : hair->GetGuideHairs())
	{
		float numVerts = guide->vertices.size();
		for (int i = 0; i < numVerts; i++)
		{
			HairVertex* vertex = guide->vertices[i];

			glm::vec3 force = glm::vec3(0.0f);
			force += glm::vec3(glm::inverse(mTransform) * glm::vec4(0.0f, GRAVITY, 0.0f, 0.0f)); //Gravity

			//Acceleration
			glm::vec4 current = mTransform * glm::vec4(vertex->startPosition, 1.0f);
			glm::vec3 acceleration = (glm::vec3(vertex->prevPosition - glm::vec3(current)) - vertex->velocity * TIMESTEP) / (TIMESTEP * TIMESTEP);
			force += acceleration * vertex->mass * 0.1f;

			//Wind
			if (windStrength > 0.0f && glm::length(windDirection) > 0)
			{
				//Higher wind strenth means more chance of each vertex being affected by wind
				int chance = (rand() % 100) / 2;
				if (chance <= windStrength)
				{
					//Main wind force
					force += glm::vec3(glm::inverse(mTransform) * glm::vec4(glm::normalize(windDirection) * windStrength, 0.0f)) * 0.8f;

					//Add randomness based on wind strength
					float a = ((float)rand() / (RAND_MAX)) * 2 - 1;
					float b = ((float)rand() / (RAND_MAX)) * 2 - 1;
					float c = ((float)rand() / (RAND_MAX)) * 2 - 1;
					glm::vec3 randomForce = glm::vec3(a, b, c) * windStrength * 0.2f;

					//Ignore small random forces to reduce jittering
					if (glm::length(randomForce) > 3.0f)
					{
						force += randomForce;
					}
				}
			}

			if (COLLISIONS)
			{
				//Hair-Sphere collisions
				glm::vec3 normal;
				if (mMesh->Contains(normal, vertex->position))
				{
					force = 5.0f * normal;
				}
			}

			vertex->forces = force;
		}
	}
}

void Simulation::ParticleSimulation(Hair* hair)
{
	for (auto& guide : hair->GetGuideHairs())
	{
		float numVertices = guide->vertices.size();
		guide->vertices[0]->tempPosition = guide->vertices[0]->position;
		HairVertex* last = guide->vertices.back();

		//Update velocities
		for (int i = 1; i < numVertices; i++)
		{
			HairVertex* vertex = guide->vertices[i];
			HairVertex* previous = guide->vertices[i - 1];

			vertex->velocity += TIMESTEP * (vertex->forces * (1.0f / vertex->mass)) * 0.5f;
			glm::vec3 stiffPosition = previous->segmentLength * previous->pointVec;
			vertex->tempPosition += glm::mix((vertex->velocity * TIMESTEP), stiffPosition, stiffness);
			vertex->forces = glm::vec3(0.0f);
			vertex->velocity *= 0.99f;
		}

		glm::vec3 direction;
		glm::vec3 currentPos;
		for (int i = 1; i < numVertices; i++)
		{
			HairVertex* previous = guide->vertices[i - 1];
			HairVertex* current = guide->vertices[i];
			currentPos = current->tempPosition;
			direction = glm::normalize(current->tempPosition - previous->tempPosition);
			current->tempPosition = previous->tempPosition + direction * previous->segmentLength;
			current->correction = currentPos - current->tempPosition;
		}

		// FTL Correction
		for (int i = 1; i < numVertices; i++)
		{
			HairVertex* previous = guide->vertices[i - 1];
			HairVertex* current = guide->vertices[i];
			previous->velocity = ((previous->tempPosition - previous->position) / TIMESTEP) + dampening * (current->correction / TIMESTEP);
			previous->position = previous->tempPosition;
		}

		last->position = last->tempPosition;
	}
}


