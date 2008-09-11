// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/layoutUI/CQGLNetworkPainter.cpp,v $
//   $Revision: 1.127 $
//   $Name:  $
//   $Author: gauges $
//   $Date: 2008/09/11 12:41:37 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <qstring.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qrect.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qsize.h>
#include <qcolor.h>
#include <qtimer.h>
#include <qcanvas.h>

#include <qfontinfo.h>
#include <qfontdatabase.h>

#include <iostream>
#include <math.h>
#include <float.h>
#include <utility>

#include "copasi/utilities/COutputHandler.h"

#include "copasi.h"

#include "FontChooser.h"

#if (defined WIN32 && !defined log2)
C_FLOAT64 log2(const C_FLOAT64 & __x)
{return log(__x) / log(2.0);}
#endif // WIN32

#include "CQGLNetworkPainter.h"
#include "CQLayoutMainWindow.h"

#include "UI/qtUtilities.h"
#include "layout/CLayout.h"
#include "utilities/CCopasiVector.h"
#include "layoutUI/CVisParameters.h"
#include "layoutUI/CDataEntity.h"
#include "layoutUI/BezierCurve.h"

// TODO change the text rendering or the texture creation. Right now it seems
// to work reasonably well under Mac OS X, but under Linux it doesn't.

// TODO Antialias Nodes and arrowheads, right now it looks as if only the edges
// do get antialiasing
const double CQGLNetworkPainter::PLANE_DEPTH = 1.0;

CQGLNetworkPainter::CQGLNetworkPainter(const QGLFormat& format, QWidget *parent, const char *name)
    : QGLWidget(format, parent, name)
{
  initializeGraphPainter(parent);
}

CQGLNetworkPainter::~CQGLNetworkPainter()
{
  std::map<std::string, RGTextureSpec*>::iterator it = labelTextureMap.begin(), endit = labelTextureMap.end();
  while (it != endit)
    {
      delete[] it->second->textureData;
      delete it->second;
      ++it;
    }
  // delete the node display list
  glDeleteLists(this->mDisplayLists, 2);
}

const CLPoint& CQGLNetworkPainter::getGraphMin()
{
  const CLPoint& mi = mgraphMin;
  return mi;
}

const CLPoint& CQGLNetworkPainter::getGraphMax()
{
  const CLPoint& ma = mgraphMax;
  return ma;
}

// set graph size and reset projection to fit new size
void CQGLNetworkPainter::setGraphSize(const CLPoint & min, const CLPoint & max)
{
  mgraphMin.setX(min.getX());
  mgraphMin.setY(min.getY());
  mgraphMax.setX(max.getX());
  mgraphMax.setY(max.getY());
}

void CQGLNetworkPainter::draw()
{
  glLoadIdentity();
  drawGraph();
}

void CQGLNetworkPainter::createGraph(CLayout *lP)
{
  keyMap.clear();
  labelNodeMap.clear();
  nodeArrowMap.clear();
  nodeCurveMap.clear();
  viewerNodes.clear();
  viewerCurves.clear();
  viewerLabels.clear();
  curvesWithArrow.clear();
  int numberOfInvertedCurves = 0;
  // copy graph to local variables
  CCopasiVector<CLMetabGlyph> nodes;
  nodes = lP->getListOfMetaboliteGlyphs();
  viewerNodes = std::vector<std::string>();
  unsigned int i;
  for (i = 0;i < nodes.size();i++)
    {
      std::string nKey = (*nodes[i]).getKey();
      std::string oKey = (*nodes[i]).getModelObjectKey();
      viewerNodes.push_back(nKey);
      nodeMap.insert(std::pair<std::string, CGraphNode>
                     (nKey,
                      CGraphNode(*nodes[i])));
      keyMap.insert(std::pair<std::string, std::string>
                    (oKey, nKey));
    }
  CCopasiVector<CLReactionGlyph> reactions;
  reactions = lP->getListOfReactionGlyphs();

  //std::cout << "number of reactions: " << reactions.size() << std::endl;

  //now extract curves to draw from reaction
  viewerCurves = std::vector<CGraphCurve>();
  //first get reaction arrow
  for (i = 0;i < reactions.size();i++)
    {
      CGraphCurve curveR = CGraphCurve((reactions[i])->getCurve());
      viewerCurves.push_back(curveR);

      CCopasiVector<CLMetabReferenceGlyph> edgesToNodesOfReaction;
      edgesToNodesOfReaction = reactions[i]->getListOfMetabReferenceGlyphs();
      unsigned int j2;
      for (j2 = 0;j2 < edgesToNodesOfReaction.size();j2++)
        {
          CGraphCurve curve = CGraphCurve(edgesToNodesOfReaction[j2]->getCurve());
          std::string nodeKey = "";
          if (edgesToNodesOfReaction[j2]->getMetabGlyph() != NULL) // i.e. there is an associated node
            {
              nodeKey = std::string(edgesToNodesOfReaction[j2]->getMetabGlyph()->getKey());
              std::map<std::string, CGraphNode>::iterator itNode;
              itNode = nodeMap.find(nodeKey);
              if (itNode != nodeMap.end())
                {
                  CLBoundingBox box = (*itNode).second.getBoundingBox();
                  if (this->checkCurve(&curve, curveR, box))
                    numberOfInvertedCurves++;
                }
            }

          CLMetabReferenceGlyph::Role r = edgesToNodesOfReaction[j2]->getRole();
          curve.setRole(r);
          if (edgesToNodesOfReaction[j2]->getMetabGlyph() != NULL)  // if there is an associated scpecies node look, whether an arrow has to be created
            {
              // if role is product or sideproduct, create arrow for line
              if ((r == CLMetabReferenceGlyph::PRODUCT) || (r == CLMetabReferenceGlyph::SIDEPRODUCT) || (r == CLMetabReferenceGlyph::ACTIVATOR) || (r == CLMetabReferenceGlyph::INHIBITOR) || (r == CLMetabReferenceGlyph::MODIFIER))
                {// create arrows just for edges to products or sideproducts
                  std::vector<CLLineSegment> segments = curve.getCurveSegments();
                  if (! segments.empty())
                    {
                      CLLineSegment lastSeg = segments[curve.getNumCurveSegments() - 1];
                      CLPoint p = lastSeg.getEnd();
                      CArrow *ar;
                      if (lastSeg.isBezier())
                        {
                          BezierCurve *bezier = new BezierCurve();
                          std::vector<CLPoint> pts = std::vector<CLPoint>();
                          pts.push_back(lastSeg.getStart());
                          pts.push_back(lastSeg.getBase1());
                          pts.push_back(lastSeg.getBase2());
                          pts.push_back(lastSeg.getEnd());
                          std::vector<CLPoint> bezierPts = bezier->curvePts(pts);
                          C_INT32 num = bezierPts.size();
                          CLLineSegment segForArrow = CLLineSegment(bezierPts[num - 2], bezierPts[num - 1]);
                          ar = new CArrow(segForArrow, bezierPts[num - 1].getX(), bezierPts[num - 1].getY(), this->mCurrentZoom);
                          delete bezier;
                        }
                      else
                        {
                          ar = new CArrow(lastSeg, p.getX(), p.getY(), this->mCurrentZoom);
                        }

                      curve.setArrowP(true);
                      curve.setArrow(*ar);
                      delete ar;
                    }
                }
              if (nodeKey != "")
                nodeCurveMap.insert(std::pair<std::string, CGraphCurve>
                                    (nodeKey,
                                     curve));
            }
          else
            {// if no species node is associated with the curve: just store curve
              viewerCurves.push_back(curve); // just collect curve in order to be shown within the graph
            }
        } // end j
    } // end i (reactions)

  CCopasiVector<CLTextGlyph> labels;
  labels = lP->getListOfTextGlyphs();
  viewerLabels = std::vector<CLabel>();
  std::map<std::string, CGraphNode>::iterator itNode;
  for (i = 0;i < labels.size();i++)
    {
      labelNodeMap.insert(std::pair<std::string, std::string>
                          (labels[i]->getKey(),
                           labels[i]->getGraphicalObjectKey()));
      std::string s1 = labels[i]->getKey();
      std::string s2 = labels[i]->getGraphicalObjectKey();
      viewerLabels.push_back(CLabel(*labels[i]));
      itNode = nodeMap.find(labels[i]->getGraphicalObjectKey());
      if (itNode != nodeMap.end())
        (*itNode).second.setLabelText(labels[i]->getText());
    }
  CLPoint p1 = CLPoint(0.0, 0.0);
  CLPoint p2 = CLPoint(lP->getDimensions().getWidth(), lP->getDimensions().getHeight());
  this->setGraphSize(p1, p2);
}

// decides whether the direction of the curve has to be inverted (meaning the order of the line segments, start and end points and base points have to be inverted
bool CQGLNetworkPainter::checkCurve(CGraphCurve * curve, CGraphCurve /* curveR */, CLBoundingBox box)
{
  bool inverted = false;
  // first checks whether the start point or the end point of the curve is closer to the center of the box defining the reactant node
  CLPoint center; // center of bounding box for node
  center.setX(box.getPosition().getX() + (box.getDimensions().getWidth() / 2.0));
  center.setY(box.getPosition().getY() + (box.getDimensions().getHeight() / 2.0));

  // get start and end point of curve (start point of first segment and end point of last segment)
  std::vector <CLPoint> points = curve->getListOfPoints();
  if (points.size() > 1)
    {// if there are at least 2 points
      CLPoint s = points[0];
      CLPoint e = points[points.size() - 1];
      // now compute the distances from these points to the center

      C_FLOAT64 dist1 = sqrt (((center.getX() - s.getX()) * (center.getX() - s.getX())) +
                              ((center.getY() - s.getY()) * (center.getY() - s.getY())));
      C_FLOAT64 dist2 = sqrt(((center.getX() - e.getX()) * (center.getX() - e.getX())) +
                             ((center.getY() - e.getY()) * (center.getY() - e.getY())));

      if (dist1 < dist2)
        {// if the start point of the curve is closer to the node than the end point
          // the curve direction should be TOWARDS the node, not away from it
          curve->invertOrderOfPoints(); // invert the order of the points in the curve
          inverted = true;
        }
    }
  return inverted;
}

void CQGLNetworkPainter::drawGraph()
{
  // create OpenGL display list
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  unsigned int i;
  glCallList(this->mDisplayLists);
  if ((pParentLayoutWindow != NULL) && this->mLabelShape == CIRCLE &&
      (pParentLayoutWindow->getMappingMode() == CVisParameters::COLOR_MODE)) // draw color legend
    {
      drawColorLegend();
    } // end color mode

  // draw curves to (reactant) nodes and arrows and circular nodes when in appropriate mode
  std::map<std::string, CGraphNode>::iterator itNode;
  std::multimap<std::string, CGraphCurve>::iterator itCurve;
  std::multimap<std::string, CArrow>::iterator itArrow;
  std::pair<std::multimap<std::string, CGraphCurve>::iterator, std::multimap<std::string, CGraphCurve>::iterator> curveRangeIt;;
  std::pair<std::multimap<std::string, CArrow>::iterator, std::multimap<std::string, CArrow>::iterator> arrowRangeIt;

  for (i = 0;i < viewerNodes.size();i++)
    {
      itNode = nodeMap.find(viewerNodes[i]);
      // draw curves of node
      curveRangeIt = nodeCurveMap.equal_range(viewerNodes[i]);
      itCurve = curveRangeIt.first;
      glColor3f(0.0f, 0.0f, 0.5f); // edges in dark blue
      while (itCurve != curveRangeIt.second)
        {
          drawEdge((*itCurve).second);
          itCurve++;
        }

      if (this->mLabelShape == CIRCLE)
        {
          //draw node as a circle
          if (itNode != nodeMap.end())
            drawNode((*itNode).second);
        }
    }

  glColor3f(0.0f, 0.0f, 0.5f); // edges in dark blue
  for (i = 0;i < viewerCurves.size();i++) // draw edges that do not directly belong to a node (reaction curves)
    {
      drawEdge(viewerCurves[i]);
    }

  // NOW DRAW LABELS

  if (this->mLabelShape == RECTANGLE)
    {
      // debug: print fonr info
      this->mf.setPointSize(this->mFontsize);
      const QFont& mfRef = this->mf;
      QFontInfo fontInfo = QFontInfo (mfRef);
      // debug end
      for (i = 0;i < viewerLabels.size();i++)
        drawLabel(viewerLabels[i]);
    }
  else
    {// draw string next to circle (to the right) or in the center if there is enough space
      for (i = 0;i < viewerLabels.size();i++)
        {
          C_FLOAT64 tWid = getTextWidth(viewerLabels[i].getText(), mFontname, static_cast<int>(floor(viewerLabels[i].getHeight())));
          C_FLOAT64 nDiam = 0.0;
          C_FLOAT64 x, y;

          const std::string& nodeKey = viewerLabels[i].getGraphicalObjectKey();
          if (!nodeKey.empty())
            {
              std::map<std::string, CGraphNode>::iterator itNodeObj;
              itNodeObj = nodeMap.find(nodeKey);
              if (itNodeObj != nodeMap.end())
                nDiam = (*itNodeObj).second.getSize();
              C_INT32 xNdCenter = (C_INT32) ((*itNodeObj).second.getX() + ((*itNodeObj).second.getWidth() / 2.0));
              C_INT32 yNdCenter = (C_INT32) (*itNodeObj).second.getY(); // + ((*itNodeObj).second.getHeight() / 2.0);
              if (pParentLayoutWindow->getMappingMode() == CVisParameters::COLOR_MODE)
                {
                  x = xNdCenter + (CVisParameters::DEFAULT_NODE_SIZE / 2.0 * this->mCurrentZoom) + 2.0 - ((viewerLabels[i].getWidth() - tWid) / 2.0); // node center + circle radius + 2.0 - texture window overhead
                  y = yNdCenter;
                }
              else if ((tWid + 4) > nDiam)
                {// label wider (+ k=4 to avoid crossing circle borders) than size of circle-> place next to circle
                  x = xNdCenter + (nDiam / 2.0) + 2.0 - ((viewerLabels[i].getWidth() - tWid) / 2.0); // + nDiam / 2.0 - ((labelWWid - (*itNodeObj).second.getWidth()) / 2.0); // node center + circle radius - texture window overhead
                  y = yNdCenter;
                }
              else
                {// place in center of circle
                  x = xNdCenter - (viewerLabels[i].getWidth() / 2.0); // - ((labelWWid - (*itNodeObj).second.getWidth()) / 2.0);
                  y = yNdCenter;
                }
            }
          else
            {// if there is no node associated, just take label position
              x = viewerLabels[i].getX();
              y = viewerLabels[i].getY();
            }
          RG_drawStringAt(viewerLabels[i].getText(), static_cast<C_INT32>(x), static_cast<C_INT32>(y), static_cast<C_INT32>(viewerLabels[i].getWidth()), static_cast<C_INT32>(viewerLabels[i].getHeight()));
        }
    }
}

void CQGLNetworkPainter::drawColorLegend()
{
  C_INT32 sx = 40; //start at position (sx,sy)
  C_INT32 sy = 20;
  C_INT32 w = 120; // size of legend rectangle w x h
  C_INT32 h = 15;

  RG_drawStringAt("MIN", 7, sy + 3, 32, 16);
  RG_drawStringAt("MAX", 165, sy + 3, 32, 16);

  // the colors should go from RGB 0,0,0 to RGB 200,0,0 to RGB 200,200,0 to RGB
  // 255,255,0
  C_INT16 i;
  QColor col = QColor();
  // the color range has 456 steps and the legend has 120 pixels
  // so the step size is 455/120
  double ratio = 455 / 120;
  double val;
  for (i = 0;i <= w;i++)
    {
      val = i * ratio;
      if (val < 200.0)
        {
          col.setRgb((int)val, 0, 0);
        }
      else if (val < 400.0)
        {
          col.setRgb(200, (int)(val - 200.0), 0);
        }
      else
        {
          col.setRgb(200 + (int)(val - 400.0), 200 + (int)(val - 400.0), 0);
        }
      QGLWidget::qglColor(col);
      // draw colored line in rectangle
      glBegin(GL_LINES);
      glVertex2d(i + sx, sy);
      glVertex2d(i + sx, sy + h);
      glEnd();
    }
}

// draw node as circle
void CQGLNetworkPainter::drawNode(CGraphNode &n) // draw node as filled circle
{
  float scaledValue = CVisParameters::DEFAULT_NODE_SIZE * mCurrentZoom;
  C_INT16 mappingMode = CVisParameters::SIZE_DIAMETER_MODE;
  if (pParentLayoutWindow != NULL)
    {
      mappingMode = pParentLayoutWindow->getMappingMode();
      if ((mappingMode == CVisParameters::SIZE_DIAMETER_MODE) ||
          (mappingMode == CVisParameters::SIZE_AREA_MODE))
        scaledValue = n.getSize(); // change of node size only for size mode
    }
  glColor3f(1.0f, 0.0f, 0.0f); // red
  GLUquadricObj *qobj;
  qobj = gluNewQuadric();

  double tx = n.getX() + (n.getWidth() / 2.0);
  double ty = n.getY() + (n.getHeight() / 2.0);
  glTranslatef((float)tx, (float)ty, 0.0f);

  if ((mappingMode == CVisParameters::SIZE_DIAMETER_MODE) ||
      (mappingMode == CVisParameters::SIZE_AREA_MODE))
    if (setOfConstantMetabolites.find(n.getOrigNodeKey()) == setOfConstantMetabolites.end())
      {
        if (setOfDisabledMetabolites.find(n.getOrigNodeKey()) == setOfDisabledMetabolites.end())
          // red as default color for all nodes in non-color modes
          // which have a substantial range of values (max - min > epsilon)
          // and which are not disabled
          glColor3f(1.0f, 0.0f, 0.0f);
        else
          {
            glColor3f(0.75f, 0.75f, 1.0f); // color for disabled nodes (not to be animated)
          }
      }
    else
      glColor3f(0.7f, 0.7f, 0.7f); // reactants with a smaller range of values (e.g. constant)
  // are not scaled and marked in grey
  else
    {// color mapping
      QColor col = QColor();
      double v = n.getSize() * 455.0; // there are 456 colors in the current gradient and the node sizes are scaled from 0.0 to 1.0 in color mode
      if (v < 200.0)
        {
          col.setRgb((int)v, 0, 0);
        }
      else if (v < 400)
        {
          col.setRgb(200, (int)(v - 200.0), 0);
        }
      else
        {
          col.setRgb(200 + (int)(v - 400), 200 + (int)(v - 400), 0);
        }
      QGLWidget::qglColor(col);
    }

  gluDisk(qobj, 0.0, scaledValue / 2.0, 25, 2);
  glColor3f(0.0f, 0.0f, 0.0f); // black
  gluDisk(qobj, scaledValue / 2.0 - 1.0, scaledValue / 2.0, 25, 2);
  glTranslatef(-(float)tx, -(float)ty, 0.0f);
  gluDeleteQuadric(qobj);
}

// draw a curve: at the moment just a line from the start point to the end point (for each segment)
void CQGLNetworkPainter::drawEdge(CGraphCurve &c)
{
  std::vector<CLLineSegment> segments = c.getCurveSegments();
  unsigned int i;
  for (int k = 0;k < c.getNumCurveSegments();k++)
    {
      CLLineSegment seg = segments[k];

      CLPoint startPoint = seg.getStart();
      CLPoint endPoint = seg.getEnd();
      // for the moment do not take type of curve into account

      if (seg.isBezier())
        {
          CLPoint base1 = seg.getBase1();
          CLPoint base2 = seg.getBase2();
          //now paint bezier as line strip
          // use an evaluator since this is probably a lot more efficient
          GLfloat controlPts[4][3] = {
                                       {startPoint.getX(), startPoint.getY(), 0.0},
                                       {base1.getX(), base1.getY(), 0.0},
                                       {base2.getX(), base2.getY(), 0.0},
                                       {endPoint.getX(), endPoint.getY(), 0.0}
                                     };
          // enable the evaluator to draw the cubic beziers
          glMap1f(GL_MAP1_VERTEX_3, 0.0f, 100.0f, 3, 4, &controlPts[0][0]);
          glEnable(GL_MAP1_VERTEX_3);
          glBegin(GL_LINE_STRIP);
          for (i = 0;i <= 100;++i)
            {
              // evaluate the function
              glEvalCoord1f((GLfloat)i);
            }
          glEnd();
          glDisable(GL_MAP1_VERTEX_3);
        }
      else
        {// just draw a straight line
          glBegin(GL_LINE_STRIP);
          double x = startPoint.getX();
          double y = startPoint.getY();
          glVertex2d(x, y);
          x = endPoint.getX();
          y = endPoint.getY();
          glVertex2d(x, y);
          glEnd();
        }
    }
  if (c.hasArrowP())
    drawArrow(c.getArrow(), c.getRole());
}

void CQGLNetworkPainter::drawArrow(CArrow a, CLMetabReferenceGlyph::Role role)
{
  // first get the two points defining the line segment (curve)
  CLPoint p2 = a.getStartOfLine();
  CLPoint p1 = a.getEndOfLine();
  // p1 and p2 define a line where the arrow peak can be placed onto,
  // peak should be at p1, the arrow peak is just a triangle

  // first compute parameters of equation of line and point on line where arrow intersects line
  C_FLOAT64 d1 = p2.getX() - p1.getX();
  C_FLOAT64 d2 = p2.getY() - p1.getY();
  C_FLOAT64 norm = sqrt((d1 * d1) + (d2 * d2));
  C_FLOAT64 qX = p1.getX() + (a.getArrowLength() / norm * (p2.getX() - p1.getX()));
  C_FLOAT64 qY = p1.getY() + (a.getArrowLength() / norm * (p2.getY() - p1.getY()));

  // now compute second and third point of triangle
  // first compute direction vector of orthogonal line (= norm vector of line)
  // if (x2-x1,y2-y1) is the direction vector of the line, then (y1-y2,x2-x1) is a norm vector of the line
  // to get a certain length, use the unit norm vector
  C_FLOAT64 unX = (p1.getY() - p2.getY()) / norm;
  C_FLOAT64 unY = (p2.getX() - p1.getX()) / norm;

  // now draw polygon, using vertices from triangle
  // now create triangle;
  if ((role == CLMetabReferenceGlyph::PRODUCT) || (role == CLMetabReferenceGlyph::SIDEPRODUCT))
    {
      glBegin(GL_POLYGON);
      glVertex2d(p1.getX(), p1.getY()); // peak of arrow
      glVertex2d(qX + (unX * a.getArrowWidth()), qY + (unY * a.getArrowWidth()));
      glVertex2d(qX - (unX * a.getArrowWidth()), qY - (unY * a.getArrowWidth()));
      glEnd();
    }
  else
    {
      GLfloat params[4];
      glGetFloatv(GL_CURRENT_COLOR, params);
      GLfloat lineWidth[1];
      glGetFloatv(GL_LINE_WIDTH, lineWidth);

      if (role == CLMetabReferenceGlyph::MODIFIER)
        {
          if (this->mLabelShape == CIRCLE)
            {
              glBegin(GL_LINES);
              glVertex2d(p1.getX() + (unX * a.getArrowWidth()),
                         p1.getY() + (unY * a.getArrowWidth()));
              glVertex2d(p1.getX() - (unX * a.getArrowWidth()),
                         p1.getY() - (unY * a.getArrowWidth()));
              glEnd();
            }
          else
            {
              glBegin(GL_LINES);
              glVertex2d(qX + (unX * a.getArrowWidth()),
                         qY + (unY * a.getArrowWidth()));
              glVertex2d(qX - (unX * a.getArrowWidth()),
                         qY - (unY * a.getArrowWidth()));
              glEnd();
            }
          glColor3f(params[0], params[1], params[2]);
        }
      else if (role == CLMetabReferenceGlyph::ACTIVATOR)
        {
          glColor3f(0.0f, 0.66f, 0.0f); // kind of green
          glLineWidth(2.0f);
          glBegin(GL_LINES);
          glVertex2d(p1.getX() + (unX * a.getArrowWidth()),
                     p1.getY() + (unY * a.getArrowWidth()));
          glVertex2d(p1.getX() - (unX * a.getArrowWidth()),
                     p1.getY() - (unY * a.getArrowWidth()));
          glEnd();

          glColor3f(params[0], params[1], params[2]);
          glLineWidth(lineWidth[0]);
        }
      else if (role == CLMetabReferenceGlyph::INHIBITOR)
        {
          glColor3f(1.0f, 0.0f, 0.0f); // red
          glLineWidth(2.0f);
          glBegin(GL_LINES);
          glVertex2d(p1.getX() + (unX * a.getArrowWidth()),
                     p1.getY() + (unY * a.getArrowWidth()));
          glVertex2d(p1.getX() - (unX * a.getArrowWidth()),
                     p1.getY() - (unY * a.getArrowWidth()));
          glLineWidth(lineWidth[0]);
          glEnd();
        }
    }
}

// draws label as a rectangular filled shape with a border and the text inside
void CQGLNetworkPainter::drawLabel(CLabel l)
{
  glColor3f(0.7f, 0.8f, 1.0f); // label background color (61,237,181)
  // draw rectangle as background for text
  double CORNER_RADIUS_FRACTION = 0.1;
  double cornerRadius = (l.getWidth() > l.getHeight()) ? l.getHeight() * CORNER_RADIUS_FRACTION : l.getWidth() * CORNER_RADIUS_FRACTION;
  glBegin(GL_POLYGON);
  glVertex2d(l.getX() + cornerRadius, l.getY());
  glVertex2d(l.getX() + l.getWidth() - cornerRadius, l.getY());
  glVertex2d(l.getX() + l.getWidth(), l.getY() + cornerRadius);
  glVertex2d(l.getX() + l.getWidth(), l.getY() + l.getHeight() - cornerRadius);
  glVertex2d(l.getX() + l.getWidth() - cornerRadius, l.getY() + l.getHeight());
  glVertex2d(l.getX() + cornerRadius, l.getY() + l.getHeight());
  glVertex2d(l.getX(), l.getY() + l.getHeight() - cornerRadius);
  glVertex2d(l.getX(), l.getY() + cornerRadius);
  glEnd();
  GLUquadricObj* qobj = NULL;
  qobj = gluNewQuadric();
  gluQuadricDrawStyle(qobj, GLU_FILL);
  glPushMatrix();
  glTranslatef(l.getX() + cornerRadius, l.getY() + cornerRadius , 0.0f);
  gluPartialDisk(qobj, 0.0, cornerRadius, 10, 10, 180, 90);
  glTranslatef(l.getWidth() - 2.0 * cornerRadius, 0.0f , 0.0f);
  gluPartialDisk(qobj, 0.0, cornerRadius, 10, 10, 90, 90);
  glTranslatef(0.0f, l.getHeight() - 2.0 * cornerRadius, 0.0f);
  gluPartialDisk(qobj, 0.0, cornerRadius, 10, 10, 0, 90);
  glTranslatef(-l.getWidth() + 2 * cornerRadius , 0.0 , 0.0f);
  gluPartialDisk(qobj, 0.0, cornerRadius, 10, 10, 270, 90);
  glPopMatrix();
  gluDeleteQuadric(qobj);
  RG_drawStringAt(l.getText(), static_cast<C_INT32>(l.getX()), static_cast<C_INT32>(l.getY()), static_cast<C_INT32>(l.getWidth()), static_cast<C_INT32>(l.getHeight()));
}

// uses QT

void CQGLNetworkPainter::RG_drawStringAt(std::string s, C_INT32 x, C_INT32 y, C_INT32 w, C_INT32 h)
{
  RGTextureSpec* texSpec = getTextureForText(s, mFontname, h);
  if (texSpec == NULL)
    {
      return;
    }
  glPushMatrix();
  glColor3f(0.0, 0.0, 0.0);
  glEnable(GL_TEXTURE_2D);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, textureNames[0]);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY8, static_cast<int>(texSpec->textureWidth), static_cast<int>(texSpec->textureHeight), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, texSpec->textureData);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTranslated(x, y, 0.5);

  double xOffset = (w - texSpec->textWidth + 2) / 2.0;
  double yOffset = (h - texSpec->textHeight + texSpec->textYOffset + 2) / 2.0;
  xOffset = (xOffset < 0.0) ? 0.0 : xOffset;
  yOffset = (yOffset < 0.0) ? 0.0 : yOffset;
  double textureXRatio = ((texSpec->textWidth + 2) / texSpec->textureWidth) / ((w - xOffset) / w);
  double textureYRatio = ((texSpec->textHeight + 2) / texSpec->textureHeight) / ((h - 2 * yOffset) / h);

  glBegin(GL_POLYGON);
  glTexCoord2f(-xOffset / texSpec->textureWidth, -yOffset / texSpec->textureHeight);
  glVertex3f(0.0, 0.0, 0.0);

  glTexCoord2f(textureXRatio, -yOffset / texSpec->textureHeight);
  glVertex3f(w, 0.0, 0.0);

  glTexCoord2f(textureXRatio, textureYRatio);
  glVertex3f(w, h, 0.0);

  glTexCoord2f(-xOffset / texSpec->textureWidth, textureYRatio);
  glVertex3f(0.0, h, 0.0);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glPopMatrix();
}

int CQGLNetworkPainter::getTextWidth(const std::string& text, const std::string& fontName, unsigned int fontSize)
{
  QFont font(QString(fontName.c_str()), fontSize);
  QFontMetrics fontMetrics = QFontMetrics(font);

  QRect rect = fontMetrics.boundingRect(QString(text.c_str()));
  int width = rect.width();

  return width;
}

int CQGLNetworkPainter::getLabelWindowWidth(int width)
{
  int exponent = static_cast<int>(ceil(log2(width + 2.0)));
  if (exponent < 6)
    {
      exponent = 6;
    }
  width = static_cast<int>(pow(2.0, exponent + 1));
  return width;
}

void CQGLNetworkPainter::createTextureForAllLabels()
{
  std::map<std::string, RGTextureSpec*>::iterator it = labelTextureMap.begin(), endit = labelTextureMap.end();
  while (it != endit)
    {
      delete[] it->second->textureData;
      delete it->second;
      ++it;
    }
  labelTextureMap.clear();
  unsigned int i = 0;
  for (i = 0;i < viewerLabels.size();i++)
    {
      C_INT32 fontSize = mFontsize;
      RGTextureSpec* pTexture = RG_createTextureForText(viewerLabels[i].getText(), mFontname, fontSize);
      labelTextureMap.insert(std::pair<std::string, RGTextureSpec*>
                             (viewerLabels[i].getText(),
                              pTexture));
    }
}

RGTextureSpec* CQGLNetworkPainter::getTextureForText(const std::string& text, const std::string& fontName, unsigned int fontSize)
{
  std::map<std::string, RGTextureSpec*>::iterator it;
  it = labelTextureMap.find(text);
  RGTextureSpec* texSpec = NULL;
  if (it != labelTextureMap.end())
    {
      texSpec = ((*it).second);
    }
  else
    {
      texSpec = RG_createTextureForText(text, fontName, fontSize);
      labelTextureMap.insert(std::pair<std::string, RGTextureSpec*>(text, texSpec));
    }
  return texSpec;
}

RGTextureSpec* CQGLNetworkPainter::RG_createTextureForText(const std::string& text, const std::string& fontName, unsigned int fontSize)
{
  QFont font(QString(fontName.c_str()), fontSize);
  QFontMetrics fontMetrics = QFontMetrics(font);

  QRect rect = fontMetrics.boundingRect(QString(text.c_str()));
  int width = rect.width();
  int height = rect.height();
  int exponent = static_cast<int>(ceil(log2(width + 2.0)));
  if (exponent < 6)
    {
      exponent = 6;
    }
  width = static_cast<int>(pow(2.0, exponent + 1));
  exponent = static_cast<int>(ceil(log2(height + 2.0)));
  if (exponent < 6)
    {
      exponent = 6;
    }
  height = static_cast<int>(pow(2.0, exponent + 1));

  QPixmap pixmap(width, height);
  pixmap.fill(QColor(255, 255, 255));
  QCanvas canvas(width, height);
  QCanvasText canvasText(QString(text.c_str()), &canvas);
  canvasText.setFont(font);
  canvasText.setColor(QColor(0, 0, 0));
  // also move one to the right and one down to generate one column
  // and one row of transparent pixels
  canvasText.moveBy(1, 1);
  canvasText.show();
  QPainter painter(&pixmap);
  canvas.drawArea(canvas.rect(), &painter);

  RGTextureSpec* texture = new RGTextureSpec();
  texture->textureData = new GLubyte[height * width];
  texture->textureWidth = width;
  texture->textureHeight = height;
  texture->textWidth = rect.width();
  texture->textHeight = rect.height();
  QImage image = pixmap.convertToImage(); // UR
  // write the texture to a file to check if they were created correctly
  //assert(image.save(text+".png","PNG"));
  int i, j;
  int firstWhitePixel = height;
  char pixelValue;
  QRgb pixel;
  for (i = 0;i < height;++i)
    {
      for (j = 0;j < width;++j)
        {
          pixel = image.pixel(j, i);
          pixelValue = static_cast<unsigned char>(255 - (qRed(pixel) + qGreen(pixel) + qBlue(pixel)) / 3);
          texture->textureData[i*width + j] = pixelValue;
          if (pixelValue != 0)
            {
              if (firstWhitePixel == height)
                {
                  firstWhitePixel = i;
                }
            }
        }
    }
  texture->textYOffset = firstWhitePixel;
  // write the actual texture to a file
  //texture->save(text+".tga");
  return texture;
}

void CQGLNetworkPainter::drawStringAt(std::string s, C_FLOAT64 x, C_FLOAT64 y, C_FLOAT64 w, C_FLOAT64 h, QColor bgCol)
{
  glColor3f(0.0f, 0.0f, 0.0f); // black

  QString str(FROM_UTF8(s));

  QFontMetrics mfm = QFontMetrics(mf);
  QRect bbox = mfm.boundingRect(FROM_UTF8(s)); // bounding rectangle for text in certain size

  int w2 = round2powN(bbox.width()); // look for smallest w2 = 2^^k with n > w2
  int h2 = round2powN(bbox.height() + 2); // look for smallest h2 = 2^^k with n > h2
  while (h2 > h)
    {// reduce fontsize in order to avoid problems with size of texture image
      this->mFontsize--;
      this->mFontsizeDouble = (double) this->mFontsize;
      mf.setPointSize(this->mFontsize);
      const QFont& mfRef = mf;
      QFontMetrics mfm = QFontMetrics(mfRef);
      bbox = mfm.boundingRect(FROM_UTF8(s));
      w2 = round2powN(bbox.width());
      h2 = round2powN(bbox.height() + 2);
    }

  QRect c(0, 0, w2, h2);

  QPixmap pm(w2, h2);

  pm.fill(bgCol);
  QPainter painter2(&pm);
  painter2.setPen(Qt::black);
  painter2.setFont(mf);
  painter2.drawText(c, Qt::AlignCenter, FROM_UTF8(s));
  painter2.end();

  QImage img = pm.convertToImage();
  QImage timg = QGLWidget::convertToGLFormat(img);

  glTexImage2D(GL_TEXTURE_2D, 0, 3, timg.width(), timg.height(), 0,
               GL_RGBA, GL_UNSIGNED_BYTE, timg.bits());
  double xoff = (w - w2) / 2.0;
  double yoff = (h - h2) / 2.0;

  xoff = 0;
  yoff = 0;

  glRasterPos2f(x + xoff, y + h - yoff);
  glDrawPixels(w2, h2, GL_RGBA, GL_UNSIGNED_BYTE, timg.bits());
}

int CQGLNetworkPainter::round2powN(double d)
{
  int n = (int)(ceil(d));
  int p = 1;
  int maxP = 12; // max size of images 2*12
  while ((p <= maxP) && (n > pow(2.0, p)))
    p++;
  return (int)pow(2.0, p);
}

void CQGLNetworkPainter::setItemAnimated(std::string key, bool animatedP)
{
  if (!animatedP)
    {
      setOfDisabledMetabolites.insert(key);
      C_FLOAT64 midValue = (pParentLayoutWindow->getMinNodeSize() + pParentLayoutWindow->getMaxNodeSize()) / 2.0; // node size used here is set to mid between min and max node size (for reactants that are not animated)
      setConstantNodeSizeForAllSteps(key, midValue);
    }
  else
    {
      setOfDisabledMetabolites.erase(key);
      rescaleNode(key, pParentLayoutWindow->getMinNodeSize(), pParentLayoutWindow->getMaxNodeSize(), pParentLayoutWindow->getScalingMode());
    }
  this->showStep(pParentLayoutWindow->getCurrentStep());
}

void CQGLNetworkPainter::rescaleDataSetsWithNewMinMax(C_FLOAT64 /* oldMin */, C_FLOAT64 /* oldMax */, C_FLOAT64 newMin, C_FLOAT64 newMax, C_INT16 scaleMode)
{
  CDataEntity dataSet;
  unsigned int s; // step number
  C_FLOAT64 val, val_new;
  setOfConstantMetabolites.clear();
  for (s = 0; s < dataSets.size(); s++) // for all steps
    {
      std::map<C_INT32, CDataEntity>::iterator iter = dataSets.find(s);
      if (iter != dataSets.end())
        {
          dataSet = (*iter).second;
          unsigned int i;
          for (i = 0; i < viewerNodes.size();i++) // iterate over string values (node keys)
            {
              // get old value
              val = dataSet.getValueForSpecies(viewerNodes[i]);
              C_FLOAT64 a = 0.0, b = 1.0;
              if (pParentLayoutWindow != NULL)
                {
                  if (scaleMode == CVisParameters::INDIVIDUAL_SCALING)
                    {
                      a = pSummaryInfo->getMinForSpecies(viewerNodes[i]);
                      b = pSummaryInfo->getMaxForSpecies(viewerNodes[i]);
                    }
                  else // scaleMode == CVisParameters::GLOBAL_SCALING
                    {
                      a = pSummaryInfo->getMinOverallConcentration();
                      b = pSummaryInfo->getMaxOverallConcentration();
                    }
                }
              C_FLOAT64 val_orig;
              if ((b - a) > CVisParameters::EPSILON)
                {
                  val_orig = dataSet.getOrigValueForSpecies(viewerNodes[i]); // get original value
                  // now scale value
                  val_new = newMin + ((val_orig - a) / (b - a) * (newMax - newMin));
                }
              else
                {// no scaling if differences are too small, just set mid value
                  val_new = (newMax + newMin) / 2.0;
                  if (s == 0) // only insert once into set
                    setOfConstantMetabolites.insert(viewerNodes[i]);
                }
              dataSet.putValueForSpecies(viewerNodes[i], val_new);
            }
          dataSets.erase(s);
          dataSets.insert (std::pair<C_INT32, CDataEntity>
                           (s, dataSet));
        }
    }
  // if there is no time course data, we set all values to 0.0
  if (dataSets.size() == 0)
    {
      CDataEntity dataSet;
      unsigned int i;
      for (i = 0; i < viewerNodes.size();i++) // iterate over string values (node keys)
        {
          dataSet.putValueForSpecies(viewerNodes[i], 0.0);
        }
      dataSets.insert (std::pair<C_INT32, CDataEntity> (s, dataSet));
    }
}

void CQGLNetworkPainter::rescaleNode(std::string key, C_FLOAT64 newMin, C_FLOAT64 newMax, C_INT16 scaleMode)
{
  CDataEntity dataSet;
  unsigned int s; // step number
  C_FLOAT64 val, val_new;
  setOfConstantMetabolites.clear();
  for (s = 0; s < dataSets.size(); s++) // for all steps
    {
      std::map<C_INT32, CDataEntity>::iterator iter = dataSets.find(s);
      if (iter != dataSets.end())
        {
          dataSet = (*iter).second;
          // get old value
          val = dataSet.getValueForSpecies(key);
          C_FLOAT64 a = 0.0, b = 1.0;
          if (pParentLayoutWindow != NULL)
            {
              if (scaleMode == CVisParameters::INDIVIDUAL_SCALING)
                {
                  a = pSummaryInfo->getMinForSpecies(key);
                  b = pSummaryInfo->getMaxForSpecies(key);
                }
              else // scaleMode == CVisParameters::GLOBAL_SCALING
                {
                  a = pSummaryInfo->getMinOverallConcentration();
                  b = pSummaryInfo->getMaxOverallConcentration();
                }
            }
          C_FLOAT64 val_orig;
          if ((b - a) > CVisParameters::EPSILON)
            {
              val_orig = dataSet.getOrigValueForSpecies(key); // get original value
              // now scale value
              val_new = newMin + ((val_orig - a) / (b - a) * (newMax - newMin));
            }
          else
            {// no scaling if differences are too small, just set mid value
              val_new = (newMax + newMin) / 2.0;
              if (s == 0) // only insert once into set
                setOfConstantMetabolites.insert(key);
            }
          dataSet.putValueForSpecies(key, val_new);
        }
      dataSets.erase(s);
      dataSets.insert (std::pair<C_INT32, CDataEntity>
                       (s, dataSet));
    }
}

void CQGLNetworkPainter::setConstantNodeSizeForAllSteps(std::string key, C_FLOAT64 val)
{
  CDataEntity dataSet;
  unsigned int s; // step number

  for (s = 0; s < dataSets.size(); s++) // for all steps
    {
      std::map<C_INT32, CDataEntity>::iterator iter = dataSets.find(s);
      if (iter != dataSets.end())
        {
          dataSet = (*iter).second;
          dataSet.putValueForSpecies(key, val);
        }
      dataSets.erase(s);
      dataSets.insert (std::pair<C_INT32, CDataEntity>
                       (s, dataSet));
    }
}

void CQGLNetworkPainter::setConstantNodeSize(std::string key, C_FLOAT64 val)
{
  CDataEntity dataSet;
  unsigned int s; // step number
  setOfConstantMetabolites.clear();
  for (s = 0; s < dataSets.size(); s++) // for all steps
    {
      std::map<C_INT32, CDataEntity>::iterator iter = dataSets.find(s);
      if (iter != dataSets.end())
        {
          dataSet = (*iter).second;
          // get old value
          dataSet.putValueForSpecies(key, val);
        }
      dataSets.erase(s);
      dataSets.insert (std::pair<C_INT32, CDataEntity>
                       (s, dataSet));
    }
}

// INFO: to rescale an inteval [a..b] to another interval [x..y] the following formula is used: (val_old in [a..b]
// val_new = x + ((val_old - a) * (y - x) / (b - a))

void CQGLNetworkPainter::rescaleDataSets(C_INT16 scaleMode)
{
  CDataEntity dataSet;
  unsigned int s; // step number
  C_FLOAT64 val, val_new;
  setOfConstantMetabolites.clear();
  for (s = 0; s < dataSets.size(); s++)
    {
      std::map<C_INT32, CDataEntity>::iterator iter = dataSets.find(s);
      if (iter != dataSets.end())
        {
          dataSet = (*iter).second;
          unsigned int i;
          // try to get VisParameters from parent (CQLayoutMainWindow)
          C_FLOAT64 minNodeSize = 10;
          C_FLOAT64 maxNodeSize = 100;
          if (pParentLayoutWindow != NULL)
            {
              if (pParentLayoutWindow->getMappingMode() == CVisParameters::COLOR_MODE)
                {
                  minNodeSize = 0.0;
                  maxNodeSize = 1.0; // 456 color values from black to red to yellow
                }
              else
                {
                  minNodeSize = pParentLayoutWindow->getMinNodeSize();
                  maxNodeSize = pParentLayoutWindow->getMaxNodeSize();
                }
            }
          for (i = 0; i < viewerNodes.size();i++) // iterate over string values (node keys)
            {
              // get old value
              val = dataSet.getValueForSpecies(viewerNodes[i]);
              if ((scaleMode == CVisParameters::INDIVIDUAL_SCALING) &&
                  (pParentLayoutWindow != NULL))
                {// global mode -> individual mode
                  // first get to original value
                  C_FLOAT64 orig_value = dataSet.getOrigValueForSpecies(viewerNodes[i]);
                  // recalculation of original value
                  if ((pSummaryInfo->getMaxForSpecies(viewerNodes[i]) - pSummaryInfo->getMinForSpecies(viewerNodes[i])) > CVisParameters::EPSILON)
                    {
                      // now rescale
                      val_new = ((orig_value - pSummaryInfo->getMinForSpecies(viewerNodes[i])) *
                                 (maxNodeSize - minNodeSize) /
                                 (pSummaryInfo->getMaxForSpecies(viewerNodes[i]) - pSummaryInfo->getMinForSpecies(viewerNodes[i])))
                                + minNodeSize;
                    }
                  else
                    {
                      val_new = (maxNodeSize + minNodeSize) / 2.0;
                      if (s == 0) // only insert once into set
                        setOfConstantMetabolites.insert(viewerNodes[i]);
                    }
                }
              else
                {// individual mode -> global mode
                  C_FLOAT64 orig_value = dataSet.getOrigValueForSpecies(viewerNodes[i]);
                  // first calculate original value
                  if ((pSummaryInfo->getMaxOverallConcentration() - pSummaryInfo->getMinOverallConcentration()) > CVisParameters::EPSILON)
                    {
                      // now rescale
                      val_new = ((orig_value - pSummaryInfo->getMinOverallConcentration()) *
                                 (maxNodeSize - minNodeSize) /
                                 (pSummaryInfo->getMaxOverallConcentration() - pSummaryInfo->getMinOverallConcentration()))
                                + minNodeSize;
                    }
                  else
                    val_new = (maxNodeSize + minNodeSize) / 2.0;
                }

              dataSet.putValueForSpecies(viewerNodes[i], val_new);
            }
          dataSets.erase(s);
          dataSets.insert (std::pair<C_INT32, CDataEntity>
                           (s, dataSet));
        }
    }
}

//tries to load data from time series,
//if this is successful true is returned, else false
bool CQGLNetworkPainter::createDataSets()
{
  int counter = 0;
  bool loadDataSuccessful = false;
  if (CCopasiDataModel::Global != NULL)
    {
      CTrajectoryTask *ptask = dynamic_cast< CTrajectoryTask * >((*CCopasiDataModel::Global->getTaskList())["Time-Course"]);
      const CTimeSeries* pTimeSer = &ptask->getTimeSeries();
      CTimeSeries dummyTimeSeries;
      if (pTimeSer->getRecordedSteps() == 0)
        {
          // create a dummy time series from the current state
          dummyTimeSeries.allocate(1);
          std::vector<CCopasiContainer*> tmpV;
          dummyTimeSeries.compile(tmpV);
          dummyTimeSeries.output(COutputInterface::DURING);
          assert(dummyTimeSeries.getRecordedSteps() == 1);
          pTimeSer = &dummyTimeSeries; // point to the dummy time series
        }
      if (pTimeSer->getNumVariables() > 0)
        {
          dataSets.clear(); // remove old data sets
          pSummaryInfo = new CSimSummaryInfo(pTimeSer->getRecordedSteps(), pTimeSer->getNumVariables(),
                                             pTimeSer->getConcentrationData(pTimeSer->getRecordedSteps() - 1, 0) - pTimeSer->getConcentrationData(0, 0));
          unsigned int i;
          unsigned int t;
          C_FLOAT64 val;
          std::string name;
          std::string objKey;
          std::string ndKey;
          C_FLOAT64 minR;
          C_FLOAT64 maxR;
          C_FLOAT64 maxAll = 0.0;
          // now get some info about the data set such as the maximum concentration values for each reactant
          for (i = 0; i < pTimeSer->getNumVariables(); i++) // iterate on reactants
            {
              maxR = - DBL_MAX;
              minR = DBL_MAX;
              name = pTimeSer->getTitle(i);
              objKey = pTimeSer->getKey(i);
              std::map<std::string, std::string>::iterator iter = keyMap.find(objKey);
              if (iter != keyMap.end())
                {// if there is a node (key)
                  ndKey = (keyMap.find(objKey))->second;
                  for (t = 0;t < pTimeSer->getRecordedSteps();t++) // iterate on time steps t=0..n
                    {
                      val = pTimeSer->getConcentrationData(t, i);

                      if (val > maxR)
                        maxR = val;
                      if (val < minR)
                        minR = val;
                    }
                  pSummaryInfo->storeMax(ndKey, maxR);
                  pSummaryInfo->storeMin(ndKey, minR);
                  if (maxR > maxAll)
                    maxAll = maxR;
                }
            }
          pSummaryInfo->setMaxOverallConcentration(maxAll);
          // now create data sets for visualization/animation
          // try to get VisParameters from parent (CQLayoutMainWindow)
          C_FLOAT64 minNodeSize = 10;
          C_FLOAT64 maxNodeSize = 100;
          if (pParentLayoutWindow != NULL)
            {
              minNodeSize = pParentLayoutWindow->getMinNodeSize();
              maxNodeSize = pParentLayoutWindow->getMaxNodeSize();
            }
          for (t = 0; t < pTimeSer->getRecordedSteps(); t++)  // iterate on time steps t=0..n
            {
              CDataEntity dataSet;
              for (i = 0;i < pTimeSer->getNumVariables();i++) // iterate on reactants
                {
                  objKey = pTimeSer->getKey(i); // object key os dbml species
                  std::map<std::string, std::string>::iterator iter = keyMap.find(objKey);
                  if (iter != keyMap.end())
                    {// if there is a node (key)
                      ndKey = (keyMap.find(objKey))->second; // key of graphical node
                      val = pTimeSer->getConcentrationData(t, i); // get concentration of species i at timepoint t
                      C_FLOAT64 scaledVal;
                      // now scale value;
                      if (pParentLayoutWindow->getScalingMode() == CVisParameters::INDIVIDUAL_SCALING)
                        {
                          minR = pSummaryInfo->getMinForSpecies(ndKey);
                          maxR = pSummaryInfo->getMaxForSpecies(ndKey);
                        }
                      else
                        {// == CVisParameters.GLOBAL_SCALING
                          minR = pSummaryInfo->getMinOverallConcentration();
                          maxR = pSummaryInfo->getMaxOverallConcentration();
                        }
                      if ((maxR - minR) > CVisParameters::EPSILON)
                        scaledVal = minNodeSize +
                                    (((maxNodeSize - minNodeSize) / (maxR - minR))
                                     * (val - minR));
                      else
                        scaledVal = (maxNodeSize + minNodeSize) / 2.0;
                      // put scaled value in data entity (collection of scaled values for one step)
                      dataSet.putValueForSpecies(ndKey, scaledVal);
                      dataSet.putOrigValueForSpecies(ndKey, val);
                    }
                }
              // now collect data set
              dataSets.insert(std::pair<C_INT32, CDataEntity>
                              (t, dataSet));
              counter++;
            }
          loadDataSuccessful = true;
        }
      else
        std::cout << "empty time series: no variables present" << std::endl;
    }
  this->mDataPresentP = loadDataSuccessful;
  if (loadDataSuccessful)
    {// if loading was successful, parent should create data table to show it in its window
      std::map<C_INT32, CDataEntity>::iterator iter = dataSets.find(0); // get data for first step
      if (iter != dataSets.end())
        pParentLayoutWindow->insertValueTable((*iter).second);
    }
  return loadDataSuccessful;
}

C_INT32 CQGLNetworkPainter::getNumberOfSteps()
{
  return dataSets.size();
}

void CQGLNetworkPainter::runAnimation()
{
  this->mLabelShape = CIRCLE;
  if (dataSets.size() == 0)
    this->createDataSets(); // load data if this was not done before

  // try to get VisParameters from parent (CQLayoutMainWindow)

  C_INT16 stepsPerSecond = 10;
  if (pParentLayoutWindow != NULL)
    {
      pParentLayoutWindow->setAnimationRunning(true);
      stepsPerSecond = pParentLayoutWindow->getStepsPerSecond();
    }

  regularTimer->start((int)(1000 / stepsPerSecond), false); // emit signal in chosen framerate
}

void CQGLNetworkPainter::triggerAnimationStep()
{
  C_INT32 numberOfSteps = 100;
  bool animationRunning = true;
  if (pParentLayoutWindow != NULL)
    {
      //check whether animation is running
      animationRunning = pParentLayoutWindow->getAnimationRunning();
    }

  numberOfSteps = getNumberOfSteps();
  if ((stepShown <= numberOfSteps) &&
      (animationRunning))
    {
      // set value in slider
      emit stepChanged(stepShown);
      this->stepShown++;
    }
  else
    {
      regularTimer->stop();
      emit endOfAnimationReached();
    }
}

CDataEntity* CQGLNetworkPainter::getDataSetAt(C_INT32 stepNumber)
{
  CDataEntity* pDataSet = NULL;
  if ((0 <= stepNumber) && (static_cast<unsigned int>(stepNumber) < dataSets.size()))
    {
      std::map<C_INT32, CDataEntity>::iterator iter = dataSets.find(stepNumber);
      if (iter != dataSets.end())
        {
          pDataSet = &((*iter).second);
        }
    }
  return pDataSet;
}

void CQGLNetworkPainter::showStep(C_INT32 stepNumber)
{
  this->stepShown = stepNumber;
  if (this->mLabelShape != CIRCLE)
    this->mLabelShape = CIRCLE;
  if ((0 <= stepNumber) && (static_cast<unsigned int>(stepNumber) < dataSets.size()))
    {
      std::map<C_INT32, CDataEntity>::iterator iter = dataSets.find(stepNumber);
      if (iter != dataSets.end())
        {
          CDataEntity dataSet = (*iter).second;
          unsigned int i;
          for (i = 0; i < viewerNodes.size();i++)
            {
              if (pParentLayoutWindow != NULL)
                {
                  C_FLOAT64 val = dataSet.getValueForSpecies(viewerNodes[i]);
                  if (pParentLayoutWindow->getMappingMode() != CVisParameters::COLOR_MODE)
                    {// no color mode

                      if (val != -DBL_MAX)
                        if (isnan(val)) // test for nan
                          {
                            std::cout << "nan value found for " << viewerNodes[i] << std::endl;

                            std::map<std::string, CGraphNode>::iterator itNodeObj = nodeMap.find(viewerNodes[i]);
                            if (itNodeObj != nodeMap.end())
                              std::cout << (*itNodeObj).second << std::endl;
                            setNodeSize(viewerNodes[i], CVisParameters::DEFAULT_NODE_SIZE);
                          }
                        else
                          setNodeSize(viewerNodes[i], val);
                    }
                  else // COLOR_MODE
                    {
                      setNodeSize(viewerNodes[i], CVisParameters::DEFAULT_NODE_SIZE);
                      if (val != -DBL_MAX)
                        if (isnan(val)) // test for nan
                          {
                            std::cout << "nan value found: " << viewerNodes[i] << std::endl;
                            setNodeSize(viewerNodes[i], CVisParameters::DEFAULT_NODE_SIZE);
                          }
                        else
                          setNodeSizeWithoutChangingCurves(viewerNodes[i], val);
                    }
                }
            }
        }
    }
  this->drawGraph();
}

void CQGLNetworkPainter::setNodeSizeWithoutChangingCurves(std::string key, C_FLOAT64 val)
{
  std::map<std::string, CGraphNode>::iterator nodeIt;
  nodeIt = nodeMap.find(key);
  if (nodeIt != nodeMap.end())
    (*nodeIt).second.setSize(val);
}

// set node sizes according to data set and change curves (meaning end points of curve segments) to nodes
void CQGLNetworkPainter::setNodeSize(std::string key, C_FLOAT64 val)
{
  // curves to nodes are changed, arrows are created newly
  nodeArrowMap.clear();
  std::map<std::string, CGraphNode>::iterator nodeIt;
  nodeIt = nodeMap.find(key);
  if (nodeIt != nodeMap.end())
    (*nodeIt).second.setSize(val);
  // now adaptCurves pointing to nodes
  std::pair<std::multimap<std::string, CGraphCurve>::iterator, std::multimap<std::string, CGraphCurve>::iterator> curveRangeIt;;
  curveRangeIt = nodeCurveMap.equal_range(key);
  std::multimap<std::string, CGraphCurve>::iterator curveIt;
  curveIt = curveRangeIt.first;
  while (curveIt != curveRangeIt.second)
    {
      CGraphCurve *pCurve = & (*curveIt).second;
      if (pCurve != NULL)
        {
          CLLineSegment* pLastSeg = pCurve->getSegmentAt(pCurve->getNumCurveSegments() - 1); // get pointer to last segment
          // move end point of segment along the line from the circle center(=from) to the current end point of the last segment
          // so that it lies on the border of the circle
          CLPoint to;
          if (pLastSeg->isBezier())
            to = pLastSeg->getBase2();
          else
            to = pLastSeg->getStart();
          CLPoint from = CLPoint((*nodeIt).second.getX() + ((*nodeIt).second.getWidth() / 2.0), (*nodeIt).second.getY() + ((*nodeIt).second.getHeight() / 2.0)); // center of bounding box and also of circle
          C_FLOAT64 distance = sqrt(((to.getX() - from.getX()) * (to.getX() - from.getX())) + ((to.getY() - from.getY()) * (to.getY() - from.getY())));

          C_FLOAT64 circleDist = ((*nodeIt).second.getSize() / 2.0) + 4.0; // near border
          C_FLOAT64 newX = from.getX() + ((to.getX() - from.getX()) / distance * circleDist);
          C_FLOAT64 newY = from.getY() + ((to.getY() - from.getY()) / distance * circleDist);

          //C_FLOAT64 dx, dy;
          //dx = to.getX() - newX;
          //dy = to.getY() - newY;
          pLastSeg->setEnd(CLPoint(newX, newY));
          // std::cout << "2. last segment: " << *pLastSeg << std::endl;
          // now insert new arrow in map
          if (pLastSeg->isBezier())
            {// for bezier curves, move base points too
              //pLastSeg->setBase1(CLPoint(pLastSeg->getBase1().getX() + dx,pLastSeg->getBase1().getY() + dy));
              //pLastSeg->setBase2(CLPoint(pLastSeg->getBase2().getX() + dx,pLastSeg->getBase2().getY() + dy));
            }
          CLPoint p = pLastSeg->getEnd();
          if (pCurve->hasArrowP())
            {
              CArrow *ar = new CArrow(*pLastSeg, p.getX(), p.getY(), this->mCurrentZoom);
              nodeArrowMap.insert(std::pair<std::string, CArrow>
                                  (key, *ar));
              pCurve->setArrow(*ar);
              delete ar;
            }
        }
      curveIt++;
    }
}

void CQGLNetworkPainter::mapLabelsToRectangles()
{
  this->mLabelShape = RECTANGLE;
  nodeArrowMap.clear(); // map is filled with new arrows
  std::pair<std::multimap<std::string, CGraphCurve>::iterator, std::multimap<std::string, CGraphCurve>::iterator> rangeCurveIt;
  std::multimap<std::string, CGraphCurve>::iterator curveIt;
  unsigned int i;
  for (i = 0;i < viewerNodes.size();i++)
    {
      rangeCurveIt = nodeCurveMap.equal_range(viewerNodes[i]);
      std::map<std::string, CGraphNode>::iterator nodeIt = nodeMap.find(viewerNodes[i]); // find all edges belonging to a node
      if (nodeIt != nodeMap.end())
        {
          curveIt = rangeCurveIt.first;
          while (curveIt != rangeCurveIt.second)
            {
              this->adaptCurveForRectangles(curveIt, (*nodeIt).second.getBoundingBox());
              curveIt++;
            }
        }
    }
  this->drawGraph(); // this function will draw the bounding box for each node
}

CLPoint CQGLNetworkPainter::getPointOnRectangle(CLBoundingBox r, CLPoint p)
{
  CLPoint onpoint;
  CLPoint q = r.getPosition();
  q.setX(r.getPosition().getX() + r.getDimensions().getWidth()); // q is now top right point of rectangle
  CLPoint center; // center of rectangle
  center.setX(r.getPosition().getX() + (r.getDimensions().getWidth() / 2.0));
  center.setY(r.getPosition().getY() + (r.getDimensions().getHeight() / 2.0)); //

  C_FLOAT64 qAngle = atan((q.getY() - center.getY()) / (q.getX() - center.getX()));
  C_FLOAT64 pAngle = atan((p.getY() - center.getY()) / (p.getX() - center.getX()));

  if (fabs(pAngle) < fabs(qAngle))
    {// intersection point is left or right side
      if (p.getX() > center.getX()) // right side
        onpoint = CLPoint(q.getX(), center.getY());
      else // left side
        onpoint = CLPoint(r.getPosition().getX(), center.getY());
    }
  else
    {//intersection point is top or bottom side
      if (p.getY() > center.getY()) // top side
        onpoint = CLPoint(center.getX(), r.getPosition().getY() + r.getDimensions().getHeight());
      else // bottom side
        onpoint = CLPoint(center.getX(), r.getPosition().getY());
    }
  return onpoint;
}

void CQGLNetworkPainter::mapLabelsToCircles()
{
  this->mLabelShape = CIRCLE;

  nodeArrowMap.clear(); // map is filled with new arrows
  std::pair<std::multimap<std::string, CGraphCurve>::iterator, std::multimap<std::string, CGraphCurve>::iterator> rangeCurveIt;
  std::multimap<std::string, CGraphCurve>::iterator curveIt;
  unsigned int i;
  for (i = 0;i < viewerNodes.size();i++)
    {
      rangeCurveIt = nodeCurveMap.equal_range(viewerNodes[i]);
      std::map<std::string, CGraphNode>::iterator nodeIt = nodeMap.find(viewerNodes[i]); // find all edges belonging to a node
      if (nodeIt != nodeMap.end())
        {
          curveIt = rangeCurveIt.first;
          while (curveIt != rangeCurveIt.second)
            {
              this->adaptCurveForCircle(curveIt, (*nodeIt).second.getBoundingBox());
              curveIt++;
            }
        }
    }

  this->drawGraph();
}

// get Point on Circle border on the line from the center of the given rectangle to the given point p
CLPoint CQGLNetworkPainter::getPointOnCircle(CLBoundingBox r, CLPoint p)
{
  CLPoint center; // center of rectangle
  center.setX(r.getPosition().getX() + (r.getDimensions().getWidth() / 2.0));
  center.setY(r.getPosition().getY() + (r.getDimensions().getHeight() / 2.0));

  C_FLOAT64 distance = sqrt(((p.getX() - center.getX()) * (p.getX() - center.getX())) + ((p.getY() - center.getY()) * (p.getY() - center.getY())));

  C_FLOAT64 onPointX = center.getX() + ((p.getX() - center.getX()) / distance * CVisParameters::DEFAULT_NODE_SIZE / 2.0);
  C_FLOAT64 onPointY = center.getY() + ((p.getY() - center.getY()) / distance * CVisParameters::DEFAULT_NODE_SIZE / 2.0);

  return CLPoint(onPointX, onPointY);
}

// get Point  on the line from the center of the given rectangle to the given point p with the distance d to the circle border
CLPoint CQGLNetworkPainter::getPointNearCircle(CLBoundingBox r, CLPoint p, C_INT16 d)
{
  CLPoint center; // center of rectangle
  center.setX(r.getPosition().getX() + (r.getDimensions().getWidth() / 2.0));
  center.setY(r.getPosition().getY() + (r.getDimensions().getHeight() / 2.0));

  C_FLOAT64 distance = sqrt(((p.getX() - center.getX()) * (p.getX() - center.getX())) + ((p.getY() - center.getY()) * (p.getY() - center.getY())));

  C_FLOAT64 onPointX = center.getX() + ((p.getX() - center.getX()) / distance * ((CVisParameters::DEFAULT_NODE_SIZE / 2.0) + d));
  C_FLOAT64 onPointY = center.getY() + ((p.getY() - center.getY()) / distance * ((CVisParameters::DEFAULT_NODE_SIZE / 2.0) + d));

  return CLPoint(onPointX, onPointY);
}

// move one or two points of a curve, so that the end point of the curve ends at the circle given by the center of the bounding box (where the diagonals intersect) that is given in the parameters and that has the default size
void CQGLNetworkPainter::adaptCurveForCircle(std::multimap<std::string, CGraphCurve>::iterator it, CLBoundingBox box)
{
  CLLineSegment* pLastSeg = (*it).second.getSegmentAt((*it).second.getNumCurveSegments() - 1);
  CLPoint pointOnCircle;

  if (pLastSeg->isBezier())
    pointOnCircle = getPointNearCircle(box, pLastSeg->getBase2(), 1);
  else
    pointOnCircle = getPointNearCircle(box, pLastSeg->getStart(), 1);

  pLastSeg->setEnd(pointOnCircle);

  // create corresponding arrow, if necessary and insert it into map
  if (((*it).second).hasArrowP())
    {
      CLPoint p = pLastSeg->getEnd();
      CArrow *ar;
      if (pLastSeg->isBezier())
        {
          BezierCurve *bezier = new BezierCurve();
          std::vector<CLPoint> pts = std::vector<CLPoint>();
          pts.push_back(pLastSeg->getStart());
          pts.push_back(pLastSeg->getBase1());
          pts.push_back(pLastSeg->getBase2());
          pts.push_back(pLastSeg->getEnd());
          std::vector<CLPoint> bezierPts = bezier->curvePts(pts);
          C_INT32 num = bezierPts.size();
          CLLineSegment segForArrow = CLLineSegment(bezierPts[num - 2], bezierPts[num - 1]);
          ar = new CArrow(segForArrow, bezierPts[num - 1].getX(), bezierPts[num - 1].getY(), this->mCurrentZoom);
        }
      else
        ar = new CArrow(*pLastSeg, p.getX(), p.getY(), this->mCurrentZoom);

      nodeArrowMap.insert(std::pair<std::string, CArrow>
                          ((*it).first, *ar));
      ((*it).second).setArrowP(true);
      ((*it).second).setArrow(*ar);
    }
}

// move one or two points of a curve, so that the end point of the curve ends at the box given in the parameters
void CQGLNetworkPainter::adaptCurveForRectangles(std::multimap<std::string, CGraphCurve>::iterator it, CLBoundingBox box)
{
  // while (it != nodeCurveMap.end()){
  CLLineSegment* pLastSeg = (*it).second.getSegmentAt((*it).second.getNumCurveSegments() - 1);
  CLPoint pointOnRect;
  if (pLastSeg->isBezier())
    pointOnRect = getPointOnRectangle(box, pLastSeg->getBase2());
  else
    pointOnRect = getPointOnRectangle(box, pLastSeg->getStart());
  pLastSeg->setEnd(pointOnRect);

  // create corresponding arrow, if necessary and insert it into map
  CLPoint p = pLastSeg->getEnd();

  if (((*it).second).hasArrowP())
    {
      CArrow *ar;
      if (pLastSeg->isBezier())
        {
          BezierCurve *bezier = new BezierCurve();
          std::vector<CLPoint> pts = std::vector<CLPoint>();
          pts.push_back(pLastSeg->getStart());
          pts.push_back(pLastSeg->getBase1());
          pts.push_back(pLastSeg->getBase2());
          pts.push_back(pLastSeg->getEnd());
          std::vector<CLPoint> bezierPts = bezier->curvePts(pts);
          C_INT32 num = bezierPts.size();
          CLLineSegment segForArrow = CLLineSegment(bezierPts[num - 2], bezierPts[num - 1]);
          ar = new CArrow(segForArrow, bezierPts[num - 1].getX(), bezierPts[num - 1].getY(), this->mCurrentZoom);
        }
      else
        ar = new CArrow(*pLastSeg, p.getX(), p.getY(), this->mCurrentZoom);

      nodeArrowMap.insert(std::pair<std::string, CArrow>
                          ((*it).first, *ar));
      ((*it).second).setArrowP(true);
      ((*it).second).setArrow(*ar);
    }
}

// looks for the best point to make a line between a given point p and a rectangle r.
// The point to connect to should always lie on the border of the rectangle and, more specifically
// on the middle of one of the border lines

void CQGLNetworkPainter::createActions()
{
  zoomInAction = new QAction ("zoom in",
                              "Zoom in",
                              CTRL + Key_P,
                              this);
  connect(zoomInAction, SIGNAL(activated()), this, SLOT(zoomIn()));

  zoomOutAction = new QAction ("zoom out",
                               "Zoom out",
                               CTRL + Key_M,
                               this);
  connect(zoomOutAction, SIGNAL(activated()), this, SLOT(zoomOut()));

  setFontSizeAction = new QAction("set font size",
                                  "Set Font Size",
                                  CTRL + Key_F,
                                  this);
  connect(setFontSizeAction, SIGNAL(activated()), this, SLOT(setFontSize()));
}

void CQGLNetworkPainter::setFontSize()
{
  FontChooser *fCh = new FontChooser(this);
  fCh->exec();
}

void CQGLNetworkPainter::zoomIn()
{
  emit signalZoomIn();
}

void CQGLNetworkPainter::zoomOut()
{
  emit signalZoomOut();
}

void CQGLNetworkPainter::zoomGraph(C_FLOAT64 zoomFactor)
{
  this->zoom(zoomFactor);
}

void CQGLNetworkPainter::zoom(C_FLOAT64 zoomFactor)
{
  //std::cout << "zoom  " << zoomFactor << std::endl;
  this->mCurrentZoom *= zoomFactor;

  CLPoint cMax = CLPoint(this->mgraphMax.getX() * zoomFactor, this->mgraphMax.getY() * zoomFactor);
  //this->setGraphSize(this->mgraphMin, cMax);

  if (pParentLayoutWindow != NULL)
    {
      C_FLOAT64 oldMin = pParentLayoutWindow->getMinNodeSize();
      C_FLOAT64 oldMax = pParentLayoutWindow->getMaxNodeSize();
      pParentLayoutWindow->setMinNodeSize(pParentLayoutWindow->getMinNodeSize() * zoomFactor);
      pParentLayoutWindow->setMaxNodeSize(pParentLayoutWindow->getMaxNodeSize() * zoomFactor);
      unsigned int i;

      if ((mDataPresentP) && (pParentLayoutWindow->getMappingMode() != CVisParameters::COLOR_MODE)) // only rescale data set values in size mode and when data to be rescaled is present
        {
          rescaleDataSetsWithNewMinMax(oldMin, oldMax, pParentLayoutWindow->getMinNodeSize(), pParentLayoutWindow->getMaxNodeSize(), pParentLayoutWindow->getScalingMode());
        }
      //scale node sizes if not in color mode
      for (i = 0;i < this->viewerNodes.size();i++)
        {
          std::map<std::string, CGraphNode>::iterator nodeIt;
          nodeIt = nodeMap.find(viewerNodes[i]);
          if (nodeIt != nodeMap.end())
            (*nodeIt).second.scale(zoomFactor, (pParentLayoutWindow->getMappingMode() != CVisParameters::COLOR_MODE)); // change position in any way, but size only when not in color mode
        }

      //scale curves not directly pointing to a reactant/species node
      for (i = 0;i < viewerCurves.size();i++)
        {
          this->viewerCurves[i].scale(zoomFactor);
        }
      //scale curves that are associated with a reactant/species node (i.e. directly points to it)
      for (i = 0; i < viewerNodes.size();i++)
        {
          std::pair<std::multimap<std::string, CGraphCurve>::iterator, std::multimap<std::string, CGraphCurve>::iterator> curveRangeIt;
          std::multimap<std::string, CGraphCurve>::iterator curveIt;

          curveRangeIt = nodeCurveMap.equal_range(viewerNodes[i]);
          curveIt = curveRangeIt.first;
          while (curveIt != curveRangeIt.second)
            {
              ((*curveIt).second).scale(zoomFactor); // scale curve
              curveIt++;
            }
        }
      // common fontname and size for all labels are stored in this class
      // each label size is always computed from the labels original size value
      // and scaled byc urrentZoom (which is the product of all zoomFactors applied so far)
      this->mFontsizeDouble = this->mFontsizeDouble * zoomFactor;
      this->mFontsize = (int)this->mFontsizeDouble;
      //std::cout << "new fontsize: " << this->mFontsize << std::endl;
      for (i = 0;i < viewerLabels.size();i++)
        {
          if (!preserveMinLabelHeightP)
            this->viewerLabels[i].scale(mCurrentZoom);
          else
            {
              //std::cout << "height of label: " << this->viewerLabels[i].getHeight() << " *  " << zoomFactor << std::endl;

              if ((this->viewerLabels[i].getOrigHeight() * mCurrentZoom) >= MIN_HEIGHT)
                this->viewerLabels[i].scale(mCurrentZoom);
              else
                {
                  //std::cout << "set font size to MIN_HEIGHT " << std::endl;
                  this->mFontsizeDouble = (double) MIN_HEIGHT;
                  this->mFontsize = MIN_HEIGHT;
                  this->viewerLabels[i].adaptToHeight(MIN_HEIGHT);
                  this->viewerLabels[i].scalePosition(zoomFactor);
                }
            }
        }
      for (i = 0;i < viewerNodes.size();i++)
        {
          std::pair<std::multimap<std::string, CArrow>::iterator, std::multimap<std::string, CArrow>::iterator> arrowRangeIt;
          std::multimap<std::string, CArrow>::iterator arrowIt;
          arrowRangeIt = nodeArrowMap.equal_range(viewerNodes[i]);
          arrowIt = arrowRangeIt.first;
          while (arrowIt != arrowRangeIt.second)
            {
              (*arrowIt).second.scale(zoomFactor); //scale arrow
              arrowIt++;
            }
        }
    }
  createTextureForAllLabels();
  this->drawGraph();
}

void CQGLNetworkPainter::setFontSizeForLabels(unsigned int fs)
{
  this->mFontsizeDouble = fs;
  this->mFontsize = (int)this->mFontsizeDouble;

  unsigned int i;
  for (i = 0;i < viewerLabels.size();i++)
    {
      this->viewerLabels[i].adaptToHeight(fs);
    }
  createTextureForAllLabels();
  this->drawGraph();
  this->update();
}

QImage CQGLNetworkPainter::getImage()
{
  return this->grabFrameBuffer();
}

void CQGLNetworkPainter::contextMenuEvent(QContextMenuEvent *cme)
{
  QPopupMenu *contextMenu = new QPopupMenu(this);
  zoomInAction->addTo(contextMenu);
  zoomOutAction->addTo(contextMenu);
  setFontSizeAction->addTo(contextMenu);
  contextMenu->exec(cme->globalPos());
}

void CQGLNetworkPainter::testOpenGL()
{
  glLoadIdentity();
  glTranslatef(10.0f, 10.0f, -1.0f);
  glBegin(GL_TRIANGLES);          // Drawing Using Triangles
  glColor3f(0.0f, 0.0f, 1.0f);
  glVertex3f(0.0f, 10.0f, 0.0f);    // Top
  glVertex3f(-10.0f, -10.0f, 0.0f);    // Bottom Left
  glVertex3f(10.0f, -10.0f, 0.0f);    // Bottom Right
  glEnd();

  glTranslatef(3.0f, 0.0f, 0.0f);
  glBegin(GL_QUADS);          // Draw A Quad
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex3f(-1.0f, 1.0f, 0.0f);    // Top Left
  glVertex3f(1.0f, 1.0f, 0.0f);    // Top Right
  glVertex3f(1.0f, -1.0f, 0.0f);    // Bottom Right
  glVertex3f(-1.0f, -1.0f, 0.0f);    // Bottom Left
  glEnd();       // Done Drawing The Quad

  glTranslatef(3.5f, 0.0f, 0.0f);
  glBegin(GL_POLYGON);     // Ein Polygon (in diesem Falle ein Achteck.)
  // jede Ecke bekommt eine andere Farbe
  glColor3f(1.0f, 0.0f, 0.0f); // rot
  glVertex3f(-0.5f, 1.5f, 0.0f); // obere Ecke links
  glVertex3f(0.5f, 1.5f, 0.0f); // obere Ecke rechts

  glColor3f(0.0f, 0.0f, 1.0f); // blau
  glVertex3f(1.5f, 0.5f, 0.0f); // rechte Ecke oben
  glVertex3f(1.5f, -0.5f, 0.0f); // rechte Ecke unten

  glColor3f(0.0f, 1.0f, 0.0f); // gruen
  glVertex3f(0.5f, -1.5f, 0.0f); // untere Ecke rechts
  glVertex3f(-0.5f, -1.5f, 0.0f); // untere Ecke links

  glColor3f(1.0f, 1.0f, 0.0f); // gelb
  glVertex3f(-1.5f, -0.5f, 0.0f); // linke Ecke unten
  glVertex3f(-1.5f, 0.5f, 0.0f); // linke Ecke oben
  glEnd(); // Zeichenaktion beenden
}

bool CQGLNetworkPainter::isCircleMode()
{
  if (this->mLabelShape == CIRCLE)
    return true;
  else return false;
}

void CQGLNetworkPainter::initializeGraphPainter(QWidget *parent)
{
  mCurrentZoom = 1.0;
  mCurrentPositionX = 0.0;
  mCurrentPositionX = 0.0;
  mLabelShape = RECTANGLE;
  mgraphMin = CLPoint(0.0, 0.0);
  mgraphMax = CLPoint(250.0, 250.0);
  //mFontname = "Helvetica";
  mFontname = "Arial";
  mFontsize = 12;
  mFontsizeDouble = 12.0; // to avoid rounding errors due to zooming in and out
  mDataPresentP = false;
  preserveMinLabelHeightP = true;

  mf = QFont(FROM_UTF8(mFontname));
  mf.setPointSize(this->mFontsize);
  const QFont& mfRef = mf;
  QFontMetrics mfm = QFontMetrics(mfRef);

  // parent structure: glPainter -> CQGLViewport -> splitter ->
  // vbox -> mainWindow
  QWidget *ancestor = parent->parentWidget();
  while (ancestor && dynamic_cast<CQLayoutMainWindow*>(ancestor) == NULL)
    {
      ancestor = ancestor->parentWidget();
    }
  assert(ancestor != NULL);
  connect(this, SIGNAL(stepChanged(C_INT32)), ancestor, SLOT(changeStepValue(C_INT32)));
  connect(this, SIGNAL(endOfAnimationReached()), ancestor, SLOT(endOfAnimationReached()));
  regularTimer = new QTimer(this);
  connect(regularTimer, SIGNAL(timeout()), this, SLOT(triggerAnimationStep()));

  CQLayoutMainWindow * tmp = dynamic_cast<CQLayoutMainWindow *>(ancestor);
  assert(tmp);
  if (tmp)
    pParentLayoutWindow = tmp;
  else
    pParentLayoutWindow = NULL;

  stepShown = 0;
  createActions();
}

void CQGLNetworkPainter::initializeGL()
{
  qglClearColor(QColor(255, 255, 240, QColor::Rgb));
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  glDisable(GL_ALPHA_TEST);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);

  glShadeModel(GL_SMOOTH);

  glGenTextures(1, textureNames);
  // convert the node into a display list that is created once and call once
  // for each node.
  // this might safe some cpu cycles, especially when the nodes get more fancy.
  this->mDisplayLists = glGenLists(2);
  GLUquadricObj* qobj = NULL;
  // the first display list if for a background plane that allows us to create
  // nice shadows
  CLPoint p1 = this->getGraphMin();
  CLPoint p2 = this->getGraphMax();
  glNewList(mDisplayLists, GL_COMPILE);
  glColor3f(1.0f, 1.0f, 0.94);
  glBegin(GL_POLYGON);
  glVertex3d(p1.getX(), p1.getY(), CQGLNetworkPainter::PLANE_DEPTH);
  glVertex3d(p2.getX(), p1.getY(), CQGLNetworkPainter::PLANE_DEPTH);
  glVertex3d(p2.getX(), p2.getY(), CQGLNetworkPainter::PLANE_DEPTH);
  glVertex3d(p1.getX(), p2.getY(), CQGLNetworkPainter::PLANE_DEPTH);
  glEnd();
  glEndList();

  // second list is for the rectangular nodes
  glNewList(mDisplayLists + 1, GL_COMPILE);
  glBegin(GL_POLYGON);
  glVertex2d(0.05, 0.0);
  glVertex2d(0.95, 0.0);
  glVertex2d(0.95, 0.05);
  glVertex2d(1.0 , 0.05);
  glVertex2d(1.0 , 0.95);
  glVertex2d(0.95, 0.95);
  glVertex2d(0.95, 1.0);
  glVertex2d(0.05, 1.0);
  glVertex2d(0.05, 0.95);
  glVertex2d(0.0 , 0.95);
  glVertex2d(0.0 , 0.05);
  glVertex2d(0.05, 0.05);
  glEnd();
  qobj = gluNewQuadric();
  gluQuadricDrawStyle(qobj, GLU_FILL);
  glPushMatrix();
  glTranslatef(0.05f, 0.05f , 0.0f);
  gluPartialDisk(qobj, 0.0, 0.05, 10, 10, 180, 90);
  glTranslatef(0.90f, 0.0f , 0.0f);
  gluPartialDisk(qobj, 0.0, 0.05, 10, 10, 90, 90);
  glTranslatef(0.0f, 0.90f, 0.0f);
  gluPartialDisk(qobj, 0.0, 0.05, 10, 10, 0, 90);
  glTranslatef(-0.90f , 0.0f , 0.0f);
  gluPartialDisk(qobj, 0.0, 0.05, 10, 10, 270, 90);
  glPopMatrix();
  gluDeleteQuadric(qobj);
  glEndList();
}

void CQGLNetworkPainter::resizeGL(int w, int h)
{
  // setup viewport, projection etc.:
  glViewport(0, 0, (GLint)w, (GLint)h);

  glMatrixMode(GL_PROJECTION);    // Select The Projection Matrix
  glLoadIdentity();             // Reset The Projection Matrix
  //  gluOrtho2D((GLdouble)mgraphMin.getX(),
  //             (GLdouble)mgraphMax.getX(),
  //             (GLdouble)mgraphMax.getY(),
  //             (GLdouble)mgraphMin.getY()); // y: 0.0 is bottom left instead of top left as in SBML
  gluOrtho2D((GLdouble)mCurrentPositionX,
             (GLdouble)(mCurrentPositionX + w / mCurrentZoom),
             (GLdouble)(mCurrentPositionY + h / mCurrentZoom),
             (GLdouble)mCurrentPositionY); // y: 0.0 is bottom left instead of top left as in SBML
  glMatrixMode(GL_MODELVIEW);  // Select The Modelview Matrix
}

void CQGLNetworkPainter::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear The Screen And The Depth Buffer
  draw();
  glFlush();
}

void CQGLNetworkPainter::printNodeMap()
{
  std::cout << " node ids to label text mappings: " << std::endl;
  std::map<std::string, CGraphNode>::iterator nodeIt;
  nodeIt = nodeMap.begin();
  while (nodeIt != nodeMap.end())
    {
      std::cout << (*nodeIt).first << "  :  " << (*nodeIt).second.getLabelText() << std::endl;
      nodeIt++;
    }
}

void CQGLNetworkPainter::printNodeInfoForKey(std::string key)
{
  std::map<std::string, CGraphNode>::iterator itNodeObj = nodeMap.find(key);
  if (itNodeObj != nodeMap.end())
    std::cout << (*itNodeObj).second << std::endl;
}

std::string CQGLNetworkPainter::getNameForNodeKey(std::string key)
{
  std::string s = "UNKNOWN";
  std::map<std::string, CGraphNode>::iterator itNodeObj = nodeMap.find(key);
  if (itNodeObj != nodeMap.end())
    s = (*itNodeObj).second.getLabelText();
  return s;
}

std::string CQGLNetworkPainter::getNodeNameEntry(int i)
{
  if (i < static_cast< int >(viewerNodes.size()))
    return viewerNodes[i];
  else
    return "";
}

void CQGLNetworkPainter::printAvailableFonts()
{
  QFontDatabase fdb;
  QStringList families = fdb.families();
  for (QStringList::Iterator f = families.begin(); f != families.end(); ++f)
    {
      QString family = *f;
      qDebug(family);
      QStringList styles = fdb.styles(family);
      for (QStringList::Iterator s = styles.begin(); s != styles.end(); ++s)
        {
          QString style = *s;
          QString dstyle = "\t" + style + " (";
          QValueList<int> smoothies = fdb.smoothSizes(family, style);
          for (QValueList<int>::Iterator points = smoothies.begin();
               points != smoothies.end(); ++points)
            {
              dstyle += QString::number(*points) + " ";
            }
          dstyle = dstyle.left(dstyle.length() - 1) + ")";
          qDebug(dstyle);
        }
    }
}

void CQGLNetworkPainter::setZoomFactor(C_FLOAT64 zoom)
{
  if (zoom != this->mCurrentZoom)
    {
      this->mCurrentZoom = zoom;
      this->updateGL();
    }
}

C_FLOAT64 CQGLNetworkPainter::getZoomFactor() const
  {
    return this->mCurrentZoom;
  }

void CQGLNetworkPainter::setCurrentPosition(C_FLOAT64 x, C_FLOAT64 y)
{
  if (this->mCurrentPositionX != x || this->mCurrentPositionY != y)
    {
      this->mCurrentPositionX = x;
      this->mCurrentPositionY = y;
      this->update();
    }
}

void CQGLNetworkPainter::update()
{
  this->resizeGL(this->width(), this->height());
  this->updateGL();
}

void CQGLNetworkPainter::setCurrentPositionX(C_FLOAT64 x)
{
  if (this->mCurrentPositionX != x)
    {
      this->mCurrentPositionX = x;
      this->update();
    }
}

void CQGLNetworkPainter::setCurrentPositionY(C_FLOAT64 y)
{
  if (this->mCurrentPositionY != y)
    {
      this->mCurrentPositionY = y;
      this->update();
    }
}

C_FLOAT64 CQGLNetworkPainter::getCurrentPositionX() const
  {
    return this->mCurrentPositionX;
  }

C_FLOAT64 CQGLNetworkPainter::getCurrentPositionY() const
  {
    return this->mCurrentPositionY;
  }

void CQGLNetworkPainter::resetView()
{
  this->setZoomFactor(1.0);
  this->setCurrentPosition(this->getGraphMin().getX(), this->getGraphMin().getY());
}

void CQGLNetworkPainter::resetGraphToLabelView()
{
  this->mLabelShape = RECTANGLE;
  this->updateGL();
}

void CQGLNetworkPainter::pauseAnimation()
{
  regularTimer->stop();
}
