#pragma once
#include "BaseMath.hpp"
#include "GameObject.h"
#include "MyRandom.h"
#include "Timer.h"
#include "SoundManager.h"
#include "DxManager.h"
#include "ObjectManager.h"
#include "Line.h"

class Particle : public GameObject
{
public:
	Particle( const std::wstring _name, GameObject* _cam, EObject _type, bool* _isRainbow );
	virtual ~Particle() = default;

public:
	void Init() override;
	void Frame() override;

	D3DXVECTOR4 defaultColor;
private:
	friend class ParticleSystem;

	bool& isRainbow;
	D3DXVECTOR4 rainbowColor;

	D3DXVECTOR3 startPos;
	D3DXVECTOR3	direction = Vector3::Backward;

	float moveSpeed;
	float respawnDelay;
	float timer;
};

class ParticleSystem : public GameObject
{
public:
	ParticleSystem( const std::wstring _name, GameObject* _cam, EObject _type, UINT _maxParticle ) : GameObject( _name, _cam, _type ), maxParticle( _maxParticle ) { }
	virtual ~ParticleSystem() = default;

public:
	void Init() override;
	void Frame() override;
	void Render( ID3D11DeviceContext* context ) override;
	void Release() override;

private:
	std::vector<InstanceData> instanceData;
	ID3D11Buffer* instanceBuffer;
	bool isRainbow = false;
	UINT maxParticle;
};