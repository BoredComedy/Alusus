/**
 * @file Core/Data/Seeker.cpp
 * Contains the implementation of class Core::Data::Seeker.
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#include "core.h"

namespace Core::Data
{

//==============================================================================
// Initialization

void Seeker::initBindingCaches()
{
  Basic::initBindingCaches(this, {
    &this->set,
    &this->remove,
    &this->foreach,
    &this->extForeach,
    &this->setByIdentifier,
    &this->setByIdentifier_level,
    &this->setByIdentifier_scope,
    &this->removeByIdentifier,
    &this->removeByIdentifier_level,
    &this->removeByIdentifier_scope,
    &this->foreachByIdentifier,
    &this->foreachByIdentifier_level,
    &this->foreachByIdentifier_scope,
    &this->setByLinkOperator,
    &this->setByLinkOperator_routing,
    &this->setByLinkOperator_scopeDotIdentifier,
    &this->setByLinkOperator_mapDotIdentifier,
    &this->removeByLinkOperator,
    &this->removeByLinkOperator_routing,
    &this->removeByLinkOperator_scopeDotIdentifier,
    &this->removeByLinkOperator_mapDotIdentifier,
    &this->foreachByLinkOperator,
    &this->foreachByLinkOperator_routing,
    &this->foreachByLinkOperator_scopeDotIdentifier,
    &this->foreachByLinkOperator_mapDotIdentifier
  });
}


void Seeker::initBindings()
{
  // Main seek functions
  this->set = &Seeker::_set;
  this->remove = &Seeker::_remove;
  this->foreach = &Seeker::_foreach;
  this->extForeach = &Seeker::_extForeach;

  // Identifier seek functions
  this->setByIdentifier = &Seeker::_setByIdentifier;
  this->setByIdentifier_level = &Seeker::_setByIdentifier_level;
  this->setByIdentifier_scope = &Seeker::_setByIdentifier_scope;
  this->removeByIdentifier = &Seeker::_removeByIdentifier;
  this->removeByIdentifier_level = &Seeker::_removeByIdentifier_level;
  this->removeByIdentifier_scope = &Seeker::_removeByIdentifier_scope;
  this->foreachByIdentifier = &Seeker::_foreachByIdentifier;
  this->foreachByIdentifier_level = &Seeker::_foreachByIdentifier_level;
  this->foreachByIdentifier_scope = &Seeker::_foreachByIdentifier_scope;

  // LinkOperator seek functions
  this->setByLinkOperator = &Seeker::_setByLinkOperator;
  this->setByLinkOperator_routing = &Seeker::_setByLinkOperator_routing;
  this->setByLinkOperator_scopeDotIdentifier = &Seeker::_setByLinkOperator_scopeDotIdentifier;
  this->setByLinkOperator_mapDotIdentifier = &Seeker::_setByLinkOperator_mapDotIdentifier;
  this->removeByLinkOperator = &Seeker::_removeByLinkOperator;
  this->removeByLinkOperator_routing = &Seeker::_removeByLinkOperator_routing;
  this->removeByLinkOperator_scopeDotIdentifier = &Seeker::_removeByLinkOperator_scopeDotIdentifier;
  this->removeByLinkOperator_mapDotIdentifier = &Seeker::_removeByLinkOperator_mapDotIdentifier;
  this->foreachByLinkOperator = &Seeker::_foreachByLinkOperator;
  this->foreachByLinkOperator_routing = &Seeker::_foreachByLinkOperator_routing;
  this->foreachByLinkOperator_scopeDotIdentifier = &Seeker::_foreachByLinkOperator_scopeDotIdentifier;
  this->foreachByLinkOperator_mapDotIdentifier = &Seeker::_foreachByLinkOperator_mapDotIdentifier;
}


//==============================================================================
// Helper Functions

Bool Seeker::trySet(TiObject const *ref, TiObject *target, TiObject *val, Word flags)
{
  Bool result = false;
  Int tracingAlias = 0;
  this->set(ref, target, [=,&result, &tracingAlias](TiInt action, TiObject *&obj)->Verb {
    if (action == Action::ALIAS_TRACE_START) {
      ++tracingAlias;
      return Verb::MOVE;
    } else if (action == Action::ALIAS_TRACE_END) {
      --tracingAlias;
      return Verb::MOVE;
    } else if (action == Action::OWNER_SCOPE) {
      if (tracingAlias == 0 && (flags & Flags::SKIP_OWNERS) != 0) return Verb::SKIP_GROUP;
      else return Verb::MOVE;
    } else if (action == Action::USE_SCOPES_START) {
      if ((flags & Flags::SKIP_USES) != 0 && (tracingAlias == 0 || (flags & Flags::SKIP_USES_FOR_ALIASES) != 0)) {
        return Verb::SKIP;
      } else return Verb::MOVE;
    } else if (action != Action::TARGET_MATCH) {
      return Verb::MOVE;
    }
    obj = val;
    result = true;
    return Verb::PERFORM_AND_MOVE;
  }, flags);
  return result;
}


Bool Seeker::tryRemove(TiObject const *ref, TiObject *target, Word flags)
{
  Bool ret = false;
  Int tracingAlias = 0;
  this->remove(ref, target, [flags, &ret, &tracingAlias](TiInt action, TiObject *o)->Verb {
    if (action == Action::ALIAS_TRACE_START) {
      ++tracingAlias;
      return Verb::MOVE;
    } else if (action == Action::ALIAS_TRACE_END) {
      --tracingAlias;
      return Verb::MOVE;
    } else if (action == Action::OWNER_SCOPE) {
      if (tracingAlias == 0 && (flags & Flags::SKIP_OWNERS) != 0) return Verb::SKIP_GROUP;
      else return Verb::MOVE;
    } else if (action == Action::USE_SCOPES_START) {
      if ((flags & Flags::SKIP_USES) != 0 && (tracingAlias == 0 || (flags & Flags::SKIP_USES_FOR_ALIASES) != 0)) {
        return Verb::SKIP;
      } else return Verb::MOVE;
    } else if (action != Action::TARGET_MATCH) {
      return Verb::MOVE;
    }
    ret = true;
    return Verb::PERFORM_AND_MOVE;
  }, flags);
  return ret;
}


Bool Seeker::tryGet(TiObject const *ref, TiObject *target, TiObject *&retVal, Word flags)
{
  Bool ret = false;
  this->extForeach(ref, target, [&ret, &retVal](TiInt action, TiObject *o)->Verb {
    retVal = o;
    ret = true;
    return Verb::STOP;
  }, flags);
  return ret;
}


Bool Seeker::find(TiObject const *ref, TiObject *target, TypeInfo const *ti, TiObject *&retVal, Word flags)
{
  Bool ret = false;
  this->extForeach(ref, target, [ti, &ret, &retVal](TiInt action, TiObject *o)->Verb {
    if (o->isDerivedFrom(ti)) {
      retVal = o;
      ret = true;
      return Verb::STOP;
    } else {
      return Verb::MOVE;
    }
  }, flags);
  return ret;
}


//==============================================================================
// Main Seek Functions

Seeker::Verb Seeker::_set(TiObject *self, TiObject const *ref, TiObject *target, SetCallback const &cb, Word flags)
{
  PREPARE_SELF(seeker, Seeker);
  if (ref->isA<Ast::Identifier>()) {
    return seeker->setByIdentifier(static_cast<Ast::Identifier const*>(ref), target, cb, flags);
  } else if (ref->isA<Ast::LinkOperator>()) {
    return seeker->setByLinkOperator(static_cast<Ast::LinkOperator const*>(ref), target, cb, flags);
  } else {
    throw EXCEPTION(InvalidArgumentException, S("ref"), S("Unrecognized reference type."));
  }
}


Seeker::Verb Seeker::_remove(
  TiObject *self, TiObject const *ref, TiObject *target, RemoveCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  if (ref->isA<Ast::Identifier>()) {
    return seeker->removeByIdentifier(static_cast<Ast::Identifier const*>(ref), target, cb, flags);
  } else if (ref->isA<Ast::LinkOperator>()) {
    return seeker->removeByLinkOperator(static_cast<Ast::LinkOperator const*>(ref), target, cb, flags);
  } else {
    throw EXCEPTION(InvalidArgumentException, S("ref"), S("Unrecognized reference type."));
  }
}


Seeker::Verb Seeker::_foreach(
  TiObject *self, TiObject const *ref, TiObject *target, ForeachCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  if (ref->isA<Ast::Identifier>()) {
    return seeker->foreachByIdentifier(static_cast<Ast::Identifier const*>(ref), target, cb, flags);
  } else if (ref->isA<Ast::LinkOperator>()) {
    return seeker->foreachByLinkOperator(static_cast<Ast::LinkOperator const*>(ref), target, cb, flags);
  } else {
    throw EXCEPTION(InvalidArgumentException, S("ref"), S("Unrecognized reference type."));
  }
}


Seeker::Verb Seeker::_extForeach(
  TiObject *self, TiObject const *ref, TiObject *target, ForeachCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  Int tracingAlias = 0;
  return seeker->foreach(ref, target, [flags, cb, &tracingAlias](TiInt action, TiObject *o)->Verb {
    if (action == Action::ALIAS_TRACE_START) {
      ++tracingAlias;
      return Verb::MOVE;
    } else if (action == Action::ALIAS_TRACE_END) {
      --tracingAlias;
      return Verb::MOVE;
    } else if (action == Action::OWNER_SCOPE) {
      if (tracingAlias == 0 && (flags & Flags::SKIP_OWNERS) != 0) return Verb::SKIP_GROUP;
      else return Verb::MOVE;
    } else if (action == Action::USE_SCOPES_START) {
      if ((flags & Flags::SKIP_USES) != 0 && (tracingAlias == 0 || (flags & Flags::SKIP_USES_FOR_ALIASES) != 0)) {
        return Verb::SKIP;
      } else return Verb::MOVE;
    } else if (action != Action::TARGET_MATCH) {
      return Verb::MOVE;
    }
    return cb(action, o);
  }, flags);
}


//==============================================================================
// Identifier set

Seeker::Verb Seeker::_setByIdentifier(
  TiObject *self, Data::Ast::Identifier const *identifier, TiObject *data, SetCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  Seeker::Verb retVal = Seeker::Verb::MOVE;
  if (data->isDerivedFrom<DataStack>()) {
    auto stack = static_cast<DataStack*>(data);
    for (Int i = stack->getCount() - 1; i >= 0; --i) {
      auto data = stack->getElement(i);
      if (data == 0) continue;
      if (i != stack->getCount() - 1) {
        retVal = cb(Action::OWNER_SCOPE, data);
        if (retVal == Verb::SKIP) continue;
        else if (retVal == Verb::SKIP_GROUP || !Seeker::isMove(retVal)) break;
      }
      retVal = seeker->setByIdentifier_level(identifier, data, cb, flags);
      if (!Seeker::isMove(retVal)) return retVal;
    }
  } else if (data->isDerivedFrom<Node>()) {
    auto node = static_cast<Node*>(data);
    while (node != 0) {
      if (node != data) {
        TiObject *obj = node;
        retVal = cb(Action::OWNER_SCOPE, obj);
        if (retVal == Verb::SKIP) {
          node = node->getOwner();
          continue;
        } else if (retVal == Verb::SKIP_GROUP || !Seeker::isMove(retVal)) break;
      }
      retVal = seeker->setByIdentifier_level(identifier, node, cb, flags);
      if (!Seeker::isMove(retVal)) return retVal;
      node = node->getOwner();
      flags |= Seeker::Flags::SKIP_OWNED;
    }
  } else {
    throw EXCEPTION(InvalidArgumentException, S("data"), S("Invalid data type."));
  }
  return retVal;
}


Seeker::Verb Seeker::_setByIdentifier_level(
  TiObject *self, Data::Ast::Identifier const *identifier, TiObject *data, SetCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  if (data->isDerivedFrom<Ast::Scope>()) {
    return seeker->setByIdentifier_scope(identifier, static_cast<Ast::Scope*>(data), cb, flags);
  } else {
    return Seeker::Verb::MOVE;
  }
}


Seeker::Verb Seeker::_setByIdentifier_scope(
  TiObject *self, Data::Ast::Identifier const *identifier, Ast::Scope *scope, SetCallback const &cb, Word flags
) {
  Seeker::Verb verb = Seeker::Verb::MOVE;
  for (Int i = 0; i < scope->getCount(); ++i) {
    auto def = ti_cast<Data::Ast::Definition>(scope->getElement(i));
    if (def != 0 && def->getName() == identifier->getValue()) {
      auto obj = def->getTarget().get();
      verb = cb(Action::TARGET_MATCH, obj);
      if (isPerform(verb)) {
        def->setTarget(getSharedPtr(obj));
      }
      if (!Seeker::isMove(verb)) break;
    }
  }
  if (Seeker::isMove(verb)) {
    TiObject *obj = 0;
    verb = cb(Action::TARGET_MATCH, obj);
    if (isPerform(verb)) {
      // Add a new definition.
      auto def = Data::Ast::Definition::create();
      def->setName(identifier->getValue());
      def->setTarget(getSharedPtr(obj));
      scope->add(def);
    }
  }
  return verb;
}


//==============================================================================
// Identifier remove

Seeker::Verb Seeker::_removeByIdentifier(
  TiObject *self, Data::Ast::Identifier const *identifier, TiObject *data, RemoveCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  Seeker::Verb retVal = Seeker::Verb::MOVE;
  if (data->isDerivedFrom<DataStack>()) {
    auto stack = static_cast<DataStack*>(data);
    for (Int i = stack->getCount() - 1; i >= 0; --i) {
      auto data = stack->getElement(i);
      if (data == 0) continue;
      if (i != stack->getCount() - 1) {
        retVal = cb(Action::OWNER_SCOPE, data);
        if (retVal == Verb::SKIP) continue;
        else if (retVal == Verb::SKIP_GROUP || !Seeker::isMove(retVal)) break;
      }
      retVal = seeker->removeByIdentifier_level(identifier, data, cb, flags);
      if (!Seeker::isMove(retVal)) return retVal;
    }
  } else if (data->isDerivedFrom<Node>()) {
    auto node = static_cast<Node*>(data);
    while (node != 0) {
      if (node != data) {
        retVal = cb(Action::OWNER_SCOPE, node);
        if (retVal == Verb::SKIP) {
          node = node->getOwner();
          continue;
        } else if (retVal == Verb::SKIP_GROUP || !Seeker::isMove(retVal)) break;
      }
      retVal = seeker->removeByIdentifier_level(identifier, node, cb, flags);
      if (!Seeker::isMove(retVal)) return retVal;
      node = node->getOwner();
      flags |= Seeker::Flags::SKIP_OWNED;
    }
  } else {
    throw EXCEPTION(InvalidArgumentException, S("data"), S("Invalid data type."));
  }
  return retVal;
}


Seeker::Verb Seeker::_removeByIdentifier_level(
  TiObject *self, Data::Ast::Identifier const *identifier, TiObject *data, RemoveCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  if (data->isDerivedFrom<Ast::Scope>()) {
    return seeker->removeByIdentifier_scope(identifier, static_cast<Ast::Scope*>(data), cb, flags);
  } else {
    return Seeker::Verb::MOVE;
  }
}


Seeker::Verb Seeker::_removeByIdentifier_scope(
  TiObject *self, Data::Ast::Identifier const *identifier, Ast::Scope *scope, RemoveCallback const &cb, Word flags
) {
  Seeker::Verb verb = Seeker::Verb::MOVE;
  for (Int i = 0; i < scope->getCount(); ++i) {
    auto def = ti_cast<Data::Ast::Definition>(scope->getElement(i));
    if (def != 0 && def->getName() == identifier->getValue()) {
      auto obj = def->getTarget().get();
      verb = cb(Action::TARGET_MATCH, obj);
      if (isPerform(verb)) {
        scope->remove(i);
        --i;
      }
      if (!Seeker::isMove(verb)) return verb;
    }
  }
  return verb;
}


//==============================================================================
// Identifier foreach

Seeker::Verb Seeker::_foreachByIdentifier(
  TiObject *self, Data::Ast::Identifier const *identifier, TiObject *data, ForeachCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  Seeker::Verb retVal = Seeker::Verb::MOVE;
  if (identifier->getValue() == S("Root")) {
    if (data->isDerivedFrom<DataStack>()) {
      auto stack = static_cast<DataStack*>(data);
      for (Int i = 0; i < stack->getCount(); ++i) {
        auto element = stack->getElement(i);
        if (element != 0) {
          return cb(Action::TARGET_MATCH, element);
        }
      }
      throw EXCEPTION(InvalidArgumentException, S("data"), S("The stack is empty."));
    } else if (data->isDerivedFrom<Node>()) {
      auto node = static_cast<Node*>(data);
      while (node->getOwner() != 0) node = node->getOwner();
      return cb(Action::TARGET_MATCH, node);
    } else {
      throw EXCEPTION(InvalidArgumentException, S("data"), S("Invalid data type."));
    }
  } else {
    if (data->isDerivedFrom<DataStack>()) {
      auto stack = static_cast<DataStack*>(data);
      for (Int i = stack->getCount() - 1; i >= 0; --i) {
        auto data = stack->getElement(i);
        if (data == 0) continue;
        if (i != stack->getCount() - 1) {
          retVal = cb(Action::OWNER_SCOPE, data);
          if (retVal == Verb::SKIP) continue;
          else if (retVal == Verb::SKIP_GROUP || !Seeker::isMove(retVal)) break;
        }
        retVal = seeker->foreachByIdentifier_level(identifier, data, cb, flags);
        if (!Seeker::isMove(retVal)) return retVal;
      }
    } else if (data->isDerivedFrom<Node>()) {
      auto node = static_cast<Node*>(data);
      while (node != 0) {
        if (node != data) {
          retVal = cb(Action::OWNER_SCOPE, node);
          if (retVal == Verb::SKIP) {
            node = node->getOwner();
            continue;
          } else if (retVal == Verb::SKIP_GROUP || !Seeker::isMove(retVal)) break;
        }
        retVal = seeker->foreachByIdentifier_level(identifier, node, cb, flags);
        if (!Seeker::isMove(retVal)) return retVal;
        node = node->getOwner();
        flags |= Seeker::Flags::SKIP_OWNED;
      }
    } else {
      throw EXCEPTION(InvalidArgumentException, S("data"), S("Invalid data type."));
    }
  }
  return retVal;
}


Seeker::Verb Seeker::_foreachByIdentifier_level(
  TiObject *self, Data::Ast::Identifier const *identifier, TiObject *data, ForeachCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  if (data->isDerivedFrom<Ast::Scope>()) {
    return seeker->foreachByIdentifier_scope(identifier, static_cast<Ast::Scope*>(data), cb, flags);
  } else {
    return Seeker::Verb::MOVE;
  }
}


Seeker::Verb Seeker::_foreachByIdentifier_scope(
  TiObject *self, Data::Ast::Identifier const *identifier, Ast::Scope *scope, ForeachCallback const &cb, Word flags
) {
  Seeker::Verb verb = Seeker::Verb::MOVE;
  for (Int i = 0; i < scope->getCount(); ++i) {
    auto def = ti_cast<Data::Ast::Definition>(scope->getElement(i));
    if (def != 0 && def->getName() == identifier->getValue()) {
      auto obj = def->getTarget().get();
      if (obj->isDerivedFrom<Ast::Alias>()) {
        verb = cb(Action::ALIAS_TRACE_START, obj);
        if (verb == Verb::SKIP) return Verb::MOVE;
        else if (!Seeker::isMove(verb)) return verb;
        PREPARE_SELF(seeker, Seeker);
        auto alias = static_cast<Ast::Alias*>(obj);
        verb = seeker->foreach(
          alias->getReference().get(), alias->getOwner(), cb, flags & ~Flags::SKIP_OWNED
        );
        if (!Seeker::isMove(verb)) return verb;
        verb = cb(Action::ALIAS_TRACE_END, obj);
        if (verb != Verb::MOVE) return verb;
      } else {
        verb = cb(Action::TARGET_MATCH, obj);
        if (!Seeker::isMove(verb)) return verb;
      }
    }
  }

  PREPARE_SELF(seeker, Seeker);

  if (scope->getBridgeCount() > 0) {
    verb = cb(Action::USE_SCOPES_START, scope);
    if (verb == Verb::SKIP) return verb;
    for (Int i = 0; i < scope->getBridgeCount(); ++i) {
      auto bridgeRef = scope->getBridge(i);
      // If the thing we are looking for is actually this bridgeRef, then we need to skip, otherwise we end up in
      // an infinite loop.
      if (bridgeRef->getTarget() == identifier) continue;
      auto bridgeTarget = seeker->tryGet(
        bridgeRef->getTarget().get(), scope, Seeker::Flags::SKIP_USES | Seeker::Flags::SKIP_USES_FOR_ALIASES
      );
      if (bridgeTarget == 0) continue;
      verb = cb(Action::USE_SCOPE, bridgeTarget);
      if (verb == Verb::SKIP) continue;
      else if (verb == Verb::SKIP_GROUP || !Seeker::isMove(verb)) break;

      verb = seeker->foreachByIdentifier_level(identifier, bridgeTarget, cb, flags);
      if (!Seeker::isMove(verb)) return verb;
    }
    verb = cb(Action::USE_SCOPES_END, scope);
  }

  return verb;
}


//==============================================================================
// LinkOperator set

Seeker::Verb Seeker::_setByLinkOperator(
  TiObject *self, Ast::LinkOperator const *link, TiObject *data, SetCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  auto first = link->getFirst().get();
  return seeker->foreach(first, data,
    [=](TiInt action, TiObject *newData)->Verb
    {
      if (action == Action::TARGET_MATCH) return seeker->setByLinkOperator_routing(link, newData, cb, flags);
      else return cb(action, newData);
    },
    flags
  );
}


Seeker::Verb Seeker::_setByLinkOperator_routing(
  TiObject *self, Ast::LinkOperator const *link, TiObject *data, SetCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  if (link->getType() == S(".")) {
    auto second = link->getSecond().get();
    if (second->isA<Ast::Identifier>()) {
      if (data->isDerivedFrom<Ast::Scope>()) {
        return seeker->setByLinkOperator_scopeDotIdentifier(
          static_cast<Ast::Identifier*>(second), static_cast<Ast::Scope*>(data), cb, flags
        );
      } else {
        auto map = ti_cast<MapContaining<TiObject>>(data);
        if (map != 0) {
          return seeker->setByLinkOperator_mapDotIdentifier(static_cast<Ast::Identifier*>(second), map, cb, flags);
        } else {
          throw EXCEPTION(InvalidArgumentException, S("data"), S("Unrecognized target data type."));
        }
      }
    } else {
      throw EXCEPTION(InvalidArgumentException, S("link"), S("Unrecognized type for link operator's second part."));
    }
  } else {
    throw EXCEPTION(InvalidArgumentException, S("link"), S("Unknown link operator type."), link->getType());
  }
}


Seeker::Verb Seeker::_setByLinkOperator_scopeDotIdentifier(
  TiObject *self, Ast::Identifier const *identifier, Ast::Scope *scope, SetCallback const &cb, Word flags
) {
  Verb verb = Verb::MOVE;
  for (Int i = 0; i < scope->getCount(); ++i) {
    auto def = ti_cast<Data::Ast::Definition>(scope->getElement(i));
    if (def != 0 && def->getName() == identifier->getValue()) {
      auto obj = def->getTarget().get();
      verb = cb(Action::TARGET_MATCH, obj);
      if (isPerform(verb)) {
        def->setTarget(getSharedPtr(obj));
      }
      if (!Seeker::isMove(verb)) break;
    }
  }
  if (Seeker::isMove(verb)) {
    TiObject *obj = 0;
    verb = cb(Action::TARGET_MATCH, obj);
    if (isPerform(verb)) {
      // Add a new definition.
      auto def = Data::Ast::Definition::create();
      def->setName(identifier->getValue());
      def->setTarget(getSharedPtr(obj));
      scope->add(def);
    }
  }
  return verb;
}


Seeker::Verb Seeker::_setByLinkOperator_mapDotIdentifier(
  TiObject *self, Ast::Identifier const *identifier, MapContaining<TiObject> *map, SetCallback const &cb, Word flags
) {
  Verb verb = Verb::MOVE;
  auto obj = map->getElement(identifier->getValue().get());
  verb = cb(Action::TARGET_MATCH, obj);
  if (isPerform(verb)) {
    map->setElement(identifier->getValue().get(), obj);
  }
  return verb;
}


//==============================================================================
// LinkOperator remove

Seeker::Verb Seeker::_removeByLinkOperator(
  TiObject *self, Data::Ast::LinkOperator const *link, TiObject *data, RemoveCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  auto first = link->getFirst().get();
  return seeker->foreach(first, data,
    [=](TiInt action, TiObject *newData)->Verb
    {
      if (action == Action::TARGET_MATCH) return seeker->removeByLinkOperator_routing(link, newData, cb, flags);
      else return cb(action, newData);
    },
    flags
  );
}


Seeker::Verb Seeker::_removeByLinkOperator_routing(
  TiObject *self, Data::Ast::LinkOperator const *link, TiObject *data, RemoveCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  if (link->getType() == S(".")) {
    auto second = link->getSecond().get();
    if (second->isA<Ast::Identifier>()) {
      if (data->isDerivedFrom<Ast::Scope>()) {
        return seeker->removeByLinkOperator_scopeDotIdentifier(
          static_cast<Ast::Identifier*>(second), static_cast<Ast::Scope*>(data), cb, flags
        );
      } else {
        auto map = ti_cast<DynamicMapContaining<TiObject>>(data);
        if (map != 0) {
          return seeker->removeByLinkOperator_mapDotIdentifier(static_cast<Ast::Identifier*>(second), map, cb, flags);
        } else {
          throw EXCEPTION(InvalidArgumentException, S("data"), S("Unrecognized target data type."));
        }
      }
    } else {
      throw EXCEPTION(InvalidArgumentException, S("link"), S("Unrecognized type for link operator's second part."));
    }
  } else {
    throw EXCEPTION(InvalidArgumentException, S("link"), S("Unknown link operator type."), link->getType());
  }
}


Seeker::Verb Seeker::_removeByLinkOperator_scopeDotIdentifier(
  TiObject *self, Data::Ast::Identifier const *identifier, Data::Ast::Scope *scope, RemoveCallback const &cb, Word flags
) {
  Verb verb = Verb::MOVE;
  for (Int i = 0; i < scope->getCount(); ++i) {
    auto def = ti_cast<Data::Ast::Definition>(scope->getElement(i));
    if (def != 0 && def->getName() == identifier->getValue()) {
      auto obj = def->getTarget().get();
      verb = cb(Action::TARGET_MATCH, obj);
      if (isPerform(verb)) {
        scope->remove(i);
        --i;
      }
      if (!Seeker::isMove(verb)) break;
    }
  }
  return verb;
}


Seeker::Verb Seeker::_removeByLinkOperator_mapDotIdentifier(
  TiObject *self, Data::Ast::Identifier const *identifier, DynamicMapContaining<TiObject> *map,
  RemoveCallback const &cb, Word flags
) {
  Verb verb = Verb::MOVE;
  auto index = map->findElementIndex(identifier->getValue().get());
  if (index != -1) {
    auto obj = map->getElement(index);
    verb = cb(Action::TARGET_MATCH, obj);
    if (isPerform(verb)) {
      map->removeElement(index);
    }
  }
  return verb;
}


//==============================================================================
// LinkOperator foreach

Seeker::Verb Seeker::_foreachByLinkOperator(
  TiObject *self, Data::Ast::LinkOperator const *link, TiObject *data, ForeachCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  auto first = link->getFirst().get();
  return seeker->foreach(first, data,
    [=](TiInt action, TiObject *newData)->Verb
    {
      if (action == Action::TARGET_MATCH) return seeker->foreachByLinkOperator_routing(link, newData, cb, flags);
      else return cb(action, newData);
    },
    flags
  );
}


Seeker::Verb Seeker::_foreachByLinkOperator_routing(
  TiObject *self, Data::Ast::LinkOperator const *link, TiObject *data, ForeachCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Seeker);
  if (link->getType() == S(".")) {
    auto second = link->getSecond().get();
    if (second->isA<Ast::Identifier>()) {
      if (data->isDerivedFrom<Ast::Scope>()) {
        return seeker->foreachByLinkOperator_scopeDotIdentifier(
          static_cast<Ast::Identifier*>(second), static_cast<Ast::Scope*>(data), cb, flags
        );
      } else {
        auto map = ti_cast<MapContaining<TiObject>>(data);
        if (map != 0) {
          return seeker->foreachByLinkOperator_mapDotIdentifier(static_cast<Ast::Identifier*>(second), map, cb, flags);
        } else {
          throw EXCEPTION(InvalidArgumentException, S("data"), S("Unrecognized target data type."));
        }
      }
    } else {
      throw EXCEPTION(InvalidArgumentException, S("link"), S("Unrecognized type for link operator's second part."));
    }
  } else {
    throw EXCEPTION(InvalidArgumentException, S("link"), S("Unknown link operator type."), link->getType());
  }
}


Seeker::Verb Seeker::_foreachByLinkOperator_scopeDotIdentifier(
  TiObject *self, Data::Ast::Identifier *identifier, Data::Ast::Scope *scope, ForeachCallback const &cb, Word flags
) {
  Verb verb = Verb::MOVE;
  for (Int i = 0; i < scope->getCount(); ++i) {
    auto def = ti_cast<Data::Ast::Definition>(scope->getElement(i));
    if (def != 0 && def->getName() == identifier->getValue()) {
      auto obj = def->getTarget().get();
      if (obj->isDerivedFrom<Ast::Alias>()) {
        verb = cb(Action::ALIAS_TRACE_START, obj);
        if (verb == Verb::SKIP) return Verb::MOVE;
        else if (!Seeker::isMove(verb)) return verb;
        PREPARE_SELF(seeker, Seeker);
        auto alias = static_cast<Ast::Alias*>(obj);
        verb = seeker->foreach(
          alias->getReference().get(), alias->getOwner(), cb, flags & ~Flags::SKIP_OWNED
        );
        if (!Seeker::isMove(verb)) break;
        verb = cb(Action::ALIAS_TRACE_END, obj);
        if (verb != Verb::MOVE) return verb;
      } else {
        verb = cb(Action::TARGET_MATCH, obj);
        if (!Seeker::isMove(verb)) break;
      }
    }
  }
  return verb;
}


Seeker::Verb Seeker::_foreachByLinkOperator_mapDotIdentifier(
  TiObject *self, Data::Ast::Identifier const *identifier, MapContaining<TiObject> *map, ForeachCallback const &cb,
  Word flags
) {
  auto index = map->findElementIndex(identifier->getValue().get());
  if (index == -1) return Verb::MOVE;
  auto obj = map->getElement(index);
  if (obj->isDerivedFrom<Ast::Alias>()) {
    auto verb = cb(Action::ALIAS_TRACE_START, obj);
    if (verb == Verb::SKIP) return Verb::MOVE;
    else if (!Seeker::isMove(verb)) return verb;
    PREPARE_SELF(seeker, Seeker);
    auto alias = static_cast<Ast::Alias*>(obj);
    verb = seeker->foreach(
      alias->getReference().get(), alias->getOwner(), cb, flags & ~Flags::SKIP_OWNED
    );
    if (verb != Verb::MOVE) return verb;
    return cb(Action::ALIAS_TRACE_END, obj);
  } else {
    return cb(Action::TARGET_MATCH, obj);
  }
}

} // namespace
