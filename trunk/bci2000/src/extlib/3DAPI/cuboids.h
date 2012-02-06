////////////////////////////////////////////////////////////////////////////////
// $Id: cuboids.h 1723 2008-01-16 17:46:33Z mellinger $
// Authors: shzeng, schalk@wadsworth.org
// Description: Two classes that represent cuboids.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef CUBOIDS_H
#define CUBOIDS_H

#include "primObj.h"
#include "component.h"
#include "halfSpace.h"

// A cuboid
class cuboid : public sceneObj
{
private:
        GLfloat mWidth,
                mHeight,
                mDepth;
        bool    mFaceVisible[6];

public:
        enum { primitiveID = 2 };
/*******************************CONSTRUCTOR************************************/
        cuboid( Scene& inScene, GLfloat x=0, GLfloat y=0, GLfloat z=0, GLfloat w=1, GLfloat h=1, GLfloat d=1)
        : sceneObj( inScene ), mHeight(h), mWidth(w), mDepth(d)
        {
          setOrigin(x,y,z);
          for(int i = 0; i < 6; ++i)
            mFaceVisible[i] = true;
        }
/**************************Modifiers & Accessors*******************************/
        void    setDimensions( GLfloat width, GLfloat height, GLfloat depth )
                { mWidth = width; mHeight = height; mDepth = depth; }
        GLfloat getWidth() const                {return mWidth;}
        GLfloat getHeight() const               {return mHeight;}
        GLfloat getDepth() const                {return mDepth;}
        void    setFaceVisible( int i, bool b ) {mFaceVisible[i%6]=b;}
        bool    getFaceVisible( int i ) const   {return mFaceVisible[i%6];}
        
        virtual CVector3  getVertex( int ) const;
        virtual halfSpace getHalfSpace( int ) const;

        virtual testResult isConvex() const     { return true_; }
        virtual testResult contains( const CVector3& ) const;
        virtual testResult intersectsVolume( const geomObj& ) const;

protected:
        virtual void onRender() const;
        void doRender( bool invert ) const;

private:
        void drawFace( int, bool ) const;

};//cuboid

// The inversion of a cuboid (all space is part of it except the cuboid's volume)
class invertedCuboid : public cuboid
{
public:
        enum { primitiveID = 3 };
        invertedCuboid(Scene& inScene, GLfloat x=0, GLfloat y=0, GLfloat z=0, GLfloat w=1, GLfloat h=1, GLfloat d=1)
        : cuboid( inScene, x, y, z, w, h, d ) {}
        virtual halfSpace getHalfSpace( int ) const;
        virtual testResult isConvex() const { return false_; }
        virtual testResult contains( const CVector3& ) const;
        virtual testResult intersectsVolume( const geomObj& ) const;

protected:
        virtual void onRender() const;
};

#endif // CUBOIDS_H
