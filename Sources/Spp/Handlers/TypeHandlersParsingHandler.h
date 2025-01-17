/**
 * @file Spp/Handlers/TypeHandlersParsingHandler.h
 * Contains the header of class Spp::Handlers::TypeHandlersParsingHandler
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#ifndef SPP_HANDLERS_TYPEHANDLERSPARSINGHANDLER_H
#define SPP_HANDLERS_TYPEHANDLERSPARSINGHANDLER_H

namespace Spp::Handlers
{

class TypeHandlersParsingHandler : public Core::Processing::Handlers::GenericParsingHandler
{
  //============================================================================
  // Type Info

  TYPE_INFO(TypeHandlersParsingHandler, Core::Processing::Handlers::GenericParsingHandler,
            "Spp.Handlers", "Spp", "alusus.org");


  //============================================================================
  // Constructor

  public: TypeHandlersParsingHandler()
  {
  }


  //============================================================================
  // Member Functions

  public: virtual void onProdEnd(Core::Processing::Parser *parser, Core::Processing::ParserState *state);

  private: SharedPtr<Core::Data::Ast::Scope> prepareBody(TioSharedPtr const &stmt);

  private: void createAssignmentHandler(
    Processing::ParserState *state, Core::Data::Ast::AssignmentOperator *assignmentOp,
    SharedPtr<Core::Data::Ast::Scope> const &body, TioSharedPtr const &retType
  );

  private: void createComparisonHandler(
    Processing::ParserState *state, Core::Data::Ast::ComparisonOperator *comparisonOp,
    SharedPtr<Core::Data::Ast::Scope> const &body, TioSharedPtr const &retType
  );

  private: void createInfixOpHandler(
    Processing::ParserState *state, Core::Data::Ast::InfixOperator *infixOp,
    SharedPtr<Core::Data::Ast::Scope> const &body, TioSharedPtr const &retType
  );

  private: void createReadHandler(
    Processing::ParserState *state, Core::Data::Ast::LinkOperator *linkOp,
    SharedPtr<Core::Data::Ast::Scope> const &body, TioSharedPtr const &retType
  );

  private: void createInitOpHandler(
    Processing::ParserState *state, Spp::Ast::InitOp *initOp,
    SharedPtr<Core::Data::Ast::Scope> const &body
  );

  private: void createTerminateOpHandler(
    Processing::ParserState *state, Spp::Ast::TerminateOp *terminateOp,
    SharedPtr<Core::Data::Ast::Scope> const &body
  );

  private: void createCastHandler(
    Processing::ParserState *state, Spp::Ast::CastOp *castOp,
    SharedPtr<Core::Data::Ast::Scope> const &body
  );

  private: void createParensOpHandler(
    Processing::ParserState *state, Core::Data::Ast::ParamPass *parensOp,
    SharedPtr<Core::Data::Ast::Scope> const &body, TioSharedPtr const &retType
  );

  private: SharedPtr<Core::Data::Ast::Definition> createBinaryOpFunction(
    Processing::ParserState *state, Char const *funcName, Char const *op, TioSharedPtr const &thisType,
    Char const *inputName, TioSharedPtr const &inputType, TioSharedPtr const &retType, TioSharedPtr const &body,
    SharedPtr<Core::Data::SourceLocation> const &sourceLocation
  );

  private: SharedPtr<Core::Data::Ast::Definition> createFunction(
    Processing::ParserState *state, Char const *funcName, Char const *op,
    SharedPtr<Core::Data::Ast::Map> const argTypes, TioSharedPtr const &retType, TioSharedPtr const &body,
    SharedPtr<Core::Data::SourceLocation> const &sourceLocation
  );

  private: Bool prepareInputArg(
    Processing::ParserState *state, TioSharedPtr input, Char const *&inputName, TioSharedPtr &inputType
  );

  private: SharedPtr<Core::Data::Ast::ParamPass> prepareThisType(
    SharedPtr<Core::Data::SourceLocation> const &sourceLocation
  );

  private: SharedPtr<Core::Data::Ast::ParamPass> prepareAssignmentRetType(
    SharedPtr<Core::Data::SourceLocation> const &sourceLocation
  );

  private: SharedPtr<Core::Data::Ast::ParamPass> prepareComparisonRetType(
    SharedPtr<Core::Data::SourceLocation> const &sourceLocation
  );

  private: SharedPtr<Core::Data::Ast::Definition> createDefinition(
    Char const *funcName, Char const *op, SharedPtr<Spp::Ast::Function> func,
    SharedPtr<Core::Data::SourceLocation> const &sourceLocation
  );

}; // class

} // namespace

#endif
