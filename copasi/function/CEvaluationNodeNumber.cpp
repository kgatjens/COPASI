// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/function/CEvaluationNodeNumber.cpp,v $
//   $Revision: 1.27 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2008/12/18 17:33:22 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <sstream>
#include <stdlib.h>

#include "copasi.h"
#include "CEvaluationNode.h"

#include "sbml/math/ASTNode.h"

CEvaluationNodeNumber::CEvaluationNodeNumber():
    CEvaluationNode(CEvaluationNode::INVALID, "")
{mPrecedence = PRECEDENCE_NUMBER;}

CEvaluationNodeNumber::CEvaluationNodeNumber(const SubType & subType,
    const Data & data):
    CEvaluationNode((Type) (CEvaluationNode::NUMBER | subType), data)
{
  char * end;
  const char * str = mData.c_str();

  switch (subType)
    {
    case DOUBLE:
    case INTEGER:
    case ENOTATION:
      mValue = strtod(str, NULL);
      break;

    case RATIONALE:
      str++; // Skip the '('
      mValue = strtod(str, &end);
      end++; // Skip the '/'
      mValue /= strtod(end, NULL);
      break;

    case INVALID:
      fatalError();
      break;
    }

  mPrecedence = PRECEDENCE_NUMBER;
}

CEvaluationNodeNumber::CEvaluationNodeNumber(const CEvaluationNodeNumber & src):
    CEvaluationNode(src)
{}

CEvaluationNodeNumber::~CEvaluationNodeNumber() {}

#ifdef WIN32
// warning C4056: overflow in floating-point constant arithmetic
// warning C4756: overflow in constant arithmetic
# pragma warning (disable: 4056 4756)
#endif

CEvaluationNode* CEvaluationNodeNumber::createNodeFromASTTree(const ASTNode& node)
{
  ASTNodeType_t type = node.getType();
  std::stringstream ss;
  SubType subType;
  std::string data = "";
  CEvaluationNode* pNode = NULL;
  switch (type)
    {
    case AST_INTEGER:
      subType = INTEGER;
      if (node.getInteger() < 0)
        {
          pNode = new CEvaluationNodeFunction(CEvaluationNodeFunction::MINUS, "-");

          ss << abs(node.getInteger());
          data = ss.str();
          pNode->addChild(new CEvaluationNodeNumber(subType, data));
        }
      else
        {
          ss << node.getInteger();
          data = ss.str();
          pNode = new CEvaluationNodeNumber(subType, data);
        }
      break;
    case AST_REAL:
      subType = DOUBLE;
      if (node.getReal() == (2*DBL_MAX))
        {
          pNode = new CEvaluationNodeConstant(CEvaluationNodeConstant::_INFINITY, "INFINITY");
        }
      else if (node.getReal() == (-2*DBL_MAX))
        {
          pNode = new CEvaluationNodeFunction(CEvaluationNodeFunction::MINUS, "-");
          pNode->addChild(new CEvaluationNodeConstant(CEvaluationNodeConstant::_INFINITY, "INFINITY"));
        }
      else if (isnan(node.getReal()))
        {
          pNode = new CEvaluationNodeConstant(CEvaluationNodeConstant::_NaN, "NAN");
        }
      else if (node.getReal() < 0.0)
        {
          pNode = new CEvaluationNodeFunction(CEvaluationNodeFunction::MINUS, "-");

          ss << fabs(node.getReal());
          data = ss.str();
          pNode->addChild(new CEvaluationNodeNumber(subType, data));
        }
      else
        {
          ss << node.getReal();
          data = ss.str();
          pNode = new CEvaluationNodeNumber(subType, data);
        }
      break;
    case AST_REAL_E:
      subType = ENOTATION;
      if (node.getReal() == (2*DBL_MAX))
        {
          pNode = new CEvaluationNodeConstant(CEvaluationNodeConstant::_INFINITY, "INFINITY");
        }
      else if (node.getReal() == (-2*DBL_MAX))
        {
          pNode = new CEvaluationNodeFunction(CEvaluationNodeFunction::MINUS, "-");
          pNode->addChild(new CEvaluationNodeConstant(CEvaluationNodeConstant::_INFINITY, "INFINITY"));
        }
      else if (isnan(node.getReal()))
        {
          pNode = new CEvaluationNodeConstant(CEvaluationNodeConstant::_NaN, "NAN");
        }
      else if (node.getReal() < 0.0)
        {
          pNode = new CEvaluationNodeFunction(CEvaluationNodeFunction::MINUS, "-");

          ss << fabs(node.getReal());
          data = ss.str();
          pNode->addChild(new CEvaluationNodeNumber(subType, data));
        }
      else
        {
          ss << node.getReal();
          data = ss.str();
          pNode = new CEvaluationNodeNumber(subType, data);
        }
      break;
    case AST_RATIONAL:
      subType = RATIONALE;
      if (node.getReal() < 0.0) // getReal returns the value of the node
        {
          pNode = new CEvaluationNodeFunction(CEvaluationNodeFunction::MINUS, "-");

          ss << "(" << abs(node.getNumerator()) << "/" << abs(node.getDenominator()) << ")";
          data = ss.str();
          pNode->addChild(new CEvaluationNodeNumber(subType, data));
        }
      else
        {
          ss << "(" << node.getNumerator() << "/" << node.getDenominator() << ")";
          data = ss.str();
          pNode = new CEvaluationNodeNumber(subType, data);
        }
      break;
    default:
      subType = INVALID;
      break;
    }
  return pNode;
}

#ifdef WIN32
# pragma warning (default: 4056 4756)
#endif

ASTNode* CEvaluationNodeNumber::toAST() const
  {
    SubType subType = (SubType)CEvaluationNode::subType(this->getType());
    ASTNode* node = new ASTNode();
    double num1;
    double num2;
    char* end;
    const char * str = mData.c_str();

    switch (subType)
      {
      case DOUBLE:
        node->setType(AST_REAL);
        node->setValue(this->value());
        break;
      case INTEGER:
        node->setType(AST_INTEGER);
        node->setValue((long)this->value());
        break;
      case ENOTATION:
        node->setType(AST_REAL_E);
        num2 = floor(log10(this->value()));
        num1 = pow(10.0, log10(this->value()) - num2);
        node->setValue(num1, (long)num2);
        break;
      case RATIONALE:
        node->setType(AST_RATIONAL);
        str++; // Skip the '('
        num1 = strtod(str, &end);
        end++; // Skip the '/'
        num2 = strtod(end, NULL);
        node->setValue((long)num1, (long)num2);
        break;
      case INVALID:
        break;
      }
    return node;
  }

#include "utilities/copasimathml.h"

void CEvaluationNodeNumber::writeMathML(std::ostream & out,
                                        const std::vector<std::vector<std::string> > & /* env */,
                                        bool /* expand */,
                                        unsigned C_INT32 l) const
  {
    out << SPC(l) << "<mn>" << mData << "</mn>" << std::endl;
    //or use mValue instead?
  }
