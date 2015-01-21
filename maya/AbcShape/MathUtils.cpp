#include "MathUtils.h"

Plane::Plane()
   : mNormal(0, 1, 0), mDist(0.0)
{
}

Plane::Plane(const V3d &n, double d)
   : mNormal(n), mDist(d)
{
}

Plane::Plane(const V3d &p0, const V3d &p1, const V3d &p2, const V3d &above)
{
   set(p0, p1, p2, above);
}

Plane::Plane(const Plane &rhs)
   : mNormal(rhs.mNormal), mDist(rhs.mDist)
{
}

Plane& Plane::operator=(const Plane &rhs)
{
   if (this != &rhs)
   {
      mNormal = rhs.mNormal;
      mDist = rhs.mDist;
   }
   return *this;
}

void Plane::set(const V3d &n, double d)
{
   mNormal = n;
   mDist = d;
}

void Plane::set(const V3d &p0, const V3d &p1, const V3d &p2, const V3d &above)
{
   V3d u = p2 - p1;
   V3d v = p0 - p1;
   mNormal = u.cross(v).normalize();
   mDist = -(mNormal.dot(p1));
   
   if (distanceTo(above) < 0.0)
   {
      mNormal.negate();
      mDist = -mDist;
   }
}

double Plane::distanceTo(const V3d &P) const
{
   return (P.dot(mNormal) + mDist);
}

// ---

Frustum::Frustum()
{
   mPlanes[  Left].set(V3d( 1,  0,  0), 0);
   mPlanes[ Right].set(V3d(-1,  0,  0), 0);
   mPlanes[   Top].set(V3d( 0, -1,  0), 0);
   mPlanes[Bottom].set(V3d( 0,  1,  0), 0);
   mPlanes[  Near].set(V3d( 0,  0, -1), 0);
   mPlanes[   Far].set(V3d( 0,  0,  1), 0);
}

Frustum::Frustum(const M44d &projViewInv)
{
   setup(projViewInv);
}

Frustum::Frustum(const Frustum &rhs)
{
   for (int i=0; i<6; ++i)
   {
      mPlanes[i] = rhs.mPlanes[i];
   }
}

Frustum& Frustum::operator=(const Frustum &rhs)
{
   if (this != &rhs)
   {
      for (int i=0; i<6; ++i)
      {
         mPlanes[i] = rhs.mPlanes[i];
      }
   }
   return *this;
}

void Frustum::setup(const M44d &projViewInv)
{
   V3d ltn, rtn, lbn, rbn, ltf, rtf, lbf, rbf;
   
   projViewInv.multVecMatrix(V3d(-1,  1, -1), ltn);
   projViewInv.multVecMatrix(V3d( 1,  1, -1), rtn);
   projViewInv.multVecMatrix(V3d(-1, -1, -1), lbn);
   projViewInv.multVecMatrix(V3d( 1, -1, -1), rbn);
   
   projViewInv.multVecMatrix(V3d(-1,  1,  1), ltf);
   projViewInv.multVecMatrix(V3d( 1,  1,  1), rtf);
   projViewInv.multVecMatrix(V3d(-1, -1,  1), lbf);
   projViewInv.multVecMatrix(V3d( 1, -1,  1), rbf);
   
   // Build planes so that their normal is pointing inside the frustum
   mPlanes[  Left].set(ltn, lbn, lbf, rbn);
   mPlanes[ Right].set(rtn, rbn, rbf, lbn);
   mPlanes[   Top].set(ltn, rtn, rtf, rbf);
   mPlanes[Bottom].set(lbn, rbn, rbf, rtf);
   mPlanes[  Near].set(ltn, rtn, rbn, rbf);
   mPlanes[   Far].set(ltf, rtf, rbf, rbn);
}

bool Frustum::isPointInside(const V3d &p) const
{
   for (int i=0; i<6; ++i)
   {
      if (mPlanes[i].distanceTo(p) < 0.0)
      {
         return false;
      }
   }
   return true;
}

bool Frustum::isPointOutside(const V3d &p) const
{
   for (int i=0; i<6; ++i)
   {
      if (mPlanes[i].distanceTo(p) < 0.0)
      {
         return true;
      }
   }
   return false;
}

bool Frustum::isBoxTotallyInside(const Box3d &b) const
{
   V3d corner;
   
   corner.setValue(b.min);
   if (!isPointInside(corner))
   {
      return false;
   }
   
   corner.setValue(b.min.x, b.min.y, b.max.z);
   if (!isPointInside(corner))
   {
      return false;
   }
   
   corner.setValue(b.min.x, b.max.y, b.min.z);
   if (!isPointInside(corner))
   {
      return false;
   }
   
   corner.setValue(b.min.x, b.max.y, b.max.z);
   if (!isPointInside(corner))
   {
      return false;
   }
   
   corner.setValue(b.max.x, b.min.y, b.min.z);
   if (!isPointInside(corner))
   {
      return false;
   }
   
   corner.setValue(b.max.x, b.min.y, b.max.z);
   if (!isPointInside(corner))
   {
      return false;
   }
   
   corner.setValue(b.max.x, b.max.y, b.min.z);
   if (!isPointInside(corner))
   {
      return false;
   }
   
   corner.setValue(b.max);
   if (!isPointInside(corner))
   {
      return false;
   }
   
   return true;
}

bool Frustum::isBoxTotallyOutside(const Box3d &b) const
{
   V3d corner;
   
   double dist;
   double dists[6] = {0, 0, 0, 0, 0, 0};
   
   for (int i=0; i<6; ++i)
   {
      dists[i] = mPlanes[i].distanceTo(b.min);
   }
   
   corner.setValue(b.min.x, b.min.y, b.max.z);
   for (int i=0; i<6; ++i)
   {
      dist = mPlanes[i].distanceTo(corner);
      if (dist * dists[i] < 0.0)
      {
         return false;
      }
   }
   
   corner.setValue(b.min.x, b.max.y, b.min.z);
   for (int i=0; i<6; ++i)
   {
      dist = mPlanes[i].distanceTo(corner);
      if (dist * dists[i] < 0.0)
      {
         return false;
      }
   }
   
   corner.setValue(b.min.x, b.max.y, b.max.z);
   for (int i=0; i<6; ++i)
   {
      dist = mPlanes[i].distanceTo(corner);
      if (dist * dists[i] < 0.0)
      {
         return false;
      }
   }
   
   corner.setValue(b.max.x, b.min.y, b.min.z);
   for (int i=0; i<6; ++i)
   {
      dist = mPlanes[i].distanceTo(corner);
      if (dist * dists[i] < 0.0)
      {
         return false;
      }
   }
   
   corner.setValue(b.max.x, b.min.y, b.max.z);
   for (int i=0; i<6; ++i)
   {
      dist = mPlanes[i].distanceTo(corner);
      if (dist * dists[i] < 0.0)
      {
         return false;
      }
   }
   
   corner.setValue(b.max.x, b.max.y, b.min.z);
   for (int i=0; i<6; ++i)
   {
      dist = mPlanes[i].distanceTo(corner);
      if (dist * dists[i] < 0.0)
      {
         return false;
      }
   }
   
   corner.setValue(b.max);
   for (int i=0; i<6; ++i)
   {
      dist = mPlanes[i].distanceTo(corner);
      if (dist * dists[i] < 0.0)
      {
         return false;
      }
   }
   
   // If we reach here we know that for any given frustum planes
   //   all box corners lie on the same side
   // If any of the the first corner (b.min) distance to any plane is below zero
   //   the box is completely outside of the frustum as all other corner
   //   will also have a value below zero
   
   for (int i=0; i<6; ++i)
   {
      if (dists[i] < 0.0)
      {
         return true;
      }
   }
   
   return false;
}