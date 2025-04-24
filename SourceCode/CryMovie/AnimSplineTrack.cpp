////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   animtrack.cpp
//  Version:     v1.00
//  Created:     22/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AnimSplineTrack.h"


//////////////////////////////////////////////////////////////////////////
//! Specialize for single float track.
template <> TAnimTcbTrack<float>::TAnimTcbTrack()
{
	AllocSpline();
	m_flags = 0;
	m_defaultValue = 0;
}
template <> void TAnimTcbTrack<float>::GetValue( float time,float &value ) { m_spline->interpolate(time,value); }
template <> EAnimTrackType TAnimTcbTrack<float>::GetType() { return ATRACK_TCB_FLOAT; }
template <> EAnimValue TAnimTcbTrack<float>::GetValueType() { return AVALUE_FLOAT; }
template <> void TAnimTcbTrack<float>::SetValue( float time,const float &value,bool bDefault )
{
	if (!bDefault)
	{
		ITcbKey key;
		key.SetValue( value );
		SetKeyAtTime( time,&key );
	}
	else
		m_defaultValue = value;
}

//////////////////////////////////////////////////////////////////////////
template<> void TAnimTcbTrack<float>::GetKeyInfo( int index,const char* &description,float &duration )
{
	duration = 0;

	static char str[64];
	description = str;
	assert( index >= 0 && index < GetNumKeys() );
	Spline::key_type &k = m_spline->key(index);
	sprintf( str,"%g",k.value );
}

//////////////////////////////////////////////////////////////////////////
//! Specialize for Vector track.
template <> TAnimTcbTrack<Vec3>::TAnimTcbTrack()
{
	AllocSpline();
	m_flags = 0;
	m_defaultValue = Vec3(0,0,0);
}
template <> void TAnimTcbTrack<Vec3>::GetValue( float time,Vec3 &value ) { m_spline->interpolate(time,value); }
template <> EAnimTrackType TAnimTcbTrack<Vec3>::GetType() { return ATRACK_TCB_VECTOR; }
template <> EAnimValue TAnimTcbTrack<Vec3>::GetValueType() { return AVALUE_VECTOR; }
template <> void TAnimTcbTrack<Vec3>::SetValue( float time,const Vec3 &value,bool bDefault )
{
	if (!bDefault)
	{
		ITcbKey key;
		key.SetValue( value );
		SetKeyAtTime( time,&key );
	}
	else
		m_defaultValue = value;
}

//////////////////////////////////////////////////////////////////////////
template <> void TAnimTcbTrack<Vec3>::GetKeyInfo( int index,const char* &description,float &duration )
{
	duration = 0;

	static char str[64];
	description = str;

	assert( index >= 0 && index < GetNumKeys() );
	Spline::key_type &k = m_spline->key(index);
	sprintf( str,"%g,%g,%g",k.value[0],k.value[1],k.value[2] );
}

//////////////////////////////////////////////////////////////////////////
//! Specialize for Quaternion track.
//! Spezialize spline creation for quaternion.
template <> TAnimTcbTrack<Quat>::TAnimTcbTrack()
{
	m_spline = new TCBQuatSpline;
	m_flags = 0;
	m_defaultValue.SetIdentity();
}

template <> void TAnimTcbTrack<Quat>::GetValue( float time,Quat &value ) { m_spline->interpolate(time,value); }
template <> EAnimTrackType TAnimTcbTrack<Quat>::GetType() { return ATRACK_TCB_QUAT; }
template <> EAnimValue TAnimTcbTrack<Quat>::GetValueType() { return AVALUE_QUAT; }
template <> void TAnimTcbTrack<Quat>::SetValue( float time,const Quat &value,bool bDefault )
{
	if (!bDefault)
	{
		ITcbKey key;
		key.SetValue( value );
		SetKeyAtTime( time,&key );
	}
	else
		m_defaultValue = value;
}

//////////////////////////////////////////////////////////////////////////
template <> void TAnimTcbTrack<Quat>::GetKeyInfo( int index,const char* &description,float &duration )
{
	duration = 0;

	static char str[64];
	description = str;

	assert( index >= 0 && index < GetNumKeys() );
	Spline::key_type &k = m_spline->key(index);
	Vec3 Angles=RAD2DEG(Ang3::GetAnglesXYZ(Matrix33(k.value)));
	sprintf( str,"%g,%g,%g",Angles.x, Angles.y, Angles.z );
}