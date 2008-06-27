// Begin CVS Header
//   $Source: /home/cvs/copasi_dev/cvs_admin/addHeader,v $
//   $Revision: 1.9 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2008/03/12 01:53:45 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

/****************************************************************************
 ** Form interface generated from reading ui file 'CMCAResultSubwidget.ui'
 **
 ** Created: Fri Jun 27 10:54:27 2008
 **      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.8   edited Jan 11 14:47 $)
 **
 ** WARNING! All changes made in this file will be lost!
 ****************************************************************************/

#ifndef CMCARESULTSUBWIDGET_H
#define CMCARESULTSUBWIDGET_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class CQArrayAnnotationsWidget;
class QLabel;
class QComboBox;
class QTabWidget;
class QPushButton;
class CMCAMethod;

class CMCAResultSubwidget : public QWidget
  {
    Q_OBJECT

  public:
    CMCAResultSubwidget(QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
    ~CMCAResultSubwidget();

    QLabel* mTopLabel;
    QComboBox* mComboScale;
    QTabWidget* mTabWidget;
    QWidget* tab;
    CQArrayAnnotationsWidget* mpArrayElasticities;
    QWidget* tab_2;
    CQArrayAnnotationsWidget* mpArrayFCC;
    QWidget* TabPage;
    CQArrayAnnotationsWidget* mpArrayCCC;
    QPushButton* mSaveButton;
    QPushButton* mpBtnPrintAsImage;

    virtual void loadAll(const CMCAMethod * mcaMethod);
    virtual void loadElasticities(const CMCAMethod * mcaMethod);
    virtual void loadConcentrationCCs(const CMCAMethod * mcaMethod);
    virtual void loadFluxCCs(const CMCAMethod * mcaMethod);
    virtual void clear();

  protected:
    virtual void init();

    QVBoxLayout* CMCAResultSubwidgetLayout;
    QHBoxLayout* layout3;
    QGridLayout* tabLayout;
    QGridLayout* tabLayout_2;
    QGridLayout* TabPageLayout;
    QHBoxLayout* layout13;
    QSpacerItem* spacer10;

  protected slots:
    virtual void languageChange();

    virtual void slotSave();
    virtual void slotScaled();
    void printAsImage();

  private:
    const CMCAMethod * mMCAMethod;

    QPixmap image0;
  };

#endif // CMCARESULTSUBWIDGET_H
