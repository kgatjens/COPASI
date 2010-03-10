// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/layout/CLPolygon.cpp,v $
//   $Revision: 1.1 $
//   $Name:  $
//   $Author: gauges $
//   $Date: 2010/03/10 12:26:12 $
// End CVS Header

// Copyright (C) 2010 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#include "CLPolygon.h"

#include "CLRenderPoint.h"
#include "CLRenderCubicBezier.h"

#include <copasi/report/CCopasiRootContainer.h>
#include <copasi/report/CKeyFactory.h>

/**
 * Constructor.
 */
CLPolygon::CLPolygon(CCopasiContainer* pParent):
    CLGraphicalPrimitive2D(),
    CCopasiObject("Polygon", pParent),
    mKey("")
{
  this->mKey = CCopasiRootContainer::getKeyFactory()->add("Polygon", this);
}

/**
 * Copy constructor
 */
CLPolygon::CLPolygon(const CLPolygon& source, CCopasiContainer* pParent):
    CLGraphicalPrimitive2D(source),
    CCopasiObject(source, pParent),
    mKey("")
{
  this->mKey = CCopasiRootContainer::getKeyFactory()->add("Polygon", this);
  unsigned int i, iMax = source.mListOfElements.size();

  for (i = 0; i < iMax; ++i)
    {
      if (dynamic_cast<const CLRenderCubicBezier*>(source.mListOfElements[i]))
        {
          this->mListOfElements.push_back(new CLRenderCubicBezier(*static_cast<const CLRenderCubicBezier*>(source.mListOfElements[i])));
        }
      else
        {
          this->mListOfElements.push_back(new CLRenderPoint(*source.mListOfElements[i]));
        }
    }
}

/**
 * Constructor to generate object from the corresponding SBML object.
 */
CLPolygon::CLPolygon(const Polygon& source, CCopasiContainer* pParent):
    CLGraphicalPrimitive2D(source),
    CCopasiObject("Polygon", pParent),
    mKey("")
{
  this->mKey = CCopasiRootContainer::getKeyFactory()->add("Polygon", this);
  unsigned int i, iMax = source.getNumElements();

  for (i = 0; i < iMax; ++i)
    {
      CLRenderPoint* pElement = NULL;

      if (dynamic_cast<const RenderCubicBezier*>(source.getElement(i)))
        {
          pElement = new CLRenderCubicBezier(*static_cast<const RenderCubicBezier*>(source.getElement(i)));
        }
      else
        {
          pElement = new CLRenderPoint(*source.getElement(i));
        }

      this->mListOfElements.push_back(pElement);
    }
}

/**
 * Destructor.
 */
CLPolygon::~CLPolygon()
{
  CCopasiRootContainer::getKeyFactory()->remove(this->mKey);
  unsigned int i, iMax = this->mListOfElements.size();

  for (i = 0; i < iMax; ++i)
    {
      delete this->mListOfElements[i];
    }
}

/**
 * Returns the number of line segments.
 */
unsigned int CLPolygon::getNumElements() const
{
  return this->mListOfElements.size();
}

/**
 * Returns a pointer to the list of curve segments.
 */
std::vector<CLRenderPoint*>* CLPolygon::getListOfElements()
{
  return &(this->mListOfElements);
}

/**
 * Returns a const pointer to the list of curve segments.
 */
const std::vector<CLRenderPoint*>* CLPolygon::getListOfElements() const
{
  return &(this->mListOfElements);
}

/**
 * Creates a new point object and adds it to the list of curve
 * segments.
 */
CLRenderPoint* CLPolygon::createPoint()
{
  this->mListOfElements.push_back(new CLRenderPoint());
  return this->mListOfElements.back();
}

/**
 * Creates a new cubicbezier object and adds it to the list of curve
 * segments.
 */
CLRenderCubicBezier* CLPolygon::createCubicBezier()
{
  this->mListOfElements.push_back(new CLRenderCubicBezier());
  return static_cast<CLRenderCubicBezier*>(this->mListOfElements.back());
}

/**
 * Returns a pointer to the line segement with with the given index or
 * NULL if the index is invalid.
 */
CLRenderPoint* CLPolygon::getElement(unsigned int index)
{
  return (index < this->mListOfElements.size()) ? (this->mListOfElements[index]) : NULL;
}

/**
 * Returns const a pointer to the line segement with with the given index or
 * NULL if the index is invalid.
 */
const CLRenderPoint* CLPolygon::getElement(unsigned int index) const
{
  return (index < this->mListOfElements.size()) ? (this->mListOfElements[index]) : NULL;
}

/**
 * Adds a copy of the given line segment to the list of line segements.
 */
void CLPolygon::addElement(const CLRenderPoint* pLS)
{
  if (dynamic_cast<const CLRenderCubicBezier*>(pLS))
    {
      this->mListOfElements.push_back(new CLRenderCubicBezier(*static_cast<const CLRenderCubicBezier*>(pLS)));
    }
  else
    {
      this->mListOfElements.push_back(new CLRenderPoint(*pLS));
    }
}

/**
 * Removes the curve segment with the given index.
 */
void CLPolygon::removeElement(unsigned int i)
{
  if (i < this->mListOfElements.size())
    {
      std::vector<CLRenderPoint*>::iterator it = this->mListOfElements.begin();
      it += i;
      delete *it;
      this->mListOfElements.erase(it);
    }
}

/**
 * Returns the key of the color definition.
 */
const std::string& CLPolygon::getKey() const
{
  return this->mKey;
}

/**
 * Converts this object to the corresponding SBML object.
 */
Polygon* CLPolygon::toSBML() const
{
  Polygon* pPolygon = new Polygon();
  this->addSBMLAttributes(pPolygon);
  unsigned int i, iMax = this->mListOfElements.size();

  for (i = 0; i < iMax; ++i)
    {
      const RenderPoint* pP = this->mListOfElements[i]->toSBML();
      pPolygon->addElement(pP);
      delete pP;
    }

  return pPolygon;
}