/**
 * @file Core/Data/Grammar/Factory.h
 * Contains the header of class Core::Data::Grammar::Factory.
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#ifndef CORE_DATA_GRAMMAR_FACTORY_H
#define CORE_DATA_GRAMMAR_FACTORY_H

namespace Core::Data::Grammar
{

class Factory
{
  //============================================================================
  // Member Variables

  protected: Context context;
  protected: Str constTokenPrefix;
  protected: Word constTokenId;

  protected: std::vector<SharedPtr<Reference>> referenceCache;


  //============================================================================
  // Types

  public: struct CommandArg
  {
    public: SharedPtr<Reference> prod;
    public: SharedPtr<TiInt> min;
    public: SharedPtr<TiInt> max;
    public: SharedPtr<TiInt> flags;
    public: CommandArg(
      SharedPtr<Reference> prod, SharedPtr<TiInt> min, SharedPtr<TiInt> max, SharedPtr<TiInt> flags
    ) : prod(prod), min(min), max(max), flags(flags)
    {
    }
  };

  public: struct CommandSection
  {
    public: SharedPtr<Map> keywords;
    public: std::initializer_list<CommandArg> args;
    public: SharedPtr<TiInt> min;
    public: SharedPtr<TiInt> max;
    public: SharedPtr<TiInt> flags;
    public: CommandSection(
      SharedPtr<Map> keywords, std::initializer_list<CommandArg> args
    ) : keywords(keywords), args(args)
    {
    }
    public: CommandSection(
      SharedPtr<Map> keywords, std::initializer_list<CommandArg> args,
      SharedPtr<TiInt> min, SharedPtr<TiInt> max, SharedPtr<TiInt> flags
    ) : keywords(keywords), args(args), min(min), max(max), flags(flags)
    {
    }
  };

  public: struct StatementSegment
  {
    public: SharedPtr<Reference> prod;
    public: SharedPtr<TiInt> min;
    public: SharedPtr<TiInt> max;
    public: StatementSegment(
      SharedPtr<Reference> prod, SharedPtr<TiInt> min, SharedPtr<TiInt> max
    ) : prod(prod), min(min), max(max)
    {
    }
  };


  //============================================================================
  // Constructor & Destructor

  protected: Factory()
  {
  }

  public: virtual ~Factory()
  {
  }


  //============================================================================
  // Member Functions

  protected: void setRootScope(DynamicContaining<TiObject> *rootScope);

  protected: void set(Char const *qualifier, TiObject *val);
  protected: void set(Char const *qualifier, TioSharedPtr const &val)
  {
    this->set(qualifier, val.get());
  }

  protected: void remove(Char const *qualifier);
  protected: Bool tryRemove(Char const *qualifier);

  protected: TiObject* get(Char const *qualifier);
  protected: Bool tryGet(Char const *qualifier, TiObject *&result);

  protected: template<class T> T* get(Char const *qualifier)
  {
    auto obj = ti_cast<T>(this->get(qualifier));
    if (obj == 0) {
      throw EXCEPTION(
        InvalidArgumentException, S("qualifier"), S("No object of the given type is found for this qualifier."),
        qualifier
      );
    }
    return obj;
  }

  protected: void initializeObject(TiObject *obj);

  /// Search the entire grammar for const token to generate.
  protected: void generateConstTokenDefinitions()
  {
    this->generateConstTokenDefinitions(this->context.getRoot()->getInterface<Containing<TiObject>>());
  }

  /// Generate lexer definitions for constant tokens used in a container tree.
  protected: void generateConstTokenDefinitions(Containing<TiObject> *container);

  /// Generate lexer definitions for constant tokens used in a term tree.
  protected: void generateConstTokenDefinitions(Term *term);

  /// Generate lexer definitions for all strings found in the given hierarchy.
  protected: void generateConstTokensForStrings(TiObject *obj);

  /**
   * @brief Add a const token definition in the lexer module.
   * A name for the token will be automatically generated and the location
   * at which it will be inserted is defined by constTokenPrefix. This prefix
   * should contain the path to the containing module and it should not
   * include the scope (the scope will be root.). The value of constTokenPrefix
   * should be set by the derived class before calling
   * generateConstTokenDefinitions.
   */
  protected: virtual Word addConstToken(Char const *text);

  /// Create the token definition with the given const text.
  protected: virtual SharedPtr<SymbolDefinition> createConstTokenDef(Char const *text);

  /**
   * @brief Generate a string key for the given text.
   * The key will begin with __ and will be followed by the text itself after
   * converting any character that isn't a letter or a number into its ASCII
   * code preceeded by a single _.
   */
  protected: static void generateKey(Char const *text, Str &result);

  protected: void createCommand(
    Char const *qualifier, std::initializer_list<CommandSection> sections, SharedPtr<BuildHandler> parsingHandler
  );

  private: SharedPtr<Term> createCommandSection(CommandSection const *section);

  protected: void createStatementVariation(
    Char const *qualifier, std::initializer_list<StatementSegment> segments, SharedPtr<BuildHandler> parsingHandler
  );

  protected: void createProdGroup(Char const *qualifier, std::initializer_list<SharedPtr<Reference>> prods);

  protected: void addProdsToGroup(Char const *qualifier, std::initializer_list<SharedPtr<Reference>> prods);

  protected: void removeProdsFromGroup(Char const *qualifier, std::initializer_list<Char const*> prods);

}; // class

} // namespace

#endif
