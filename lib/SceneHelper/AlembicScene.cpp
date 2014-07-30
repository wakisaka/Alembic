#include "AlembicScene.h"
//#include <maya/MGlobal.h>
#include <algorithm>

// ---

static std::string gsEmptyString("");

// ---

AlembicNode::AlembicNode()
   : mParent(0)
   , mMasterPath("")
   , mMaster(0)
   , mInstanceNumber(0)
   , mType(AlembicNode::TypeGeneric)
   , mInheritsTransform(true)
   , mVisible(true)
{
   mSelfBounds.makeEmpty();
   mChildBounds.makeEmpty();
   mSelfMatrix.makeIdentity();
   mWorldMatrix.makeIdentity();
}

AlembicNode::AlembicNode(Alembic::Abc::IObject iObj, AlembicNode *parent)
   : mIObj(iObj)
   , mParent(parent)
   , mMasterPath("")
   , mMaster(0)
   , mInstanceNumber(0)
   , mType(AlembicNode::TypeGeneric)
   , mInheritsTransform(true)
   , mVisible(true)
{
   mSelfBounds.makeEmpty();
   mChildBounds.makeEmpty();
   mSelfMatrix.makeIdentity();
   mWorldMatrix.makeIdentity();
   
   if (mIObj.valid())
   {
      if (mIObj.isInstanceRoot())
      {
         mMasterPath = mIObj.instanceSourcePath();
      }
      else if (mIObj.isInstanceDescendant())
      {
         mMasterPath = mIObj.getHeader().getFullName();
      }
      
      mChildren.resize(mIObj.valid() ? mIObj.getNumChildren() : 0);
      
      for (Array::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
      {
         AlembicNode *c = Wrap(mIObj.getChild(it - mChildren.begin()), this);
         if (c)
         {
            *it = c;
            mChildByName[c->name()] = c;
         }
      }
   }
}

AlembicNode::~AlembicNode()
{
   while (mInstances.size() > 0)
   {
      AlembicNode *i = mInstances.back();
      mInstances.pop_back();
      i->resetMaster();
   }
   
   if (mMaster)
   {
      mMaster->removeInstance(this);
      mMaster = 0;
   }
   
   mChildByName.clear();
   
   while (mChildren.size() > 0)
   {
      AlembicNode *c = mChildren.back();
      mChildren.pop_back();
      delete c;
   }
   
   mParent = 0;
}

AlembicNode::NodeType AlembicNode::type() const
{
   return mType;
}

const char* AlembicNode::typeName() const
{
   static const char* sTypeNames[] = 
   {
      "generic",
      "mesh",
      "subd",
      "points",
      "nupatch",
      "curves",
      "xform"
   };
   
   if (mType < 0 || mType >= LastType)
   {
      return "";
   }
   else
   {
      return sTypeNames[mType];
   }
}

bool AlembicNode::isValid() const
{
   return mIObj.valid();
}

const std::string& AlembicNode::name() const
{
   return (mIObj.valid() ? mIObj.getName() : gsEmptyString);
}

const std::string& AlembicNode::path() const
{
   return (mIObj.valid() ? mIObj.getFullName() : gsEmptyString);
}

std::string AlembicNode::partialPath() const
{
   std::string pp = name();
   
   if (this->isInstance() || this->numInstances() > 0)
   {
      if (mParent)
      {
         pp = mParent->partialPath() + "/" + pp;
      }
   }
   
   return pp;
}

std::string AlembicNode::formatName(const char *prefix) const
{
   if (!prefix)
   {
      return name();
   }
   else
   {
      return (prefix + name());
   }
}

void AlembicNode::formatPath(std::string &path, const char *prefix, PrefixType type, char sep) const
{
   if (prefix)
   {
      if (type == GlobalPrefix)
      {
         path = prefix + path;
      }
      else
      {
         std::string p = path;
         std::string ssep(1, sep);
         
         path = "";
         
         size_t p0 = 0;
         size_t p1 = p.find('/', p0);
         
         while (p1 != std::string::npos)
         {
            path += prefix + p.substr(p0, p1-p0) + ssep;
            p0 = p1 + 1;
            p1 = p.find('/', p0);
         }
         
         if (p0 < p.length())
         {
            path += prefix + p.substr(p0);
         }
         
         // set sep to '/' to avoid replace loop
         sep = '/';
      }
   }
   
   if (sep != '/')
   {
      size_t p = path.find('/');
      while (p != std::string::npos)
      {
         path[p] = sep;
         p = path.find('/', p);
      }
   }
}

std::string AlembicNode::formatPath(const char *prefix, PrefixType type, char sep) const
{
   std::string pp = path();
   formatPath(pp, prefix, type, sep);
   return pp;
}

std::string AlembicNode::formatPartialPath(const char *prefix, PrefixType type, char sep) const
{
   std::string pp = partialPath();
   formatPath(pp, prefix, type, sep);
   return pp;
}

AlembicNode* AlembicNode::parent()
{
   return mParent;
}

const AlembicNode* AlembicNode::parent() const
{
   return mParent;
}

bool AlembicNode::hasAncestor(const AlembicNode *node) const
{
   return (this == node || (mParent && mParent->hasAncestor(node)));
}

size_t AlembicNode::childCount() const
{
   return mChildren.size();
}

AlembicNode* AlembicNode::child(size_t i)
{
   return (i < mChildren.size() ? mChildren[i] : 0);
}

const AlembicNode* AlembicNode::child(size_t i) const
{
   return (i < mChildren.size() ? mChildren[i] : 0);
}

AlembicNode* AlembicNode::child(const char *name)
{
   Map::iterator it = mChildByName.find(name);
   return (it != mChildByName.end() ? it->second : 0);
}

const AlembicNode* AlembicNode::child(const char *name) const
{
   Map::const_iterator it = mChildByName.find(name);
   return (it != mChildByName.end() ? it->second : 0);
}

AlembicNode::Array::iterator AlembicNode::beginChild()
{
   return mChildren.begin();
}

AlembicNode::Array::iterator AlembicNode::endChild()
{
   return mChildren.end();
}

AlembicNode::Array::const_iterator AlembicNode::beginChild() const
{
   return mChildren.begin();
}

AlembicNode::Array::const_iterator AlembicNode::endChild() const
{
   return mChildren.end();
}

void AlembicNode::setSelfBounds(const Alembic::Abc::Box3d &b)
{
   if (!isInstance())
   {
      mSelfBounds = b;
   }
}

void AlembicNode::setSelfMatrix(const Alembic::Abc::M44d &m)
{
   if (!isInstance())
   {
      mSelfMatrix = m;
   }
}

void AlembicNode::setInheritsTransform(bool on)
{
   if (!isInstance())
   {
      mInheritsTransform = on;
   }
}

void AlembicNode::setVisible(bool on)
{
   if (!isInstance())
   {
      mVisible = on;
   }
}

bool AlembicNode::isVisible(bool inherited) const
{
   if (mMaster)
   {
      return mMaster->isVisible();
   }
   else if (mVisible)
   {
      return ((inherited && mParent) ? mParent->isVisible() : true);
   }
   else
   {
      return false;
   }
}

void AlembicNode::resetWorldMatrix()
{
   mWorldMatrix.makeIdentity();
}

void AlembicNode::resetChildBounds()
{
   mChildBounds.makeEmpty();
}

void AlembicNode::updateWorldMatrix()
{
   if (mParent && mInheritsTransform)
   {
      mWorldMatrix = selfMatrix() * mParent->worldMatrix();
   }
   else
   {
      mWorldMatrix = selfMatrix();
   }
}

void AlembicNode::updateChildBounds()
{
   mChildBounds.makeEmpty();
   
   if (childCount() > 0)
   {
      for (Array::iterator it = beginChild(); it != endChild(); ++it)
      {
         Alembic::Abc::Box3d bb = (*it)->childBounds();
         if (!bb.isEmpty())
         {
            mChildBounds.extendBy(bb);
         }
      }
   }
   else
   {
      Alembic::Abc::Box3d bb = selfBounds();
      if (!bb.isEmpty())
      {
         Alembic::Abc::V3d bbmin = bb.min;
         Alembic::Abc::V3d bbmax = bb.max;
         Alembic::Abc::M44d wm = worldMatrix(); // already take inheritsTransfor into account
         
         mChildBounds.extendBy(Alembic::Abc::V3d(bbmin.x, bbmin.y, bbmin.z) * wm);
         mChildBounds.extendBy(Alembic::Abc::V3d(bbmin.x, bbmin.y, bbmax.z) * wm);
         mChildBounds.extendBy(Alembic::Abc::V3d(bbmin.x, bbmax.y, bbmin.z) * wm);
         mChildBounds.extendBy(Alembic::Abc::V3d(bbmin.x, bbmax.y, bbmax.z) * wm);
         mChildBounds.extendBy(Alembic::Abc::V3d(bbmax.x, bbmin.y, bbmin.z) * wm);
         mChildBounds.extendBy(Alembic::Abc::V3d(bbmax.x, bbmin.y, bbmax.z) * wm);
         mChildBounds.extendBy(Alembic::Abc::V3d(bbmax.x, bbmax.y, bbmin.z) * wm);
         mChildBounds.extendBy(Alembic::Abc::V3d(bbmax.x, bbmax.y, bbmax.z) * wm);
      }
   }
}

bool AlembicNode::isInstance() const
{
   return (mMasterPath.length() > 0);
}

size_t AlembicNode::instanceNumber() const
{
   return mInstanceNumber;
}

void AlembicNode::addInstance(AlembicNode *node)
{
   Array::iterator it = std::find(mInstances.begin(), mInstances.end(), node);
   if (it == mInstances.end())
   {
      mInstances.push_back(node);
      node->mInstanceNumber = mInstances.size();
   }
}

void AlembicNode::removeInstance(AlembicNode *node)
{
   Array::iterator it = std::find(mInstances.begin(), mInstances.end(), node);
   if (it != mInstances.end())
   {
      Array::iterator rit = it;
      ++rit;
      
      for (; rit != mInstances.end(); ++rit)
      {
         (*rit)->mInstanceNumber -= 1;
      }
      
      mInstances.erase(it);
   }
}

void AlembicNode::resetMaster()
{
   mMaster = 0;
}

AlembicNode* AlembicNode::find(const char *path)
{
   if (!path)
   {
      return 0;
   }
   
   const char *p = strchr(path, '/');
   
   if (p)
   {
      std::string n(path, p - path);
      AlembicNode *c = child(n);
      return (c ? c->find(p + 1) : 0);
   }
   else
   {
      return child(path);
   }
}

const AlembicNode* AlembicNode::find(const char *path) const
{
   if (!path)
   {
      return 0;
   }
   
   const char *p = strchr(path, '/');
   
   if (p)
   {
      std::string n(path, p - path);
      const AlembicNode *c = child(n);
      return (c ? c->find(p + 1) : 0);
   }
   else
   {
      return child(path);
   }
}

void AlembicNode::resolveInstances(AlembicScene *scene)
{
   if (isInstance() && !mMaster)
   {
      mMaster = scene->find(mMasterPath.c_str());
      
      if (mMaster)
      {
         mMaster->addInstance(this);
      }
   }
   
   for (Array::iterator it=mChildren.begin(); it!=mChildren.end(); ++it)
   {
      (*it)->resolveInstances(scene);
   }
}

size_t AlembicNode::numInstances() const
{
   return mInstances.size();
}

const std::string& AlembicNode::masterPath() const
{
   return mMasterPath;
}

AlembicNode* AlembicNode::master()
{
   return mMaster;
}

const AlembicNode* AlembicNode::master() const
{
   return mMaster;
}

AlembicNode* AlembicNode::instance(size_t i)
{
   return (i < mInstances.size() ? mInstances[i] : 0);
}

const AlembicNode* AlembicNode::instance(size_t i) const
{
   return (i < mInstances.size() ? mInstances[i] : 0);
}
   
Alembic::Abc::IObject AlembicNode::object() const
{
   return mIObj;
}

/*
void AlembicNode::getSamplesInfo(double iFrame,
                                 Alembic::AbcCoreAbstract::TimeSamplingPtr iTime,
                                 size_t numSamps,
                                 Alembic::AbcCoreAbstract::index_t &oFloorIndex,
                                 Alembic::AbcCoreAbstract::index_t &oCeilIndex,
                                 double &oFloorFrame,
                                 double &oCeilFrame,
                                 double &oBlend) const
{
    if (numSamps == 0)
    {
        numSamps = 1;
    }

    std::pair<Alembic::AbcCoreAbstract::index_t, double> floorTime = iTime->getFloorIndex(iFrame, numSamps);

    oFloorIndex = floorTime.first;
    oFloorFrame = floorTime.second;
    oCeilIndex = oFloorIndex;
    oCeilFrame = oFloorFrame;
    
    if (fabs(iFrame - floorTime.second) < 0.0001)
    {
        oBlend = 0.0;
        return;
    }

    std::pair<Alembic::AbcCoreAbstract::index_t, double> ceilTime = iTime->getCeilIndex(iFrame, numSamps);

    if (oFloorIndex == ceilTime.first)
    {
        oBlend = 0.0;
        return;
    }

    oCeilIndex = ceilTime.first;
    oCeilFrame = ceilTime.second;

    double blend = (iFrame - floorTime.second) / (ceilTime.second - floorTime.second);

    if (fabs(1.0 - blend) < 0.0001)
    {
        oFloorIndex = oCeilIndex;
        oFloorFrame = oCeilFrame;
        oBlend = 0.0;
    }
    else
    {
        oBlend = blend;
    }
}
*/

// ---

AlembicNode* AlembicNode::Wrap(Alembic::Abc::IObject iObj, AlembicNode *iParent)
{
   if (!iObj.valid())
   {
      return 0;
   }
   
   if (iObj.isInstanceDescendant())
   {
      return new AlembicNode(iObj, iParent);
   }
   else if (Alembic::AbcGeom::IXform::matches(iObj.getHeader()))
   {
      Alembic::AbcGeom::IXform iXform(iObj, Alembic::Abc::kWrapExisting);
      return new AlembicXform(iXform, iParent);
   }
   else if (Alembic::AbcGeom::IPoints::matches(iObj.getHeader()))
   {
      Alembic::AbcGeom::IPoints iPoints(iObj, Alembic::Abc::kWrapExisting);
      return new AlembicPoints(iPoints, iParent);
   }
   else if (Alembic::AbcGeom::IPolyMesh::matches(iObj.getHeader()))
   {
      Alembic::AbcGeom::IPolyMesh iMesh(iObj, Alembic::Abc::kWrapExisting);
      return new AlembicMesh(iMesh, iParent);
   }
   else if (Alembic::AbcGeom::ISubD::matches(iObj.getHeader()))
   {
      Alembic::AbcGeom::ISubD iSubD(iObj, Alembic::Abc::kWrapExisting);
      return new AlembicSubD(iSubD, iParent);
   }
   else if (Alembic::AbcGeom::INuPatch::matches(iObj.getHeader()))
   {
      Alembic::AbcGeom::INuPatch iNuPatch(iObj, Alembic::Abc::kWrapExisting);
      return new AlembicNuPatch(iNuPatch, iParent);
   }
   else if (Alembic::AbcGeom::ICurves::matches(iObj.getHeader()))
   {
      Alembic::AbcGeom::ICurves iCurves(iObj, Alembic::Abc::kWrapExisting);
      return new AlembicCurves(iCurves, iParent);
   }
   else
   {
      return 0;
   }
}

// ---

AlembicScene::AlembicScene(Alembic::Abc::IArchive iArch)
   : AlembicNode(iArch.getTop()), mArchive(iArch), mFiltered(false)
{
   resolveInstances(this);
}

AlembicScene::~AlembicScene()
{
}

void AlembicScene::setFilter(const std::string &expr)
{
   mFilteredNodes.clear();
   mFiltered = (expr.length() > 0);
   
   if (mFiltered)
   {
      try
      {
         mFilter.assign(expr, boost::regex::perl);
         filter(this);
      }
      catch (std::exception &e)
      {
         std::cout << "Invalid expression: \"" << expr << "\": " << e.what() << std::endl;
         mFiltered = false;
      }
   }
}

bool AlembicScene::isFiltered(AlembicNode *node) const
{
   return (!mFiltered || mFilteredNodes.find(node) != mFilteredNodes.end());
}

bool AlembicScene::filter(AlembicNode *node)
{
   size_t numChildren = 0;
   
   bool matched = boost::regex_search(node->path(), mFilter);
   
   for (Array::iterator it=node->beginChild(); it!=node->endChild(); ++it)
   {
      if (filter(*it))
      {
         ++numChildren;
         mFilteredNodes.insert(*it);
      }
   }
   
   // Even if result is 0, if any of the children was filtered successfully, keep this node
   return (numChildren > 0 || matched);
}

AlembicNode::Set::iterator AlembicScene::beginFiltered()
{
   return mFilteredNodes.begin();
}

AlembicNode::Set::iterator AlembicScene::endFiltered()
{
   return mFilteredNodes.end();
}

AlembicNode::Set::const_iterator AlembicScene::beginFiltered() const
{
   return mFilteredNodes.begin();
}

AlembicNode::Set::const_iterator AlembicScene::endFiltered() const
{
   return mFilteredNodes.end();
}

const AlembicNode* AlembicScene::find(const char *path) const
{
   if (!path || strlen(path) == 0 || path[0] != '/')
   {
      return 0;
   }
   else
   {
      return AlembicNode::find(path + 1);
   }
}

AlembicNode* AlembicScene::find(const char *path)
{
   if (!path || strlen(path) == 0 || path[0] != '/')
   {
      return 0;
   }
   else
   {
      return AlembicNode::find(path + 1);
   }
}
