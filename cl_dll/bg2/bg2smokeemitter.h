//BG2 - Tjoppen - CBG2SmokeEmitter
class CBG2SmokeEmitter : public CSimpleEmitter
{
public:
							CBG2SmokeEmitter( const char *pDebugName );
	static CSmartPtr<CBG2SmokeEmitter>	Create( const char *pDebugName );
	virtual void			UpdateVelocity( SimpleParticle *pParticle, float timeDelta );

private:
							CBG2SmokeEmitter( const CBG2SmokeEmitter & );
};


CBG2SmokeEmitter::CBG2SmokeEmitter( const char *pDebugName ) : CSimpleEmitter( pDebugName )
{
}


CSmartPtr<CBG2SmokeEmitter> CBG2SmokeEmitter::Create( const char *pDebugName )
{
	return new CBG2SmokeEmitter( pDebugName );
}


void CBG2SmokeEmitter::UpdateVelocity( SimpleParticle *pParticle, float timeDelta )
{
	//slow down...
	pParticle->m_vecVelocity *= expf( -2.5f * timeDelta );

	//..and drift upwards
	pParticle->m_vecVelocity.z += 25.f * timeDelta;	//TWEAKME


	/*float	speed = VectorNormalize( pParticle->m_vecVelocity );
	Vector	offset;

	speed -= ( 1.0f * timeDelta );

	offset.Random( -0.025f, 0.025f );
	offset[2] = 0.0f;

	pParticle->m_vecVelocity += offset;
	VectorNormalize( pParticle->m_vecVelocity );

	pParticle->m_vecVelocity *= speed;*/
}
//