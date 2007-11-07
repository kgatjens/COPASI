// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/CQMetabolite.h,v $
//   $Revision: 1.8 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2007/11/07 17:05:36 $
// End CVS Header

// Copyright (C) 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/****************************************************************************
 ** Form interface generated from reading ui file 'CQMetabolite.ui'
 **
 ** Created: Wed Nov 7 10:02:37 2007
 **      by: The User Interface Compiler ($Id: CQMetabolite.h,v 1.8 2007/11/07 17:05:36 shoops Exp $)
 **
 ** WARNING! All changes made in this file will be lost!
 ****************************************************************************/

#ifndef CQMETABOLITE_H
#define CQMETABOLITE_H

#include <qvariant.h>
#include <qpixmap.h>
#include <string>
#include "copasiWidget.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class CQExpressionWidget;
class QLabel;
class QLineEdit;
class QComboBox;
class QFrame;
class QToolButton;
class QListView;
class QListViewItem;
class QPushButton;
class CQExpressionWidget;
class CMetab;
class CCompartment;
class CExpression;

class CQMetabolite : public CopasiWidget
  {
    Q_OBJECT

  public:
    CQMetabolite(QWidget* parent = 0, const char* name = 0);
    ~CQMetabolite();

    QLabel* mpLblReactions;
    QLineEdit* mpEditTransitionTime;
    QLabel* mpLblExpression;
    QLabel* mpLblInitialValue;
    QComboBox* mpComboBoxInitialType;
    QLabel* mpLblType;
    QLineEdit* mpEditCurrentValue;
    QLabel* mpLblRate;
    QLabel* mpLblValue;
    QLineEdit* mpEditName;
    QFrame* mpLine1;
    QComboBox* mpComboBoxCompartment;
    QLineEdit* mpEditRate;
    CQExpressionWidget* mpEditExpression;
    QToolButton* mpBtnObject;
    QLineEdit* mpEditInitialValue;
    QComboBox* mpComboBoxInitialSelection;
    QComboBox* mpComboBoxType;
    QLabel* mpLblTransientValue;
    QLabel* mpLblCompartment;
    QListView* mpReactionTable;
    QLabel* mpLblTransitionTime;
    QLabel* mpLblName;
    QFrame* mpLine2;
    QPushButton* mpBtnCommit;
    QPushButton* mpBtnRevert;
    QPushButton* mpBtnNew;
    QPushButton* mpBtnDelete;

    virtual bool enter(const std::string & key);
    virtual bool leave();
    virtual bool update(ListViews::ObjectType objectType, ListViews::Action action, const std::string &);
    virtual void setFramework(int framework);

  protected:
    QVBoxLayout* CQMetaboliteLayout;
    QSpacerItem* mpSpacer;
    QGridLayout* layout7;
    QHBoxLayout* mpLayoutInitialValues;
    QHBoxLayout* mpHBoxLayout;
    QVBoxLayout* mpVBoxLayout;
    QSpacerItem* mpSpacerObject;
    QVBoxLayout* mpLayoutInitialConcentration;
    QHBoxLayout* mpBtnLayout;

  protected slots:
    virtual void languageChange();

  private:
    bool mChanged;
    bool mInitialNumberLastChanged;
    std::string mKey;
    CMetab * mpMetab;
    const CCompartment * mpCurrentCompartment;
    std::vector< int > mItemToType;
    double mInitialNumber;
    double mInitialConcentration;

    QPixmap image0;
    QPixmap image1;

    void init();
    void load();
    void save();
    void destroy();
    void loadReactionTable();

  private slots:
    void slotBtnCommit();
    void slotBtnRevert();
    void slotBtnNew();
    void slotBtnDelete();
    void slotCompartmentChanged(int compartment);
    void slotTypeChanged(int type);
    void slotInitialTypeChanged(int);
    void slotInitialAssignment(int);
    void slotNameLostFocus();
    void slotExpressionValid(bool valid);
    void slotReactionTableCurrentChanged(QListViewItem * pItem);
    void slotInitialValueLostFocus();
  };

#endif // CQMETABOLITE_H
