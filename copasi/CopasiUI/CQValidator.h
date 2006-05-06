/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/CQValidator.h,v $
   $Revision: 1.5 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/05/06 03:07:17 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#ifndef COPASI_CQValidator
#define COPASI_CQValidator

#include <qvalidator.h>
#include <qcolor.h>
#include <qlineedit.h>

#include "copasi.h"

class CQValidator : public QValidator
  {
    // Operations
  public:
    CQValidator(QLineEdit * parent, const char * name = 0);

    virtual State validate(QString & input, int & pos) const;

    virtual State revalidate();

    virtual void force(const QString & input) const;

    virtual void saved() const;

  protected:
    State setColor(const State & state) const;

    //Attributes
    QLineEdit * mpLineEdit;

  private:
    QString mLastAccepted;

    QColor mSavedColor;

    QColor mAcceptableColor;

    QColor mErrorColor;
  };

class CQValidatorNotEmpty : public CQValidator
  {
    // Operations
  public:
    CQValidatorNotEmpty(QLineEdit * parent, const char * name = 0);

    virtual State validate(QString & input, int & pos) const;
  };

class CQValidatorBound : public CQValidator
  {
    // Operations
  public:
    CQValidatorBound(QLineEdit * parent, const char * name = 0);

    virtual State validate(QString & input, int & pos) const;

    virtual void force(const QString & input) const;

    //Attributes
  protected:
    QDoubleValidator * mpDoubleValidator;

    QString mValidBound;
  };

class CQValidatorDouble : public CQValidator
  {
    // Operations
  public:
    CQValidatorDouble(QLineEdit * parent, const char * name = 0);

    virtual State validate(QString & input, int & pos) const;

    void setRange(const C_FLOAT64 & lowerBound, const C_FLOAT64 & upperBound);

    //Attributes
  protected:
    QDoubleValidator * mpDoubleValidator;
  };

class CQValidatorInt : public CQValidator
  {
    // Operations
  public:
    CQValidatorInt(QLineEdit * parent, const char * name = 0);

    virtual State validate(QString & input, int & pos) const;

    void setRange(const C_INT32 & lowerBound, const C_INT32 & upperBound);

    //Attributes
  protected:
    QIntValidator * mpIntValidator;
  };

#endif // COPASI_CQValidator
