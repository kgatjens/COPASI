// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/compareExpressions/unittests/test_simplify.cpp,v $
//   $Revision: 1.1 $
//   $Name:  $
//   $Author: gauges $
//   $Date: 2008/10/15 15:42:33 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#include "test_simplify.h"

#include "../CNormalFraction.h"
#include "../CNormalSum.h"
#include "../CNormalProduct.h"
#include "../CNormalItemPower.h"
#include "../CNormalGeneralPower.h"
#include "../CNormalItem.h"

void test_simplify::setUp()
{}

void test_simplify::tearDown()
{}

void test_simplify::test_simplify_1()
{
  CNormalItem* pItemA = new CNormalItem();
  pItemA->setType(CNormalItem::VARIABLE);
  pItemA->setName("A");

  CNormalItem* pItemB = new CNormalItem();
  pItemB->setType(CNormalItem::VARIABLE);
  pItemB->setName("B");

  CNormalItem* pItemC = new CNormalItem();
  pItemC->setType(CNormalItem::VARIABLE);
  pItemC->setName("C");

  CNormalItem* pItemD = new CNormalItem();
  pItemD->setType(CNormalItem::VARIABLE);
  pItemD->setName("D");

  CNormalItem* pItemE = new CNormalItem();
  pItemE->setType(CNormalItem::VARIABLE);
  pItemE->setName("E");

  CNormalItem* pItemF = new CNormalItem();
  pItemF->setType(CNormalItem::VARIABLE);
  pItemF->setName("F");

  // Z=(((-1) * E + F + 4 * D * E))^2
  // -1.0 * E + F + 4 * D * E
  CNormalFraction* pFraction = new CNormalFraction();
  CNormalSum* pSum = &pFraction->getNumerator();
  // -1 * E
  CNormalProduct* pProduct = new CNormalProduct();
  pProduct->multiply(-1.0);
  pProduct->multiply(*pItemE);
  pSum->add(*pProduct);
  delete pProduct;
  // F
  pProduct = new CNormalProduct();
  pProduct->multiply(*pItemF);
  pSum->add(*pProduct);
  delete pProduct;
  // 4 * D  * E
  pProduct = new CNormalProduct();
  pProduct->multiply(4.0);
  pProduct->multiply(*pItemD);
  pProduct->multiply(*pItemE);
  pSum->add(*pProduct);
  delete pProduct;
  pProduct = new CNormalProduct();
  pFraction->getDenominator().add(*pProduct);
  delete pProduct;
  CNormalItemPower* pZ = new CNormalItemPower();
  CNormalGeneralPower* pGeneralPower = new CNormalGeneralPower();
  pGeneralPower->setType(CNormalGeneralPower::POWER);
  pGeneralPower->setLeft(*pFraction);
  delete pFraction;
  pFraction = CNormalFraction::createUnitFraction();
  pGeneralPower->setRight(*pFraction);
  delete pFraction;
  pZ->setExp(2.0);
  pZ->setItem(*pGeneralPower);
  delete pGeneralPower;

  // Y=(((-16) * D^2 * E^2 + Z + 4 * D * E^2))
  CNormalFraction* pY = new CNormalFraction();
  pProduct = new CNormalProduct();
  pProduct->multiply(-16.0);
  // D^2
  CNormalItemPower* pItemPower = new CNormalItemPower();
  pItemPower->setExp(2.0);
  pItemPower->setItem(*pItemD);
  pProduct->multiply(*pItemPower);
  delete pItemPower;
  // E^2
  pItemPower = new CNormalItemPower();
  pItemPower->setExp(2.0);
  pItemPower->setItem(*pItemE);
  pProduct->multiply(*pItemPower);
  delete pItemPower;
  pSum = &pY->getNumerator();
  pSum->add(*pProduct);
  delete pProduct;
  pProduct = new CNormalProduct();
  pProduct->multiply(*pZ);
  delete pZ;
  pSum->add(*pProduct);
  delete pProduct;
  // 4 * D * E^2
  pProduct = new CNormalProduct();
  pProduct->multiply(4.0);
  pProduct->multiply(*pItemD);
  pItemPower = new CNormalItemPower();
  pItemPower->setExp(2.0);
  pItemPower->setItem(*pItemE);
  pProduct->multiply(*pItemPower);
  delete pItemPower;
  pSum->add(*pProduct);
  delete pProduct;
  // set the denominator of pY to 1
  pProduct = new CNormalProduct();
  pY->getDenominator().add(*pProduct);
  delete pProduct;

  // X=((-32) * B * C * D^2 * E + (-8) * B * C * D * F + 8 * B * C * D * E + 8 * B * C * D * Y^0.5)/((-4) * D + 1)
  CNormalFraction* pX = new CNormalFraction();
  // denominator -4 * D + 1
  pProduct = new CNormalProduct();
  pProduct->multiply(-4.0);
  pProduct->multiply(*pItemD);
  pX->getDenominator().add(*pProduct);
  delete pProduct;
  pProduct = new CNormalProduct();
  pX->getDenominator().add(*pProduct);
  delete pProduct;
  // numerator ((-32) * B * C * D^2 * E + (-8) * B * C * D * F + 8 * B * C * D * E + 8 * B * C * D * Y^0.5)
  pSum = &pX->getNumerator();
  // (-32) * B * C * D^2 * E
  pProduct = new CNormalProduct();
  pProduct->multiply(-32.0);
  pProduct->multiply(*pItemB);
  pProduct->multiply(*pItemC);
  pItemPower = new CNormalItemPower();
  pItemPower->setExp(2.0);
  pItemPower->setItem(*pItemD);
  pProduct->multiply(*pItemPower);
  delete pItemPower;
  pProduct->multiply(*pItemE);
  pSum->add(*pProduct);
  delete pProduct;
  // (-8) * B * C * D * F
  pProduct = new CNormalProduct();
  pProduct->multiply(-8.0);
  pProduct->multiply(*pItemB);
  pProduct->multiply(*pItemC);
  pProduct->multiply(*pItemD);
  pProduct->multiply(*pItemF);
  pSum->add(*pProduct);
  delete pProduct;
  // 8 * B * C * D * E
  pProduct = new CNormalProduct();
  pProduct->multiply(8.0);
  pProduct->multiply(*pItemB);
  pProduct->multiply(*pItemC);
  pProduct->multiply(*pItemD);
  pProduct->multiply(*pItemE);
  pSum->add(*pProduct);
  delete pProduct;
  // 8 * B * C * D * Y^0.5)
  pProduct = new CNormalProduct();
  pProduct->multiply(8.0);
  pProduct->multiply(*pItemB);
  pProduct->multiply(*pItemC);
  pProduct->multiply(*pItemD);
  pItemPower = new CNormalItemPower();
  pItemPower->setExp(0.5);
  pItemPower->setItem(*pY);
  delete pY;
  pProduct->multiply(*pItemPower);
  pSum->add(*pProduct);
  delete pProduct;

  // W=((-8) * B * C * D * E + X)/(A)
  CNormalFraction* pW = new CNormalFraction();
  // denominator A
  pSum = &pW->getDenominator();
  pProduct = new CNormalProduct();
  pProduct->multiply(*pItemA);
  pSum->add(*pProduct);
  delete pProduct;
  // numerator ((-8) * B * C * D * E + X)
  pSum = &pW->getNumerator();
  // (-8) * B * C * D * E
  pProduct = new CNormalProduct();
  pProduct->multiply(8.0);
  pProduct->multiply(*pItemB);
  pProduct->multiply(*pItemC);
  pProduct->multiply(*pItemD);
  pProduct->multiply(*pItemE);
  pSum->add(*pProduct);
  delete pProduct;

  pSum->add(*pX);
  delete pX;
  delete pItemA;
  delete pItemB;
  delete pItemC;
  delete pItemD;
  delete pItemE;
  delete pItemF;
  // test what simplify does to this
  pW->simplify();

  delete pW;
}
