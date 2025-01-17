/**
 * @file Core/Notices/notices.h
 * Contains the definitions and include statements of all user notices.
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#ifndef CORE_NOTICES_NOTICES_H
#define CORE_NOTICES_NOTICES_H

/**
 * @defgroup core_notices Notices
 * @ingroup core
 * @brief Classes that contains notices.
 *
 * This namespace contains definitions for classes and types for build notices.
 */


//==============================================================================
// Forward Class Definitions

namespace Core::Notices
{
  class Notice;
}


//==============================================================================
// Macros

#define DEFINE_NOTICE(name, typeNamespace, packageName, url, code, severity, msg) \
  class name : public Core::Notices::Notice \
  { \
    TYPE_INFO(name, Core::Notices::Notice, typeNamespace, packageName, url); \
    public: using Core::Notices::Notice::Notice; \
    public: virtual Str const& getCode() const \
    { \
      static Str _code(S(code)); \
      return _code; \
    } \
    public: virtual Int getSeverity() const \
    { \
      return severity; \
    } \
    public: virtual void buildDescription(Str &str) const \
    { \
      str = Core::Notices::L18nDictionary::getSingleton()->get(this->getCode(), S(msg)); \
    } \
  }


//==============================================================================
// Global Functions

namespace Core::Notices
{

/**
 * @brief Print the provided notices to the console.
 * @ingroup core_notices
 *
 * Printed notice includes severity, msg code, location, as well as
 * description.
 */
void printNotice(Notice const *msg);

} // namespace


//==============================================================================
// Type Names

DEFINE_TYPE_NAME(Core::Notices::Store, "alusus.org/Core/Core.Notices.Store");


//==============================================================================
// Main Classes

#include "L18nDictionary.h"

#include "Notice.h"
#include "Store.h"

#include "CustomNotice.h"


//==============================================================================
// Lexer Notices

#include "UnrecognizedCharNotice.h"

namespace Core::Notices
{

DEFINE_NOTICE(BufferFullNotice, "Core.Notices", "Core", "alusus.org", "CL1002", 1,
  "Input buffer is full. A single token is too long to fit in the input buffer. "
  "The token may have been broken into more than one token."
);
DEFINE_NOTICE(TokenClampedNotice, "Core.Notices", "Core", "alusus.org", "CL2003", 2,
  "Input buffer is full. A single token is too long to fit in the input buffer. "
  "Some characters that are part of the token has been ignored."
);

} /// namespace


//==============================================================================
// Parser Notices

namespace Core::Notices
{

DEFINE_NOTICE(SyntaxErrorNotice, "Core.Notices", "Core", "alusus.org", "CP1001", 1,
  "Parser syntax error."
);
DEFINE_NOTICE(UnexpectedEofNotice, "Core.Notices", "Core", "alusus.org", "CP1002", 1,
  "Parsing exited while needing more tokens."
);
DEFINE_NOTICE(UnexpectedTokenNotice, "Core.Notices", "Core", "alusus.org", "CP1003", 1,
  "Parsing has already folded out to completion."
);
DEFINE_NOTICE(AmbiguityNotice, "Core.Notices", "Core", "alusus.org", "CP2004", 2,
  "Ambiguity is causing state branching."
);
DEFINE_NOTICE(UnexpectedModifierNotice, "Core.Notices", "Core", "alusus.org", "CP1005", 1,
  "Unexpected modifier encountered."
);
DEFINE_NOTICE(InvalidImportNotice, "Core.Notices", "Core", "alusus.org", "CP1006", 1,
  "Invalid import parameter."
);

} // namespace


//==============================================================================
// Data Notices

namespace Core::Notices
{

DEFINE_NOTICE(MissingDefLinkNotice, "Core.Notices", "Core", "alusus.org", "CD1001", 1,
  "Missing link expression in 'def' command."
);
DEFINE_NOTICE(MissingDefNameNotice, "Core.Notices", "Core", "alusus.org", "CD1002", 1,
  "Missing definition name in 'def' command."
);
DEFINE_NOTICE(InvalidDefCommandNotice, "Core.Notices", "Core", "alusus.org", "CD1003", 1,
  "Invalid 'def' command."
);
DEFINE_NOTICE(IncompatibleDefMergeNotice, "Core.Notices", "Core", "alusus.org", "CD1004", 1,
  "Incompatible 'def' merge."
);

} // namespace


//==============================================================================
// Generic Notices

namespace Core::Notices
{

DEFINE_NOTICE(UnrecognizedErrorNotice, "Core.Notices", "Core", "alusus.org", "CG1001", 1,
  "Unrecognized error."
);
DEFINE_NOTICE(InvalidDumpArgNotice, "Core.Notices", "Core", "alusus.org", "CG1002", 1,
  "Invalid argument for 'dump_ast' command."
);

} // namespace

#include "ImportLoadFailedNotice.h"

#endif
