//BG2 - Tjoppen - CBG2SmokeEmitter
class CBG2SmokeEmitter : public CSimpleEmitter
{
public:
	CBG2SmokeEmitter( const char *pDebugName );
	static CSmartPtr<CBG2SmokeEmitter>	Create( const char *pDebugName );
	virtual void UpdateVelocity( SimpleParticle *pParticle, float timeDelta );

private:
	CBG2SmokeEmitter( const CBG2SmokeEmitter & );
	Vector	m_vDrift;
};


CBG2SmokeEmitter::CBG2SmokeEmitter( const char *pDebugName ) : CSimpleEmitter( pDebugName )
{
	//velocity will converge to around (|m_vDrift| / -DROPOFF) for small timeDeltas

#define DROPOFF	-1.5f	//exponential dropoff rate of velocity

	m_vDrift = Vector(	random->RandomFloat( -10, 10 ),
						random->RandomFloat( -10, 10 ),
						random->RandomFloat( 15, 30 ) );
}


CSmartPtr<CBG2SmokeEmitter> CBG2SmokeEmitter::Create( const char *pDebugName )
{
	return new CBG2SmokeEmitter( pDebugName );
}


void CBG2SmokeEmitter::UpdateVelocity( SimpleParticle *pParticle, float timeDelta )
{
	//slow down...
	pParticle->m_vecVelocity *= expf( DROPOFF * timeDelta );

	//..and drift upwards and sideways as dictated by m_vDrift
	pParticle->m_vecVelocity += m_vDrift * timeDelta;
}
//