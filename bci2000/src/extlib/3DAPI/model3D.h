////////////////////////////////////////////////////////////////////////////////
// $Id: model3D.h 1723 2008-01-16 17:46:33Z mellinger $
// Authors: shzeng, schalk@wadsworth.org
// Description: A primObj representing a 3D model.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef MODEL_3D_H
#define MODEL_3D_H

#include "primObj.h"
#include "component.h"

class model3D : public sceneObj
{
private:
        //3ds file name
        std::string mFile;
        t3DModel    mModelData;

public:
        enum { primitiveID = 9 };
        model3D( Scene& inScene, const std::string& modelFile )
          : sceneObj( inScene ), mFile( modelFile ) {}
        virtual ~model3D();
        const std::string& getModelFile() const
          { return mFile; }

        virtual void onInitialize();
        virtual void onCleanup();
        virtual void onRender() const;
};

#endif // MODEL_3D_H
