#ifndef ABCSHAPE_ABCSHAPE_H_
#define ABCSHAPE_ABCSHAPE_H_

#include "AlembicScene.h"
#include "GeometryData.h"

#include <maya/MPxSurfaceShape.h>
#include <maya/MPxSurfaceShapeUI.h>
#include <maya/MBoundingBox.h>
#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MPlug.h>
#include <maya/MDataHandle.h>
#include <maya/MDGContext.h>
#include <maya/MTime.h>
#include <maya/M3dView.h>


class AbcShape : public MPxSurfaceShape
{
public:
    
    static const MTypeId ID;
    
    static MObject aFilePath;
    static MObject aObjectExpression;
    static MObject aDisplayMode;
    static MObject aTime;
    static MObject aSpeed;
    static MObject aStartFrame;
    static MObject aEndFrame;
    static MObject aOffset;
    static MObject aCycleType;
    static MObject aIgnoreXforms;
    static MObject aIgnoreInstances;
    static MObject aIgnoreVisibility;
    static MObject aNumShapes;
    static MObject aPointWidth;
    static MObject aLineWidth;
    
    static void* creator();
    static MStatus initialize();
    
    enum CycleType
    {
        CT_hold = 0,
        CT_loop,
        CT_reverse,
        CT_bounce
    };
    
    enum DisplayMode
    {
        DM_box = 0,
        DM_boxes,
        DM_points,
        DM_geometry
    };
    
public:
    
    AbcShape();
    virtual ~AbcShape();
    
    virtual void postConstructor();
    
    virtual MStatus compute(const MPlug &, MDataBlock &);
    
    virtual bool isBounded() const;
    virtual MBoundingBox boundingBox() const;
    
    virtual bool getInternalValueInContext(const MPlug &plug,
                                           MDataHandle &handle,
                                           MDGContext &ctx);
    virtual bool setInternalValueInContext(const MPlug &plug,
                                           const MDataHandle &handle,
                                           MDGContext &ctx);
    virtual void copyInternalData(MPxNode *source);
    
    inline AlembicScene* scene() { return mScene; }
    inline const SceneGeometryData* sceneGeometry() const { return &mGeometry; }
    inline bool ignoreInstances() const { return mIgnoreInstances; }
    inline bool ignoreTransforms() const { return mIgnoreTransforms; }
    inline bool ignoreVisibility() const { return mIgnoreVisibility; }
    inline DisplayMode displayMode() const { return mDisplayMode; }
    inline float lineWidth() const { return mLineWidth; }
    inline float pointWidth() const { return mPointWidth; }
    inline unsigned int numShapes() const { return mNumShapes; }

private:
    
    double getFPS() const;
    double computeAdjustedTime(double inputTime, double speed, double timeOffset) const;
    double computeRetime(double inputTime, double firstTime, double lastTime, CycleType cycleType) const;
    double getSampleTime() const;
    
    void printInfo(bool detailed=false) const;
    void printSceneBounds() const;
    
    void pullInternals();
    
    bool updateInternals(const std::string &filePath, const std::string &objectExpression, double st, bool forceGeometrySampling=false);
    void updateWorld();
    void updateSceneBounds();
    void updateShapesCount();
    void updateGeometry();
    
private:
    
    std::string mFilePath;
    std::string mObjectExpression;
    MTime mTime;
    double mOffset;
    double mSpeed;
    CycleType mCycleType;
    double mStartFrame;
    double mEndFrame;
    double mSampleTime;
    bool mIgnoreInstances;
    bool mIgnoreTransforms;
    bool mIgnoreVisibility;
    AlembicScene *mScene;
    DisplayMode mDisplayMode;
    SceneGeometryData mGeometry;
    unsigned int mNumShapes;
    float mPointWidth;
    float mLineWidth;
};

class AbcShapeUI : public MPxSurfaceShapeUI
{
public:
    
    static void *creator();
    
public:
    
    enum DrawToken
    {
        kDrawBox = 0,
        kDrawPoints,
        kDrawGeometry
    };
    
    AbcShapeUI();
    virtual ~AbcShapeUI();
    
    virtual void getDrawRequests(const MDrawInfo &info,
                                 bool objectAndActiveOnly,
                                 MDrawRequestQueue &queue);
    
    virtual void draw(const MDrawRequest &request, M3dView &view) const;
    
    virtual bool select(MSelectInfo &selectInfo,
                        MSelectionList &selectionList,
                        MPointArray &worldSpaceSelectPts) const;

private:
    
    void drawBox(AbcShape *shape, const MDrawRequest &request, M3dView &view) const;
    void drawPoints(AbcShape *shape, const MDrawRequest &request, M3dView &view) const;
    void drawGeometry(AbcShape *shape, const MDrawRequest &request, M3dView &view) const;
};

#endif
