/*******************************************************************
 **  $ CopasiUI/CompartmentsWidget.cpp                 
 **  $ Author  : Mudita Singhal
 **
 ** This file is used to create the GUI FrontPage for the 
 ** information obtained from the data model about the 
 ** Compartments----It is Basically the First level of Compartments
 ********************************************************************/

#include <qlayout.h>
#include <qwidget.h>
#include <qmessagebox.h>
#include "CompartmentsWidget.h"
#include "listviews.h"
#include "model/CMetab.h"
#include <qfont.h>
#include "utilities/CGlobals.h"
string outstring;
CWriteConfig *Mod;

/**
 *  Constructs a Widget for the Compartments subsection of the tree.
 *  This widget is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'. 
 *  @param model The CModel class which contains the metabolites 
 *  to be displayed.
 *  @param parent The widget which this widget is a child of.
 *  @param name The object name is a text that can be used to identify 
 *  this QObject. It's particularly useful in conjunction with the Qt 
 
Designer.
 *  You can find an object by name (and type) using child(), and more than one 
 *  using queryList(). 
 *  @param flags Flags for this widget. Redfer Qt::WidgetFlags of Qt 
 
documentation 
 *  for more information about these flags.
 */
CompartmentsWidget::CompartmentsWidget(QWidget *parent, const char * name, WFlags f)
    : QWidget(parent, name, f)
{
  Mod = new CWriteConfig("try.gps");
  Copasi = new CGlobals;
  mModel = NULL;
  table = new MyTable(0, 2, this, "tblCompartments");
  QVBoxLayout *vBoxLayout = new QVBoxLayout(this, 0);
  vBoxLayout->addWidget(table);

  //Setting table headers
  QHeader *tableHeader = table->horizontalHeader();
  tableHeader->setLabel(0, "Name");
  tableHeader->setLabel(1, "Volume");

  btnOK = new QPushButton("&OK", this);
  btnCancel = new QPushButton("&Cancel", this);

  btnOK->setFont(QFont("Times", 10, QFont::Bold));
  btnCancel->setFont(QFont("Times", 10, QFont::Bold));

  QHBoxLayout *hBoxLayout = new QHBoxLayout(vBoxLayout, 0);

  //To match the Table left Vertical Header Column Width.
  hBoxLayout->addSpacing(32);

  hBoxLayout->addSpacing(50);
  hBoxLayout->addWidget(btnOK);
  hBoxLayout->addSpacing(5);
  hBoxLayout->addWidget(btnCancel);
  hBoxLayout->addSpacing(50);

  table->sortColumn (0, TRUE, TRUE);
  table->setSorting (TRUE);
  table->setFocusPolicy(QWidget::WheelFocus);

  // signals and slots connections
  connect(table, SIGNAL(doubleClicked(int, int, int, const QPoint &)), this, SLOT(slotTableCurrentChanged(int, int, int, const QPoint &)));
  connect(this, SIGNAL(name(QString &)), (ListViews*)parent, SLOT(slotCompartmentTableChanged(QString &)));

  //connect(table, SIGNAL(selectionChanged ()), this, SLOT(slotTableSelectionChanged ()));
  connect(btnOK, SIGNAL(clicked ()), this, SLOT(slotBtnOKClicked()));

  connect(table, SIGNAL(valueChanged(int , int)), this, SLOT(tableValueChanged(int, int)));
  //connect(btnOK, SIGNAL(clicked ()), this,SLOT(
}

void CompartmentsWidget::loadCompartments(CModel *model)
{
  if (model != NULL)
    {
      mModel = model;
      //Emptying the table
      int numberOfRows = table->numRows();

      for (int i = 0; i < numberOfRows; i++)
        {
          table->removeRow(0);
        }

      /*CWriteConfig *Fun = new CWriteConfig("oo.gps"); */
      CCopasiVectorNS < CCompartment > & compartments =

        mModel->getCompartments();

      C_INT32 noOfCompartmentsRows = compartments.size();

      table->setNumRows(noOfCompartmentsRows);

      //Now filling the table.
      CCompartment *compartn;

      for (C_INT32 j = 0; j < noOfCompartmentsRows; j++)
        {
          compartn = compartments[j];
          table->setText(j, 0, compartn->getName().c_str());
          table->setText(j, 1, QString::number(compartn->getVolume()));
          /*Compartment_Name = new QString(compartn->getName().c_str());
          outstring = table->text(j,0);
            Fun->setVariable((string) "Compartment",(string) "string", 

          (void *) &outstring);
            Copasi->FunctionDB.save(*Fun);
            delete Fun;
          QMessageBox::information(this, "Moiety 

          Widget",outstring.c_str()); */

          /*CWriteConfig *Mod = new CWriteConfig("try.gps");
          Copasi->Compartmentfile.save(*Mod);
          delete Mod; */
        }
    }
}

void CompartmentsWidget::slotTableCurrentChanged(int row, int col, int m , const QPoint & n)
{
  QString x = table->text(row, col);
  emit name(x);
  //QMessageBox::information(this, "Compartments Widget",x);
}

void CompartmentsWidget::slotTableSelectionChanged()
{
  if (!table->hasFocus())
    {
      table->setFocus();
    }
}

void CompartmentsWidget::resizeEvent(QResizeEvent * re)
{
  if (isVisible())
    {
      int newWidth = re->size().width();

      newWidth -= 35; //Accounting for the left (vertical) header width.
      float weight0 = 3.5, weight1 = 6.5;
      float weightSum = weight0 + weight1;
      int w0, w1;
      w0 = newWidth * (weight0 / weightSum);
      w1 = newWidth - w0;
      table->setColumnWidth(0, w0);
      table->setColumnWidth(1, w1);
    }
}

/***********ListViews::showMessage(QString caption,QString text)------------------------>
 ** ** Parameters:- 1. QString :- The Title that needs to be displayed in message box
 **              2. QString :_ The Text that needs to be displayed in the message box
 ** Returns  :-  void(Nothing)
 ** Description:- This method is used to show the message box on the screen
 
 ****************************************************************************************/

void CompartmentsWidget::showMessage(QString title, QString text)
{
  QMessageBox::about (this, title, text);
}

void CompartmentsWidget::slotBtnOKClicked()
{
  QMessageBox::information(this, "Moiety Widget", outstring.c_str());
  //CWriteConfig *Fun = new CWriteConfig("oo.gps");
  //string outstring = "Laber";
  // Fun->setVariable((string) "Compartment",(string) "string", (void *) &outstring);
  //Copasi->FunctionDB.save(*Fun);
  //delete Fun;

  //CWriteConfig ModelFile("model.gps");
  //CWriteConfig *Mod = new CWriteConfig("model.gps");
  //CCopasiVectorNS < CCompartment > & compartments =

  mModel->getCompartments();
  //CCompartment *compartn;
}

void CompartmentsWidget::slotBtnCancelClicked()
{
  emit signal_emitted(*Compartment_Name);
}

void CompartmentsWidget::tableValueChanged(int row, int col)
{
  //CWriteConfig *Mod = new CWriteConfig("try.gps");

  CCopasiVectorNS < CCompartment > & compartments1 = mModel->getCompartments();
  CCompartment *compartn1;
  compartn1 = compartments1[row];

  string x = table->text(row, col).latin1();

  if (col == 0)
    {
      compartn1->setName(x);
    }
  else
    {
      compartn1->setVolume(int(x.c_str()));
    }

  compartn1->save(*Mod);
  //Copasi->Compartmentfile.save(*Mod);
  delete Mod;
}
