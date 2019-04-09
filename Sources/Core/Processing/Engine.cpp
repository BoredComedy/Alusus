/**
 * @file Core/Processing/Engine.cpp
 * Contains the implementation of class Core::Processing::Engine.
 *
 * @copyright Copyright (C) 2019 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <http://alusus.net/alusus_license_1_0>.
 */
//==============================================================================

#include "core.h"

namespace Core { namespace Processing
{

//==============================================================================
// Member Functions

void Engine::initialize(SharedPtr<Data::Ast::Scope> const &rootScope)
{
  // Prepare the lexer.
  this->lexer.initialize(rootScope);
  this->noticeSignal.relay(this->lexer.noticeSignal);

  // Prepare the parser.
  this->parser.initialize(rootScope);
  this->noticeSignal.relay(this->parser.noticeSignal);
  this->lexer.tokenGenerated.connect(this->parser.handleNewTokenSlot);
}


SharedPtr<TiObject> Engine::processString(Char const *str, Char const *name)
{
  if (str == 0) {
    throw EXCEPTION(InvalidArgumentException, S("str"), S("Cannot be null."), str);
  }

  this->parser.beginParsing();

  // Start passing characters to the lexer.

  Data::SourceLocationRecord sourceLocation;
  sourceLocation.filename = std::make_shared<Str>(name);
  sourceLocation.line = 1;
  sourceLocation.column = 1;
  lexer.handleNewString(str, sourceLocation);

  auto endLine = sourceLocation.line;
  auto endColumn = sourceLocation.column;

  lexer.handleNewChar(FILE_TERMINATOR, sourceLocation);

  sourceLocation.line = endLine;
  sourceLocation.column = endColumn;

  return this->parser.endParsing(sourceLocation);
}


SharedPtr<TiObject> Engine::processFile(Char const *filename)
{
  // Open the file.
  std::ifstream fin(filename);
  if (fin.fail()) {
    throw EXCEPTION(InvalidArgumentException, S("filename"), S("Could not open file."), filename);
  }
  return this->processStream(&fin, filename);
}


SharedPtr<TiObject> Engine::processStream(InStream *is, Char const *streamName)
{
  // Open the file.
  if (is == 0) {
    throw EXCEPTION(InvalidArgumentException, S("is"), S("Cannot be null."));
  }

  parser.beginParsing();

  // Start passing characters to the lexer.
  Data::SourceLocationRecord sourceLocation;
  sourceLocation.filename = std::make_shared<Str>(streamName);
  sourceLocation.line = 1;
  sourceLocation.column = 1;
  Char c;
  is->get(c);
  while (!is->eof()) {
    lexer.handleNewChar(c, sourceLocation);
    is->get(c);
  }

  auto endLine = sourceLocation.line;
  auto endColumn = sourceLocation.column;

  lexer.handleNewChar(FILE_TERMINATOR, sourceLocation);

  sourceLocation.line = endLine;
  sourceLocation.column = endColumn;

  return parser.endParsing(sourceLocation);
}

} } // namespace
