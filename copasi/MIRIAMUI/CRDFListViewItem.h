// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/MIRIAMUI/Attic/CRDFListViewItem.h,v $
//   $Revision: 1.4 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2008/01/29 20:16:42 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#ifndef COPASI_CRDFListViewItem
#define COPASI_CRDFListViewItem

#include <qlistview.h>

class CRDFListView;
class CRDFNode;
class CRDFEdge;

class CRDFListViewItem: public QListViewItem
  {
    // Operations
  public:
    /**
     * Default Constructor
     * @param CRDFListView * pParent
     * @param CRDFListViewItem * pAfter (default: NULL)
     */
    CRDFListViewItem(CRDFListView * pParent, CRDFListViewItem * pAfter = NULL);

    /**
     * Specific Constructor
     * @param CRDFListViewItem * pParent
     * @param CRDFListViewItem * pAfter (default: NULL)
     */
    CRDFListViewItem(CRDFListViewItem * pParent, CRDFListViewItem * pAfter = NULL);

    /**
     * Destructor
     */
    virtual ~CRDFListViewItem();

    /**
     * Set the RDF node represented by this item
     * @param const CRDFNode * pNode
     */
    void setNode(const CRDFNode * pNode);

    // Attributes
  private:
    /**
     * The node presented by this item
     */
    CRDFNode * mpNode;

    /**
     * The edge connecting this item to its parent
     */
    CRDFEdge * mpEdge;
  };

#endif // COPASI_CRDFListViewItem