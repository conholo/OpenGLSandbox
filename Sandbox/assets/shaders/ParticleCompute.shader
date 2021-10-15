#type compute

#version 450
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

const vec4 Sphere = vec4(0.0, 0.0, 0.0, 25.0);

uniform float u_DeltaTime;

struct Particle
{
	vec4 Position;
	vec4 Color;
};

layout(std140, binding = 4) buffer ParticlesBuffer
{
	Particle Particles[];
};

layout(std140, binding = 5) buffer Velocity
{
	vec3 Velocities[];
};

vec3 Bounce(vec3 inputVector, vec3 normal)
{
	vec3 reflection = reflect(inputVector, normal);
	return reflection;
}

vec3 BounceSphere(vec3 position, vec3 velocity, vec4 sphere)
{
	vec3 normal = normalize(position - sphere.xyz);
	return Bounce(velocity, normal);
}

bool OutOfSphere(vec3 position, vec4 sphere)
{
	float dist = length(position - sphere.xyz);
	return dist > sphere.w;
}

float InverseLerp(float a, float b, float t)
{
	return clamp((t - a) / (b - a), 0, 1);
}

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main()
{
	const vec3 gravity = vec3(0.0, -9.8, 0.0);
	const float deltaTime = u_DeltaTime;

	uint gid = gl_GlobalInvocationID.x;

	vec3 position = Particles[gid].Position.xyz;
	vec3 velocity = Velocities[gid].xyz;

	vec3 deltaPosition = position + velocity * deltaTime + 0.5 * deltaTime * deltaTime * gravity;
	vec3 deltaVelocity = velocity + gravity * deltaTime;

	float percentHeight = InverseLerp(-Sphere.w / 2, Sphere.w / 2, deltaPosition.y);

	if (OutOfSphere(deltaPosition, Sphere))
	{
		deltaVelocity = BounceSphere(position, velocity, Sphere);
		deltaPosition = position + deltaVelocity * deltaTime + 0.5 * deltaTime * deltaTime * gravity;
	}

	float percentXVelocity = InverseLerp(-5.0, 5.0, deltaVelocity.x);
	float percentYVelocity = InverseLerp(-5.0, 5.0, deltaVelocity.y);
	float percentZVelocity = InverseLerp(-5.0, 5.0, deltaVelocity.z);
	Particles[gid].Color.xyz = vec3(percentXVelocity, percentYVelocity, percentZVelocity);
	Particles[gid].Position.xyz = deltaPosition;
	Velocities[gid].xyz = deltaVelocity;
}
