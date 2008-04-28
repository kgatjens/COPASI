// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/layoutUI/CQCurrentValueTable.h,v $
//   $Revision: 1.2 $
//   $Name:  $
//   $Author: urost $
//   $Date: 2008/04/28 12:05:00 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#ifndef CQCURRENTVALUETABLE_H_
#define  CQCURRENTVALUETABLE_H_

#include <string>

#include "copasi.h"

#include <qtable.h>

#include "CDataEntity.h"

class CQCurrentValueTable : public QTable
  {

    Q_OBJECT        // must include this if you use Qt signals/slots

  public:
    CQCurrentValueTable(QWidget *parent = 0, const char *name = 0);
    ~CQCurrentValueTable();

    void setRowInTable(int row, std::string s, C_FLOAT64 val);
    void setValues(CDataEntity *dataSet);
    void setValue(int row, C_FLOAT64 val);

    void setAllBoxesChecked();
    void setAllBoxesUnchecked();
    void init();

  private slots:
    void mouseClickedOverTable(int row, int col , int button, const QPoint & mousepos);
  };

#endif /*CQCURRENTVALUETABLE_H_*/
