/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/ReactionsWidget.h,v $
   $Revision: 1.20 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2003/11/05 18:38:04 $
   End CVS Header */

/****************************************************************************
 **  $ CopasiUI/ReactionsWidget.h               
 **  $ Author  : Mudita Singhal
 **  
 ** This is the header file for the Reactions Widget, i.e the First level 
 ** of Reactions.
 **
 *****************************************************************************/

#ifndef REACTIONS_WIDGET_H
#define REACTIONS_WIDGET_H

#include <qtable.h>

#include "copasiWidget.h"

#include "CReactionInterface.h"

class CModel;
class QPushButton;
class QGridLayout;
class QTable;
class MyTable;

class ReactionsWidget : public CopasiWidget
  {
    Q_OBJECT

  protected:
    MyTable *table;
    QPushButton *btnOK;
    QPushButton *btnCancel;
    QPushButton *btnDelete;
    bool binitialized;
    std::vector<std::string> mKeys;

    CReactionInterface mRi;

  public:
    ReactionsWidget(QWidget *parent, const char * name = 0, WFlags f = 0);
    void resizeEvent(QResizeEvent * re);

    virtual bool update(ListViews::ObjectType objectType, ListViews::Action action, const std::string & key);
    virtual bool leave();
    virtual bool enter(const std::string & key = "");

  protected slots:
    virtual void slotTableCurrentChanged(int, int, int, const QPoint &);

    virtual void slotTableSelectionChanged();
    virtual void slotBtnOKClicked();
    virtual void slotBtnCancelClicked();
    virtual void slotBtnDeleteClicked();
    virtual void tableValueChanged(int, int);

    virtual void CurrentValueChanged(int, int);

  private:
    void fillTable();
    void createNewObject();

    int m_SavedCol;
    int m_SavedRow;
    int prev_row;
    int prev_col;
  };

#endif
