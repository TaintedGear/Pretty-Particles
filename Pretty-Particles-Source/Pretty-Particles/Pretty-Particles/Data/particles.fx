//CBUFFERS
cbuffer cbWorld : register(b0)
{
	float4x4 world;
};
cbuffer cbView : register(b1)
{
	float4x4 view;
};
cbuffer cbProj : register(b2)
{
	float4x4 proj;
};

cbuffer cbMassPoint : register(b3)
{
	float3 massPos;
	float pointMass;
	float particleMass;
	float dragForce;
	float gravityConst;
	float forceDistance;
	float dt;
	float3 pad;
};

//STRUCTS
struct VS_IN
{
	float4 pos : SV_POSITION;
	float3 vel : VELOCITY;
	float4 col : COLOR;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 vel : VELOCITY;
	float4 col : COLOR;
};

//RENDER SHADERS

PS_IN VS_MAIN(VS_IN vIn)
{
	PS_IN pIn;

	pIn.pos = vIn.pos;

	pIn.pos = mul(vIn.pos, world);
	pIn.pos = mul(pIn.pos, view);
	pIn.pos = mul(pIn.pos, proj);

	pIn.vel = vIn.vel;
	pIn.col = vIn.col;

	return pIn;
}

float4 PS_MAIN(PS_IN pIn) : SV_TARGET
{
	return pIn.col;
}

//UPDATE SHADERS

struct GS_IN
{
	float4 pos : SV_POSITION;
	float3 vel : VELOCITY;
	float4 col : COLOR;
};

GS_IN VS_MAIN_SO(VS_IN vIn)
{
	GS_IN gIn;

	gIn.pos = vIn.pos;
	gIn.vel = vIn.vel;
	gIn.col = vIn.col;

	//Find the distance between the particle position to the mass point position
	float r = distance((float3)gIn.pos, massPos);
	r = r*r;

	//1.0f is the mass of the particles
	float m = particleMass * pointMass;

	float f = 0.0f;

	//newtons universal law of gravitation
	if (r >= forceDistance)
	{
		
		f = gravityConst * (m / r);
	}

	//direction towards masspoint
	float3 dir = massPos - (float3)gIn.pos;
	dir = normalize(dir);

	//verlet physics step
	float3 acc = float3(0.0f, 0.0f, 0.0f);
	float3 newVel = float3(0.0f, 0.0f, 0.0f);
	float3 newPos = float3(0.0f, 0.0f, 0.0f);
	float3 force = float3(0.0f, 0.0f, 0.0f);

	force = dir * f;
	acc = force / 1.0f;
	newVel = vIn.vel + acc;

	//Apply resistance
	newVel = newVel * dragForce;

	newPos = (float3)vIn.pos + (vIn.vel + newVel) * 0.5f /** dt*/;

	gIn.pos = (float4(newPos, 1.0f));
	gIn.vel = newVel;

	//Set the colour to that of the vel
	gIn.col = float4(1.0f, 1.0f, 1.0f, 1.0f) + (float4(gIn.vel, 1.0f));

	return gIn;
}

//Stream the vertex out to the stream buffer
[maxvertexcount(1)]
void GS_MAIN_SO(point GS_IN gIn[1], inout PointStream<GS_IN> output)
{
	GS_IN pIn;

	pIn.pos = gIn[0].pos;
	pIn.vel = gIn[0].vel;
	pIn.col = gIn[0].col;

	output.Append(pIn);
}